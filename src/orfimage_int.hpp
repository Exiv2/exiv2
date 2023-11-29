// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef ORFIMAGE_INT_HPP_
#define ORFIMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "tiffimage_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2::Internal {
// *****************************************************************************
// class definitions

/*!
  @brief Olympus ORF header structure.
 */
class OrfHeader : public TiffHeaderBase {
 public:
  //! @name Creators
  //@{
  //! Default constructor
  explicit OrfHeader(ByteOrder byteOrder = littleEndian);
  //@}

  //! @name Manipulators
  //@{
  bool read(const byte* pData, size_t size) override;
  //@}

  //! @name Accessors
  //@{
  [[nodiscard]] DataBuf write() const override;
  //@}
 private:
  // DATA
  uint16_t sig_{0x4f52};  ///< The actual magic number
};

}  // namespace Exiv2::Internal

#endif  // #ifndef ORFIMAGE_INT_HPP_
