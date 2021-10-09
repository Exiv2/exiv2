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
}

#ifdef EXV_ENABLE_BMFF
namespace Exiv2
{
    struct Iloc
    {
        explicit Iloc(uint32_t ID = 0, uint32_t start = 0, uint32_t length = 0)
            : ID_(ID), start_(start), length_(length){};
        virtual ~Iloc() = default;

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
        const int bmff = 19;  //!< BMFF (bmff) image type (see class BMFF)
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
        BmffImage(BasicIo::UniquePtr io, bool create);
        //@}

        //@{
        /*!
          @brief parse embedded tiff file (Exif metadata)
          @param root_tag root of parse tree Tag::root, Tag::cmt2 etc.
          @param length tiff block length
          @param start offset in file (default, io_->tell())
         @
         */
        void parseTiff(uint32_t root_tag, uint64_t length);
        void parseTiff(uint32_t root_tag, uint64_t length,uint64_t start);
        //@}

        //@{
        /*!
          @brief parse embedded xmp/xml
          @param length xmp block length
          @param start offset in file
         @
         */
        void parseXmp(uint64_t length,uint64_t start);
        //@}

        //@{
        /*!
        @brief Parse a Canon PRVW or THMB box and add an entry to the set
            of native previews.
        @param data Buffer containing the box
        @param out Logging stream
        @param bTrace Controls logging
        @param width_offset Index of image width field in data
        @param height_offset Index of image height field in data
        @param size_offset Index of image size field in data
        @param relative_position Location of the start of image data in the file,
            relative to the current file position indicator.
        */
        void parseCr3Preview(DataBuf &data,
                             std::ostream &out,
                             bool bTrace,
                             uint16_t width_offset,
                             uint16_t height_offset,
                             uint32_t size_offset,
                             uint16_t relative_position);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata() override /* override */;
        void writeMetadata() override /* override */;
        void setComment(const std::string& comment) override /* override */;
        void printStructure(std::ostream& out, Exiv2::PrintStructureOption option, int depth) override;
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const override /* override */;
        int pixelWidth() const override;
        int pixelHeight() const override;
        //@}
        
        Exiv2::ByteOrder endian_{Exiv2::bigEndian};

    private:
        void openOrThrow();
        /*!
          @brief recursiveBoxHandler
          @throw Error if we visit a box more than once
          @param pbox_end The end location of the parent box. Boxes are
              nested, so we must not read beyond this.
          @return address of next box
          @warning This function should only be called by readMetadata()
         */
        long boxHandler(std::ostream& out, Exiv2::PrintStructureOption option,
                        const long pbox_end, int depth);
        std::string indent(int i)
        {
            return std::string(2*i,' ');
        }

        uint32_t                 fileType_{0};
        std::set<uint64_t>       visits_;
        uint64_t                 visits_max_{0};
        uint16_t                 unknownID_{0xffff};
        uint16_t                 exifID_{0xffff};
        uint16_t                 xmpID_{0};
        std::map<uint32_t, Iloc> ilocs_;
        bool                     bReadMetadata_{false};
        //@}

        /*!
          @brief box utilities
         */
        static std::string toAscii(long n);
        std::string boxName(uint32_t box);
        static bool superBox(uint32_t box);
        static bool fullBox(uint32_t box);
        static std::string uuidName(Exiv2::DataBuf& uuid);

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
    EXIV2API Image::UniquePtr newBmffInstance(BasicIo::UniquePtr io, bool create);

    //! Check if the file iIo is a BMFF image.
    EXIV2API bool isBmffType(BasicIo& iIo, bool advance);
}  // namespace Exiv2
#endif // EXV_ENABLE_BMFF
