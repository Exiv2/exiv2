// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2009 Andreas Huggel <ahuggel@gmx.net>
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
  File:      tiffimage.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Mar-06, ahu: created

 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "tiffimage.hpp"
#include "tiffimage_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffvisitor_int.hpp"
#include "makernote_int.hpp"
#include "image.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "types.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <memory>

/* --------------------------------------------------------------------------

   Todo:

   + CR2 Makernotes don't seem to have a next pointer but Canon Jpeg Makernotes
     do. What a mess. (That'll become an issue when it comes to writing to CR2)
   + Sony makernotes in RAW files do not seem to have header like those in Jpegs.
     And maybe no next pointer either.

   in crwimage.* :

   + Fix CiffHeader according to TiffHeader
   + Combine Error(15) and Error(33), add format argument %1
   + Search crwimage for todos, fix writeMetadata comment
   + rename loadStack to getPath for consistency

   -------------------------------------------------------------------------- */

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    TiffImage::TiffImage(BasicIo::AutoPtr io, bool /*create*/)
        : Image(ImageType::tiff, mdExif | mdIptc, io)
    {
    } // TiffImage::TiffImage

    std::string TiffImage::mimeType() const
    {
        return "image/tiff";
    }

    std::string TiffImage::primaryGroup() const
    {
        static const char* keys[] = {
            "Exif.Image.NewSubfileType",
            "Exif.SubImage1.NewSubfileType",
            "Exif.SubImage2.NewSubfileType",
            "Exif.SubImage3.NewSubfileType",
            "Exif.SubImage4.NewSubfileType"
        };
        // Find the group of the primary image, default to "Image"
        std::string groupName = "Image";
        for (unsigned int i = 0; i < EXV_COUNTOF(keys); ++i) {
            ExifData::const_iterator md = exifData_.findKey(ExifKey(keys[i]));
            // Is it the primary image?
            if (md != exifData_.end() && md->count() > 0 && md->toLong() == 0) {
                groupName = md->groupName();
                break;
            }
        }
        return groupName;
    }

    int TiffImage::pixelWidth() const
    {
        ExifKey key(std::string("Exif.") + primaryGroup() + std::string(".ImageWidth"));
        ExifData::const_iterator imageWidth = exifData_.findKey(key);
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            return imageWidth->toLong();
        }
        return 0;
    }

    int TiffImage::pixelHeight() const
    {
        ExifKey key(std::string("Exif.") + primaryGroup() + std::string(".ImageLength"));
        ExifData::const_iterator imageHeight = exifData_.findKey(key);
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            return imageHeight->toLong();
        }
        return 0;
    }

    void TiffImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "TIFF"));
    }

    void TiffImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading TIFF file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) throw Error(9, io_->path(), strError());
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isTiffType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "TIFF");
        }
        clearMetadata();
        ByteOrder bo = TiffParser::decode(exifData_,
                                          iptcData_,
                                          xmpData_,
                                          io_->mmap(),
                                          io_->size());
        setByteOrder(bo);
    } // TiffImage::readMetadata

    void TiffImage::writeMetadata()
    {
#ifdef DEBUG
        std::cerr << "Writing TIFF file " << io_->path() << "\n";
#endif
        ByteOrder bo = byteOrder();
        byte* pData = 0;
        long size = 0;
        IoCloser closer(*io_);
        if (io_->open() == 0) {
            // Ensure that this is the correct image type
            if (isTiffType(*io_, false)) {
                pData = io_->mmap(true);
                size = io_->size();
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
        TiffParser::encode(*io_, pData, size, bo, exifData_, iptcData_, xmpData_); // may throw
    } // TiffImage::writeMetadata

    ByteOrder TiffParser::decode(
              ExifData& exifData,
              IptcData& iptcData,
              XmpData&  xmpData,
        const byte*     pData,
              uint32_t  size
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

    WriteMethod TiffParser::encode(
              BasicIo&  io,
        const byte*     pData,
              uint32_t  size,
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
            panaRawIfdId
        };
        for (unsigned int i = 0; i < EXV_COUNTOF(filteredIfds); ++i) {
#ifdef DEBUG
            std::cerr << "Warning: Exif IFD " << filteredIfds[i] << " not encoded\n";
#endif
            ed.erase(std::remove_if(ed.begin(),
                                    ed.end(),
                                    FindExifdatum(filteredIfds[i])),
                     ed.end());
        }

        std::auto_ptr<TiffHeaderBase> header(new TiffHeader(byteOrder));
        return TiffParserWorker::encode(io,
                                        pData,
                                        size,
                                        ed,
                                        iptcData,
                                        xmpData,
                                        Tag::root,
                                        TiffMapping::findEncoder,
                                        header.get());
    } // TiffParser::encode

    // *************************************************************************
    // free functions
    Image::AutoPtr newTiffInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new TiffImage(io, create));
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

}                                       // namespace Exiv2

// Shortcuts for the newTiffBinaryArray templates.
#define EXV_BINARY_ARRAY(arrayCfg, arrayDef) (newTiffBinaryArray<&arrayCfg, EXV_COUNTOF(arrayDef), arrayDef>)
#define EXV_SIMPLE_BINARY_ARRAY(arrayCfg) (newTiffBinaryArray<&arrayCfg>)
#define EXV_COMPLEX_BINARY_ARRAY(arraySet, cfgSelFct) (newTiffBinaryArray2<arraySet, EXV_COUNTOF(arraySet), cfgSelFct>)

namespace Exiv2 {
    namespace Internal {

    //! Constant for non-encrypted binary arrays
    const CryptFct notEncrypted = 0;

    //! Canon Camera Settings binary array - configuration
    extern const ArrayCfg canonCsCfg = {
        Group::canoncs,   // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        true,             // With size element
        false,            // No fillers
        { 0, ttUnsignedShort, 1 }
    };
    //! Canon Camera Settings binary array - definition
    extern const ArrayDef canonCsDef[] = {
        { 46, ttUnsignedShort, 3 } // Exif.CanonCs.Lens
    };

    //! Canon Shot Info binary array - configuration
    extern const ArrayCfg canonSiCfg = {
        Group::canonsi,   // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        true,             // With size element
        false,            // No fillers
        { 0, ttUnsignedShort, 1 }
    };

    //! Canon Panorama binary array - configuration
    extern const ArrayCfg canonPaCfg = {
        Group::canonpa,   // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        { 0, ttUnsignedShort, 1 }
    };

    //! Canon Custom Function binary array - configuration
    extern const ArrayCfg canonCfCfg = {
        Group::canoncf,   // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        true,             // With size element
        false,            // No fillers
        { 0, ttUnsignedShort, 1 }
    };

    //! Canon Picture Info binary array - configuration
    extern const ArrayCfg canonPiCfg = {
        Group::canonpi,   // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        { 0, ttUnsignedShort, 1 }
    };

    //! Canon File Info binary array - configuration
    extern const ArrayCfg canonFiCfg = {
        Group::canonfi,   // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUnsignedShort,  // Type for array entry and size element
        notEncrypted,     // Not encrypted
        true,             // Has a size element
        false,            // No fillers
        { 0, ttSignedShort, 1 }
    };
    //! Canon File Info binary array - definition
    extern const ArrayDef canonFiDef[] = {
        { 2, ttUnsignedLong, 1 }
    };

