// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef ORFIMAGE_INT_HPP_
#define ORFIMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "tiffimage_int.hpp"
#include "types.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

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
        explicit OrfHeader(ByteOrder byteOrder =littleEndian);
        //! Destructor.
        ~OrfHeader() override = default;
        //@}

        //! @name Manipulators
        //@{
        bool read(const byte* pData, uint32_t size) override;
        //@}

        //! @name Accessors
        //@{
        DataBuf write() const override;
        //@}
    private:
        // DATA
        uint16_t sig_;       //<! The actual magic number
    }; // class OrfHeader

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef ORFIMAGE_INT_HPP_
