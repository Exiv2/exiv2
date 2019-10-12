// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2019 Exiv2 authors
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
/*!
  @file    heifimage_int.hpp
  @brief   Include HEIF header files.
  @author  Peter Kovář (1div0)
           <a href="mailto:peter.kovar@reflexion.tv">peter.kovar@reflexion.tv</a>
  @date    25-Dec-18, 1div0: created
  @author  Gilles Caulier (cgilles)
           <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
  @date    12-Oct-19, cgilles: add read exif, iptc, and xmp support
 */
// *****************************************************************************
// included header files
#include "config.h"

#include "basicio.hpp"
#include "error.hpp"
#include "enforce.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "heifimage_int.hpp"
#include "types.hpp"

// + standard includes
#include <cassert>
#include <cstring>
#include <iostream>
#include <iterator>
#include <string>

// libheif includes
#include <libheif/heif.h>

const unsigned char HeifSignature[] = { 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70, 0x68, 0x65, 0x69, 0x63 };
const unsigned char HeifBlank[]     = { 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70, 0x68, 0x65, 0x69, 0x63 };

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    using namespace Exiv2::Internal;

    HeifImage::HeifImage(BasicIo::UniquePtr io, bool create)
            : Image(ImageType::heif, mdExif | mdIptc | mdXmp, std::move(io))
    {
        if (create)
        {
            if (io_->open() == 0)
            {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Exiv2::HeifImage:: Creating HEIF image to memory" << std::endl;
#endif
                IoCloser closer(*io_);

                if (io_->write(HeifBlank, sizeof(HeifBlank)) != sizeof(HeifBlank))
                {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cerr << "Exiv2::HeifImage:: Failed to create HEIF image on memory" << std::endl;
#endif
                }
            }
        }
    } // HeifImage::HeifImage

    HeifImage::~HeifImage()
    {
    }

    std::string HeifImage::mimeType() const
    {
        return "image/heif";
    }

    void HeifImage::setComment(const std::string& /*comment*/)
    {
        throw(Error(kerInvalidSettingForImage, "Image comment", "HEIF"));
    } // HeifImage::setComment

    void HeifImage::readMetadata()
    {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Exiv2::HeifImage::readMetadata: Reading HEIF file " << io_->path() << std::endl;
#endif
        if (io_->open() != 0)
        {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }

        IoCloser closer(*io_);

        // Ensure that this is the correct image type
        if (!isHeifType(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage, "HEIF");
        }

        std::shared_ptr<heif_context> ctx(heif_context_alloc(),
                                    [] (heif_context* c) { heif_context_free(c); });
        if (!ctx) throw Error(kerFailedToReadImageData);

        struct heif_error err;
        err = heif_context_read_from_file(ctx.get(), io_->path().c_str(), nullptr);

        if (err.code != 0)
        {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cerr << "Exiv2::HeifImage::readMetadata: Could not read HEIF file: " << err.message << std::endl;
#endif
            throw Error(kerFailedToReadImageData);
        }

        heif_item_id primary_image_id;
        heif_context_get_primary_image_ID(ctx.get(), &primary_image_id);

        struct heif_image_handle* handle = nullptr;
        err = heif_context_get_image_handle(ctx.get(), primary_image_id, &handle);

        if (err.code)
        {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cerr << "Exiv2::HeifImage::readMetadata: " << err.message << std::endl;
#endif
            throw Error(kerFailedToReadImageData);
        }

        pixelWidth_  = heif_image_handle_get_width(handle);
        pixelHeight_ = heif_image_handle_get_height(handle);

        heif_item_id dataIds[10];
        int num_metadata = heif_image_handle_get_list_of_metadata_block_IDs(handle,
                                                                            nullptr,
                                                                            dataIds,
                                                                            10);

#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Exiv2::HeifImage::readMetadata: " << "Found " << num_metadata << " HEIF metadata chunck" << std::endl;
#endif

        if (num_metadata > 0)
        {
            for (int i = 0 ; i < num_metadata ; ++i)
            {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Parsing HEIF metadata chunck:" << heif_image_handle_get_metadata_type(handle, dataIds[i]) << std::endl;
#endif
                if (std::string(heif_image_handle_get_metadata_type(handle, dataIds[i])) == std::string("Exif"))
                {
                    // Read Exif chunk.

                    size_t data_size    = heif_image_handle_get_metadata_size(handle, dataIds[i]);
                    uint8_t* const data = (uint8_t*) alloca(data_size);
                    err                 = heif_image_handle_get_metadata(handle, dataIds[i], data);

                    if (err.code)
                    {
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cerr << "Exiv2::HeifImage::readMetadata: " << err.message << std::endl;
#endif
                        throw Error(kerFailedToReadImageData);
                    }

                    if (data_size > 4)
                    {
                        // The first 4 bytes indicate the
                        // offset to the start of the TIFF header of the Exif data.

                        int skip = ((data[0] << 24) |
                                    (data[1] << 16) |
                                    (data[2] << 8)  |
                                     data[3]) + 4;

                        if (data_size > (size_t)skip)
                        {
                            // Copy the real exif data into the byte array

#ifdef EXIV2_DEBUG_MESSAGES
                            std::cerr << "Exiv2::HeifImage:: HEIF exif container found with size:" << data_size - skip << std::endl;
#endif

                            // hexdump (std::cerr, data, data_size);

                            ByteOrder bo = ExifParser::decode(exifData_, data + skip, data_size - skip);
                            setByteOrder(bo);

                            if (data_size > 0 && byteOrder() == invalidByteOrder)
                            {
#ifndef SUPPRESS_WARNINGS
                                EXV_WARNING << "Failed to decode Exif metadata.\n";
#endif
                                exifData_.clear();
                            }
                        }
                    }
                }

                if (
                    (std::string(heif_image_handle_get_metadata_type(handle, dataIds[i]))         == std::string("mime")) &&
                    (std::string(heif_image_handle_get_metadata_content_type(handle, dataIds[i])) == std::string("application/rdf+xml"))
                   )
                {
                    // Read Xmp chunk.

                    size_t data_size    = heif_image_handle_get_metadata_size(handle, dataIds[i]);
                    uint8_t* const data = (uint8_t*) alloca(data_size);
                    err                 = heif_image_handle_get_metadata(handle, dataIds[i], data);

                    if (err.code)
                    {
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cerr << "Exiv2::HeifImage::readMetadata: " << err.message << std::endl;
#endif
                        throw Error(kerFailedToReadImageData);
                    }

#ifdef EXIV2_DEBUG_MESSAGES
                    std::cerr << "Exiv2::HeifImage:: HEIF Xmp container found with size:" << data_size << std::endl;
#endif

                    xmpPacket_.assign(reinterpret_cast<char *>(data), data_size);
                    std::string::size_type idx = xmpPacket_.find_first_of('<');

                    if (idx != std::string::npos && idx > 0)
                    {
#ifndef SUPPRESS_WARNINGS
                        EXV_WARNING << "Removing " << static_cast<uint32_t>(idx)
                                    << " characters from the beginning of the Xmp packet" << std::endl;
#endif
                        xmpPacket_ = xmpPacket_.substr(idx);
                    }

                    if (xmpPacket_.size() > 0 && XmpParser::decode(xmpData_, xmpPacket_))
                    {
#ifndef SUPPRESS_WARNINGS
                        EXV_WARNING << "Failed to decode Xmp metadata." << std::endl;
#endif
                    }
                }

                if (std::string(heif_image_handle_get_metadata_type(handle, dataIds[i])) == std::string("iptc"))
                {
                    // Read Iptc chunk.

                    size_t data_size    = heif_image_handle_get_metadata_size(handle, dataIds[i]);
                    uint8_t* const data = (uint8_t*) alloca(data_size);
                    err                 = heif_image_handle_get_metadata(handle, dataIds[i], data);

                    if (err.code)
                    {
#ifdef EXIV2_DEBUG_MESSAGES
                        std::cerr << "Exiv2::HeifImage::readMetadata: " << err.message << std::endl;
#endif
                        throw Error(kerFailedToReadImageData);
                    }

#ifdef EXIV2_DEBUG_MESSAGES
                    std::cerr << "Exiv2::HeifImage:: HEIF Iptc container found with size:" << data_size << std::endl;
#endif
                    if (IptcParser::decode(iptcData_, data, data_size))
                    {
#ifndef SUPPRESS_WARNINGS
                        EXV_WARNING << "Failed to decode Iptc metadata." << std::endl;
#endif
                        iptcData_.clear();
                    }
                }

            }

            heif_image_handle_release(handle);
        }

    } // HeifImage::readMetadata

    void HeifImage::printStructure(std::ostream& /*out*/, PrintStructureOption /*option*/, int /*depth*/)
    {
        if (io_->open() != 0)
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());

        // Ensure that this is the correct image type
        if (!isHeifType(*io_, false))
        {
            if (io_->error() || io_->eof())
                throw Error(kerFailedToReadImageData);

            throw Error(kerNotExpectedFormat);
        }
    }  // HeifImage::printStructure

    void HeifImage::writeMetadata()
    {
        if (io_->open() != 0)
        {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        BasicIo::UniquePtr tempIo(new MemIo);
        assert (tempIo.get() != 0);

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw

    } // HeifImage::writeMetadata

    void HeifImage::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(kerInputDataReadFailed);
        if (!outIo.isopen()) throw Error(kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::HeifImage::doWriteMetadata: Writing HEIF file " << io_->path() << std::endl;
        std::cout << "Exiv2::HeifImage::doWriteMetadata: tmp file created " << outIo.path() << std::endl;
#endif

        // Ensure that this is the correct image type
        if (!isHeifType(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(kerInputDataReadFailed);
            throw Error(kerNoImageInInputData);
        }

        // Write HEIF Signature.
        if (outIo.write(HeifSignature, sizeof(HeifSignature)) != sizeof(HeifSignature)) throw Error(kerImageWriteFailed);
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::HeifImage::doWriteMetadata: EOF" << std::endl;
#endif

    } // HeifImage::doWriteMetadata

    Image::UniquePtr newHeifInstance(BasicIo::UniquePtr io, bool create)
    {
        Image::UniquePtr image(new HeifImage(std::move(io), create));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isHeifType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 12;
        byte buf[len];
        iIo.read(buf, len);

        if (iIo.error() || iIo.eof())
        {
            return false;
        }


        bool matched = (
                         (memcmp(&buf[4], "ftyp", 4) == 0) ||
                         (memcmp(&buf[8], "heic", 4) == 0) ||
                         (memcmp(&buf[8], "heix", 4) == 0) ||
                         (memcmp(&buf[8], "mif1", 4) == 0)
                       );

#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::HeifImage::isHeifType() = " << matched << std::endl;
#endif
        if (!advance || !matched)
        {
            iIo.seek(-len, BasicIo::cur);
        }

        return matched;
    }

} // namespace Exiv2

