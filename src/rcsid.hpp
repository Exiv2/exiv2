// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*!
  @file    rcsid.hpp
  @brief   Define an RCS id string in every file that includes rcsid.hpp.

  This hack has the following purposes:
  1. To define the RCS id string variable in the local namespace, so
     that there won't be any duplicate extern symbols at link time.
  2. To avoid warnings of the type "variable declared and never used".

  @version $Name:  $ $Revision: 1.2 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    02-Feb-04, ahu: created
 */
#ifndef RCSID_HPP_
#define RCSID_HPP_

#if !defined (EXIV2_RCSID)
/*!
  @brief Macro to store version information in each object file.
         Include the following two lines at the beginning of each *.cpp file.
         See the ident(1) manual pages for more information.

         @code
         #include "rcsid.hpp"
         EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.2 $ $RCSfile: rcsid.hpp,v $")
         @endcode
 */
#define EXIV2_RCSID(id) \
    namespace { \
        inline const char* getRcsId(const char*) \
        { \
            return id ; \
        } \
        const char* rcsId = getRcsId(rcsId); \
    }

#endif // #if !defined (EXIV2_RCSID)
#endif // #ifndef RCSID_HPP_
