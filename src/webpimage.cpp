// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2017 Andreas Huggel <ahuggel@gmx.net>
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
  Version:   $Rev: 4633 $
 */
/*
  Google's WEBP container spec can be found at the link below:
  https://developers.google.com/speed/webp/docs/riff_container
*/

// *****************************************************************************
#include "rcsid_int.hpp"

// *****************************************************************************
// included header files
#include "config.h"

#include "webpimage.hpp"
#include "image_int.hpp"
#include "futils.hpp"
#include "basicio.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"
#include "tiffimage.hpp"
#include "tiffimage_int.hpp"
#include "convert.hpp"
#include <cmath>
#include <iomanip>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <cassert>
#include <cstdio>

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

    /* Misc. */
    const byte WebPImage::WEBP_PAD_ODD = 0;
    const int WebPImage::WEBP_TAG_SIZE = 0x4;

    /* VP8X feature flags */
    const int WebPImage::WEBP_VP8X_ICC_BIT = 0x20;
    const int WebPImage::WEBP_VP8X_ALPHA_BIT = 0x10;
    const int WebPImage::WEBP_VP8X_EXIF_BIT = 0x8;
    const int WebPImage::WEBP_VP8X_XMP_BIT = 0x4;

    /* Chunk header names */
    const char* WebPImage::WEBP_CHUNK_HEADER_VP8X = "VP8X";
    const char* WebPImage::WEBP_CHUNK_HEADER_VP8L = "VP8L";
    const char* WebPImage::WEBP_CHUNK_HEADER_VP8  = "VP8 ";
    const char* WebPImage::WEBP_CHUNK_HEADER_ANMF = "ANMF";
    const char* WebPImage::WEBP_CHUNK_HEADER_ANIM = "ANIM";
    const char* WebPImage::WEBP_CHUNK_HEADER_ICCP = "ICCP";
    const char* WebPImage::WEBP_CHUNK_HEADER_EXIF = "EXIF";
    const char* WebPImage::WEBP_CHUNK_HEADER_XMP  = "XMP ";

    /* =========================================== */

    void WebPImage::setIptcData(const IptcData& /*iptcData*/)
    {
        // not supported
        // just quietly ignore the request
        // throw(Error(32, "IPTC metadata", "WebP"));
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
        BasicIo::AutoPtr tempIo(new MemIo);
        assert (tempIo.get() != 0);

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw
    } // WebPImage::writeMetadata


    void WebPImage::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(20);
        if (!outIo.isopen()) throw Error(21);

#ifdef DEBUG
        std::cout << "Writing metadata" << std::endl;
#endif

        byte    data   [WEBP_TAG_SIZE*3];
        DataBuf chunkId(WEBP_TAG_SIZE+1);
        chunkId.pData_ [WEBP_TAG_SIZE] = '\0';

        io_->read(data, WEBP_TAG_SIZE * 3);
        uint64_t filesize = Exiv2::getULong(data + WEBP_TAG_SIZE, littleEndian);

        /* Set up header */
        if (outIo.write(data, WEBP_TAG_SIZE * 3) != WEBP_TAG_SIZE * 3)
            throw Error(21);

        /* Parse Chunks */
        bool has_size  = false;
        bool has_xmp   = false;
        bool has_exif  = false;
        bool has_vp8x  = false;
        bool has_alpha = false;
        bool has_icc   = iccProfileDefined();

        int width      = 0;
        int height     = 0;

        byte       size_buff[WEBP_TAG_SIZE];
        Blob       blob;

        if (exifData_.count() > 0) {
            ExifParser::encode(blob, littleEndian, exifData_);
            if (blob.size() > 0) {
                has_exif = true;
            }
        }

        if (xmpData_.count() > 0 && !writeXmpFromPacket()) {
            XmpParser::encode(xmpPacket_, xmpData_,
                              XmpParser::useCompactFormat |
                              XmpParser::omitAllFormatting);
        }
        has_xmp = xmpPacket_.size() > 0;

        /* Verify for a VP8X Chunk First before writing in
         case we have any exif or xmp data, also check
         for any chunks with alpha frame/layer set */
        while ( !io_->eof() && (uint64_t) io_->tell() < filesize) {
            io_->read(chunkId.pData_, WEBP_TAG_SIZE);
            io_->read(size_buff, WEBP_TAG_SIZE);
            long size = Exiv2::getULong(size_buff, littleEndian);
            DataBuf payload(size);
            io_->read(payload.pData_, payload.size_);
            byte c;
            if ( payload.size_ % 2 ) io_->read(&c,1);

            /* Chunk with information about features
             used in the file. */
            if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8X) && !has_vp8x) {
                has_vp8x = true;
            }
            if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8X) && !has_size) {
                has_size = true;
                byte size_buf[WEBP_TAG_SIZE];

                // Fetch width - stored in 24bits
                memcpy(&size_buf, &payload.pData_[4], 3);
                size_buf[3] = 0;
                width = Exiv2::getULong(size_buf, littleEndian) + 1;

                // Fetch height - stored in 24bits
                memcpy(&size_buf, &payload.pData_[7], 3);
                size_buf[3] = 0;
                height = Exiv2::getULong(size_buf, littleEndian) + 1;
            }

            /* Chunk with with animation control data. */
