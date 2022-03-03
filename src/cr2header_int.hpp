// SPDX-License-Identifier: GPL-2.0-or-later

/*!
  @file    cr2image_int.hpp
  @brief   Internal classes to support CR2 image format
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    23-Apr-08, ahu: created
 */
#ifndef CR2IMAGE_INT_HPP_
#define CR2IMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "tiffimage_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    /// @brief Canon CR2 header structure.
    class Cr2Header : public TiffHeaderBase {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        explicit Cr2Header(ByteOrder byteOrder =littleEndian);
        //! Destructor.
        ~Cr2Header() override = default;
        //@}

        //! @name Manipulators
        //@{
        bool read(const byte* pData, uint32_t size) override;
        //@}

        //! @name Accessors
        //@{
        DataBuf write() const override;
        bool isImageTag(uint16_t tag, IfdId group, const PrimaryGroups* pPrimaryGroups) const override;
        //@}

        //! Return the address of offset2 from the start of the header
        static uint32_t offset2addr() { return 12; }

    private:
        // DATA
        uint32_t              offset2_;   //!< Bytes 12-15 from the header
        static constexpr auto cr2sig_ = "CR\2\0";  //!< Signature for CR2 type TIFF
    }; // class Cr2Header

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef CR2IMAGE_INT_HPP_
