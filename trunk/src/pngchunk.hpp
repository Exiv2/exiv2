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
  @file    pngchunk.hpp
  @brief   Class PngChunk to parse PNG chunk data.
  @version $Rev: 823 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Gilles Caulier (gc)
           <a href="mailto:caulier dot gilles at kdemail dot net">caulier dot gilles at kdemail dot net</a>
  @date    12-Jun-06, gc: submitted
 */
#ifndef PNGCHUNK_HPP_
#define PNGCHUNK_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <iosfwd>
#include <cassert>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations

    class Image;

// *****************************************************************************
// class definitions

    /*!
      @brief Stateless parser class for data in PNG chunk format. Images use this
             class to decode and encode PNG-based data.
     */
    class PngChunk {
    public:
        /*!
          @brief Decode PNG chunk metadata from a data buffer \em pData of length
                 \em size into \em pImage.

          @param pImage    Pointer to the image to hold the metadata
          @param pData     Pointer to the data buffer. Must point to PNG chunk data;
                           no checks are performed.
          @param size      Length of the data buffer.
        */
        static void decode(Image*       pImage,
                           const byte*  pData,
                           long         size);

                               private:
        //! @name Accessors
        //@{

        /*!
          @brief Todo: Decode ImageMagick raw text profile including encoded Exif/Iptc metadata byte array.
         */
        static DataBuf readRawProfile(const DataBuf& text);

        //@}

    }; // class PngChunk

}                                       // namespace Exiv2

#endif                                  // #ifndef PNGCHUNK_HPP_
