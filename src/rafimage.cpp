// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
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
// *****************************************************************************
// included header files
#include "config.h"

#include "rafimage.hpp"
#include "tiffimage.hpp"
#include "image_int.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "enforce.hpp"
#include "safe_op.hpp"

// +standard includes
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    RafImage::RafImage(BasicIo::UniquePtr io, bool /*create*/)
        : Image(ImageType::raf, mdExif | mdIptc | mdXmp, std::move(io))
    {
    } // RafImage::RafImage

    std::string RafImage::mimeType() const
    {
        return "image/x-fuji-raf";
    }

    int RafImage::pixelWidth() const
    {
        auto widthIter = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
        if (widthIter != exifData_.end() && widthIter->count() > 0) {
            return widthIter->toLong();
        }
        return 0;
    }

    int RafImage::pixelHeight() const
    {
        auto heightIter = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
        if (heightIter != exifData_.end() && heightIter->count() > 0) {
            return heightIter->toLong();
        }
        return 0;
    }

    void RafImage::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(kerInvalidSettingForImage, "Exif metadata", "RAF"));
    }

    void RafImage::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(kerInvalidSettingForImage, "IPTC metadata", "RAF"));
    }

    void RafImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(kerInvalidSettingForImage, "Image comment", "RAF"));
    }
    
    void RafImage::printStructure(std::ostream& out, PrintStructureOption option, int depth) {
        if (io_->open() != 0) {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        // Ensure this is the correct image type
        if (!isRafType(*io_, true)) {
            if (io_->error() || io_->eof()) throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage, "RAF");
        }
        size_t address  = 0 ;
        size_t address2 = 0 ;
        const bool bPrint = option==kpsBasic || option==kpsRecursive;
        if ( bPrint ) {
            io_->seek(0,BasicIo::beg); // rewind
            address = io_->tell();
            const char* format = " %8d | %8d | ";

            {
                out << Internal::indent(depth)
                    << "STRUCTURE OF RAF FILE: "
                    << io().path()
                    << std::endl;
                out << Internal::indent(depth)
                    << Internal::stringFormat("  Address |   Length | Payload")
                    << std::endl;
            }

            byte magicdata [17];
            io_->read(magicdata, 16);
            magicdata[16] = 0;
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address, 16, 0)
                    << "      magic : "
                    << (char*) magicdata
                    << std::endl;
            }

            address = io_->tell();
            byte data1 [5];
            io_->read(data1, 4);
            data1[4] = 0;
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address, 4, 16)
                    << "      data1 : "
                    << std::string((char*)&data1)
                    << std::endl;
            }

            address = io_->tell();
            byte data2 [9];
            io_->read(data2, 8);
            data2[8] = 0;
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address, 8, 20)
                    << "      data2 : "
                    << std::string((char*)&data2)
                    << std::endl;
            }

            address = io_->tell();
            byte camdata [33];
            io_->read(camdata, 32);
            camdata[32] = 0;
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address, 32, 28)
                    << "     camera : "
                    << std::string((char*)&camdata)
                    << std::endl;
            }

            address = io_->tell();
            byte dir_version [5];
            io_->read(dir_version, 4);
            dir_version[4] = 0;
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address, 4, 60)
                    << "    version : "
                    << std::string((char*)&dir_version)
                    << std::endl;
            }

            address = io_->tell();
            DataBuf   unknown(20);
            io_->read(unknown.pData_,unknown.size_);
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address, 20)
                    << "    unknown : "
                    << Internal::binaryToString(makeSlice(unknown, 0,unknown.size_))
                    << std::endl;
            }


            address = io_->tell();
            byte jpg_img_offset [4];
            io_->read(jpg_img_offset, 4);
            byte jpg_img_length [4];
            address2 = io_->tell();
            io_->read(jpg_img_length, 4);

            long jpg_img_off = Exiv2::getULong((const byte *) jpg_img_offset, bigEndian);
            long jpg_img_len = Exiv2::getULong((const byte *) jpg_img_length, bigEndian);
            std::stringstream j_off;
            std::stringstream j_len;
            j_off << jpg_img_off;
            j_len << jpg_img_len;
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address, 4)
                    << "JPEG Offset : "
                    << j_off.str()
                    << std::endl;
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address2, 4)
                    << "JPEG Length : "
                    << j_len.str()
                    << std::endl;
            }

            address = io_->tell();
            byte cfa_header_offset [4];
            io_->read(cfa_header_offset, 4);
            byte cfa_header_length [4];
            address2 = io_->tell();
            io_->read(cfa_header_length, 4);
            long cfa_hdr_off = Exiv2::getULong((const byte *) cfa_header_offset, bigEndian);
            long cfa_hdr_len = Exiv2::getULong((const byte *) cfa_header_length, bigEndian);
            std::stringstream ch_off;
            std::stringstream ch_len;
            ch_off << cfa_hdr_off;
            ch_len << cfa_hdr_len;
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address, 4)
                    << " CFA Offset : "
                    << ch_off.str()
                    << std::endl;
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address2, 4)
                    << " CFA Length : "
                    << ch_len.str()
                    << std::endl;
            }

            byte cfa_offset [4];
            address = io_->tell();
            io_->read(cfa_offset, 4);
            byte cfa_length [4];
            address2 = io_->tell();
            io_->read(cfa_length, 4);
            long cfa_off = Exiv2::getULong((const byte *) cfa_offset, bigEndian);
            long cfa_len = Exiv2::getULong((const byte *) cfa_length, bigEndian);
            std::stringstream c_off;
            std::stringstream c_len;
            c_off << cfa_off;
            c_len << cfa_len;
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address,4)
                    << "TIFF Offset : "
                    << c_off.str()
                    << std::endl;
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address2,4)
                    << "TIFF Length : "
                    << c_len.str()
                    << std::endl;
            }

            io_->seek(jpg_img_off, BasicIo::beg); // rewind
            address = io_->tell();
            DataBuf payload(16); // header is different from chunks
            io_->read(payload.pData_, payload.size_);
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address, jpg_img_len) // , jpg_img_off)
                    << "       JPEG : "
                    << Internal::binaryToString(makeSlice(payload, 0, payload.size_))
                    << std::endl;
            }

            io_->seek(cfa_hdr_off, BasicIo::beg); // rewind
            address = io_->tell();
            io_->read(payload.pData_, payload.size_);
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address, cfa_hdr_len, cfa_hdr_off)
                    << "        CFA : "
                    << Internal::binaryToString(makeSlice(payload, 0, payload.size_))
                    << std::endl;
            }

            io_->seek(cfa_off, BasicIo::beg); // rewind
            address = io_->tell();
            io_->read(payload.pData_, payload.size_);
            {
                out << Internal::indent(depth)
                    << Internal::stringFormat(format,address, cfa_len, cfa_off)
                    << "       TIFF : "
                    << Internal::binaryToString(makeSlice(payload, 0, payload.size_))
                    << std::endl;
            }
        }
    } // RafImage::printStructure

    void RafImage::readMetadata()
    {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Reading RAF file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isRafType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage, "RAF");
        }

        clearMetadata();

        if (io_->seek(84,BasicIo::beg) != 0) throw Error(kerFailedToReadImageData);
        byte jpg_img_offset [4];
        if (io_->read(jpg_img_offset, 4) != 4) throw Error(kerFailedToReadImageData);
        byte jpg_img_length [4];
        if (io_->read(jpg_img_length, 4) != 4) throw Error(kerFailedToReadImageData);
        uint32_t jpg_img_off_u32 = Exiv2::getULong((const byte *) jpg_img_offset, bigEndian);
        uint32_t jpg_img_len_u32 = Exiv2::getULong((const byte *) jpg_img_length, bigEndian);

        enforce(Safe::add(jpg_img_off_u32, jpg_img_len_u32) <= io_->size(), kerCorruptedMetadata);

