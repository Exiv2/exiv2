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
  @file    makernote2.hpp
  @brief   Makernote base classes, factory and registry
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Apr-06, ahu: created
 */
#ifndef MAKERNOTE2_HPP_
#define MAKERNOTE2_HPP_

// *****************************************************************************
// included header files
#include "tiffcomposite.hpp"
#include "tiffvisitor.hpp"
#include "types.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    //! Type for a pointer to a function creating a makernote
    typedef TiffComponent* (*NewMnFct)(uint16_t    tag,
                                       uint16_t    group,
                                       uint16_t    mnGroup,
                                       const byte* pData, 
                                       uint32_t    size, 
                                       ByteOrder   byteOrder);

    //! Makernote registry structure
    struct TiffMnRegistry {
        struct Key;
        /*!
          @brief Compare a TiffMnRegistry structure with a TiffMnRegistry::Key
                 The two are equal if TiffMnRegistry::make_ equals a substring
                 of the key of the same size. E.g., registry = "OLYMPUS",
                 key = "OLYMPUS OPTICAL CO.,LTD" (found in the makernote of 
                 the image) match.
         */
        bool operator==(const Key& key) const;

        // DATA
        const char* make_;                      //!< Camera make
        NewMnFct    newMnFct_;                  //!< Makernote create function
        uint16_t    mnGroup_;                   //!< Group identifier
    };

    //! Search key for Makernote registry structure.
    struct TiffMnRegistry::Key {
        //! Constructor
        Key(const std::string& make) : make_(make) {}
        std::string make_;                      //!< Camera make
    };

    /*!
      @brief TIFF makernote factory for concrete TIFF makernotes. 
     */
    class TiffMnCreator {
    public:
        /*!
          @brief Create the Makernote for camera \em make and details from
                 the makernote entry itself if needed. Return a pointer to
                 the newly created TIFF component. Set tag and group of the 
                 new component to \em tag and \em group.
          @note  Ownership for the component is transferred to the caller,
                 who is responsible to delete the component. No smart pointer
                 is used to indicate this transfer here in order to reduce
                 file dependencies.
        */
        static TiffComponent* create(uint16_t    tag,
                                     uint16_t    group,
                                     std::string make,
                                     const byte* pData,
                                     uint32_t    size,
                                     ByteOrder   byteOrder);
    protected:
        //! Prevent destruction (needed if used as a policy class)
        ~TiffMnCreator() {}
    private:
        static const TiffMnRegistry registry_[]; //<! List of makernotes
    }; // class TiffMnCreator

    //! Makernote header interface. This class is used with TIFF makernotes.
    class MnHeader {
    public:
        //! @name Creators
        //@{
        //! Virtual destructor.
        virtual ~MnHeader() {}
        //@}        
        //! @name Manipulators
        //@{
        //! Read the header from a data buffer, return true if successful
        virtual bool read(const byte* pData, 
                          uint32_t    size,
                          ByteOrder byteOrder) =0;
        //@}
        //! @name Accessors
        //@{
        //! Size of the header
        virtual uint32_t size() const =0;
        /*!
          @brief Start of the makernote directory relative to the start of the
                 header.
         */
        virtual uint32_t ifdOffset() const =0;
        //! Check the header, return true if ok
        virtual bool check() const =0;
        //@}

    }; // class MnHeader

    /*!
      @brief Tiff IFD Makernote. Defines the interface for all IFD makernotes.
             Contains an IFD and implements child mgmt functions to deal with 
             the IFD entries.
     */
    class TiffIfdMakernote : public TiffComponent {
        friend class TiffReader;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffIfdMakernote(uint16_t tag, uint16_t group, uint16_t mnGroup)
            : TiffComponent(tag, group), ifd_(tag, mnGroup) {}
        //! Virtual destructor
        virtual ~TiffIfdMakernote() {}
        //@}

        //! @name Manipulators
        //@{
        //! Read the header from a data buffer, return true if successful
        bool readHeader(const byte* pData, uint32_t size, ByteOrder byteOrder);
        //@}

        //! @name Accessors
        //@{
        //! Check the header, return true if ok
        bool checkHeader() const;
        /*!
          @brief Return the offset to the start of the Makernote IFD from
                 the start of the Makernote.
         */
        uint32_t ifdOffset() const;
        /*!
          @brief Get status information relevant for the makernote. 

          State includes byte order, offset and TIFF component factory.
          This method allows the TiffReader to change state, i.e., change
          these parameters, to parse the Makernote and its sub components
          (if any).

          @param mnOffset Offset to the makernote from the start of the
                 TIFF header.
         */
        TiffRwState::AutoPtr getState(uint32_t mnOffset) const;
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual void doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAddNext(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        //! Implements readHeader();
        virtual bool doReadHeader(const byte* pData, 
                                  uint32_t    size,
                                  ByteOrder   byteOrder) =0;
        //@}

        //! @name Accessors
        //@{
        //! Implements checkHeader()
        virtual bool doCheckHeader() const =0;
        //! Implements ifdOffset()
        virtual uint32_t doIfdOffset() const =0;
        //! Implements getState(). The default implementation returns a 0-pointer.
        virtual TiffRwState::AutoPtr doGetState(uint32_t mnOffset) const;
        //@}

    private:
        // DATA
        TiffDirectory ifd_;                     //!< Makernote IFD
        
    }; // class TiffIfdMakernote

}                                       // namespace Exiv2

#endif                                  // #ifndef MAKERNOTE2_HPP_
