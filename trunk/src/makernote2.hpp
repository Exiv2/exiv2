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
  @brief   
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
#include "types.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations

    class TiffComponent;
    template<typename CreationPolicy> class TiffReader;

// *****************************************************************************
// class definitions

    namespace Group {
        const uint16_t olympmn = 257; //!< Olympus makernote
    }

    //! Type for a pointer to a function creating a makernote
    typedef TiffComponent* (*NewMnFct)(uint16_t    tag,
                                       uint16_t    group,
                                       const byte* pData, 
                                       uint32_t    size, 
                                       ByteOrder   byteOrder);

    //! Makernote registry
    struct TiffMnRegistry {
        struct Key;
        //! Compare a TiffMnRegistry structure with a TiffMnRegistry::Key
        bool operator==(const Key& key) const;

        // DATA
        const char* make_;                      //!< Camera make
        NewMnFct    newMnFct_;                  //!< Makernote create function
    };

    //! Search key for TIFF structure.
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
                          uint32_t    size) =0;
        //@}
        //! @name Accessors
        //@{
        //! Size of the header
        virtual uint32_t size() const =0;
        /*!
          @brief Start of the makernote directory relative to the start of the
                 header.
         */
        virtual uint32_t offset() const =0;
        //! Check the header, return true if ok
        virtual bool check() const =0;
        //@}

    }; // class MnHeader

    //! Header of an Olympus Makernote
    class OlympusMnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        OlympusMnHeader();
        //! Virtual destructor.
        virtual ~OlympusMnHeader() {}
        //@}
        //! @name Manipulators
        //@{
        virtual bool     read(const byte* pData, 
                              uint32_t    size);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size()   const { return header_.size_; }
        virtual uint32_t offset() const { return size_; }
        virtual bool     check()  const;
        //@}

    private:
        DataBuf header_;               //!< Data buffer for the makernote header
        static const char* signature_; //!< Olympus makernote header signature
        static const uint32_t size_;   //!< Size of the signature

    }; // class OlympusMnHeader

    /*!
      @brief Olympus Makernote
     */
    class TiffOlympusMn : public TiffComponent {
        template<typename CreationPolicy>
        friend class TiffReader;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffOlympusMn(uint16_t tag, uint16_t group) 
            : TiffComponent(tag, group), ifd_(tag, Group::olympmn) {}
        //! Virtual destructor
        virtual ~TiffOlympusMn() {}
        //@}

    private:
        //! @name Manipulators
        //@{
        virtual void doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAddNext(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        //@}

    private:
        // DATA
        OlympusMnHeader header_;                //!< Makernote header
        TiffDirectory ifd_;                     //!< Makernote IFD

    }; // TiffOlympusMn

// *****************************************************************************
// template, inline and free functions

    //! Function to create an Olympus makernote
    TiffComponent* newOlympusMn(uint16_t    tag,
                                uint16_t    group,
                                const byte* pData,
                                uint32_t    size, 
                                ByteOrder   byteOrder);

}                                       // namespace Exiv2

#endif                                  // #ifndef MAKERNOTE2_HPP_