#if LONG_MAX < UINT_MAX
        enforce(jpg_img_off_u32 <= static_cast<uint32_t>(std::numeric_limits<long>::max()),
                kerCorruptedMetadata);
        enforce(jpg_img_len_u32 <= static_cast<uint32_t>(std::numeric_limits<long>::max()),
                kerCorruptedMetadata);
#endif

        long jpg_img_off = static_cast<long>(jpg_img_off_u32);
        long jpg_img_len = static_cast<long>(jpg_img_len_u32);

        enforce(jpg_img_len >= 12, kerCorruptedMetadata);

        DataBuf buf(jpg_img_len - 12);
        if (io_->seek(jpg_img_off + 12,BasicIo::beg) != 0) throw Error(kerFailedToReadImageData);
        io_->read(buf.pData_, buf.size_);
        if (io_->error() || io_->eof()) throw Error(kerFailedToReadImageData);

        io_->seek(0,BasicIo::beg); // rewind

        ByteOrder bo = TiffParser::decode(exifData_,
                                          iptcData_,
                                          xmpData_,
                                          buf.pData_,
                                          buf.size_);

        exifData_["Exif.Image2.JPEGInterchangeFormat"] = getULong(jpg_img_offset, bigEndian);
        exifData_["Exif.Image2.JPEGInterchangeFormatLength"] = getULong(jpg_img_length, bigEndian);

        setByteOrder(bo);

        // parse the tiff
        byte     readBuff[4];
        if (io_->seek(100, BasicIo::beg) != 0) throw Error(kerFailedToReadImageData);
        if (io_->read(readBuff, 4) != 4      ) throw Error(kerFailedToReadImageData);
        uint32_t tiffOffset = Exiv2::getULong(readBuff, bigEndian);

        if (io_->read(readBuff, 4) != 4) throw Error(kerFailedToReadImageData);
        uint32_t tiffLength = Exiv2::getULong(readBuff, bigEndian);

        // sanity check.  Does tiff lie inside the file?
        enforce(Safe::add(tiffOffset, tiffLength) <= io_->size(), kerCorruptedMetadata);

        DataBuf  tiff(tiffLength);
        if (io_->seek(tiffOffset, BasicIo::beg) != 0) throw Error(kerFailedToReadImageData);
        io_->read(tiff.pData_, tiff.size_);

        if (!io_->error() && !io_->eof())
        {
            TiffParser::decode(exifData_,
                               iptcData_,
                               xmpData_,
                               tiff.pData_,
                               tiff.size_);
        }
    } // RafImage::readMetadata

    void RafImage::writeMetadata()
    {
        //! Todo: implement me!
        throw(Error(kerWritingImageFormatUnsupported, "RAF"));
    } // RafImage::writeMetadata

    // *************************************************************************
    // free functions
    Image::UniquePtr newRafInstance(BasicIo::UniquePtr io, bool create)
    {
        Image::UniquePtr image(new RafImage(std::move(io), create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isRafType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 8;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        int rc = memcmp(buf, "FUJIFILM", 8);
        if (!advance || rc != 0) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc == 0;
    }

}                                       // namespace Exiv2
