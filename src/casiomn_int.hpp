// SPDX-License-Identifier: GPL-2.0-or-later

/*!
  @file    casiomn_int.hpp
  @brief   Casio MakerNote implemented using the following references:
           <a href="http://gvsoft.homedns.org/exif/makernote-casio-type1.html">Casio MakerNote Information</a> by
  GVsoft, Casio.pm of <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/">ExifTool</a> by Phil Harvey, <a
  href="http://www.ozhiker.com/electronics/pjmt/jpeg_info/casio_mn.html#Casio_Type_1_Tags">Casio Makernote Format
  Specification</a> by Evan Hunter.
  @date    30-Oct-13, ahu: created
 */
#ifndef EXIV2_CASIOMN_INT_HPP
#define EXIV2_CASIOMN_INT_HPP

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "types.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {
// *****************************************************************************
// class definitions

//! MakerNote for Casio cameras
class CasioMakerNote {
 public:
  //! Return read-only list of built-in Casio tags
  static const TagInfo* tagList();
  //! Print ObjectDistance
  static std::ostream& print0x0006(std::ostream& os, const Value& value, const ExifData*);
  //! Print FirmwareDate
  static std::ostream& print0x0015(std::ostream& os, const Value& value, const ExifData*);

 private:
  //! Makernote tag list
  static const TagInfo tagInfo_[];

};  // class CasioMakerNote

//! MakerNote for Casio2 cameras
class Casio2MakerNote {
 public:
  //! Return read-only list of built-in Casio2 tags
  static const TagInfo* tagList();
  //! Print FirmwareDate
  static std::ostream& print0x2001(std::ostream& os, const Value& value, const ExifData*);
  //! Print ObjectDistance
  static std::ostream& print0x2022(std::ostream& os, const Value& value, const ExifData*);

 private:
  //! Makernote tag list
  static const TagInfo tagInfo_[];

};  // class Casio2MakerNote

}  // namespace Exiv2::Internal

#endif  // EXIV2_CASIOMN_INT_HPP
