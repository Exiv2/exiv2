// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SAMSUNGMN_INT_HPP_
#define SAMSUNGMN_INT_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {
// *****************************************************************************
// class definitions

//! MakerNote for Samsung cameras
class Samsung2MakerNote {
 public:
  //! Return read-only list of built-in Samsung tags
  static const TagInfo* tagList();
  //! Return read-only list of built-in PictureWizard tags
  static const TagInfo* tagListPw();

 private:
  //! Tag information
  static const TagInfo tagInfo_[];
  //! PictureWizard tag information
  static const TagInfo tagInfoPw_[];

};  // class Samsung2MakerNote

}  // namespace Exiv2::Internal

#endif  // #ifndef SAMSUNGMN_INT_HPP_
