// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef FUJIMN_INT_HPP_
#define FUJIMN_INT_HPP_

// *****************************************************************************
// included header files
#include "tags.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

// *****************************************************************************
// class definitions

    //! MakerNote for Fujifilm cameras
    class FujiMakerNote {
    public:
        //! Return read-only list of built-in Fujifilm tags
        static const TagInfo* tagList();

    private:
        //! Tag information
        static const TagInfo tagInfo_[];

    }; // class FujiMakerNote

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef FUJIMN_INT_HPP_
