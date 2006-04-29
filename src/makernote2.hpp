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
  @brief   Makernote related classes
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

    namespace Group {
        const uint16_t olympmn  = 257; //!< Olympus makernote
        const uint16_t fujimn   = 258; //!< Fujifilm makernote
        const uint16_t canonmn  = 259; //!< Canon makernote
        const uint16_t canoncs  = 260; //!< Canon camera settings
        const uint16_t canonsi  = 261; //!< Canon shot info
        const uint16_t canoncf  = 262; //!< Canon customer functions
        const uint16_t nikonmn  = 263; //!< Any Nikon makernote (pseudo group)
        const uint16_t nikon1mn = 264; //!< Nikon1 makernote
        const uint16_t nikon2mn = 265; //!< Nikon2 makernote
        const uint16_t nikon3mn = 266; //!< Nikon3 makernote
        const uint16_t panamn   = 267; //!< Panasonic makernote
        const uint16_t sigmamn  = 268; //!< Sigma makernote
        const uint16_t sonymn   = 269; //!< Any Sony makernote (pseudo group)
        const uint16_t sony1mn  = 270; //!< Sony1 makernote
        const uint16_t sony2mn  = 271; //!< Sony2 makernote
    }

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
        static TiffComponent* create(uint16_t           tag,
                                     uint16_t           group,
                                     const std::string& make,
                                     const byte*        pData,
                                     uint32_t           size,
                                     ByteOrder          byteOrder);
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
        //! Read the header from a data buffer, return true if ok
        virtual bool read(const byte* pData, 
                          uint32_t    size,
                          ByteOrder   byteOrder) =0;
        //@}
        //! @name Accessors
        //@{
        //! Return the size of the header (in bytes).
        virtual uint32_t size() const =0;
        /*!
          @brief Return the offset to the start of the Makernote IFD from
                 the start of the Makernote (= the start of the header).
         */
        virtual uint32_t ifdOffset() const { return 0; }
 	/*!
          @brief Return the byte order for the makernote. If the return value is
                 invalidByteOrder, this means that the byte order of the the
                 image should be used for the makernote.
         */
        virtual ByteOrder byteOrder() const { return invalidByteOrder; }
        /*!
          @brief Return the base offset for the makernote IFD entries relative 
                 to the start of the TIFF header.

          @param mnOffset Offset to the makernote from the start of the
                 TIFF header.
         */
        virtual uint32_t baseOffset(uint32_t mnOffset) const { return 0; }
        //@}

    }; // class MnHeader

    /*!
      @brief Tiff IFD Makernote. This is a concrete class suitable for all 
             IFD makernotes.

             Contains a makernote header (which can be 0) and an IFD and
             implements child mgmt functions to deal with the IFD entries. The
             various makernote weirdnesses are taken care of in the makernote
             header.
     */
    class TiffIfdMakernote : public TiffComponent {
        friend class TiffReader;
    public:
        //! @name Creators
        //@{
        //! Default constructor
        TiffIfdMakernote(uint16_t  tag, 
                         uint16_t  group, 
                         uint16_t  mnGroup,
                         MnHeader* pHeader,
                         bool      hasNext =true)
            : TiffComponent(tag, group), 
              pHeader_(pHeader),
              ifd_(tag, mnGroup, hasNext) {}
        //! Virtual destructor
        virtual ~TiffIfdMakernote();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the header from a data buffer, return true if successful.

          The default implementation simply returns true.
         */
        bool readHeader(const byte* pData, uint32_t size, ByteOrder byteOrder);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Return the offset to the start of the Makernote IFD from
                 the start of the Makernote.
         */
        uint32_t ifdOffset() const;
 	/*!
          @brief Return the byte order for the makernote. Default (if there is
                 no header) is invalidByteOrder. This means that the byte order
                 of the the image should be used for the makernote.
         */
        ByteOrder byteOrder() const;
        /*!
          @brief Return the base offset for the makernote IFD entries relative 
                 to the start of the TIFF header. The default, if there is no 
                 header, is 0.

          @param mnOffset Offset to the makernote from the start of the
                 TIFF header.
         */
        uint32_t baseOffset (uint32_t mnOffset) const;
        //@}

    protected:
        //! @name Manipulators
        //@{
        virtual void doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual void doAddNext(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        //@}

    private:
        // DATA
        MnHeader*     pHeader_;                 //!< Makernote header
        TiffDirectory ifd_;                     //!< Makernote IFD
        
    }; // class TiffIfdMakernote

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
        virtual bool read(const byte* pData, 
                          uint32_t    size,
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size()      const { return header_.size_; }
        virtual uint32_t ifdOffset() const { return size_; }
        //@}

    private:
        DataBuf header_;                //!< Data buffer for the makernote header
        static const byte signature_[]; //!< Olympus makernote header signature
        static const uint32_t size_;    //!< Size of the signature

    }; // class OlympusMnHeader

    //! Header of a Fujifilm Makernote
    class FujiMnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        FujiMnHeader();
        //! Virtual destructor.
        virtual ~FujiMnHeader() {}
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData, 
                          uint32_t    size, 
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t  size()      const { return header_.size_; }
        virtual uint32_t  ifdOffset() const { return start_; }
        virtual ByteOrder byteOrder() const { return byteOrder_; }
        virtual uint32_t baseOffset(uint32_t mnOffset) const { return mnOffset; }
        //@}

    private:
        DataBuf header_;                //!< Data buffer for the makernote header
        static const byte signature_[]; //!< Fujifilm makernote header signature
        static const uint32_t size_;    //!< Size of the signature
        static const ByteOrder byteOrder_; //!< Byteorder for makernote (II)
        uint32_t start_;                //!< Start of the mn IFD rel. to mn start

    }; // class FujiMnHeader

    //! Header of a Nikon 2 Makernote
    class Nikon2MnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        Nikon2MnHeader();
        //! Virtual destructor.
        virtual ~Nikon2MnHeader() {}
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData, 
                          uint32_t    size, 
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size()      const { return size_; }
        virtual uint32_t ifdOffset() const { return start_; }
        //@}

    private:
        DataBuf buf_;                   //!< Raw header data
        uint32_t start_;                //!< Start of the mn IFD rel. to mn start
        static const byte signature_[]; //!< Nikon 2 makernote header signature
        static const uint32_t size_;    //!< Size of the signature

    }; // class Nikon2MnHeader

    //! Header of a Nikon 3 Makernote
    class Nikon3MnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        Nikon3MnHeader();
        //! Virtual destructor.
        virtual ~Nikon3MnHeader() {}
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData, 
                          uint32_t    size, 
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t  size()      const { return size_; }
        virtual uint32_t  ifdOffset() const { return start_; }
        virtual ByteOrder byteOrder() const { return byteOrder_; }
        virtual uint32_t  baseOffset(uint32_t mnOffset) const { return mnOffset + 10; }
        //@}

    private:
        DataBuf buf_;                   //!< Raw header data
        ByteOrder byteOrder_;           //!< Byteorder for makernote
        uint32_t start_;                //!< Start of the mn IFD rel. to mn start
        static const byte signature_[]; //!< Nikon 3 makernote header signature
        static const uint32_t size_;    //!< Size of the signature

    }; // class Nikon3MnHeader

    //! Header of a Panasonic Makernote
    class PanasonicMnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        PanasonicMnHeader();
        //! Virtual destructor.
        virtual ~PanasonicMnHeader() {}
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData, 
                          uint32_t    size, 
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size()      const { return size_; }
        virtual uint32_t ifdOffset() const { return start_; }
        //@}

    private:
        DataBuf buf_;                   //!< Raw header data
        uint32_t start_;                //!< Start of the mn IFD rel. to mn start
        static const byte signature_[]; //!< Panasonic makernote header signature
        static const uint32_t size_;    //!< Size of the signature

    }; // class PanasonicMnHeader

    //! Header of a Sigma Makernote
    class SigmaMnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        SigmaMnHeader();
        //! Virtual destructor.
        virtual ~SigmaMnHeader() {}
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData, 
                          uint32_t    size, 
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size()      const { return size_; }
        virtual uint32_t ifdOffset() const { return start_; }
        //@}

    private:
        DataBuf buf_;                    //!< Raw header data
        uint32_t start_;                 //!< Start of the mn IFD rel. to mn start
        static const byte signature1_[]; //!< Sigma makernote header signature 1
        static const byte signature2_[]; //!< Sigma makernote header signature 2
        static const uint32_t size_;     //!< Size of the signature

    }; // class SigmaMnHeader

    //! Header of a Sony Makernote
    class SonyMnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        SonyMnHeader();
        //! Virtual destructor.
        virtual ~SonyMnHeader() {}
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData, 
                          uint32_t    size, 
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size()      const { return size_; }
        virtual uint32_t ifdOffset() const { return start_; }
        //@}

    private:
        DataBuf buf_;                   //!< Raw header data
        uint32_t start_;                //!< Start of the mn IFD rel. to mn start
        static const byte signature_[]; //!< Sony makernote header signature
        static const uint32_t size_;    //!< Size of the signature

    }; // class SonyMnHeader

