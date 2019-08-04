// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2019 Exiv2 authors
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
  @file    heifimage_int.hpp
  @brief   Include HEIF header files.
  @author  Peter Kovář (1div0)
           <a href="mailto:peter.kovar@reflexion.tv">peter.kovar@reflexion.tv</a>
  @date    25-Dec-18, 1div0: created
 */
#pragma once

#include <libheif/heif.h>

// *****************************************************************************

// included header files
#include "image_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2
{
    /*!
      @brief Class to access HEIF files.
     */
    class EXIV2API HeifImage : public Image
    {
    public:
        //! @name Creators
        //@{
        HeifImage(BasicIo::UniquePtr io, bool create);
        //@}

        ~HeifImage();

        //! @name Manipulators
        //@{
        void readMetadata() override;
        void writeMetadata() override;
        void printStructure(std::ostream& out, PrintStructureOption option,int depth) override;
        //@}

        /*!
          @brief Not supported. Calling this function will throw an Error(kerInvalidSettingForImage).
         */
        void setComment(const std::string& comment) override;
        //void setIptcData(const IptcData& /*iptcData*/) override;

        //! @name Accessors
        //@{
        std::string mimeType() const override;
        //@}

        HeifImage& operator=(const HeifImage& rhs) = delete;
        HeifImage& operator=(const HeifImage&& rhs) = delete;
        HeifImage(const HeifImage& rhs) = delete;
        HeifImage(const HeifImage&& rhs) = delete;

    private:
        void doWriteMetadata(BasicIo& outIo);
        //! @name NOT Implemented
        //@{
        long getHeaderOffset(byte *data, long data_size,
                             byte *header, long header_size);
        void debugPrintHex(byte *data, long size);
        void decodeChunks(uint64_t filesize);

    }; //Class HeifImage

    /*!
      @brief Create a new HeifImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::UniquePtr newHeifInstance(BasicIo::UniquePtr io, bool create);

    //! Check if the file iIo is a HEIF image.
    EXIV2API bool isHeifType(BasicIo& iIo, bool advance);
}
