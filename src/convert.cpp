// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
  File:      convert.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   17-Mar-07, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "exif.hpp"
#include "iptc.hpp"
#include "xmp.hpp"
#include "convert.hpp"

// + standard includes

// *****************************************************************************
namespace {

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! Metadata conversions.
    class Converter {
    public:
        /*!
          @brief Type for metadata converter functions, taking two key strings,
                 \em from and \em to.

          These functions have access to both the source and destination metadata
          containers and store the result directly in the destination container.
         */
        typedef void (Converter::*ConvertFct)(const char* from, const char* to);
        //! Structure to define conversions between two keys.
        struct Conversion {
            MetadataId  metadataId; //!< Type of metadata for the first key.
            const char* key1;       //!< First metadata key.
            const char* key2;       //!< Second metadata key (always an XMP key for now).
            ConvertFct  key1ToKey2; //!< Conversion from first to second key.
            ConvertFct  key2ToKey1; //!< Conversion from second to first key.
        };
    public:
        //! @name Creators
        //@{
        //! Constructor for Exif tags and XMP properties.
        Converter(ExifData& exifData, XmpData& xmpData);
        //@}
        //! @name Manipulators
        //@{
        //! Convert Exif tags to XMP properties according to the conversion table.
        void cnvExifToXmp();
        //! Convert XMP properties to Exif tags according to the conversion table.
        void cnvXmpToExif();
        /*!
          @brief Set the erase flag.

          This flag indicates whether successfully converted source records are erased.
         */
        void setErase(bool onoff =true) { erase_ = onoff; }
        //@}
        //! @name Conversion functions (manipulators)
        //@{
        /*!
          @brief Simple Exif to XMP conversion function.

          Sets the XMP property to an XmpText value containing the Exif value string.
         */
        void cnvExifValue(const char* from, const char* to);
        /*!
          @brief Simple XMP to Exif conversion function.

          Sets the Exif tag according to the XMP property.
         */
        void cnvXmpValue(const char* from, const char* to);
        //@}
        //! @name Accessors
        //@{
        //! Get the value of the erase flag, see also setErase(bool on).
        bool erase() const { return erase_; }
        //@}
    private:
        // DATA
        static const Conversion conversion_[];  //<! Conversion rules
        bool erase_;
        ExifData& exifData_;  // use pointers?
        XmpData&  xmpData_;

    }; // class Converter

    const Converter::Conversion Converter::conversion_[] = {
        { mdExif, "Exif.Image.ImageWidth",    "Xmp.tiff.ImageWidth",    &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.ImageLength",   "Xmp.tiff.ImageLength",   &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.BitsPerSample", "Xmp.tiff.BitsPerSample", &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.Compression",   "Xmp.tiff.Compression",   &Converter::cnvExifValue, &Converter::cnvXmpValue }
        // ...
    };

    Converter::Converter(ExifData& exifData, XmpData& xmpData)
        : erase_(false), exifData_(exifData), xmpData_(xmpData)
    {
    }

    void Converter::cnvExifToXmp()
    {
        for (unsigned int i = 0; i < EXV_COUNTOF(conversion_); ++i) {
            const Conversion& c = conversion_[i];
            if (c.metadataId == mdExif) {
                EXV_CALL_MEMBER_FN(*this, c.key1ToKey2)(c.key1, c.key2);
            }
        }
    }

    void Converter::cnvXmpToExif()
    {
        for (unsigned int i = 0; i < EXV_COUNTOF(conversion_); ++i) {
            const Conversion& c = conversion_[i];
            if (c.metadataId == mdExif) {
                EXV_CALL_MEMBER_FN(*this, c.key2ToKey1)(c.key2, c.key1);
            }
        }
    }

    void Converter::cnvExifValue(const char* from, const char* to)
    {
        Exiv2::ExifData::iterator pos = exifData_.findKey(ExifKey(from));
        if (pos == exifData_.end()) return;
        std::string value = pos->value().toString();
        if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
            return;
        }
        xmpData_[to] = value;
        if (erase_) exifData_.erase(pos);
    }

    void Converter::cnvXmpValue(const char* from, const char* to)
    {
        Exiv2::XmpData::iterator pos = xmpData_.findKey(XmpKey(from));
        if (pos == xmpData_.end()) return;
        std::string value = pos->value().toString();
        if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
            return;
        }
        exifData_[to] = value;
        if (erase_) xmpData_.erase(pos);
    }

    // *************************************************************************
    // free functions
    void copyExifToXmp(const ExifData& exifData, XmpData& xmpData)
    {
        Converter converter(const_cast<ExifData&>(exifData), xmpData);
        converter.cnvExifToXmp();
    }

    void moveExifToXmp(ExifData& exifData, XmpData& xmpData)
    {
        Converter converter(const_cast<ExifData&>(exifData), xmpData);
        converter.setErase();
        converter.cnvExifToXmp();
    }

    void copyXmpToExif(const XmpData& xmpData, ExifData& exifData)
    {
        Converter converter(exifData, const_cast<XmpData&>(xmpData));
        converter.cnvXmpToExif();
    }

    void moveXmpToExif(XmpData& xmpData, ExifData& exifData)
    {
        Converter converter(exifData, const_cast<XmpData&>(xmpData));
        converter.setErase();
        converter.cnvXmpToExif();
    }

    void copyIptcToXmp(const IptcData& iptcData, XmpData& xmpData)
    {
        // Todo: implement me!
    }

    void moveIptcToXmp(IptcData& iptcData, XmpData& xmpData)
    {
        // Todo: implement me!
    }

    void copyXmpToIptc(const XmpData& xmpData, IptcData& iptcData)
    {
        // Todo: implement me!
    }

    void moveXmpToIptc(XmpData& xmpData, IptcData& iptcData)
    {
        // Todo: implement me!
    }

}                                       // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {


}
