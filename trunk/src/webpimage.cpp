// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
 *
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*
  File:      webpimage.cpp
  Version:   $Rev: 3845 $
  Author(s): Ben Touchette <draekko.software+exiv2@gmail.com>
  History:   29-Jul-16
  Credits:   See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"

// *****************************************************************************
// included header files
#include "config.h"

#include "webpimage.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"
#include "tiffimage.hpp"
#include "tiffimage_int.hpp"
#include "exiv2/convert.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <cstdio>

#include <zlib.h>     // To uncompress or compress text chunk

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {


}}                                      // namespace Internal, Exiv2

namespace Exiv2 {
    using namespace Exiv2::Internal;

    WebPImage::WebPImage(BasicIo::AutoPtr io)
            : Image(ImageType::webp, mdNone, io)
    {
    } // WebPImage::WebPImage

    std::string WebPImage::mimeType() const
    {
        return "image/webp";
    }

    /* =========================================== */

    void WebPImage::setIptcData(const IptcData& /*iptcData*/)
    {
        // not supported
        throw(Error(32, "IPTC metadata", "WebP"));
    }

    void WebPImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "WebP"));
    }

    /* =========================================== */

    void WebPImage::writeMetadata()
    {
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        BasicIo::AutoPtr tempIo(io_->temporary()); // may throw
        assert (tempIo.get() != 0);

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw
    } // WebPImage::writeMetadata


    void WebPImage::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(20);
        if (!outIo.isopen()) throw Error(21);

        byte data[12];
        DataBuf chunkId(5);
        const int TAG_SIZE = 4;
        chunkId.pData_[4] = '\0' ;

        io_->read(data, TAG_SIZE * 3);
        uint64_t filesize = Exiv2::getULong(data + 4, littleEndian);
        uint64_t endoffile = 12;

        /* Set up header */
        if (outIo.write(data, TAG_SIZE * 3) != TAG_SIZE * 3)
            throw Error(21);

        /* Parse Chunks */
        bool has_xmp = false;
        bool has_exif = false;
        byte size_buff[4];
        std::string xmpData;
        Blob blob;

        if (iptcData_.count() > 0) {
            std::cout << "Found iptc data\n";
        }

        if (exifData_.count() > 0) {
            ExifParser::encode(blob, littleEndian, exifData_);
            if (blob.size() > 0) {
                has_exif = true;
            }
        }

        if (xmpData_.count() > 0) {
            copyIptcToXmp(iptcData_, xmpData_);
            copyExifToXmp(exifData_, xmpData_);
            XmpParser::encode(xmpPacket_, xmpData_,
                              XmpParser::useCompactFormat |
                              XmpParser::omitAllFormatting);
            if (xmpPacket_.size() > 0) {
                has_xmp = true;
                xmpData = xmpPacket_.data();
            }
        }

        while (!io_->eof()) {
            io_->read(chunkId.pData_, 4);
            io_->read(size_buff, 4);

            if (endoffile >= filesize) {
                break;
            }

            uint64_t size = Exiv2::getULong(size_buff, littleEndian);

            DataBuf payload(size);
            io_->read(payload.pData_, payload.size_);

            if (equalsWebPTag(chunkId, "VP8X")) {
              if (has_xmp){
                payload.pData_[0] |= 0x4;
              } else {
                payload.pData_[0] &= ~0x4;
              }
              if (has_exif) {
                payload.pData_[0] |= 0x8;
              } else {
                payload.pData_[0] &= ~0x8;
              }
              if (outIo.write(chunkId.pData_, TAG_SIZE) != TAG_SIZE)
                  throw Error(21);
              if (outIo.write(size_buff, 4) != 4)
                  throw Error(21);
              if (outIo.write(payload.pData_, payload.size_) != payload.size_)
                  throw Error(21);
            } else if (equalsWebPTag(chunkId, "EXIF")) {
              // Skip and add new data afterwards
            } else if (equalsWebPTag(chunkId, "XMP ")) {
              // Skip and add new data afterwards
            } else {
              if (outIo.write(chunkId.pData_, TAG_SIZE) != TAG_SIZE)
                  throw Error(21);
              if (outIo.write(size_buff, 4) != 4)
                  throw Error(21);
              if (outIo.write(payload.pData_, payload.size_) != payload.size_)
                  throw Error(21);
            }

            endoffile = io_->tell();
            if (endoffile >= filesize) {
                break;
            }
        }

        if (has_exif) {
            std::string header = "EXIF";
            if (outIo.write((const byte*)header.data(), 4) != 4)
                throw Error(21);

            us2Data(data, blob.size()+10, bigEndian);
            static const char exifHeader[] = { 0xff, 0x1, 0xFF, 0xE1, data[0], data[1], 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
            std::string rawExif =   std::string(exifHeader, 12)
                                  + std::string((const char*)&blob[0], blob.size());
            ul2Data(data, rawExif.size(), littleEndian);
            if (outIo.write(data, 4) != 4) throw Error(21);
            if (outIo.write((const byte*)rawExif.data(), static_cast<long>(rawExif.size())) != (long)rawExif.size())
            {
                throw Error(21);
            }
        }

        if (has_xmp) {
            std::string header = "XMP ";
            if (outIo.write((const byte*)header.data(), TAG_SIZE) != TAG_SIZE) throw Error(21);
            ul2Data(data, xmpData.size(), littleEndian);
            if (outIo.write(data, 4) != 4) throw Error(21);
            if (outIo.write((const byte*)xmpData.data(), static_cast<long>(xmpData.size())) != (long)xmpData.size()) {
                throw Error(21);
            }
            for (int lp=0; lp<12; lp++)
                data[0] = 0;
            if (outIo.write(data, 1) != 1) throw Error(21);
        }

        // Fix File Size Payload Data
        if (has_xmp || has_exif) {
          outIo.seek(0, BasicIo::beg);
          filesize = outIo.size() - 8;
          outIo.seek(4, BasicIo::beg);
          ul2Data(data, filesize, littleEndian);
          if (outIo.write(data, 4) != 4) throw Error(21);
        }

    } // WebPImage::writeMetadata

    /* =========================================== */

    void WebPImage::readMetadata()
    {
    	std::cout << "1\n";
        if (io_->open() != 0) throw Error(9, io_->path(), strError());
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isWebPType(*io_, true)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(15);
        }
        clearMetadata();

        byte data[12];
        DataBuf chunkId(5);
        const int TAG_SIZE = 4;
        chunkId.pData_[4] = '\0' ;

        io_->read(data, TAG_SIZE * 3);

        WebPImage::decodeChunks(Exiv2::getULong(data + 4, littleEndian) + 12);

    } // WebPImage::readMetadata

    void WebPImage::decodeChunks(uint64_t filesize)
    {
    	std::cout << "2\n";
        DataBuf  chunkId(5);
        byte     size_buff[4];
        uint64_t size;
        uint64_t endoffile = 12;

        chunkId.pData_[4] = '\0' ;

        while (!io_->eof()) {
            io_->read(chunkId.pData_, 4);
            io_->read(size_buff, 4);
            size = Exiv2::getULong(size_buff, littleEndian);

            DataBuf payload(size);

            if (equalsWebPTag(chunkId, "VP8X")) {
                io_->read(payload.pData_, payload.size_);
                byte size_buf[4];
                memcpy(&size_buf, &payload.pData_[4], 3);
                size_buf[3] = 0;
                pixelWidth_ = Exiv2::getULong(size_buf, littleEndian) + 1;
                memcpy(&size_buf, &payload.pData_[7], 3);
                size_buf[3] = 0;
                pixelHeight_ = Exiv2::getULong(size_buf, littleEndian) + 1;
            } else if (equalsWebPTag(chunkId, "ICCP")) {
#ifdef __SVN__ /* COULD BE ENABLED FOR SVN VERSION */
                io_->read(payload.pData_, payload.size_);
                this->setIccProfile(payload);
#else
                io_->seek(size, BasicIo::cur);
#endif
            } else if (equalsWebPTag(chunkId, "EXIF")) {
                io_->read(payload.pData_, payload.size_);

                const byte exifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
                long pos = -1;

                for (long i=0 ; i < payload.size_-(long)sizeof(exifHeader) ; i++)
                {
                    if (memcmp(exifHeader, &payload.pData_[i], sizeof(exifHeader)) == 0)
                    {
                        pos = i;
                        break;
                    }
                }

                if (pos != -1)
                {
                    IptcData iptcData;
                    XmpData  xmpData;
                    pos = pos + sizeof(exifHeader);
                    ByteOrder bo = ExifParser::decode(exifData_,
                                                      payload.pData_ + pos,
                                                      payload.size_ - pos);
                    setByteOrder(bo);
                }
                else
                {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode Exif metadata.\n";
#endif
                    exifData_.clear();
                }
            } else if (equalsWebPTag(chunkId, "XMP ")) {
                io_->read(payload.pData_, payload.size_);
                xmpPacket_.assign(reinterpret_cast<char*>(payload.pData_), payload.size_);
                if (xmpPacket_.size() > 0 && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
                } else {
                    copyXmpToIptc(xmpData_, iptcData_);
                    copyXmpToExif(xmpData_, exifData_);
                }
            } else {
                io_->seek(size, BasicIo::cur);
            }

            endoffile = io_->tell();
            if (endoffile >= filesize) {
                break;
            }
        }
    }

    /* =========================================== */

    Image::AutoPtr newWebPInstance(BasicIo::AutoPtr io, bool /*create*/)
    {
        Image::AutoPtr image(new WebPImage(io));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isWebPType(BasicIo& iIo, bool /*advance*/)
    {
        const int32_t len = 4;
        const unsigned char RiffImageId[4] = { 'R', 'I', 'F' ,'F'};
        const unsigned char WebPImageId[4] = { 'W', 'E', 'B' ,'P'};
        byte webp[len];
        byte data[len];
        byte riff[len];
        iIo.read(riff, len);
        iIo.read(data, len);
        iIo.read(webp, len);
        bool matched_riff = (memcmp(riff, RiffImageId, len) == 0);
        bool matched_webp = (memcmp(webp, WebPImageId, len) == 0);
        iIo.seek(-12, BasicIo::cur);
        bool result = matched_riff && matched_webp;
    	return result;
    }

    /*!
      @brief Function used to check equality of a Tags with a
          particular string (ignores case while comparing).
      @param buf Data buffer that will contain Tag to compare
      @param str char* Pointer to string
      @return Returns true if the buffer value is equal to string.
     */
    bool WebPImage::equalsWebPTag(Exiv2::DataBuf& buf, const char* str) {
        for(int i = 0; i < 4; i++ )
            if(toupper(buf.pData_[i]) != str[i])
                return false;
        return true;
    }

} // namespace Exiv2
