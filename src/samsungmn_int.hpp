// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SAMSUNGMN_INT_HPP_
#define SAMSUNGMN_INT_HPP_

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
struct TagInfo;

namespace Internal {
// *****************************************************************************
// class definitions

//! MakerNote for Samsung cameras
class Samsung2MakerNote {
 public:
  //! Return read-only list of built-in Samsung tags
  static constexpr auto tagList() {
    return tagInfo_;
  }
  //! Return read-only list of built-in PictureWizard tags
  static constexpr auto tagListPw() {
    return tagInfoPw_;
  }

 private:
  //! Tag information
  static const TagInfo tagInfo_[];
  //! PictureWizard tag information
  static const TagInfo tagInfoPw_[];

};  // class Samsung2MakerNote

}  // namespace Internal
}  // namespace Exiv2

#endif  // #ifndef SAMSUNGMN_INT_HPP_
