// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef RW2IMAGE_INT_HPP_
#define RW2IMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "tiffimage_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {
// *****************************************************************************
// class definitions

/*!
  @brief Panasonic RW2 header structure.
 */
class Rw2Header : public TiffHeaderBase {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  Rw2Header();
  //@}

  //! @name Accessors
  //@{
  //! Not yet implemented. Does nothing and returns an empty buffer.
  [[nodiscard]] DataBuf write() const override;
  //@}

};  // class Rw2Header

}  // namespace Exiv2::Internal

#endif  // #ifndef RW2IMAGE_INT_HPP_
