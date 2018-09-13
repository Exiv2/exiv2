// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @file    version.hpp
  @brief   Precompiler define and a function to test the %Exiv2 version.
           References: Similar versioning defines are used in KDE, GTK and other
           libraries. See http://apr.apache.org/versioning.html for accompanying
           guidelines.
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    31-May-06, ahu: created
 */
#ifndef VERSION_HPP_
#define VERSION_HPP_

#include "exv_conf.h"
#include "exiv2lib_export.h"

// *****************************************************************************
// included header files
// + standard includes
#include <vector>

#if defined(EXV_HAVE_REGEX_H)
# include <regex.h>
  /*!
   @brief exv_grep_keys_t is a vector of keys to match to strings
  */
   typedef std::vector<regex_t> exv_grep_keys_t ;
# else
  /*!
   @brief exv_grep_key_t is a simple string and the ignore flag
  */
   struct Exiv2_grep_key_t {
    /*!
    @brief Exiv2_grep_key_t constructor
    */
     Exiv2_grep_key_t(std::string pattern,bool bIgnoreCase)
       :pattern_(pattern),bIgnoreCase_(bIgnoreCase) {}

     //! simple string to match
     std::string pattern_;

     //! should we ignore cast in the match?
     bool        bIgnoreCase_;
   };
  /*!
   @brief exv_grep_keys_t is a vector of keys to match to strings
  */
   typedef std::vector<Exiv2_grep_key_t> exv_grep_keys_t ;
#endif

/*!
  @brief Make an integer version number for comparison from a major, minor and
         a patch version number.
 */
#define EXIV2_MAKE_VERSION(major,minor,patch) \
    (((major) << 16) | ((minor) << 8) | (patch))
/*!
  @brief The %Exiv2 version number of the library used at compile-time as
         an integer number for easy comparison.
 */
#define EXIV2_VERSION \
    EXIV2_MAKE_VERSION(EXIV2_MAJOR_VERSION,EXIV2_MINOR_VERSION,EXIV2_PATCH_VERSION)

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    /*!
      @brief Return the version of %Exiv2 available at runtime as an integer.
    */
    EXIV2API int versionNumber();
    /*!
      @brief Return the version string Example: "0.25.0" (major.minor.patch)
    */
    EXIV2API std::string versionString();
    /*!
      @brief Return the version of %Exiv2 as hex string of fixed length 6.
    */
    EXIV2API std::string versionNumberHexString();

    /*!
      @brief dumpLibraryInfo implements the exiv2 option --version --verbose
             used by exiv2 test suite to inspect libraries loaded at run-time
     */
    EXIV2API void dumpLibraryInfo(std::ostream& os,const exv_grep_keys_t& keys);
}                                       // namespace Exiv2



#endif                                  // VERSION_HPP_
