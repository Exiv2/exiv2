// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef RW2IMAGE_INT_HPP_
#define RW2IMAGE_INT_HPP_

// *****************************************************************************
// included header files
#include "tiffimage_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

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
        //! Destructor.
        ~Rw2Header() override = default;
        //@}

        //! @name Accessors
        //@{
        //! Not yet implemented. Does nothing and returns an empty buffer.
        DataBuf write() const override;
        //@}

    }; // class Rw2Header

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef RW2IMAGE_INT_HPP_
