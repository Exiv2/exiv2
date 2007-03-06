// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006-2007 Andreas Huggel <ahuggel@gmx.net>
 *
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
  @brief   Define to check the %Exiv2 version. 
           References: Similar versioning defines are used in KDE, GTK and other
           libraries. See http://apr.apache.org/versioning.html for accompanying
           guidelines.
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    31-May-06, ahu: created
 */
#ifndef VERSION_HPP_
#define VERSION_HPP_

/*!
  @brief %Exiv2 MAJOR version number of the library used at compile-time.
 */
#define EXIV2_MAJOR_VERSION  (0)
/*!
  @brief %Exiv2 MINOR version number of the library used at compile-time.
 */
#define EXIV2_MINOR_VERSION (13)
/*!
  @brief %Exiv2 PATCH version number of the library used at compile-time.
 */
#define EXIV2_PATCH_VERSION  (0)
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
/*!
  @brief Check the version of the available %Exiv2 library at runtime. Return 
         true if it is the same as or newer than the passed-in version.

  Versions are denoted using a standard triplet of integers:
  MAJOR.MINOR.PATCH. The basic intent is that MAJOR versions are incompatible,
  large-scale upgrades of the API. MINOR versions retain source and binary
  compatibility with older minor versions, and changes in the PATCH level are
  perfectly compatible, forwards and backwards.

  Details of these guidelines are described in http://apr.apache.org/versioning.html

  It is important to note that as long as the library has not reached 1.0.0 it
  is not subject to the guidelines described in the document above. Before a 1.0
  release (version 0.x.y), the API can and will be changing freely, without
  regard to the restrictions detailed in the above document.

  @code
  // Don't include the version.hpp file, it is included by types.hpp
  // Early Exiv2 versions didn't have this file and the macros

  #ifndef EXIV2_CHECK_VERSION
  # define EXIV2_CHECK_VERSION(a,b,c) (false)
  #endif

  // ...

  std::cout << "Compiled with Exiv2 version " << EXV_PACKAGE_VERSION << "\n"
            << "Runtime Exiv2 version is    " << Exiv2::version() << "\n";

  // Check the Exiv2 version available at runtime
  if (EXIV2_CHECK_VERSION(0,13,0)) {
      std::cout << "Available Exiv2 version is equal to or greater than 0.13\n";
  }
  else {
      std::cout << "Installed Exiv2 version is less than 0.13\n";
  }
  return 0;
  @endcode
 */
#define EXIV2_CHECK_VERSION(major,minor,patch) \
    ( Exiv2::versionNumber() >= EXIV2_MAKE_VERSION(major,minor,patch) )

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    /*!
      @brief Return the version of %Exiv2 available at runtime as an integer.
    */
    int versionNumber();
    /*!
      @brief Return the version of %Exiv2 available at runtime as a string.
    */
    const char* version();
}                                       // namespace Exiv2
#endif                                  // VERSION_HPP_
