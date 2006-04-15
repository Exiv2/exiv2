// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  @file    tiffparser.hpp
  @brief   Class TiffParser to parse TIFF data.
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Mar-06, ahu: created
 */
#ifndef TIFFPARSER_HPP_
#define TIFFPARSER_HPP_

// *****************************************************************************
// included header files
#include "tiffcomposite.hpp"
#include "tiffvisitor.hpp"
#include "tiffvisitor_tmpl.hpp"
#include "error.hpp"
#include "types.hpp"

// + standard includes
#include <iosfwd>
#include <cassert>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations

    struct TiffStructure;
    class Image;

// *****************************************************************************
// class definitions

    /*!
      Type for a function pointer for functions to create TIFF components.
      Todo: This may eventually need to also have access to the image or parse tree
      in order to make decisions based on the value of other tags.
     */
    typedef TiffComponent::AutoPtr (*NewTiffCompFct)(const TiffStructure* ts);

    /*!
      @brief Data structure used as a row (element) of a table (array)
             describing the TIFF structure of an image format for reading and
             writing.  Different tables can be used to support different TIFF
             based image formats.
     */
    struct TiffStructure {
        struct Key;
        //! Comparison operator to compare a TiffStructure with a TiffStructure::Key
        bool operator==(const Key& key) const;
        //! Return the tag corresponding to the extended tag
        uint16_t tag() const { return static_cast<uint16_t>(extendedTag_ & 0xffff); }

        // DATA
        uint32_t       extendedTag_;    //!< Tag (32 bit so that it can contain special tags)
        uint16_t       group_;          //!< Group that contains the tag
        NewTiffCompFct newTiffCompFct_; //!< Function to create the correct TIFF component
        uint16_t       newGroup_;       //!< Group of the newly created component
    };

    //! Search key for TIFF structure.
    struct TiffStructure::Key {
        //! Constructor
        Key(uint32_t e, uint16_t g) : e_(e), g_(g) {}
        uint32_t e_;                    //!< Extended tag
        uint16_t g_;                    //!< %Group
    };

    /*!
      @brief TIFF component factory for standard TIFF components. This class is
             used as a policy class.
     */
    class TiffCreator {
    public:
        /*!
          @brief Create the TiffComponent for TIFF entry \em extendedTag and 
                 \em group based on the embedded lookup table.

          If a tag and group combination is not found in the table, a TiffEntry
          is created.  If the pointer that is returned is 0, then the TIFF entry
          should be ignored.
        */
        static TiffComponent::AutoPtr create(uint32_t extendedTag,
                                             uint16_t group);
    protected:
        //! Prevent destruction
        ~TiffCreator() {}
    private:
        static const TiffStructure tiffStructure_[]; //<! TIFF structure
    }; // class TiffCreator

    /*!
      @brief Stateless parser class for data in TIFF format. Images use this
             class to decode and encode TIFF-based data. Uses class 
             CreationPolicy for the creation of TIFF components.
     */
    template<typename CreationPolicy>
    class TiffParser : public CreationPolicy {
    public:
        /*!
          @brief Decode TIFF metadata from a data buffer \em pData of length
                 \em size into \em pImage.

          This is the entry point to access image data in TIFF format. The
          parser uses classes TiffHeade2 and the TiffComponent and TiffVisitor
          hierarchies.

          @param pImage         Pointer to the image to hold the metadata
          @param pData          Pointer to the data buffer. Must point to data
                                in TIFF format; no checks are performed.
          @param size           Length of the data buffer.
        */
        static void decode(      Image*         pImage,
                           const byte*          pData,
                                 uint32_t       size);
    }; // class TiffParser

// *****************************************************************************
// template, inline and free functions

    //! Function to create and initialize a new TIFF directory
    TiffComponent::AutoPtr newTiffDirectory(const TiffStructure* ts);

    //! Function to create and initialize a new TIFF sub-directory
    TiffComponent::AutoPtr newTiffSubIfd(const TiffStructure* ts);

    //! Function to create and initialize a new TIFF makernote entry
    TiffComponent::AutoPtr newTiffMnEntry(const TiffStructure* ts);

    template<typename CreationPolicy>
    void TiffParser<CreationPolicy>::decode(Image* pImage,
                                            const byte* pData,
                                            uint32_t size)
    {
        assert(pImage != 0);
        assert(pData != 0);

        TiffHeade2 tiffHeader;
        if (!tiffHeader.read(pData, size) || tiffHeader.offset() >= size) {
            throw Error(3, "TIFF");
        }
        TiffComponent::AutoPtr rootDir = CreationPolicy::create(Tag::root, 
                                                                Group::none);
        if (0 == rootDir.get()) return;
        rootDir->setStart(pData + tiffHeader.offset());

        TiffReader<CreationPolicy> reader(pData,
                                          size,
                                          tiffHeader.byteOrder(),
                                          rootDir.get());
        rootDir->accept(reader);

        TiffMetadataDecoder decoder(pImage);
        rootDir->accept(decoder);

    } // TiffParser::decode

}                                       // namespace Exiv2

#endif                                  // #ifndef TIFFPARSER_HPP_
