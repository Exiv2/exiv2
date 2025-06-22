// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SIGMAMN_INT_HPP_
#define SIGMAMN_INT_HPP_

// *****************************************************************************
// included header files
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
class ExifData;
class Value;
struct TagInfo;

namespace Internal {
// *****************************************************************************
// class definitions

//! MakerNote for Sigma (Foveon) cameras
class SigmaMakerNote {
 public:
  //! Return read-only list of built-in Sigma tags
  static const TagInfo* tagList();

  //! @name Print functions for Sigma (Foveon) %MakerNote tags
  //@{
  //! Strip the label from the value and print the remainder
  static std::ostream& printStripLabel(std::ostream& os, const Value& value, const ExifData*);
  //! Print exposure mode
  static std::ostream& print0x0008(std::ostream& os, const Value& value, const ExifData*);
  //! Print metering mode
  static std::ostream& print0x0009(std::ostream& os, const Value& value, const ExifData*);
  //@}

 private:
  //! Tag information
  static const TagInfo tagInfo_[];

};  // class SigmaMakerNote

}  // namespace Internal
}  // namespace Exiv2

#endif  // #ifndef SIGMAMN_INT_HPP_
