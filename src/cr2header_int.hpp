// SPDX-License-Identifier: GPL-2.0-or-later

/*!
  @file    cr2image_int.hpp
  @brief   Internal classes to support CR2 image format
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    23-Apr-08, ahu: created
 */
#ifndef EXIV2_CR2HEADER_INT_HPP
#define EXIV2_CR2HEADER_INT_HPP

// *****************************************************************************
// included header files
#include "tiffimage_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {
// *****************************************************************************
// class definitions

/// @brief Canon CR2 header structure.
class Cr2Header : public TiffHeaderBase {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  explicit Cr2Header(ByteOrder byteOrder = littleEndian);
  //@}

  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size) override;
  //@}

  //! @name Accessors
  //@{
  [[nodiscard]] DataBuf write() const override;
  bool isImageTag(uint16_t tag, IfdId group, const PrimaryGroups* pPrimaryGroups) const override;
  //@}

  //! Return the address of offset2 from the start of the header
  static uint32_t offset2addr() {
    return 12;
  }

 private:
  // DATA
  uint32_t offset2_{0x00000000};             //!< Bytes 12-15 from the header
  static constexpr auto cr2sig_ = "CR\2\0";  //!< Signature for CR2 type TIFF
};

}  // namespace Exiv2::Internal

#endif  // EXIV2_CR2HEADER_INT_HPP
