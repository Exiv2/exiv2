// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
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
  File:      tiffimage.cpp
 */
// *****************************************************************************
// included header files
#include "config.h"

#include "tiffimage.hpp"
#include "tiffimage_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffvisitor_int.hpp"
#include "orfimage.hpp"
#include "makernote_int.hpp"
#include "nikonmn_int.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "types.hpp"
#include "basicio.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cstdarg>

/* --------------------------------------------------------------------------

   Todo:

   + CR2 Makernotes don't seem to have a next pointer but Canon Jpeg Makernotes
     do. What a mess. (That'll become an issue when it comes to writing to CR2)
   + Sony makernotes in RAW files do not seem to have header like those in Jpegs.
     And maybe no next pointer either.

   in crwimage.* :

   + Fix CiffHeader according to TiffHeader
   + Combine Error(kerNotAJpeg) and Error(kerNotACrwImage), add format argument %1
   + Search crwimage for todos, fix writeMetadata comment
   + rename loadStack to getPath for consistency

   -------------------------------------------------------------------------- */

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    TiffImage::TiffImage(BasicIo::UniquePtr io, bool /*create*/)
        : Image(ImageType::tiff, mdExif | mdIptc | mdXmp, std::move(io)),
          pixelWidth_(0), pixelHeight_(0)
    {
    } // TiffImage::TiffImage

    //! Structure for TIFF compression to MIME type mappings
    struct MimeTypeList {
        //! Comparison operator for compression
        bool operator==(int compression) const { return compression_ == compression; }
        int compression_;                       //!< TIFF compression
        const char* mimeType_;                  //!< MIME type
    };

    //! List of TIFF compression to MIME type mappings
    MimeTypeList mimeTypeList[] = {
        { 32770, "image/x-samsung-srw" },
        { 34713, "image/x-nikon-nef"   },
        { 65535, "image/x-pentax-pef"  }
    };

    std::string TiffImage::mimeType() const
    {
        if (!mimeType_.empty()) return mimeType_;

        mimeType_ = std::string("image/tiff");
        std::string key = "Exif." + primaryGroup() + ".Compression";
        ExifData::const_iterator md = exifData_.findKey(ExifKey(key));
        if (md != exifData_.end() && md->count() > 0) {
            const MimeTypeList* i = find(mimeTypeList, static_cast<int>(md->toLong()));
            if (i) mimeType_ = std::string(i->mimeType_);
        }
        return mimeType_;
    }

    std::string TiffImage::primaryGroup() const
    {
        if (!primaryGroup_.empty()) return primaryGroup_;

        static const char* keys[] = {
            "Exif.Image.NewSubfileType",
            "Exif.SubImage1.NewSubfileType",
            "Exif.SubImage2.NewSubfileType",
            "Exif.SubImage3.NewSubfileType",
            "Exif.SubImage4.NewSubfileType",
            "Exif.SubImage5.NewSubfileType",
            "Exif.SubImage6.NewSubfileType",
            "Exif.SubImage7.NewSubfileType",
            "Exif.SubImage8.NewSubfileType",
            "Exif.SubImage9.NewSubfileType"
        };
        // Find the group of the primary image, default to "Image"
        primaryGroup_ = std::string("Image");
        for (auto& i : keys) {
            ExifData::const_iterator md = exifData_.findKey(ExifKey(i));
            // Is it the primary image?
            if (md != exifData_.end() && md->count() > 0 && md->toLong() == 0) {
                // Sometimes there is a JPEG primary image; that's not our first choice
                primaryGroup_ = md->groupName();
                std::string key = "Exif." + primaryGroup_ + ".JPEGInterchangeFormat";
                if (exifData_.findKey(ExifKey(key)) == exifData_.end())
                    break;
            }
        }
        return primaryGroup_;
    }

    int TiffImage::pixelWidth() const
    {
        if (pixelWidth_ != 0) return pixelWidth_;

        ExifKey key(std::string("Exif.") + primaryGroup() + std::string(".ImageWidth"));
        ExifData::const_iterator imageWidth = exifData_.findKey(key);
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            pixelWidth_ = static_cast<int>(imageWidth->toLong());
        }
        return pixelWidth_;
    }

    int TiffImage::pixelHeight() const
    {
        if (pixelHeight_ != 0) return pixelHeight_;

        ExifKey key(std::string("Exif.") + primaryGroup() + std::string(".ImageLength"));
        ExifData::const_iterator imageHeight = exifData_.findKey(key);
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            pixelHeight_ = imageHeight->toLong();
        }
        return pixelHeight_;
    }

    void TiffImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(kerInvalidSettingForImage, "Image comment", "TIFF"));
    }

    void TiffImage::readMetadata()
    {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Reading TIFF file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }

        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isTiffType(*io_, false)) {
            if (io_->error() || io_->eof())
                throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage, "TIFF");
        }
        clearMetadata();

        ByteOrder bo = TiffParser::decode(exifData_,
                                          iptcData_,
                                          xmpData_,
                                          io_->mmap(),
                                          (uint32_t) io_->size());
        setByteOrder(bo);

        // read profile from the metadata
        Exiv2::ExifKey            key("Exif.Image.InterColorProfile");
        Exiv2::ExifData::iterator pos   = exifData_.findKey(key);
        if ( pos != exifData_.end() ) {
            const size_t size = pos->count() * pos->typeSize();
            if (size == 0) {
                throw Error(kerFailedToReadImageData);
            }
            iccProfile_.alloc(size);
            pos->copy(iccProfile_.pData_,bo);
        }

    }

    void TiffImage::writeMetadata()
    {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Writing TIFF file " << io_->path() << "\n";
#endif
        ByteOrder bo = byteOrder();
        byte* pData = nullptr;
        long size = 0;
        IoCloser closer(*io_);
        if (io_->open() == 0) {
            // Ensure that this is the correct image type
            if (isTiffType(*io_, false)) {
                pData = io_->mmap(true);
                size = (long) io_->size();
                TiffHeader tiffHeader;
                if (0 == tiffHeader.read(pData, 8)) {
                    bo = tiffHeader.byteOrder();
                }
            }
        }
        if (bo == invalidByteOrder) {
            bo = littleEndian;
        }
        setByteOrder(bo);

        // fixup ICC profile
        Exiv2::ExifKey            key("Exif.Image.InterColorProfile");
        Exiv2::ExifData::iterator pos   = exifData_.findKey(key);
        bool                      found = pos != exifData_.end();
        if ( iccProfileDefined() ) {
            Exiv2::DataValue value(iccProfile_.pData_, (long)iccProfile_.size_);
            if (found)
                pos->setValue(&value);
            else
                exifData_.add(key, &value);
        } else {
            if ( found ) exifData_.erase(pos);
        }

        // set usePacket to influence TiffEncoder::encodeXmp() called by TiffVisitor.encode()
        xmpData().usePacket(writeXmpFromPacket());

        TiffParser::encode(*io_, pData, size, bo, exifData_, iptcData_, xmpData_); // may throw
    } // TiffImage::writeMetadata

    ByteOrder TiffParser::decode(ExifData& exifData,
              IptcData& iptcData,
              XmpData&  xmpData,
        const byte*     pData,
              size_t size
    )
    {
        return TiffParserWorker::decode(exifData,
                                        iptcData,
                                        xmpData,
                                        pData,
                                        size,
                                        Tag::root,
                                        TiffMapping::findDecoder);
    } // TiffParser::decode

    WriteMethod TiffParser::encode(BasicIo&  io,
        const byte*     pData,
              size_t size,
              ByteOrder byteOrder,
        const ExifData& exifData,
        const IptcData& iptcData,
        const XmpData&  xmpData
    )
    {
        // Copy to be able to modify the Exif data
        ExifData ed = exifData;

        // Delete IFDs which do not occur in TIFF images
        static const IfdId filteredIfds[] = {
            panaRawId
        };
        for (auto filteredIfd : filteredIfds) {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cerr << "Warning: Exif IFD " << filteredIfds[i] << " not encoded\n";
#endif
            ed.erase(std::remove_if(ed.begin(), ed.end(), FindExifdatum(filteredIfd)), ed.end());
        }

        std::unique_ptr<TiffHeaderBase> header(new TiffHeader(byteOrder));
        return TiffParserWorker::encode(io, pData, size, ed, iptcData, xmpData, Tag::root, TiffMapping::findEncoder,
                                        header.get(), nullptr);
    } // TiffParser::encode

    // *************************************************************************
    // free functions
    Image::UniquePtr newTiffInstance(BasicIo::UniquePtr io, bool create)
    {
        Image::UniquePtr image(new TiffImage(std::move(io), create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isTiffType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 8;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        TiffHeader tiffHeader;
        bool rc = tiffHeader.read(buf, len);
        if (!advance || !rc) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc;
    }

    void TiffImage::printStructure(std::ostream& out, Exiv2::PrintStructureOption option,int depth)
    {
        if (io_->open() != 0) throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        // Ensure that this is the correct image type
        if ( imageType() == ImageType::none )
        if (!isTiffType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(kerFailedToReadImageData);
            throw Error(kerNotAJpeg);
        }

        io_->seek(0,BasicIo::beg);

        printTiffStructure(io(),out,option,depth-1);
    }

}                                       // namespace Exiv2
