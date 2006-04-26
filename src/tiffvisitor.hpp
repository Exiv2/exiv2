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
#include "tiffcomposite.hpp"

// + standard includes
#include <memory>
#include <iostream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations

    class TiffIfdMakernote;
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
        //! Operation to perform for a TIFF data entry
        virtual void visitDataEntry(TiffDataEntry* object) =0;
        //! Operation to perform for a TIFF size entry
        virtual void visitSizeEntry(TiffSizeEntry* object) =0;
        //! Operation to perform for a TIFF directory
        virtual void visitDirectory(TiffDirectory* object) =0;
        /*!
          @brief Operation to perform for a TIFF directory, after all components
                 and before the next entry is processed.
         */
        virtual void visitDirectoryNext(TiffDirectory* object) {}
        /*!
          @brief Operation to perform for a TIFF directory, at the end of the 
                 processing.
         */
        virtual void visitDirectoryEnd(TiffDirectory* object) {}
        //! Operation to perform for a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object) =0;
        //! Operation to perform for the makernote component
        virtual void visitMnEntry(TiffMnEntry* object) =0;
        //! Operation to perform for an IFD makernote
        virtual void visitIfdMakernote(TiffIfdMakernote* object) =0;
        //! Operation to perform after processing an IFD makernote
        virtual void visitIfdMakernoteEnd(TiffIfdMakernote* object) {}
        //! Operation to perform for an array entry (as found in Canon makernotes)
        virtual void visitArrayEntry(TiffArrayEntry* object) =0;
        //! Operation to perform for an array element
        virtual void visitArrayElement(TiffArrayElement* object) =0;
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
        //! Find tag and group in a TIFF data entry
        virtual void visitDataEntry(TiffDataEntry* object);
        //! Find tag and group in a TIFF size entry
        virtual void visitSizeEntry(TiffSizeEntry* object);
        //! Find tag and group in a TIFF directory
        virtual void visitDirectory(TiffDirectory* object);
        //! Find tag and group in a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Find tag and group in a TIFF makernote
        virtual void visitMnEntry(TiffMnEntry* object);
        //! Find tag and group in an IFD makernote
        virtual void visitIfdMakernote(TiffIfdMakernote* object);
        //! Find tag and group in an array entry component        
        virtual void visitArrayEntry(TiffArrayEntry* object);
        //! Find tag and group in an array element      
        virtual void visitArrayElement(TiffArrayElement* object);

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
        /*!
          Constructor, taking the image to add the metadata to
          and an optional threshold. Unknown tags with values
          larger (in bytes) than the threshold will be ignored.
          Default is not to ignore any tags (0).
         */
        TiffMetadataDecoder(Image* pImage, uint32_t threshold =0) 
            : pImage_(pImage), threshold_(threshold) {}
        //! Virtual destructor
        virtual ~TiffMetadataDecoder() {}
        //@}

        //! @name Manipulators
        //@{
        //! Decode a TIFF entry
        virtual void visitEntry(TiffEntry* object);
        //! Decode a TIFF data entry
        virtual void visitDataEntry(TiffDataEntry* object);
        //! Decode a TIFF size entry
        virtual void visitSizeEntry(TiffSizeEntry* object);
        //! Decode a TIFF directory
        virtual void visitDirectory(TiffDirectory* object);
        //! Decode a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Decode a TIFF makernote
        virtual void visitMnEntry(TiffMnEntry* object);
        //! Decode an IFD makernote
        virtual void visitIfdMakernote(TiffIfdMakernote* object);
        //! Decode an array entry component
        virtual void visitArrayEntry(TiffArrayEntry* object);
        //! Decode an array element
        virtual void visitArrayElement(TiffArrayElement* object);

        //! Decode a standard TIFF entry
        void decodeTiffEntry(const TiffEntryBase* object);
        //@}

    private:
        // DATA
        Image* pImage_; //!< Pointer to the image to which the metadata is added
        const uint32_t threshold_; //!< Threshold, see constructor documentation.
    }; // class TiffMetadataDecoder

    /*!
      @brief Simple state class containing relevant state information for 
             the TIFF reader. This is in a separate class so that the 
             reader can change state if needed (e.g., to read certain complex
             makernotes).
     */
    struct TiffRwState {
        //! TiffRWState auto_ptr type
        typedef std::auto_ptr<TiffRwState> AutoPtr;        
        //! Constructor.
        explicit TiffRwState(ByteOrder byteOrder, 
                             uint32_t baseOffset,
                             TiffCompFactoryFct createFct)
            : byteOrder_(byteOrder),
              baseOffset_(baseOffset),
              createFct_(createFct) {}
        /*!
          Applicable byte order. May be different for the Makernote and the 
          rest of the TIFF entries.
         */
        const ByteOrder byteOrder_;
        /*!
          Base offset. TIFF standard format uses byte offsets which are
          always relative to the start of the TIFF file, i.e., relative to the
          start of the TIFF image header. In this case, the base offset is 0.
          However, some camera vendors encode their makernotes in TIFF IFDs
          using offsets relative to (somewhere near) the start of the makernote
          data. In this case, base offset added to the start of the TIFF image
          header points to the basis for such makernote offsets.
         */
        const uint32_t baseOffset_;
        /*!
          Factory function to create new TIFF components. Different create
          functions may use different lookup tables, so that makernotes
          can independently use their own factory function and lookup table,
          which can be defined together with the makernote implementation.
         */
        TiffCompFactoryFct createFct_;
    }; // TiffRwState

    /*!
      @brief TIFF composite visitor to read the TIFF structure from a block of
             memory and build the composite from it (Visitor pattern). Used by
             TiffParser to read the TIFF data from a block of memory.
     */
    class TiffReader : public TiffVisitor {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor. The data buffer and table describing the TIFF
                           structure of the data are set in the constructor.
          @param pData     Pointer to the data buffer, starting with a TIFF header.
          @param size      Number of bytes in the data buffer.
          @param pRoot     Root element of the TIFF composite.
          @param state     State object for creation function, byteorder and 
                           base offset.
         */
        TiffReader(const byte*          pData,
                   uint32_t             size,
                   TiffComponent*       pRoot,
                   TiffRwState::AutoPtr state);

        //! Virtual destructor
        virtual ~TiffReader();
        //@}

        //! @name Manipulators
        //@{
        //! Read a TIFF entry from the data buffer
        virtual void visitEntry(TiffEntry* object);
        //! Read a TIFF data entry from the data buffer
        virtual void visitDataEntry(TiffDataEntry* object);
        //! Read a TIFF size entry from the data buffer
        virtual void visitSizeEntry(TiffSizeEntry* object);
        //! Read a TIFF directory from the data buffer
        virtual void visitDirectory(TiffDirectory* object);
        //! Read a TIFF sub-IFD from the data buffer
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Read a TIFF makernote entry from the data buffer
        virtual void visitMnEntry(TiffMnEntry* object);
        //! Read an IFD makernote from the data buffer
        virtual void visitIfdMakernote(TiffIfdMakernote* object);
        //! Reset reader to its original state, undo makernote specific settings
        virtual void visitIfdMakernoteEnd(TiffIfdMakernote* object);
        //! Read an array entry component from the data buffer
        virtual void visitArrayEntry(TiffArrayEntry* object);
        //! Read an array element from the data buffer
        virtual void visitArrayElement(TiffArrayElement* object);

        //! Read a standard TIFF entry from the data buffer
        void readTiffEntry(TiffEntryBase* object);
        //! Set the \em state class. Assumes ownership of the object passed in.
        void changeState(TiffRwState::AutoPtr state);
        //! Reset the state to the original state as set in the constructor.
        void resetState();
        //@}

        //! @name Accessors
        //@{
        //! Return the byte order.
        ByteOrder byteOrder() const;
        //! Return the base offset. See class TiffRwState for details
        uint32_t baseOffset() const;
        //! Create a TIFF component for \em extendedTag and group
        TiffComponent::AutoPtr create(uint32_t extendedTag, uint16_t group) const;
        //@}

    private:
        // DATA
        const byte*          pData_;      //!< Pointer to the memory buffer
        const uint32_t       size_;       //!< Size of the buffer
        const byte*          pLast_;      //!< Pointer to the last byte
        TiffComponent* const pRoot_;      //!< Root element of the composite
        TiffRwState*         pState_;     //!< State class
        TiffRwState*         pOrigState_; //!< State class as set in the c'tor

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
        //! Print a TIFF data entry.
        virtual void visitDataEntry(TiffDataEntry* object);
        //! Print a TIFF size entry.
        virtual void visitSizeEntry(TiffSizeEntry* object);
        //! Print a TIFF directory
        virtual void visitDirectory(TiffDirectory* object);
        //! Print header before next directory
        virtual void visitDirectoryNext(TiffDirectory* object);
        //! Cleanup before leaving this directory
        virtual void visitDirectoryEnd(TiffDirectory* object);
        //! Print a TIFF sub-IFD
        virtual void visitSubIfd(TiffSubIfd* object);
        //! Print a TIFF makernote
        virtual void visitMnEntry(TiffMnEntry* object);
        //! Print an IFD makernote
        virtual void visitIfdMakernote(TiffIfdMakernote* object);
        //! Print an array entry component
        virtual void visitArrayEntry(TiffArrayEntry* object);
        //! Print an array element
        virtual void visitArrayElement(TiffArrayElement* object);

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

}                                       // namespace Exiv2

#endif                                  // #ifndef TIFFVISITOR_HPP_
