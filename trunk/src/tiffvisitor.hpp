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
  @file    tiffvisitor.hpp
  @brief   
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Apr-06, ahu: created
 */
#ifndef TIFFVISITOR_HPP_
#define TIFFVISITOR_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <iostream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations

    class TiffComponent;
    class TiffEntryBase;
    class TiffEntry;
    class TiffDirectory;
    class TiffSubIfd;
    class TiffMakernote;
    class TiffOlympusMn;
    class Image;

// *****************************************************************************
// class definitions

    /*!
      @brief Abstract base class defining the interface for TIFF composite 
             vistors (Visitor pattern)

      A concrete visitor class is used as shown in the example below. Accept()
      will invoke the member function corresponding to the concrete type of each
      component in the composite.

      @code
      void visitorExample(Exiv2::TiffComponent* tiffComponent, Exiv2::TiffVisitor& visitor)
      {
          tiffComponent->accept(visitor);
      }
      @endcode
     */
    class TiffVisitor {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffVisitor() : go_(true) {}
        //! Virtual destructor
        virtual ~TiffVisitor() {}
        //@}

        //! @name Manipulators
        //@{
        //! Set the stop/go flag: true for go, false for stop
        void setGo(bool go) { go_ = go; }
        //! Operation to perform for a TIFF entry
        virtual void visitEntry(TiffEntry* object) =0;
        //! Operation to perform for a TIFF directory
        virtual void visitDirectory(TiffDirectory* object) =0;
        /*!
          Operation to perform for a TIFF directory, after all components and 
          before the next entry is processed.
         */
        virtual void visitDirectoryNext(TiffDirectory* object) {}
        /*!
          Operation to perform for a TIFF directory, at the end of the 
          processing.
         */
        virtual void visitDirectoryEnd(TiffDirectory* object) {}
        //! Operation to perform for a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object) =0;
        //! Operation to perform for the makernote component
        virtual void visitMakernote(TiffMakernote* object) =0;
        //! Operation to perform for an Olympus makernote
        virtual void visitOlympusMn(TiffOlympusMn* object) =0;
        //@}

        //! @name Accessors
        //@{
        //! Check if stop flag is clear, return true if it's clear.
        bool go() { return go_; }
        //@}

    private:
        bool go_;    //!< Set this to false to abort the iteration

    }; // class TiffVisitor

    /*!
      @brief Search the composite for a component with \em tag and \em group.
             Return a pointer to the component or 0, if not found. The class
             is ready for a first search after construction and can be 
             re-initialized with init().
    */
    class TiffFinder : public TiffVisitor {
    public:
        //! @name Creators
        //@{
        //! Constructor, taking the image to add the metadata to
        TiffFinder(uint16_t tag, uint16_t group) 
            : tag_(tag), group_(group), tiffComponent_(0) {}
        //! Virtual destructor
        virtual ~TiffFinder() {}
        //@}

        //! @name Manipulators
        //@{
        //! Find tag and group in a TIFF entry
        virtual void visitEntry(TiffEntry* object);
        //! Find tag and group in a TIFF directory
        virtual void visitDirectory(TiffDirectory* object);
        //! Find tag and group in a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Find tag and group in a TIFF makernote
        virtual void visitMakernote(TiffMakernote* object);
        //! Find tag and group in an Olympus makernote
        virtual void visitOlympusMn(TiffOlympusMn* object);

        //! Check if \em object matches \em tag and \em group
        void findObject(TiffComponent* object);
        //! Initialize the Finder for a new search.
        void init(uint16_t tag, uint16_t group);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Return the search result. 0 if no TIFF component was found
                 for the tag and group combination.
         */
        TiffComponent* result() const { return tiffComponent_; }
        //@}
        
    private:
        uint16_t tag_;
        uint16_t group_;
        TiffComponent* tiffComponent_;
    }; // class TiffFinder

    /*!
      @brief TIFF composite visitor to decode metadata from the TIFF tree and
             add it to an Image, which is supplied in the constructor (Visitor
             pattern). Used by TiffParser to decode the metadata from a
             TIFF composite.
     */
    class TiffMetadataDecoder : public TiffVisitor {
    public:
        //! @name Creators
        //@{
        //! Constructor, taking the image to add the metadata to
        TiffMetadataDecoder(Image* pImage) : pImage_(pImage) {}
        //! Virtual destructor
        virtual ~TiffMetadataDecoder() {}
        //@}

        //! @name Manipulators
        //@{
        //! Decode a TIFF entry
        virtual void visitEntry(TiffEntry* object);
        //! Decode a TIFF directory
        virtual void visitDirectory(TiffDirectory* object);
        //! Decode a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Decode a TIFF makernote
        virtual void visitMakernote(TiffMakernote* object);
        //! Decode an Olympus makernote
        virtual void visitOlympusMn(TiffOlympusMn* object);

        //! Decode a standard TIFF entry
        void decodeTiffEntry(const TiffEntryBase* object);
        //@}

    private:
        // DATA
        Image* pImage_; //!< Pointer to the image to which the metadata is added

    }; // class TiffMetadataDecoder

    /*!
      @brief TIFF composite visitor to read the TIFF structure from a block of
             memory and build the composite from it (Visitor pattern). Used by
             TiffParser to read the TIFF data from a block of memory. Uses
             the policy class CreationPolicy for the creation of TIFF components.
     */
    template<typename CreationPolicy>
    class TiffReader : public TiffVisitor, public CreationPolicy {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor. The data buffer and table describing the TIFF
                           structure of the data are set in the constructor.
          @param pData     Pointer to the data buffer, starting with a TIFF header.
          @param size      Number of bytes in the data buffer.
          @param byteOrder Applicable byte order (little or big endian).
          @param pRoot     Root element of the TIFF composite.
         */
        TiffReader(const byte*    pData,
                   uint32_t       size,
                   ByteOrder      byteOrder,
                   TiffComponent* pRoot);

        //! Virtual destructor
        virtual ~TiffReader() {}
        //@}

        //! @name Manipulators
        //@{
        //! Read a TIFF entry from the data buffer
        virtual void visitEntry(TiffEntry* object);
        //! Read a TIFF directory from the data buffer
        virtual void visitDirectory(TiffDirectory* object);
        //! Read a TIFF sub-IFD from the data buffer
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Read a TIFF makernote entry from the data buffer
        virtual void visitMakernote(TiffMakernote* object);
        //! Read an Olympus makernote from the data buffer
        virtual void visitOlympusMn(TiffOlympusMn* object);

        //! Read a standard TIFF entry from the data buffer
        void readTiffEntry(TiffEntryBase* object);
        //@}

    private:
        // DATA
        const byte* pData_;                   //!< Pointer to the memory buffer
        const uint32_t size_;                 //!< Size of the buffer
        const byte* pLast_;                   //!< Pointer to the last byte
        const ByteOrder byteOrder_;           //!< Byteorder for the image
        TiffComponent* const pRoot_;          //!< Root element of the composite

    }; // class TiffReader

    /*!
      @brief TIFF composite visitor to print the TIFF structure to an output 
             stream.
    */
    class TiffPrinter : public TiffVisitor {
    public:
        //! @name Creators
        //@{
        //! Constructor, takes an output stream to write to.
        TiffPrinter(std::ostream& os, const std::string& prefix ="")
            : os_(os), prefix_(prefix) {}
        //! Virtual destructor
        virtual ~TiffPrinter() {}
        //@}

        //! @name Manipulators
        //@{
        //! Print a TIFF entry.
        virtual void visitEntry(TiffEntry* object);
        //! Print a TIFF directory
        virtual void visitDirectory(TiffDirectory* object);
        //! Print header before next directory
        virtual void visitDirectoryNext(TiffDirectory* object);
        //! Cleanup before leaving this directory
        virtual void visitDirectoryEnd(TiffDirectory* object);
        //! Print a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Print a TIFF makernote
        virtual void visitMakernote(TiffMakernote* object);
        //! Print an Olympus makernote
        virtual void visitOlympusMn(TiffOlympusMn* object);

        //! Increment the indent by one level
        void incIndent();
        //! Decrement the indent by one level
        void decIndent();
        //@}

        //! @name Accessors
        //@{
        //! Print a standard TIFF entry.
        void printTiffEntry(TiffEntryBase* object, 
                            const std::string& prefix ="") const;
        //! Return the current prefix
        std::string prefix() const { return prefix_; }
        //@}

    private:
        // DATA
        std::ostream& os_;                      //!< Output stream to write to
        std::string prefix_;                    //!< Current prefix

        static const std::string indent_;       //!< Indent for one level
    }; // class TiffPrinter

// *****************************************************************************
// template, inline and free functions

}                                       // namespace Exiv2

#endif                                  // #ifndef TIFFVISITOR_HPP_
