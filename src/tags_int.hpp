// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2010 Andreas Huggel <ahuggel@gmx.net>
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
/*!
  @file    tags_int.hpp
  @brief   Internal Exif tag and type information
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Jan-04, ahu: created<BR>
           11-Feb-04, ahu: isolated as a component
 */
#ifndef TAGS_INT_HPP_
#define TAGS_INT_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "value.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    class ExifData;

    namespace Internal {

// *****************************************************************************
// class definitions

    /*!
      @brief Helper structure for lookup tables for translations of numeric
             tag values to human readable labels.
     */
    struct TagDetails {
        long val_;                              //!< Tag value
        const char* label_;                     //!< Translation of the tag value

        //! Comparison operator for use with the find template
        bool operator==(long key) const { return val_ == key; }
    }; // struct TagDetails

    /*!
      @brief Helper structure for lookup tables for translations of bitmask
             values to human readable labels.
     */
    struct TagDetailsBitmask {
        uint32_t mask_;                         //!< Bitmask value
        const char* label_;                     //!< Description of the tag value
    }; // struct TagDetailsBitmask

    /*!
      @brief Helper structure for lookup tables for translations of controlled
             vocabulary strings to their descriptions.
     */
    struct TagVocabulary {
        const char* voc_;                       //!< Vocabulary string
        const char* label_;                     //!< Description of the vocabulary string

        /*!
          @brief Comparison operator for use with the find template

          Compare vocabulary strings like "PR-NON" with keys like
          "http://ns.useplus.org/ldf/vocab/PR-NON" and return true if the vocabulary
          string matches the end of the key.
         */
        bool operator==(const std::string& key) const;
    }; // struct TagDetails

    /*!
      @brief Generic pretty-print function to translate a long value to a description
             by looking up a reference table.
     */
    template <int N, const TagDetails (&array)[N]>
    std::ostream& printTag(std::ostream& os, const Value& value, const ExifData*)
    {
        const TagDetails* td = find(array, value.toLong());
        if (td) {
            os << exvGettext(td->label_);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

//! Shortcut for the printTag template which requires typing the array name only once.
#define EXV_PRINT_TAG(array) printTag<EXV_COUNTOF(array), array>

    /*!
      @brief Generic print function to translate a long value to a description
             by looking up bitmasks in a reference table.
     */
    template <int N, const TagDetailsBitmask (&array)[N]>
    std::ostream& printTagBitmask(std::ostream& os, const Value& value, const ExifData*)
    {
        const uint32_t val = static_cast<uint32_t>(value.toLong());
        if (val == 0 && N > 0) {
            const TagDetailsBitmask* td = *(&array);
            if (td->mask_ == 0) return os << exvGettext(td->label_);
        }
        bool sep = false;
        for (int i = 0; i < N; ++i) {
            // *& acrobatics is a workaround for a MSVC 7.1 bug
            const TagDetailsBitmask* td = *(&array) + i;

            if (val & td->mask_) {
                if (sep) {
                    os << ", " << exvGettext(td->label_);
                }
                else {
                    os << exvGettext(td->label_);
                    sep = true;
                }
            }
        }
        return os;
    }

//! Shortcut for the printTagBitmask template which requires typing the array name only once.
#define EXV_PRINT_TAG_BITMASK(array) printTagBitmask<EXV_COUNTOF(array), array>

    /*!
      @brief Generic pretty-print function to translate a controlled vocabulary value (string)
             to a description by looking up a reference table.
     */
    template <int N, const TagVocabulary (&array)[N]>
    std::ostream& printTagVocabulary(std::ostream& os, const Value& value, const ExifData*)
    {
        const TagVocabulary* td = find(array, value.toString());
        if (td) {
            os << exvGettext(td->label_);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

//! Shortcut for the printTagVocabulary template which requires typing the array name only once.
#define EXV_PRINT_VOCABULARY(array) printTagVocabulary<EXV_COUNTOF(array), array>

// *****************************************************************************
// free functions

    //! @name Functions printing interpreted tag values
    //@{
    //! Default print function, using the Value output operator
    std::ostream& printValue(std::ostream& os, const Value& value, const ExifData*);
    //! Print the value converted to a long
    std::ostream& printLong(std::ostream& os, const Value& value, const ExifData*);
    //! Print a Rational or URational value in floating point format
    std::ostream& printFloat(std::ostream& os, const Value& value, const ExifData*);
    //! Print a longitude or latitude value
    std::ostream& printDegrees(std::ostream& os, const Value& value, const ExifData*);
    //! Print function converting from UCS-2LE to UTF-8
    std::ostream& printUcs2(std::ostream& os, const Value& value, const ExifData*);
    //! Print function for Exif units
    std::ostream& printExifUnit(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS version
    std::ostream& print0x0000(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS altitude ref
    std::ostream& print0x0005(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS altitude
    std::ostream& print0x0006(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS timestamp
    std::ostream& print0x0007(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS status
    std::ostream& print0x0009(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS measurement mode
    std::ostream& print0x000a(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS speed ref
    std::ostream& print0x000c(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS destination distance ref
    std::ostream& print0x0019(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS differential correction
    std::ostream& print0x001e(std::ostream& os, const Value& value, const ExifData*);
    //! Print orientation
    std::ostream& print0x0112(std::ostream& os, const Value& value, const ExifData*);
    //! Print YCbCrPositioning
    std::ostream& print0x0213(std::ostream& os, const Value& value, const ExifData*);
    //! Print the copyright
    std::ostream& print0x8298(std::ostream& os, const Value& value, const ExifData*);
    //! Print the exposure time
    std::ostream& print0x829a(std::ostream& os, const Value& value, const ExifData*);
    //! Print the f-number
    std::ostream& print0x829d(std::ostream& os, const Value& value, const ExifData*);
    //! Print exposure program
    std::ostream& print0x8822(std::ostream& os, const Value& value, const ExifData*);
    //! Print ISO speed ratings
    std::ostream& print0x8827(std::ostream& os, const Value& value, const ExifData*);
    //! Print components configuration specific to compressed data
    std::ostream& print0x9101(std::ostream& os, const Value& value, const ExifData*);
    //! Print exposure time converted from APEX shutter speed value
    std::ostream& print0x9201(std::ostream& os, const Value& value, const ExifData*);
    //! Print f-number converted from APEX aperture value
    std::ostream& print0x9202(std::ostream& os, const Value& value, const ExifData*);
    //! Print the exposure bias value
    std::ostream& print0x9204(std::ostream& os, const Value& value, const ExifData*);
    //! Print the subject distance
    std::ostream& print0x9206(std::ostream& os, const Value& value, const ExifData*);
    //! Print metering mode
    std::ostream& print0x9207(std::ostream& os, const Value& value, const ExifData*);
    //! Print light source
    std::ostream& print0x9208(std::ostream& os, const Value& value, const ExifData*);
    //! Print the actual focal length of the lens
    std::ostream& print0x920a(std::ostream& os, const Value& value, const ExifData*);
    //! Print the user comment
    std::ostream& print0x9286(std::ostream& os, const Value& value, const ExifData*);
    //! Print color space
    std::ostream& print0xa001(std::ostream& os, const Value& value, const ExifData*);
    //! Print sensing method
    std::ostream& print0xa217(std::ostream& os, const Value& value, const ExifData*);
    //! Print file source
    std::ostream& print0xa300(std::ostream& os, const Value& value, const ExifData*);
    //! Print scene type
    std::ostream& print0xa301(std::ostream& os, const Value& value, const ExifData*);
    //! Print custom rendered
    std::ostream& print0xa401(std::ostream& os, const Value& value, const ExifData*);
    //! Print exposure mode
    std::ostream& print0xa402(std::ostream& os, const Value& value, const ExifData*);
    //! Print white balance
    std::ostream& print0xa403(std::ostream& os, const Value& value, const ExifData*);
    //! Print digital zoom ratio
    std::ostream& print0xa404(std::ostream& os, const Value& value, const ExifData*);
    //! Print 35mm equivalent focal length
    std::ostream& print0xa405(std::ostream& os, const Value& value, const ExifData*);
    //! Print scene capture type
    std::ostream& print0xa406(std::ostream& os, const Value& value, const ExifData*);
    //! Print gain control
    std::ostream& print0xa407(std::ostream& os, const Value& value, const ExifData*);
    //! Print saturation
    std::ostream& print0xa409(std::ostream& os, const Value& value, const ExifData*);
    //! Print subject distance range
    std::ostream& print0xa40c(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS direction ref
    std::ostream& printGPSDirRef(std::ostream& os, const Value& value, const ExifData*);
    //! Print contrast, sharpness (normal, soft, hard)
    std::ostream& printNormalSoftHard(std::ostream& os, const Value& value, const ExifData*);
    //! Print any version packed in 4 Bytes format : major major minor minor
    std::ostream& printExifVersion(std::ostream& os, const Value& value, const ExifData*);
    //! Print any version encoded in the ASCII string majormajorminorminor
    std::ostream& printXmpVersion(std::ostream& os, const Value& value, const ExifData*);
    //! Print a date following the format YYYY-MM-DDTHH:MM:SSZ
    std::ostream& printXmpDate(std::ostream& os, const Value& value, const ExifData*);
    //@}

    //! Calculate F number from an APEX aperture value
    float fnumber(float apertureValue);

    //! Calculate the exposure time from an APEX shutter speed value
    URational exposureTime(float shutterSpeedValue);

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef TAGS_INT_HPP_