    //! Nikon Vibration Reduction binary array - configuration
    extern const ArrayCfg nikonVrCfg = {
        Group::nikonvr,   // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Vibration Reduction binary array - definition
    extern const ArrayDef nikonVrDef[] = {
        { 0, ttUndefined,     4 }, // Version
        { 7, ttUnsignedByte,  1 }  // The array contains 8 bytes
    };

    //! Nikon Picture Control binary array - configuration
    extern const ArrayCfg nikonPcCfg = {
        Group::nikonpc,   // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Picture Control binary array - definition
    extern const ArrayDef nikonPcDef[] = {
        {  0, ttUndefined,     4 }, // Version
        {  4, ttAsciiString,  20 },
        { 24, ttAsciiString,  20 },
        { 57, ttUnsignedByte,  1 }  // The array contains 58 bytes
    };

    //! Nikon World Time binary array - configuration
    extern const ArrayCfg nikonWtCfg = {
        Group::nikonwt,   // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon World Time binary array - definition
    extern const ArrayDef nikonWtDef[] = {
        { 0, ttSignedShort,   1 },
        { 3, ttUnsignedByte,  1 }
    };

    //! Nikon ISO info binary array - configuration
    extern const ArrayCfg nikonIiCfg = {
        Group::nikonii,   // Group for the elements
        bigEndian,        // Byte order
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon ISO info binary array - definition
    extern const ArrayDef nikonIiDef[] = {
        {  4, ttUnsignedShort, 1 },
        { 10, ttUnsignedShort, 1 },
        { 13, ttUnsignedByte,  1 }  // The array contains 14 bytes
    };

    //! Nikon Auto Focus binary array - configuration
    extern const ArrayCfg nikonAfCfg = {
        Group::nikonaf,   // Group for the elements
        littleEndian,     // Byte order
        ttUndefined,      // Type for array entry
        notEncrypted,     // Not encrypted
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Auto Focus binary array - definition
    extern const ArrayDef nikonAfDef[] = {
        { 2, ttUnsignedShort, 1 } // The array contains 4 bytes
    };

    //! Nikon Shot Info binary array - configuration 1 (D80)
    extern const ArrayCfg nikonSi1Cfg = {
        Group::nikonsi1,  // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Shot Info binary array - definition 1 (D80)
    extern const ArrayDef nikonSi1Def[] = {
        {    0, ttUndefined,    4 }, // Version
        {  586, ttUnsignedLong, 1 }, // ShutterCount
        { 1155, ttUnsignedByte, 1 }  // The array contains 1156 bytes
    };
    //! Nikon Shot Info binary array - configuration 2 (D40)
    extern const ArrayCfg nikonSi2Cfg = {
        Group::nikonsi2,  // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Shot Info binary array - definition 2 (D40)
    extern const ArrayDef nikonSi2Def[] = {
        {    0, ttUndefined,    4 }, // Version
        {  582, ttUnsignedLong, 1 }, // ShutterCount
        { 1112, ttUnsignedByte, 1 }  // The array contains 1113 bytes
    };
    //! Nikon Shot Info binary array - configuration 3 (D300a)
    extern const ArrayCfg nikonSi3Cfg = {
        Group::nikonsi3,  // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Shot Info binary array - definition 3 (D300a)
    extern const ArrayDef nikonSi3Def[] = {
        {    0, ttUndefined,     4 }, // Version
        {  633, ttUnsignedLong,  1 }, // ShutterCount
        {  721, ttUnsignedShort, 1 }, // AFFineTuneAdj
        {  814, ttUndefined,  4478 }  // The array contains 5291 bytes
    };
    //! Nikon Shot Info binary array - configuration 4 (D300b)
    extern const ArrayCfg nikonSi4Cfg = {
        Group::nikonsi4,  // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Shot Info binary array - definition 4 (D300b)
    extern const ArrayDef nikonSi4Def[] = {
        {    0, ttUndefined,     4 }, // Version
        {  644, ttUnsignedLong,  1 }, // ShutterCount
        {  732, ttUnsignedShort, 1 }, // AFFineTuneAdj
        {  826, ttUndefined,  4478 }  // The array contains 5303 bytes
    };
    //! Nikon Shot Info binary array - configuration 5 (ver 02.xx)
    extern const ArrayCfg nikonSi5Cfg = {
        Group::nikonsi5,  // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        false,            // Write all tags (don't know how many)
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Shot Info binary array - definition 5 (ver 01.xx and ver 02.xx)
    extern const ArrayDef nikonSi5Def[] = {
        {    0, ttUndefined,     4 }, // Version
        {  106, ttUnsignedLong,  1 }, // ShutterCount
        {  110, ttUnsignedLong,  1 }, // DeletedImageCount
        {  343, ttUndefined,     2 }, // ShutterCount
        {  630, ttUnsignedLong,  1 }  // ShutterCount
    };
    //! Nikon Shot Info binary array - configuration 6 (ver 01.xx)
    extern const ArrayCfg nikonSi6Cfg = {
        Group::nikonsi6,  // Group for the elements
        bigEndian,        // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Encryption function
        false,            // No size element
        false,            // Write all tags (don't know how many)
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Lens Data configurations and definitions
    extern const ArraySet nikonSiSet[] = {
        { nikonSi1Cfg, nikonSi1Def, EXV_COUNTOF(nikonSi1Def) },
        { nikonSi2Cfg, nikonSi2Def, EXV_COUNTOF(nikonSi2Def) },
        { nikonSi3Cfg, nikonSi3Def, EXV_COUNTOF(nikonSi3Def) },
        { nikonSi4Cfg, nikonSi4Def, EXV_COUNTOF(nikonSi4Def) },
        { nikonSi5Cfg, nikonSi5Def, EXV_COUNTOF(nikonSi5Def) },
        { nikonSi6Cfg, nikonSi5Def, EXV_COUNTOF(nikonSi5Def) }  // uses nikonSi5Def
    };

    //! Nikon Lens Data binary array - configuration 1
    extern const ArrayCfg nikonLd1Cfg = {
        Group::nikonld1,  // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Encryption function
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Lens Data binary array - configuration 2
    extern const ArrayCfg nikonLd2Cfg = {
        Group::nikonld2,  // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Lens Data binary array - configuration 3
    extern const ArrayCfg nikonLd3Cfg = {
        Group::nikonld3,  // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        true,             // Write all tags
        { 0, ttUnsignedByte,  1 }
    };
    //! Nikon Lens Data binary array - definition
    extern const ArrayDef nikonLdDef[] = {
        { 0, ttUndefined, 4 } // Version
    };
    //! Nikon Lens Data configurations and definitions
    extern const ArraySet nikonLdSet[] = {
        { nikonLd1Cfg, nikonLdDef, EXV_COUNTOF(nikonLdDef) },
        { nikonLd2Cfg, nikonLdDef, EXV_COUNTOF(nikonLdDef) },
        { nikonLd3Cfg, nikonLdDef, EXV_COUNTOF(nikonLdDef) }
    };

    //! Nikon Color Balance binary array - configuration 1
    extern const ArrayCfg nikonCb1Cfg = {
        Group::nikoncb1,  // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Encryption function
        false,            // No size element
        false,            // Write all tags
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - configuration 2
    extern const ArrayCfg nikonCb2Cfg = {
        Group::nikoncb2,  // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        false,            // Write all tags
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - configuration 2a
    extern const ArrayCfg nikonCb2aCfg = {
        Group::nikoncb2a, // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        false,            // Write all tags
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - configuration 2b
    extern const ArrayCfg nikonCb2bCfg = {
        Group::nikoncb2b, // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        false,            // Write all tags
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - configuration 3
    extern const ArrayCfg nikonCb3Cfg = {
        Group::nikoncb3,  // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        notEncrypted,     // Encryption function
        false,            // No size element
        false,            // Write all tags
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - configuration 4
    extern const ArrayCfg nikonCb4Cfg = {
        Group::nikoncb4,  // Group for the elements
        invalidByteOrder, // Use byte order from parent
        ttUndefined,      // Type for array entry
        nikonCrypt,       // Encryption function
        false,            // No size element
        false,            // Write all tags
        { 0, ttUnsignedShort, 1 }
    };
    //! Nikon Color Balance binary array - definition 1 (D100)
    extern const ArrayDef nikonCb1Def[] = {
        {  0, ttUndefined,        4 }, // Version
        {  4, ttUnsignedShort,   34 }, // Unknown
        { 72, ttUnsignedShort,    4 }, // Color balance levels
        { 80, ttUnsignedShort, 9999 }  // Unknown
    };
    //! Nikon Color Balance binary array - definition 2 (D2H)
    extern const ArrayDef nikonCb2Def[] = {
        {  0, ttUndefined,        4 }, // Version
        {  4, ttUnsignedShort,    3 }, // Unknown
        { 10, ttUnsignedShort,    4 }, // Color balance levels
        { 18, ttUnsignedShort, 9999 }  // Unknown
    };
    //! Nikon Color Balance binary array - definition 2a (D50)
    extern const ArrayDef nikonCb2aDef[] = {
        {  0, ttUndefined,        4 }, // Version
        {  4, ttUnsignedShort,    7 }, // Unknown
        { 18, ttUnsignedShort,    4 }, // Color balance levels
        { 26, ttUnsignedShort, 9999 }  // Unknown
    };
    //! Nikon Color Balance binary array - definition 2b (D2X=0204,D2Hs=0206,D200=0207,D40=0208)
    extern const ArrayDef nikonCb2bDef[] = {
        {  0, ttUndefined,        4 }, // Version
        {  4, ttUnsignedShort,  140 }, // Unknown
        {284, ttUnsignedShort,    3 }, // Unknown (encrypted)
        {290, ttUnsignedShort,    4 }, // Color balance levels
        {298, ttUnsignedShort, 9999 }  // Unknown (encrypted)
    };
    //! Nikon Color Balance binary array - definition 3 (D70)
    extern const ArrayDef nikonCb3Def[] = {
        {  0, ttUndefined,        4 }, // Version
        {  4, ttUnsignedShort,    8 }, // Unknown
        { 20, ttUnsignedShort,    4 }, // Color balance levels
        { 28, ttUnsignedShort, 9999 }  // Unknown
    };
    //! Nikon Color Balance binary array - definition 4 (D3)
    extern const ArrayDef nikonCb4Def[] = {
        {  0, ttUndefined,        4 }, // Version
        {  4, ttUnsignedShort,  140 }, // Unknown
        {284, ttUnsignedShort,    5 }, // Unknown (encrypted)
        {294, ttUnsignedShort,    4 }, // Color balance levels
        {302, ttUnsignedShort, 9999 }  // Unknown (encrypted)
    };
    //! Nikon Color Balance configurations and definitions
    extern const ArraySet nikonCbSet[] = {
        { nikonCb1Cfg,  nikonCb1Def,  EXV_COUNTOF(nikonCb1Def)  },
        { nikonCb2Cfg,  nikonCb2Def,  EXV_COUNTOF(nikonCb2Def)  },
        { nikonCb2aCfg, nikonCb2aDef, EXV_COUNTOF(nikonCb2aDef) },
        { nikonCb2bCfg, nikonCb2bDef, EXV_COUNTOF(nikonCb2bDef) },
        { nikonCb3Cfg,  nikonCb3Def,  EXV_COUNTOF(nikonCb3Def)  },
        { nikonCb4Cfg,  nikonCb4Def,  EXV_COUNTOF(nikonCb4Def)  }
    };

    //! Minolta Camera Settings (old) binary array - configuration
    extern const ArrayCfg minoCsoCfg = {
        Group::minocso,   // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        { 0, ttUnsignedLong, 1 }
    };

    //! Minolta Camera Settings (new) binary array - configuration
    extern const ArrayCfg minoCsnCfg = {
        Group::minocsn,   // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        { 0, ttUnsignedLong, 1 }
    };

    //! Minolta 7D Camera Settings binary array - configuration
    extern const ArrayCfg minoCs7Cfg = {
        Group::minocs7,   // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        { 0, ttUnsignedShort, 1 }
    };
    //! Minolta 7D Camera Settings binary array - definition
    extern const ArrayDef minoCs7Def[] = {
        {  60, ttSignedShort, 1 }, // Exif.MinoltaCs7D.ExposureCompensation
        { 126, ttSignedShort, 1 }  // Exif.MinoltaCs7D.ColorTemperature
    };

    //! Minolta 5D Camera Settings binary array - configuration
    extern const ArrayCfg minoCs5Cfg = {
        Group::minocs5,   // Group for the elements
        bigEndian,        // Big endian
        ttUndefined,      // Type for array entry and size element
        notEncrypted,     // Not encrypted
        false,            // No size element
        false,            // No fillers
        { 0, ttUnsignedShort, 1 }
    };
    //! Minolta 5D Camera Settings binary array - definition
    extern const ArrayDef minoCs5Def[] = {
        { 146, ttSignedShort, 1 } // Exif.MinoltaCs5D.ColorTemperature
    };

    /*
      This table lists for each group in a tree, its parent group and tag.
      Root identifies the root of a TIFF tree, as there is a need for multiple
      trees. Groups are the nodes of a TIFF tree. A group is an IFD or any
      other composite component.

      With this table, it is possible, for a given group (and tag) to find a
      path, i.e., a list of groups and tags, from the root to that group (tag).
    */
    const TiffTreeStruct TiffCreator::tiffTreeStruct_[] = {
        // root      group             parent group      parent tag
        //---------  ----------------- ----------------- ----------
        { Tag::root, Group::none,      Group::none,      Tag::root },
        { Tag::root, Group::ifd0,      Group::none,      Tag::root },
        { Tag::root, Group::subimg1,   Group::ifd0,      0x014a    },
        { Tag::root, Group::subimg2,   Group::ifd0,      0x014a    },
        { Tag::root, Group::subimg3,   Group::ifd0,      0x014a    },
        { Tag::root, Group::subimg4,   Group::ifd0,      0x014a    },
        { Tag::root, Group::exif,      Group::ifd0,      0x8769    },
        { Tag::root, Group::gps,       Group::ifd0,      0x8825    },
        { Tag::root, Group::iop,       Group::exif,      0xa005    },
        { Tag::root, Group::ifd1,      Group::ifd0,      Tag::next },
        { Tag::root, Group::ifd2,      Group::ifd1,      Tag::next },
        { Tag::root, Group::olymp1mn,  Group::exif,      0x927c    },
        { Tag::root, Group::olymp2mn,  Group::exif,      0x927c    },
        { Tag::root, Group::olympeq,   Group::olymp2mn,  0x2010    },
        { Tag::root, Group::olympcs,   Group::olymp2mn,  0x2020    },
        { Tag::root, Group::olymprd,   Group::olymp2mn,  0x2030    },
        { Tag::root, Group::olymprd2,  Group::olymp2mn,  0x2031    },
        { Tag::root, Group::olympip,   Group::olymp2mn,  0x2040    },
        { Tag::root, Group::olympfi,   Group::olymp2mn,  0x2050    },
        { Tag::root, Group::olympfe1,  Group::olymp2mn,  0x2100    },
        { Tag::root, Group::olympfe2,  Group::olymp2mn,  0x2200    },
        { Tag::root, Group::olympfe3,  Group::olymp2mn,  0x2300    },
        { Tag::root, Group::olympfe4,  Group::olymp2mn,  0x2400    },
        { Tag::root, Group::olympfe5,  Group::olymp2mn,  0x2500    },
        { Tag::root, Group::olympfe6,  Group::olymp2mn,  0x2600    },
        { Tag::root, Group::olympfe7,  Group::olymp2mn,  0x2700    },
        { Tag::root, Group::olympfe8,  Group::olymp2mn,  0x2800    },
        { Tag::root, Group::olympfe9,  Group::olymp2mn,  0x2900    },
        { Tag::root, Group::olympri,   Group::olymp2mn,  0x3000    },
        { Tag::root, Group::fujimn,    Group::exif,      0x927c    },
        { Tag::root, Group::canonmn,   Group::exif,      0x927c    },
        { Tag::root, Group::canoncs,   Group::canonmn,   0x0001    },
        { Tag::root, Group::canonsi,   Group::canonmn,   0x0004    },
        { Tag::root, Group::canonpa,   Group::canonmn,   0x0005    },
        { Tag::root, Group::canoncf,   Group::canonmn,   0x000f    },
        { Tag::root, Group::canonpi,   Group::canonmn,   0x0012    },
        { Tag::root, Group::canonfi,   Group::canonmn,   0x0093    },
        { Tag::root, Group::nikon1mn,  Group::exif,      0x927c    },
        { Tag::root, Group::nikon2mn,  Group::exif,      0x927c    },
        { Tag::root, Group::nikon3mn,  Group::exif,      0x927c    },
        { Tag::root, Group::nikonpv,   Group::nikon3mn,  0x0011    },
        { Tag::root, Group::nikonvr,   Group::nikon3mn,  0x001f    },
        { Tag::root, Group::nikonpc,   Group::nikon3mn,  0x0023    },
        { Tag::root, Group::nikonwt,   Group::nikon3mn,  0x0024    },
        { Tag::root, Group::nikonii,   Group::nikon3mn,  0x0025    },
        { Tag::root, Group::nikonaf,   Group::nikon3mn,  0x0088    },
        { Tag::root, Group::nikonsi1,  Group::nikon3mn,  0x0091    },
        { Tag::root, Group::nikonsi2,  Group::nikon3mn,  0x0091    },
        { Tag::root, Group::nikonsi3,  Group::nikon3mn,  0x0091    },
        { Tag::root, Group::nikonsi4,  Group::nikon3mn,  0x0091    },
        { Tag::root, Group::nikonsi5,  Group::nikon3mn,  0x0091    },
        { Tag::root, Group::nikonsi6,  Group::nikon3mn,  0x0091    },
        { Tag::root, Group::nikoncb1,  Group::nikon3mn,  0x0097    },
        { Tag::root, Group::nikoncb2,  Group::nikon3mn,  0x0097    },
        { Tag::root, Group::nikoncb2a, Group::nikon3mn,  0x0097    },
        { Tag::root, Group::nikoncb2b, Group::nikon3mn,  0x0097    },
        { Tag::root, Group::nikoncb3,  Group::nikon3mn,  0x0097    },
        { Tag::root, Group::nikoncb4,  Group::nikon3mn,  0x0097    },
        { Tag::root, Group::nikonld1,  Group::nikon3mn,  0x0098    },
        { Tag::root, Group::nikonld2,  Group::nikon3mn,  0x0098    },
        { Tag::root, Group::nikonld3,  Group::nikon3mn,  0x0098    },
        { Tag::root, Group::panamn,    Group::exif,      0x927c    },
        { Tag::root, Group::pentaxmn,  Group::exif,      0x927c    },
        { Tag::root, Group::sigmamn,   Group::exif,      0x927c    },
        { Tag::root, Group::sony1mn,   Group::exif,      0x927c    },
        { Tag::root, Group::sony2mn,   Group::exif,      0x927c    },
        { Tag::root, Group::minoltamn, Group::exif,      0x927c    },
        { Tag::root, Group::minocso,   Group::minoltamn, 0x0001    },
        { Tag::root, Group::minocsn,   Group::minoltamn, 0x0003    },
        { Tag::root, Group::minocs7,   Group::minoltamn, 0x0004    },
        { Tag::root, Group::minocs5,   Group::minoltamn, 0x0114    },
        // ---------------------------------------------------------
        // Panasonic RW2 raw images
        { Tag::pana, Group::none,      Group::none,      Tag::pana },
        { Tag::pana, Group::panaraw,   Group::none,      Tag::pana },
        { Tag::pana, Group::exif,      Group::panaraw,   0x8769    },
        { Tag::pana, Group::gps,       Group::panaraw,   0x8825    }
    };

    /*
      This table describes the layout of each known TIFF group (including
      non-standard structures and IFDs only seen in RAW images).

      The key of the table consists of the first two attributes, (extended) tag
      and group. Tag is the TIFF tag or one of a few extended tags, group
      identifies the IFD or any other composite component.

      Each entry of the table defines for a particular tag and group combination
      the corresponding TIFF component create function.
     */
    const TiffGroupStruct TiffCreator::tiffGroupStruct_[] = {
        // ext. tag  group             create function
        //---------  ----------------- -----------------------------------------
        // Root directory
        { Tag::root, Group::none,      newTiffDirectory<Group::ifd0>             },

        // IFD0
        {    0x8769, Group::ifd0,      newTiffSubIfd<Group::exif>                },
        {    0x8825, Group::ifd0,      newTiffSubIfd<Group::gps>                 },
        {    0x0111, Group::ifd0,      newTiffImageData<0x0117, Group::ifd0>     },
        {    0x0117, Group::ifd0,      newTiffImageSize<0x0111, Group::ifd0>     },
        {    0x0144, Group::ifd0,      newTiffImageData<0x0145, Group::ifd0>     },
        {    0x0145, Group::ifd0,      newTiffImageSize<0x0144, Group::ifd0>     },
        {    0x0201, Group::ifd0,      newTiffImageData<0x0202, Group::ifd0>     },
        {    0x0202, Group::ifd0,      newTiffImageSize<0x0201, Group::ifd0>     },
        {    0x014a, Group::ifd0,      newTiffSubIfd<Group::subimg1>             },
        { Tag::next, Group::ifd0,      newTiffDirectory<Group::ifd1>             },
        {  Tag::all, Group::ifd0,      newTiffEntry                              },

        // Subdir subimg1
        {    0x0111, Group::subimg1,   newTiffImageData<0x0117, Group::subimg1>  },
        {    0x0117, Group::subimg1,   newTiffImageSize<0x0111, Group::subimg1>  },
        {    0x0144, Group::subimg1,   newTiffImageData<0x0145, Group::subimg1>  },
        {    0x0145, Group::subimg1,   newTiffImageSize<0x0144, Group::subimg1>  },
        {    0x0201, Group::subimg1,   newTiffImageData<0x0202, Group::subimg1>  },
        {    0x0202, Group::subimg1,   newTiffImageSize<0x0201, Group::subimg1>  },
        { Tag::next, Group::subimg1,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::subimg1,   newTiffEntry                              },

        // Subdir subimg2
        {    0x0111, Group::subimg2,   newTiffImageData<0x0117, Group::subimg2>  },
        {    0x0117, Group::subimg2,   newTiffImageSize<0x0111, Group::subimg2>  },
        {    0x0144, Group::subimg2,   newTiffImageData<0x0145, Group::subimg2>  },
        {    0x0145, Group::subimg2,   newTiffImageSize<0x0144, Group::subimg2>  },
        {    0x0201, Group::subimg2,   newTiffImageData<0x0202, Group::subimg2>  },
        {    0x0202, Group::subimg2,   newTiffImageSize<0x0201, Group::subimg2>  },
        { Tag::next, Group::subimg2,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::subimg2,   newTiffEntry                              },

        // Subdir subimg3
        {    0x0111, Group::subimg3,   newTiffImageData<0x0117, Group::subimg3>  },
        {    0x0117, Group::subimg3,   newTiffImageSize<0x0111, Group::subimg3>  },
        {    0x0144, Group::subimg3,   newTiffImageData<0x0145, Group::subimg3>  },
        {    0x0145, Group::subimg3,   newTiffImageSize<0x0144, Group::subimg3>  },
        {    0x0201, Group::subimg3,   newTiffImageData<0x0202, Group::subimg3>  },
        {    0x0202, Group::subimg3,   newTiffImageSize<0x0201, Group::subimg3>  },
        { Tag::next, Group::subimg3,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::subimg3,   newTiffEntry                              },

        // Subdir subimg4
        {    0x0111, Group::subimg4,   newTiffImageData<0x0117, Group::subimg4>  },
        {    0x0117, Group::subimg4,   newTiffImageSize<0x0111, Group::subimg4>  },
        {    0x0144, Group::subimg4,   newTiffImageData<0x0145, Group::subimg4>  },
        {    0x0145, Group::subimg4,   newTiffImageSize<0x0144, Group::subimg4>  },
        {    0x0201, Group::subimg4,   newTiffImageData<0x0202, Group::subimg4>  },
        {    0x0202, Group::subimg4,   newTiffImageSize<0x0201, Group::subimg4>  },
        { Tag::next, Group::subimg4,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::subimg4,   newTiffEntry                              },

        // Exif subdir
        {    0xa005, Group::exif,      newTiffSubIfd<Group::iop>                 },
        {    0x927c, Group::exif,      newTiffMnEntry                            },
        { Tag::next, Group::exif,      newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::exif,      newTiffEntry                              },

        // GPS subdir
        { Tag::next, Group::gps,       newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::gps,       newTiffEntry                              },

        // IOP subdir
        { Tag::next, Group::iop,       newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::iop,       newTiffEntry                              },

        // IFD1
        {    0x0111, Group::ifd1,      newTiffThumbData<0x0117, Group::ifd1>     },
        {    0x0117, Group::ifd1,      newTiffThumbSize<0x0111, Group::ifd1>     },
        {    0x0201, Group::ifd1,      newTiffThumbData<0x0202, Group::ifd1>     },
        {    0x0202, Group::ifd1,      newTiffThumbSize<0x0201, Group::ifd1>     },
        { Tag::next, Group::ifd1,      newTiffDirectory<Group::ifd2>             },
        {  Tag::all, Group::ifd1,      newTiffEntry                              },

        // IFD2 (eg, in Pentax PEF files)
        {    0x0111, Group::ifd2,      newTiffImageData<0x0117, Group::ifd2>     },
        {    0x0117, Group::ifd2,      newTiffImageSize<0x0111, Group::ifd2>     },
        {    0x0201, Group::ifd2,      newTiffImageData<0x0202, Group::ifd2>     },
        {    0x0202, Group::ifd2,      newTiffImageSize<0x0201, Group::ifd2>     },
        { Tag::next, Group::ifd2,      newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::ifd2,      newTiffEntry                              },

        // Olympus makernote - some Olympus cameras use Minolta structures
        // Todo: Adding such tags will not work (maybe result in a Minolta makernote), need separate groups
        {    0x0001, Group::olymp1mn,  EXV_SIMPLE_BINARY_ARRAY(minoCsoCfg)       },
        {    0x0003, Group::olymp1mn,  EXV_SIMPLE_BINARY_ARRAY(minoCsnCfg)       },
        { Tag::next, Group::olymp1mn,  newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::olymp1mn,  newTiffEntry                              },

        // Olympus2 makernote
        {    0x0001, Group::olymp2mn,  EXV_SIMPLE_BINARY_ARRAY(minoCsoCfg)       },
        {    0x0003, Group::olymp2mn,  EXV_SIMPLE_BINARY_ARRAY(minoCsnCfg)       },
        {    0x2010, Group::olymp2mn,  newTiffSubIfd<Group::olympeq>             },
        {    0x2020, Group::olymp2mn,  newTiffSubIfd<Group::olympcs>             },
        {    0x2030, Group::olymp2mn,  newTiffSubIfd<Group::olymprd>             },
        {    0x2031, Group::olymp2mn,  newTiffSubIfd<Group::olymprd2>            },
        {    0x2040, Group::olymp2mn,  newTiffSubIfd<Group::olympip>             },
        {    0x2050, Group::olymp2mn,  newTiffSubIfd<Group::olympfi>             },
        {    0x2100, Group::olymp2mn,  newTiffSubIfd<Group::olympfe1>            },
        {    0x2200, Group::olymp2mn,  newTiffSubIfd<Group::olympfe2>            },
        {    0x2300, Group::olymp2mn,  newTiffSubIfd<Group::olympfe3>            },
        {    0x2400, Group::olymp2mn,  newTiffSubIfd<Group::olympfe4>            },
        {    0x2500, Group::olymp2mn,  newTiffSubIfd<Group::olympfe5>            },
        {    0x2600, Group::olymp2mn,  newTiffSubIfd<Group::olympfe6>            },
        {    0x2700, Group::olymp2mn,  newTiffSubIfd<Group::olympfe7>            },
        {    0x2800, Group::olymp2mn,  newTiffSubIfd<Group::olympfe8>            },
        {    0x2900, Group::olymp2mn,  newTiffSubIfd<Group::olympfe9>            },
        {    0x3000, Group::olymp2mn,  newTiffSubIfd<Group::olympri>             },
        { Tag::next, Group::olymp2mn,  newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::olymp2mn,  newTiffEntry                              },

        // Olympus2 equipment subdir
        {  Tag::all, Group::olympeq,   newTiffEntry                              },

        // Olympus2 camera settings subdir
        {    0x0101, Group::olympcs,   newTiffImageData<0x0102, Group::olympcs>  },
        {    0x0102, Group::olympcs,   newTiffImageSize<0x0101, Group::olympcs>  },
        {  Tag::all, Group::olympcs,   newTiffEntry                              },

        // Olympus2 raw development subdir
        {  Tag::all, Group::olymprd,   newTiffEntry                              },

        // Olympus2 raw development 2 subdir
        {  Tag::all, Group::olymprd2,  newTiffEntry                              },

        // Olympus2 image processing subdir
        {  Tag::all, Group::olympip,   newTiffEntry                              },

        // Olympus2 focus info subdir
        {  Tag::all, Group::olympfi,   newTiffEntry                              },

        // Olympus2 FE 1 subdir
        {  Tag::all, Group::olympfe1,  newTiffEntry                              },

        // Olympus2 FE 2 subdir
        {  Tag::all, Group::olympfe2,  newTiffEntry                              },

        // Olympus2 FE 3 subdir
        {  Tag::all, Group::olympfe3,  newTiffEntry                              },

        // Olympus2 FE 4 subdir
        {  Tag::all, Group::olympfe4,  newTiffEntry                              },

        // Olympus2 FE 5 subdir
        {  Tag::all, Group::olympfe5,  newTiffEntry                              },

        // Olympus2 FE 6 subdir
        {  Tag::all, Group::olympfe6,  newTiffEntry                              },

        // Olympus2 FE 7 subdir
        {  Tag::all, Group::olympfe7,  newTiffEntry                              },

        // Olympus2 FE 8 subdir
        {  Tag::all, Group::olympfe8,  newTiffEntry                              },

        // Olympus2 FE 9 subdir
        {  Tag::all, Group::olympfe9,  newTiffEntry                              },

        // Olympus2 Raw Info subdir
        {  Tag::all, Group::olympri,   newTiffEntry                              },

        // Fujifilm makernote
        { Tag::next, Group::fujimn,    newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::fujimn,    newTiffEntry                              },

        // Canon makernote
        {    0x0001, Group::canonmn,   EXV_BINARY_ARRAY(canonCsCfg, canonCsDef)  },
        {    0x0004, Group::canonmn,   EXV_SIMPLE_BINARY_ARRAY(canonSiCfg)       },
        {    0x0005, Group::canonmn,   EXV_SIMPLE_BINARY_ARRAY(canonPaCfg)       },
        {    0x000f, Group::canonmn,   EXV_SIMPLE_BINARY_ARRAY(canonCfCfg)       },
        {    0x0012, Group::canonmn,   EXV_SIMPLE_BINARY_ARRAY(canonPiCfg)       },
        {    0x0093, Group::canonmn,   EXV_BINARY_ARRAY(canonFiCfg, canonFiDef)  },
        { Tag::next, Group::canonmn,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::canonmn,   newTiffEntry                              },

        // Canon makernote composite tags
        {  Tag::all, Group::canoncs,   newTiffBinaryElement                      },
        {  Tag::all, Group::canonsi,   newTiffBinaryElement                      },
        {  Tag::all, Group::canonpa,   newTiffBinaryElement                      },
        {  Tag::all, Group::canoncf,   newTiffBinaryElement                      },
        {  Tag::all, Group::canonpi,   newTiffBinaryElement                      },
        {  Tag::all, Group::canonfi,   newTiffBinaryElement                      },

        // Nikon1 makernote
        { Tag::next, Group::nikon1mn,  newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::nikon1mn,  newTiffEntry                              },

        // Nikon2 makernote
        { Tag::next, Group::nikon2mn,  newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::nikon2mn,  newTiffEntry                              },

        // Nikon3 makernote
        { Tag::next, Group::nikon3mn,  newTiffDirectory<Group::ignr>             },
        {    0x0011, Group::nikon3mn,  newTiffSubIfd<Group::nikonpv>             },
        {    0x001f, Group::nikon3mn,  EXV_BINARY_ARRAY(nikonVrCfg, nikonVrDef)  },
        {    0x0023, Group::nikon3mn,  EXV_BINARY_ARRAY(nikonPcCfg, nikonPcDef)  },
        {    0x0024, Group::nikon3mn,  EXV_BINARY_ARRAY(nikonWtCfg, nikonWtDef)  },
        {    0x0025, Group::nikon3mn,  EXV_BINARY_ARRAY(nikonIiCfg, nikonIiDef)  },
        {    0x0088, Group::nikon3mn,  EXV_BINARY_ARRAY(nikonAfCfg, nikonAfDef)  },
        {    0x0091, Group::nikon3mn,  EXV_COMPLEX_BINARY_ARRAY(nikonSiSet, nikonSelector) },
        {    0x0097, Group::nikon3mn,  EXV_COMPLEX_BINARY_ARRAY(nikonCbSet, nikonSelector) },
        {    0x0098, Group::nikon3mn,  EXV_COMPLEX_BINARY_ARRAY(nikonLdSet, nikonSelector) },
        {  Tag::all, Group::nikon3mn,  newTiffEntry                              },

        // Nikon3 makernote preview subdir
        {    0x0201, Group::nikonpv,   newTiffThumbData<0x0202, Group::nikonpv>  },
        {    0x0202, Group::nikonpv,   newTiffThumbSize<0x0201, Group::nikonpv>  },
        { Tag::next, Group::nikonpv,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::nikonpv,   newTiffEntry                              },

        // Nikon3 vibration reduction
        {  Tag::all, Group::nikonvr,   newTiffBinaryElement                      },

        // Nikon3 picture control
        {  Tag::all, Group::nikonpc,   newTiffBinaryElement                      },

        // Nikon3 world time
        {  Tag::all, Group::nikonwt,   newTiffBinaryElement                      },

        // Nikon3 ISO info
        {  Tag::all, Group::nikonii,   newTiffBinaryElement                      },

        // Nikon3 auto focus
        {  Tag::all, Group::nikonaf,   newTiffBinaryElement                      },

        // Nikon3 shot info
        {  Tag::all, Group::nikonsi1,  newTiffBinaryElement                      },
        {  Tag::all, Group::nikonsi2,  newTiffBinaryElement                      },
        {  Tag::all, Group::nikonsi3,  newTiffBinaryElement                      },
        {  Tag::all, Group::nikonsi4,  newTiffBinaryElement                      },
        {  Tag::all, Group::nikonsi5,  newTiffBinaryElement                      },
        {  Tag::all, Group::nikonsi6,  newTiffBinaryElement                      },

        // Nikon3 color balance
        {  Tag::all, Group::nikoncb1,  newTiffBinaryElement                      },
        {  Tag::all, Group::nikoncb2,  newTiffBinaryElement                      },
        {  Tag::all, Group::nikoncb2a, newTiffBinaryElement                      },
        {  Tag::all, Group::nikoncb2b, newTiffBinaryElement                      },
        {  Tag::all, Group::nikoncb3,  newTiffBinaryElement                      },
        {  Tag::all, Group::nikoncb4,  newTiffBinaryElement                      },

        // Nikon3 lens data
        {  Tag::all, Group::nikonld1,  newTiffBinaryElement                      },
        {  Tag::all, Group::nikonld2,  newTiffBinaryElement                      },
        {  Tag::all, Group::nikonld3,  newTiffBinaryElement                      },

        // Panasonic makernote
        { Tag::next, Group::panamn,    newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::panamn,    newTiffEntry                              },

        // Pentax makernote
        {    0x0003, Group::pentaxmn,  newTiffThumbSize<0x0004, Group::pentaxmn> },
        {    0x0004, Group::pentaxmn,  newTiffThumbData<0x0003, Group::pentaxmn> },
        { Tag::next, Group::pentaxmn,  newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::pentaxmn,  newTiffEntry                              },

        // Sigma/Foveon makernote
        { Tag::next, Group::sigmamn,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::sigmamn,   newTiffEntry                              },

        // Sony1 makernote
        { Tag::next, Group::sony1mn,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::sony1mn,   newTiffEntry                              },

        // Sony2 makernote
        { Tag::next, Group::sony2mn,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::sony2mn,   newTiffEntry                              },

        // Minolta makernote
        {    0x0001, Group::minoltamn, EXV_SIMPLE_BINARY_ARRAY(minoCsoCfg)       },
        {    0x0003, Group::minoltamn, EXV_SIMPLE_BINARY_ARRAY(minoCsnCfg)       },
        {    0x0004, Group::minoltamn, EXV_BINARY_ARRAY(minoCs7Cfg, minoCs7Def)  },
        {    0x0088, Group::minoltamn, newTiffThumbData<0x0089, Group::minoltamn>},
        {    0x0089, Group::minoltamn, newTiffThumbSize<0x0088, Group::minoltamn>},
        {    0x0114, Group::minoltamn, EXV_BINARY_ARRAY(minoCs5Cfg, minoCs5Def)  },
        { Tag::next, Group::minoltamn, newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::minoltamn, newTiffEntry                              },

        // Minolta makernote composite tags
        {  Tag::all, Group::minocso,   newTiffBinaryElement                      },
        {  Tag::all, Group::minocsn,   newTiffBinaryElement                      },
        {  Tag::all, Group::minocs7,   newTiffBinaryElement                      },
        {  Tag::all, Group::minocs5,   newTiffBinaryElement                      },

        // -----------------------------------------------------------------------
        // Root directory of Panasonic RAW images
        { Tag::pana, Group::none,      newTiffDirectory<Group::panaraw>          },

        // IFD0 of Panasonic RAW images
        {    0x8769, Group::panaraw,   newTiffSubIfd<Group::exif>                },
        {    0x8825, Group::panaraw,   newTiffSubIfd<Group::gps>                 },
//        {    0x0111, Group::panaraw,   newTiffImageData<0x0117, Group::panaraw>  },
//        {    0x0117, Group::panaraw,   newTiffImageSize<0x0111, Group::panaraw>  },
        { Tag::next, Group::panaraw,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::panaraw,   newTiffEntry                              },

        // -----------------------------------------------------------------------
        // Tags which are not de/encoded
        { Tag::next, Group::ignr,      newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::ignr,      newTiffEntry                              }
    };

    // TIFF mapping table for special decoding and encoding requirements
    const TiffMappingInfo TiffMapping::tiffMappingInfo_[] = {
        { "*",       Tag::all, Group::ignr,    0, 0 }, // Do not decode tags with group == Group::ignr
        { "*",         0x02bc, Group::ifd0,    &TiffDecoder::decodeXmp,          0 /*done before the tree is traversed*/ },
        { "*",         0x83bb, Group::ifd0,    &TiffDecoder::decodeIptc,         0 /*done before the tree is traversed*/ },
        { "*",         0x8649, Group::ifd0,    &TiffDecoder::decodeIptc,         0 /*done before the tree is traversed*/ }
    };

    DecoderFct TiffMapping::findDecoder(const std::string& make,
                                              uint32_t     extendedTag,
                                              uint16_t     group)
    {
        DecoderFct decoderFct = &TiffDecoder::decodeStdTiffEntry;
        const TiffMappingInfo* td = find(tiffMappingInfo_,
                                         TiffMappingInfo::Key(make, extendedTag, group));
        if (td) {
            // This may set decoderFct to 0, meaning that the tag should not be decoded
            decoderFct = td->decoderFct_;
        }
        return decoderFct;
    }

    EncoderFct TiffMapping::findEncoder(
        const std::string& make,
              uint32_t     extendedTag,
              uint16_t     group
    )
    {
        EncoderFct encoderFct = 0;
        const TiffMappingInfo* td = find(tiffMappingInfo_,
                                         TiffMappingInfo::Key(make, extendedTag, group));
        if (td) {
            // Returns 0 if no special encoder function is found
            encoderFct = td->encoderFct_;
        }
        return encoderFct;
    }

    bool TiffGroupStruct::operator==(const TiffGroupStruct::Key& key) const
    {
        return    (Tag::all == extendedTag_ || key.e_ == extendedTag_)
               && key.g_ == group_;
    }

    bool TiffTreeStruct::operator==(const TiffTreeStruct::Key& key) const
    {
        return key.r_ == root_ && key.g_ == group_;
    }

    TiffComponent::AutoPtr TiffCreator::create(uint32_t extendedTag,
                                               uint16_t group)
    {
        TiffComponent::AutoPtr tc(0);
        uint16_t tag = static_cast<uint16_t>(extendedTag & 0xffff);
        const TiffGroupStruct* ts = find(tiffGroupStruct_,
                                         TiffGroupStruct::Key(extendedTag, group));
        if (ts && ts->newTiffCompFct_) {
            tc = ts->newTiffCompFct_(tag, group);
        }
#ifdef DEBUG
        else {
            if (!ts) {
                std::cerr << "Warning: No TIFF structure entry found for ";
            }
            else {
                std::cerr << "Warning: No TIFF component creator found for ";
            }
            std::cerr << "extended tag 0x" << std::setw(4) << std::setfill('0')
                      << std::hex << std::right << extendedTag
                      << ", group " << tiffGroupName(group) << "\n";
        }
#endif
        return tc;
    } // TiffCreator::create

    void TiffCreator::getPath(TiffPath& tiffPath,
                              uint32_t  extendedTag,
                              uint16_t  group,
                              uint32_t  root)
    {
        const TiffTreeStruct* ts = 0;
        do {
            tiffPath.push(TiffPathItem(extendedTag, group));
            ts = find(tiffTreeStruct_, TiffTreeStruct::Key(root, group));
            assert(ts != 0);
            extendedTag = ts->parentExtTag_;
            group = ts->parentGroup_;
        } while (!(ts->root_ == root && ts->group_ == Group::none));

    } // TiffCreator::getPath

    ByteOrder TiffParserWorker::decode(
              ExifData&          exifData,
              IptcData&          iptcData,
              XmpData&           xmpData,
        const byte*              pData,
              uint32_t           size,
              uint32_t           root,
              FindDecoderFct     findDecoderFct,
              TiffHeaderBase*    pHeader
    )
    {
        // Create standard TIFF header if necessary
        std::auto_ptr<TiffHeaderBase> ph;
        if (!pHeader) {
            ph = std::auto_ptr<TiffHeaderBase>(new TiffHeader);
            pHeader = ph.get();
        }
        TiffComponent::AutoPtr rootDir = parse(pData, size, root, pHeader);
        if (0 != rootDir.get()) {
            TiffDecoder decoder(exifData,
                                iptcData,
                                xmpData,
                                rootDir.get(),
                                findDecoderFct);
            rootDir->accept(decoder);
        }
        return pHeader->byteOrder();

    } // TiffParserWorker::decode

    WriteMethod TiffParserWorker::encode(
              BasicIo&           io,
        const byte*              pData,
              uint32_t           size,
        const ExifData&          exifData,
        const IptcData&          iptcData,
        const XmpData&           xmpData,
              uint32_t           root,
              FindEncoderFct     findEncoderFct,
              TiffHeaderBase*    pHeader
    )
    {
        /*
           1) parse the binary image, if one is provided, and
           2) attempt updating the parsed tree in-place ("non-intrusive writing")
           3) else, create a new tree and write a new TIFF structure ("intrusive
              writing"). If there is a parsed tree, it is only used to access the
              image data in this case.
         */
        assert(pHeader);
        assert(pHeader->byteOrder() != invalidByteOrder);
        WriteMethod writeMethod = wmIntrusive;
        TiffComponent::AutoPtr createdTree;
        TiffComponent::AutoPtr parsedTree = parse(pData, size, root, pHeader);
        if (0 != parsedTree.get()) {
            // Attempt to update existing TIFF components based on metadata entries
            TiffEncoder encoder(exifData,
                                iptcData,
                                xmpData,
                                parsedTree.get(),
                                pHeader->byteOrder(),
                                findEncoderFct);
            parsedTree->accept(encoder);
            if (!encoder.dirty()) writeMethod = wmNonIntrusive;
        }
        if (writeMethod == wmIntrusive) {
            createdTree = TiffCreator::create(root, Group::none);
            TiffEncoder encoder(exifData,
                                iptcData,
                                xmpData,
                                createdTree.get(),
                                pHeader->byteOrder(),
                                findEncoderFct);
            // Add entries from metadata to composite
            encoder.add(createdTree.get(), parsedTree.get(), root);
            // Write binary representation from the composite tree
            DataBuf header = pHeader->write();
            BasicIo::AutoPtr tempIo(io.temporary()); // may throw
            assert(tempIo.get() != 0);
            IoWrapper ioWrapper(*tempIo, header.pData_, header.size_);
            uint32_t imageIdx(uint32_t(-1));
            createdTree->write(ioWrapper,
                               pHeader->byteOrder(),
                               header.size_,
                               uint32_t(-1),
                               uint32_t(-1),
                               imageIdx);
            io.transfer(*tempIo); // may throw
#ifdef DEBUG
            std::cerr << "Intrusive writing\n";
#endif
        }
#ifdef DEBUG
        else {
            std::cerr << "Non-intrusive writing\n";
        }
#endif
        return writeMethod;
    } // TiffParserWorker::encode

    TiffComponent::AutoPtr TiffParserWorker::parse(
        const byte*              pData,
              uint32_t           size,
              uint32_t           root,
              TiffHeaderBase*    pHeader
    )
    {
        if (pData == 0 || size == 0) return TiffComponent::AutoPtr(0);
        if (!pHeader->read(pData, size) || pHeader->offset() >= size) {
            throw Error(3, "TIFF");
        }
        TiffComponent::AutoPtr rootDir = TiffCreator::create(root, Group::none);
        if (0 != rootDir.get()) {
            rootDir->setStart(pData + pHeader->offset());
            TiffRwState::AutoPtr state(
                new TiffRwState(pHeader->byteOrder(), 0));
            TiffReader reader(pData, size, rootDir.get(), state);
            rootDir->accept(reader);
            reader.postProcess();
        }
        return rootDir;

    } // TiffParserWorker::parse

    TiffHeaderBase::TiffHeaderBase(uint16_t  tag,
                                   uint32_t  size,
                                   ByteOrder byteOrder,
                                   uint32_t  offset)
        : tag_(tag),
          size_(size),
          byteOrder_(byteOrder),
          offset_(offset)
    {
    }

    TiffHeaderBase::~TiffHeaderBase()
    {
    }

    bool TiffHeaderBase::read(const byte* pData, uint32_t size)
    {
        if (!pData || size < 8) return false;

        if (pData[0] == 0x49 && pData[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (pData[0] == 0x4d && pData[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            return false;
        }
        if (tag_ != getUShort(pData + 2, byteOrder_)) return false;
        offset_ = getULong(pData + 4, byteOrder_);

        return true;
    } // TiffHeaderBase::read

    DataBuf TiffHeaderBase::write() const
    {
        DataBuf buf(8);
        switch (byteOrder_) {
        case littleEndian:
            buf.pData_[0] = 0x49;
            buf.pData_[1] = 0x49;
            break;
        case bigEndian:
            buf.pData_[0] = 0x4d;
            buf.pData_[1] = 0x4d;
            break;
        case invalidByteOrder:
            assert(false);
            break;
        }
        us2Data(buf.pData_ + 2, tag_, byteOrder_);
        ul2Data(buf.pData_ + 4, 0x00000008, byteOrder_);
        return buf;
    }

    void TiffHeaderBase::print(std::ostream& os, const std::string& prefix) const
    {
        os << prefix
           << _("TIFF header, offset") << " = 0x"
           << std::setw(8) << std::setfill('0') << std::hex << std::right
           << offset_;

        switch (byteOrder_) {
        case littleEndian:     os << ", " << _("little endian encoded"); break;
        case bigEndian:        os << ", " << _("big endian encoded");    break;
        case invalidByteOrder: break;
        }
        os << "\n";
    } // TiffHeaderBase::print

    ByteOrder TiffHeaderBase::byteOrder() const
    {
        return byteOrder_;
    }

    void TiffHeaderBase::setByteOrder(ByteOrder byteOrder)
    {
        byteOrder_ = byteOrder;
    }

    uint32_t TiffHeaderBase::offset() const
    {
        return offset_;
    }

    void TiffHeaderBase::setOffset(uint32_t offset)
    {
        offset_ = offset;
    }

    uint32_t TiffHeaderBase::size() const
    {
        return size_;
    }

    uint16_t TiffHeaderBase::tag() const
    {
        return tag_;
    }

    TiffHeader::TiffHeader(ByteOrder byteOrder, uint32_t offset)
        : TiffHeaderBase(42, 8, byteOrder, offset)
    {
    }

    TiffHeader::~TiffHeader()
    {
    }

}}                                       // namespace Internal, Exiv2
