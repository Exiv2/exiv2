// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FUJIMN_INT_HPP_
#define FUJIMN_INT_HPP_

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
struct TagInfo;

namespace Internal {
// *****************************************************************************
// class definitions

//! MakerNote for Fujifilm cameras
class FujiMakerNote {
 public:
  //! Return read-only list of built-in Fujifilm tags
  static constexpr auto tagList() {
    return tagInfo_;
  }

 private:
  //! Tag information
  static const TagInfo tagInfo_[];
};  // class FujiMakerNote

}  // namespace Internal
}  // namespace Exiv2

#endif  // #ifndef FUJIMN_INT_HPP_
