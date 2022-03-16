// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SIGMAMN_INT_HPP_
#define SIGMAMN_INT_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {
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

}  // namespace Exiv2::Internal

#endif  // #ifndef SIGMAMN_INT_HPP_