#ifdef __CHECK_FOR_ALPHA__  // Maybe in the future
            if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ANIM) && !has_alpha) {
                has_alpha = true;
            }
#endif

            /* Chunk with with lossy image data. */
#ifdef __CHECK_FOR_ALPHA__ // Maybe in the future
            if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8) && !has_alpha) {
                has_alpha = true;
            }
#endif
            if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8) && !has_size) {
                has_size = true;
                byte size_buf[2];

                /* Refer to this https://tools.ietf.org/html/rfc6386
                   for height and width reference for VP8 chunks */

                // Fetch width - stored in 16bits
                memcpy(&size_buf, &payload.pData_[6], 2);
                width = Exiv2::getUShort(size_buf, littleEndian) & 0x3fff;

                // Fetch height - stored in 16bits
                memcpy(&size_buf, &payload.pData_[8], 2);
                height = Exiv2::getUShort(size_buf, littleEndian) & 0x3fff;
            }

            /* Chunk with with lossless image data. */
            if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8L) && !has_alpha) {
                if ((payload.pData_[5] & WEBP_VP8X_ALPHA_BIT) == WEBP_VP8X_ALPHA_BIT) {
                    has_alpha = true;
                }
            }
            if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8L) && !has_size) {
                has_size = true;
                byte size_buf_w[2];
                byte size_buf_h[3];

                /* For VP8L chunks width & height are stored in 28 bits
                   of a 32 bit field requires bitshifting to get actual
                   sizes. Width and height are split even into 14 bits
                   each. Refer to this https://goo.gl/bpgMJf */

                // Fetch width - 14 bits wide
                memcpy(&size_buf_w, &payload.pData_[1], 2);
                size_buf_w[1] &= 0x3F;
                width = Exiv2::getUShort(size_buf_w, littleEndian) + 1;

                // Fetch height - 14 bits wide
                memcpy(&size_buf_h, &payload.pData_[2], 3);
                size_buf_h[0] =
                  ((size_buf_h[0] >> 6) & 0x3) |
                    ((size_buf_h[1] & 0x3F) << 0x2);
                size_buf_h[1] =
                  ((size_buf_h[1] >> 6) & 0x3) |
                    ((size_buf_h[2] & 0xF) << 0x2);
                height = Exiv2::getUShort(size_buf_h, littleEndian) + 1;
            }

            /* Chunk with animation frame. */
            if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ANMF) && !has_alpha) {
                if ((payload.pData_[5] & 0x2) == 0x2) {
                    has_alpha = true;
                }
            }
            if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ANMF) && !has_size) {
                has_size = true;
                byte size_buf[WEBP_TAG_SIZE];

                // Fetch width - stored in 24bits
                memcpy(&size_buf, &payload.pData_[6], 3);
                size_buf[3] = 0;
                width = Exiv2::getULong(size_buf, littleEndian) + 1;

                // Fetch height - stored in 24bits
                memcpy(&size_buf, &payload.pData_[9], 3);
                size_buf[3] = 0;
                height = Exiv2::getULong(size_buf, littleEndian) + 1;
            }

            /* Chunk with alpha data. */
            if (equalsWebPTag(chunkId, "ALPH") && !has_alpha) {
                has_alpha = true;
            }
        }

        /* Inject a VP8X chunk if one isn't available. */
        if (!has_vp8x) {
            inject_VP8X(outIo, has_xmp, has_exif, has_alpha,
                        has_icc, width, height);
        }

        io_->seek(12, BasicIo::beg);
        while ( !io_->eof() && (uint64_t) io_->tell() < filesize) {
            io_->read(chunkId.pData_, 4);
            io_->read(size_buff, 4);

            long size = Exiv2::getULong(size_buff, littleEndian);

            DataBuf payload(size);
            io_->read(payload.pData_, size);
            if ( io_->tell() % 2 ) io_->seek(+1,BasicIo::cur); // skip pad

            if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8X)) {
                if (has_icc){
                    payload.pData_[0] |= WEBP_VP8X_ICC_BIT;
                } else {
                    payload.pData_[0] &= ~WEBP_VP8X_ICC_BIT;
                }

                if (has_xmp){
                    payload.pData_[0] |= WEBP_VP8X_XMP_BIT;
                } else {
                    payload.pData_[0] &= ~WEBP_VP8X_XMP_BIT;
                }

                if (has_exif) {
                    payload.pData_[0] |= WEBP_VP8X_EXIF_BIT;
                } else {
                    payload.pData_[0] &= ~WEBP_VP8X_EXIF_BIT;
                }

                if (outIo.write(chunkId.pData_, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
                    throw Error(21);
                if (outIo.write(size_buff, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
                    throw Error(21);
                if (outIo.write(payload.pData_, payload.size_) != payload.size_)
                    throw Error(21);
                if (outIo.tell() % 2) {
                    if (outIo.write(&WEBP_PAD_ODD, 1) != 1) throw Error(21);
                }

                if (has_icc) {
                    if (outIo.write((const byte*)WEBP_CHUNK_HEADER_ICCP, WEBP_TAG_SIZE) != WEBP_TAG_SIZE) throw Error(21);
                    ul2Data(data, (uint32_t) iccProfile_.size_, littleEndian);
                    if (outIo.write(data, WEBP_TAG_SIZE) != WEBP_TAG_SIZE) throw Error(21);
                    if (outIo.write(iccProfile_.pData_, (long)iccProfile_.size_) != (long)iccProfile_.size_) {
                        throw Error(21);
                    }
                    has_icc = false;
                }
            } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ICCP)) {
                // Skip it altogether handle it prior to here :)
            } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_EXIF)) {
                // Skip and add new data afterwards
            } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_XMP)) {
                // Skip and add new data afterwards
            } else {
                if (outIo.write(chunkId.pData_, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
                    throw Error(21);
                if (outIo.write(size_buff, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
                    throw Error(21);
                if (outIo.write(payload.pData_, payload.size_) != payload.size_)
                    throw Error(21);
            }

            // Encoder required to pad odd sized data with a null byte
            if (outIo.tell() % 2) {
                if (outIo.write(&WEBP_PAD_ODD, 1) != 1) throw Error(21);
            }
        }

        if (has_exif) {
            if (outIo.write((const byte*)WEBP_CHUNK_HEADER_EXIF, WEBP_TAG_SIZE) != WEBP_TAG_SIZE) throw Error(21);
            us2Data(data, (uint16_t) blob.size()+8, bigEndian);
            ul2Data(data, (uint32_t) blob.size(), littleEndian);
            if (outIo.write(data, WEBP_TAG_SIZE) != WEBP_TAG_SIZE) throw Error(21);
            if (outIo.write((const byte*)&blob[0], static_cast<long>(blob.size())) != (long)blob.size())
            {
                throw Error(21);
            }
            if (outIo.tell() % 2) {
                if (outIo.write(&WEBP_PAD_ODD, 1) != 1) throw Error(21);
            }
        }

        if (has_xmp) {
            if (outIo.write((const byte*)WEBP_CHUNK_HEADER_XMP, WEBP_TAG_SIZE) != WEBP_TAG_SIZE) throw Error(21);
            ul2Data(data, (uint32_t) xmpPacket().size(), littleEndian);
            if (outIo.write(data, WEBP_TAG_SIZE) != WEBP_TAG_SIZE) throw Error(21);
            if (outIo.write((const byte*)xmpPacket().data(), static_cast<long>(xmpPacket().size())) != (long)xmpPacket().size()) {
                throw Error(21);
            }
            if (outIo.tell() % 2) {
                if (outIo.write(&WEBP_PAD_ODD, 1) != 1) throw Error(21);
            }
        }

        // Fix File Size Payload Data
        outIo.seek(0, BasicIo::beg);
        filesize = outIo.size() - 8;
        outIo.seek(4, BasicIo::beg);
        ul2Data(data, (uint32_t) filesize, littleEndian);
        if (outIo.write(data, WEBP_TAG_SIZE) != WEBP_TAG_SIZE) throw Error(21);

    } // WebPImage::writeMetadata

    /* =========================================== */

    void WebPImage::printStructure(std::ostream& out, PrintStructureOption option,int depth)
    {
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        // Ensure this is the correct image type
        if (!isWebPType(*io_, true)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "WEBP");
        }

        bool bPrint  = option==kpsBasic || option==kpsRecursive;
        if ( bPrint || option == kpsXMP || option == kpsIccProfile || option == kpsIptcErase ) {
            byte      data [WEBP_TAG_SIZE * 2];
            io_->read(data, WEBP_TAG_SIZE * 2);
            uint64_t filesize = Exiv2::getULong(data + WEBP_TAG_SIZE, littleEndian);
            DataBuf  chunkId(5)      ;
            chunkId.pData_[4] = '\0' ;

            if ( bPrint ) {
                out << Internal::indent(depth)
                << "STRUCTURE OF WEBP FILE: "
                << io().path()
                << std::endl;
                out << Internal::indent(depth)
                << Internal::stringFormat(" Chunk |   Length |   Offset | Payload")
                << std::endl;
            }

            io_->seek(0,BasicIo::beg); // rewind
            while ( !io_->eof() && (uint64_t) io_->tell() < filesize) {
                uint64_t offset = (uint64_t) io_->tell();
                byte     size_buff[WEBP_TAG_SIZE];
                io_->read(chunkId.pData_, WEBP_TAG_SIZE);
                io_->read(size_buff, WEBP_TAG_SIZE);
                long size = Exiv2::getULong(size_buff, littleEndian);
                DataBuf payload(offset?size:WEBP_TAG_SIZE); // header is different from chunks
                io_->read(payload.pData_, payload.size_);

                if ( bPrint ) {
                    out << Internal::indent(depth)
                    << Internal::stringFormat("  %s | %8u | %8u | ", (const char*)chunkId.pData_,size,(uint32_t)offset)
                    << Internal::binaryToString(payload,payload.size_>32?32:payload.size_)
                    << std::endl;
                }

                if ( equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_EXIF) && option==kpsRecursive ) {
                    // create memio object with the payload, then print the structure
                    BasicIo::AutoPtr p = BasicIo::AutoPtr(new MemIo(payload.pData_,payload.size_));
                    printTiffStructure(*p,out,option,depth);
                }

                bool bPrintPayload = (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_XMP) && option==kpsXMP)
                                  || (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ICCP) && option==kpsIccProfile)
                                   ;
                if ( bPrintPayload ) {
                    out.write((const char*) payload.pData_,payload.size_);
                }

                if ( offset && io_->tell() % 2 ) io_->seek(+1, BasicIo::cur); // skip padding byte on sub-chunks
            }
        }
    }

    /* =========================================== */

    void WebPImage::readMetadata()
    {
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
        chunkId.pData_[4] = '\0' ;

        io_->read(data, WEBP_TAG_SIZE * 3);

        WebPImage::decodeChunks(Exiv2::getULong(data + WEBP_TAG_SIZE, littleEndian) + 12);

    } // WebPImage::readMetadata

    void WebPImage::decodeChunks(uint64_t filesize)
    {
        DataBuf   chunkId(5);
        byte      size_buff[WEBP_TAG_SIZE];
        bool      has_canvas_data = false;

#ifdef DEBUG
        std::cout << "Reading metadata" << std::endl;
#endif

        chunkId.pData_[4] = '\0' ;
        while ( !io_->eof() && (uint64_t) io_->tell() < filesize) {
            io_->read(chunkId.pData_, WEBP_TAG_SIZE);
            io_->read(size_buff, WEBP_TAG_SIZE);
            long size = Exiv2::getULong(size_buff, littleEndian);

            DataBuf payload(size);

            if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8X) && !has_canvas_data) {
                has_canvas_data = true;
                byte size_buf[WEBP_TAG_SIZE];

                io_->read(payload.pData_, payload.size_);

                // Fetch width
                memcpy(&size_buf, &payload.pData_[4], 3);
                size_buf[3] = 0;
                pixelWidth_ = Exiv2::getULong(size_buf, littleEndian) + 1;

                // Fetch height
                memcpy(&size_buf, &payload.pData_[7], 3);
                size_buf[3] = 0;
                pixelHeight_ = Exiv2::getULong(size_buf, littleEndian) + 1;
            } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8) && !has_canvas_data) {
                has_canvas_data = true;
                io_->read(payload.pData_, payload.size_);
                byte size_buf[WEBP_TAG_SIZE];

                // Fetch width""
                memcpy(&size_buf, &payload.pData_[6], 2);
                size_buf[2] = 0;
                size_buf[3] = 0;
                pixelWidth_ = Exiv2::getULong(size_buf, littleEndian) & 0x3fff;

                // Fetch height
                memcpy(&size_buf, &payload.pData_[8], 2);
                size_buf[2] = 0;
                size_buf[3] = 0;
                pixelHeight_ = Exiv2::getULong(size_buf, littleEndian) & 0x3fff;
            } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8L) && !has_canvas_data) {
                has_canvas_data = true;
                byte size_buf_w[2];
                byte size_buf_h[3];

                io_->read(payload.pData_, payload.size_);

                // Fetch width
                memcpy(&size_buf_w, &payload.pData_[1], 2);
                size_buf_w[1] &= 0x3F;
                pixelWidth_ = Exiv2::getUShort(size_buf_w, littleEndian) + 1;

                // Fetch height
                memcpy(&size_buf_h, &payload.pData_[2], 3);
                size_buf_h[0] = ((size_buf_h[0] >> 6) & 0x3) | ((size_buf_h[1]  & 0x3F) << 0x2);
                size_buf_h[1] = ((size_buf_h[1] >> 6) & 0x3) | ((size_buf_h[2] & 0xF) << 0x2);
                pixelHeight_ = Exiv2::getUShort(size_buf_h, littleEndian) + 1;
            } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ANMF) && !has_canvas_data) {
                has_canvas_data = true;
                byte size_buf[WEBP_TAG_SIZE];

                io_->read(payload.pData_, payload.size_);

                // Fetch width
                memcpy(&size_buf, &payload.pData_[6], 3);
                size_buf[3] = 0;
                pixelWidth_ = Exiv2::getULong(size_buf, littleEndian) + 1;

                // Fetch height
                memcpy(&size_buf, &payload.pData_[9], 3);
                size_buf[3] = 0;
                pixelHeight_ = Exiv2::getULong(size_buf, littleEndian) + 1;
            } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ICCP)) {
                io_->read(payload.pData_, payload.size_);
                this->setIccProfile(payload);
            } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_EXIF)) {
                io_->read(payload.pData_, payload.size_);

                byte  size_buff[2];
                byte  exifLongHeader[]   = { 0xFF, 0x01, 0xFF, 0xE1 };
                byte  exifShortHeader[]  = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
                byte  exifTiffLEHeader[] = { 0x49, 0x49, 0x2A };       // "MM*"
                byte  exifTiffBEHeader[] = { 0x4D, 0x4D, 0x00, 0x2A }; // "II\0*"
                byte* rawExifData = NULL;
                long  offset = 0;
                bool  s_header = false;
                bool  le_header = false;
                bool  be_header = false;
                long  pos = -1;

                pos = getHeaderOffset (payload.pData_, (long)payload.size_, (byte*)&exifLongHeader, 4);
                if (pos == -1) {
                    pos = getHeaderOffset (payload.pData_, (long)payload.size_, (byte*)&exifLongHeader, 6);
                    if (pos != -1) {
                        s_header = true;
                    }
                }
                if (pos == -1) {
                    pos = getHeaderOffset (payload.pData_, (long)payload.size_, (byte*)&exifTiffLEHeader, 3);
                    if (pos != -1) {
                        le_header = true;
                    }
                }
                if (pos == -1) {
                    pos = getHeaderOffset (payload.pData_, (long)payload.size_, (byte*)&exifTiffBEHeader, 4);
                    if (pos != -1) {
                        be_header = true;
                    }
                }

                if (s_header) {
                    offset += 6;
                }
                if (be_header || le_header) {
                    offset += 12;
                }

                long size = payload.size_ + offset;
                rawExifData = (byte*)malloc(size);

                if (s_header) {
                    us2Data(size_buff, (uint16_t) (size - 6), bigEndian);
                    memcpy(rawExifData, (char*)&exifLongHeader, 4);
                    memcpy(rawExifData + 4, (char*)&size_buff, 2);
                }

                if (be_header || le_header) {
                    us2Data(size_buff, (uint16_t) (size - 6), bigEndian);
                    memcpy(rawExifData, (char*)&exifLongHeader, 4);
                    memcpy(rawExifData + 4, (char*)&size_buff, 2);
                    memcpy(rawExifData + 6, (char*)&exifShortHeader, 6);
                }

                memcpy(rawExifData + offset, payload.pData_, (long)payload.size_);

#ifdef DEBUG
                std::cout << "Display Hex Dump [size:" << (unsigned long)size << "]" << std::endl;
                std::cout << Internal::binaryToHex(rawExifData, size);
#endif

                if (pos != -1) {
                    XmpData  xmpData;
                    ByteOrder bo = ExifParser::decode(exifData_,
                                                      payload.pData_ + pos,
                                                      payload.size_ - pos);
                    setByteOrder(bo);
                }
                else
                {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode Exif metadata." << std::endl;
#endif
                    exifData_.clear();
                }

                if (rawExifData) free(rawExifData);
            } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_XMP)) {
                io_->read(payload.pData_, payload.size_);
                xmpPacket_.assign(reinterpret_cast<char*>(payload.pData_), payload.size_);
                if (xmpPacket_.size() > 0 && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode XMP metadata." << std::endl;
#endif
                } else {
#ifdef DEBUG
                    std::cout << "Display Hex Dump [size:" << (unsigned long)payload.size_ << "]" << std::endl;
                    std::cout << Internal::binaryToHex(payload.pData_, payload.size_);
#endif
                }
            } else {
                io_->seek(size, BasicIo::cur);
            }

            if ( io_->tell() % 2 ) io_->seek(+1, BasicIo::cur);
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
        return matched_riff && matched_webp;
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


    /*!
     @brief Function used to add missing EXIF & XMP flags
     to the feature section.
     @param  iIo get BasicIo pointer to inject data
     @param has_xmp Verify if we have xmp data and set required flag
     @param has_exif Verify if we have exif data and set required flag
     @return Returns void
     */
    void WebPImage::inject_VP8X(BasicIo& iIo, bool has_xmp,
                                bool has_exif, bool has_alpha,
                                bool has_icc, int width, int height) {
        byte size[4]  = { 0x0A, 0x00, 0x00, 0x00 };
        byte data[10] = { 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00 };
        iIo.write((const byte*)WEBP_CHUNK_HEADER_VP8X, WEBP_TAG_SIZE);
        iIo.write(size, WEBP_TAG_SIZE);

        if (has_alpha) {
            data[0] |= WEBP_VP8X_ALPHA_BIT;
        }

        if (has_icc) {
            data[0] |= WEBP_VP8X_ICC_BIT;
        }

        if (has_xmp) {
            data[0] |= WEBP_VP8X_XMP_BIT;
        }

        if (has_exif) {
            data[0] |= WEBP_VP8X_EXIF_BIT;
        }

        /* set width - stored in 24bits*/
        int w = width - 1;
        data[4] = w & 0xFF;
        data[5] = (w >> 8) & 0xFF;
        data[6] = (w >> 16) & 0xFF;

        /* set height - stored in 24bits */
        int h = height - 1;
        data[7] = h & 0xFF;
        data[8] = (h >> 8) & 0xFF;
        data[9] = (h >> 16) & 0xFF;

        iIo.write(data, 10);

        /* Handle inject an icc profile right after VP8X chunk */
        if (has_icc) {
            byte size_buff[WEBP_TAG_SIZE];
            ul2Data(size_buff, iccProfile_.size_, littleEndian);
            if (iIo.write((const byte*)WEBP_CHUNK_HEADER_VP8X, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
                throw Error(21);
            if (iIo.write(size_buff, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
                throw Error(21);
            if (iIo.write(iccProfile_.pData_, iccProfile_.size_) != iccProfile_.size_)
                throw Error(21);
            if (iIo.tell() % 2) {
                if (iIo.write(&WEBP_PAD_ODD, 1) != 1) throw Error(21);
            }

            has_icc = false;
        }
    }

    long WebPImage::getHeaderOffset(byte *data, long data_size,
                                    byte *header, long header_size) {
        long pos = -1;
        for (long i=0; i < data_size - header_size; i++) {
            if (memcmp(header, &data[i], header_size) == 0) {
                pos = i;
                break;
            }
        }
        return pos;
    }

} // namespace Exiv2