// *****************************************************************************
// template, inline and free functions

    //! Function to create a Canon makernote
    TiffComponent* newCanonMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* pData,
                              uint32_t    size, 
                              ByteOrder   byteOrder);

    //! Function to create an Olympus makernote
    TiffComponent* newOlympusMn(uint16_t    tag,
                                uint16_t    group,
                                uint16_t    mnGroup,
                                const byte* pData,
                                uint32_t    size, 
                                ByteOrder   byteOrder);

    //! Function to create a Fujifilm makernote
    TiffComponent* newFujiMn(uint16_t    tag,
                             uint16_t    group,
                             uint16_t    mnGroup,
                             const byte* pData,
                             uint32_t    size, 
                             ByteOrder   byteOrder);

    /*!
      @brief Function to create a Nikon makernote. This will create the 
             appropriate Nikon 1, 2 or 3 makernote, based on the arguments.
     */
    TiffComponent* newNikonMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* pData,
                              uint32_t    size, 
                              ByteOrder   byteOrder);

    //! Function to create a Panasonic makernote
    TiffComponent* newPanasonicMn(uint16_t    tag,
                                  uint16_t    group,
                                  uint16_t    mnGroup,
                                  const byte* pData,
                                  uint32_t    size, 
                                  ByteOrder   byteOrder);

    //! Function to create a Sigma makernote
    TiffComponent* newSigmaMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* pData,
                              uint32_t    size, 
                              ByteOrder   byteOrder);

    //! Function to create a Sony makernote
    TiffComponent* newSonyMn(uint16_t    tag,
                             uint16_t    group,
                             uint16_t    mnGroup,
                             const byte* pData,
                             uint32_t    size, 
                             ByteOrder   byteOrder);

}                                       // namespace Exiv2

#endif                                  // #ifndef MAKERNOTE2_HPP_
