// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2021 Exiv2 authors
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

#pragma once

// *****************************************************************************
#include "exiv2lib_export.h"

// included header files
#include "image.hpp"
#include "iostream"

// *****************************************************************************
// namespace extensions
namespace Exiv2
{
    EXIV2API bool enableBMFF(bool enable = true);

    struct Iloc
    {
        Iloc(uint32_t ID = 0, uint32_t start = 0, uint32_t length = 0) : ID_(ID), start_(start), length_(length){};
        virtual ~Iloc(){};

        uint32_t ID_;
        uint32_t start_;
        uint32_t length_;

        std::string toString() const;
    };  // class Iloc

    // *****************************************************************************
    // class definitions

    // Add Base Media File Format to the supported image formats
    namespace ImageType
    {
        const int bmff = 15;  //!< BMFF (bmff) image type (see class BMFF)
    }

    /*!
      @brief Class to access BMFF images.
     */
    class EXIV2API BmffImage : public Image
    {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor to open a BMFF image. Since the
              constructor can not return a result, callers should check the
              good() method after object construction to determine success
              or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method.  Use the Image::io()
              method to get a temporary reference.
          @param create Specifies if an existing image should be read (false)
              or if a new file should be created (true).
         */
        BmffImage(BasicIo::AutoPtr io, bool create);
        //@}

        //@{
        /*!
          @brief parse embedded tiff file (Exif metadata)
          @param root_tag root of parse tree Tag::root, Tag::cmt2 etc.
          @param length tiff block length
          @param start offset in file (default, io_->tell())
         @
         */
        void parseTiff(uint32_t root_tag, uint32_t length);
        void parseTiff(uint32_t root_tag, uint32_t length,uint32_t start);
        //@}

        //@{
        /*!
          @brief parse embedded xmp/xml
          @param length xmp block length
          @param start offset in file
         @
         */
        void parseXmp(uint32_t length,uint32_t start);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata() /* override */;
        void writeMetadata() /* override */;
        void setComment(const std::string& comment) /* override */;
        void printStructure(std::ostream& out, Exiv2::PrintStructureOption option,int depth);
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const /* override */;
        int pixelWidth() const;
        int pixelHeight() const;
        //@}
        
        Exiv2::ByteOrder endian_ ;

    private:
        void openOrThrow();
        /*!
          @brief recursiveBoxHandler
          @throw Error if we visit a box more than once
          @return address of next box
          @warning This function should only be called by readMetadata()
         */
        long boxHandler(std::ostream& out=std::cout, Exiv2::PrintStructureOption option=kpsNone,int depth = 0);
        std::string indent(int i)
        {
            return std::string(2*i,' ');
        }

        uint32_t                 fileType_;
        std::set<uint64_t>       visits_;
        uint64_t                 visits_max_;
        uint16_t                 unknownID_;  // 0xffff
        uint16_t                 exifID_;
        uint16_t                 xmpID_;
        std::map<uint32_t, Iloc> ilocs_;
        bool                     bReadMetadata_;
        //@}

        /*!
          @brief box utilities
         */
        std::string toAscii(long n);
        std::string boxName(uint32_t box);
        bool        superBox(uint32_t box);
        bool        fullBox(uint32_t box);
        std::string uuidName(Exiv2::DataBuf& uuid);

    };  // class BmffImage

    // *****************************************************************************
    // template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new BMFF instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that
             it will be deleted.
     */
    EXIV2API Image::AutoPtr newBmffInstance(BasicIo::AutoPtr io, bool create);

    //! Check if the file iIo is a BMFF image.
    EXIV2API bool isBmffType(BasicIo& iIo, bool advance);
}  // namespace Exiv2
