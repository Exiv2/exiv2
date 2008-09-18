// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
  @file    makernote_int.hpp
  @brief   Internal Makernote TIFF composite class TiffIfdMakernote and classes
           for various makernote headers.
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Apr-06, ahu: created
 */
#ifndef MAKERNOTE_INT_HPP_
#define MAKERNOTE_INT_HPP_

// *****************************************************************************
// included header files
#include "tiffcomposite_int.hpp"
#include "types.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {
    namespace Internal {

    namespace Group {
        const uint16_t olympmn   = 257; //!< any Olympus makernote
        const uint16_t fujimn    = 258; //!< Fujifilm makernote
        const uint16_t canonmn   = 259; //!< Canon makernote
        const uint16_t canoncs   = 260; //!< Canon camera settings
        const uint16_t canonsi   = 261; //!< Canon shot info
        const uint16_t canoncf   = 262; //!< Canon custom functions
        const uint16_t nikonmn   = 263; //!< Any Nikon makernote (pseudo group)
        const uint16_t nikon1mn  = 264; //!< Nikon1 makernote
        const uint16_t nikon2mn  = 265; //!< Nikon2 makernote
        const uint16_t nikon3mn  = 266; //!< Nikon3 makernote
        const uint16_t panamn    = 267; //!< Panasonic makernote
        const uint16_t sigmamn   = 268; //!< Sigma makernote
        const uint16_t sonymn    = 269; //!< Any Sony makernote (pseudo group)
        const uint16_t sony1mn   = 270; //!< Sony1 makernote
        const uint16_t sony2mn   = 271; //!< Sony2 makernote
        const uint16_t minoltamn = 272; //!< Minolta makernote
        const uint16_t minocso   = 273; //!< Minolta camera settings (old)
        const uint16_t minocsn   = 274; //!< Minolta camera settings (new)
        const uint16_t minocs5   = 275; //!< Minolta camera settings (D5)
        const uint16_t minocs7   = 276; //!< Minolta camera settings (D7)
        const uint16_t canonpi   = 277; //!< Canon picture info
        const uint16_t canonpa   = 278; //!< Canon panorama
        const uint16_t pentaxmn  = 279; //!< Pentax makernote
        const uint16_t nikonpv   = 280; //!< Nikon preview sub-IFD
        const uint16_t olymp1mn  = 281; //!< Olympus makernote
        const uint16_t olymp2mn  = 282; //!< Olympus II makernote
        const uint16_t olympcs   = 283; //!< Olympus camera settings
    }

// *****************************************************************************
// class definitions

    //! Type for a pointer to a function creating a makernote (image)
    typedef TiffComponent* (*NewMnFct)(uint16_t    tag,
                                       uint16_t    group,
                                       uint16_t    mnGroup,
                                       const byte* pData,
                                       uint32_t    size,
                                       ByteOrder   byteOrder);

    //! Type for a pointer to a function creating a makernote (group)
    typedef TiffComponent* (*NewMnFct2)(uint16_t   tag,
                                        uint16_t   group,
                                        uint16_t   mnGroup);

    //! Makernote registry structure
    struct TiffMnRegistry {
        struct MakeKey;
        /*!
          @brief Compare a TiffMnRegistry structure with a key being the make
                 string from the image. The two are equal if
                 TiffMnRegistry::make_ equals a substring of the key of the
                 same size. E.g., registry = "OLYMPUS",
                 key = "OLYMPUS OPTICAL CO.,LTD" (found in the image) match.
         */
        bool operator==(const std::string& key) const;

        //! Compare a TiffMnRegistry structure with a makernote group
        bool operator==(const uint16_t& key) const;

        // DATA
        const char* make_;                      //!< Camera make
        uint16_t    mnGroup_;                   //!< Group identifier
        NewMnFct    newMnFct_;                  //!< Makernote create function (image)
        NewMnFct2   newMnFct2_;                 //!< Makernote create function (group)
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
                 new component to \em tag and \em group. This method is used
                 when a makernote is parsed from the Exif block.
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
        /*!
          @brief Create the Makernote for a given group. This method is used
                 when a makernote is written back from Exif tags.
         */
        static TiffComponent* create(uint16_t           tag,
                                     uint16_t           group,
                                     uint16_t           mnGroup);

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
        //! Write the header to a data buffer, return the number of bytes written.
        virtual uint32_t write(Blob&     blob,
                               ByteOrder byteOrder) const =0;
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
                 to the start of the TIFF header. \em mnOffset is the offset
                 to the makernote from the start of the TIFF header.
         */
        virtual uint32_t baseOffset(uint32_t /*mnOffset*/) const { return 0; }
        //@}

    }; // class MnHeader

    /*!
      @brief Tiff IFD Makernote. This is a concrete class suitable for all
             IFD makernotes.

             Contains a makernote header (which can be 0) and an IFD and
             implements child mgmt functions to deal with the IFD entries. The
             various makernote weirdnesses are taken care of in the makernote
             header (and possibly in special purpose IFD entries).
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
        //! Return the size of the header in bytes.
        uint32_t sizeHeader() const;
        //! Write the header to a data buffer, return the number of bytes written.
        uint32_t writeHeader(Blob& blob, ByteOrder byteOrder) const;
        //@}
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
        virtual TiffComponent* doAddPath(uint16_t tag, TiffPath& tiffPath);
        virtual TiffComponent* doAddChild(TiffComponent::AutoPtr tiffComponent);
        virtual TiffComponent* doAddNext(TiffComponent::AutoPtr tiffComponent);
        virtual void doAccept(TiffVisitor& visitor);
        //@}

        //! @name Write support (Manipulators)
        //@{
        /*!
          @brief Implements write(). Write the Makernote header, TIFF directory,
                 values and additional data to the blob, return the number of
                 bytes written.
         */
        virtual uint32_t doWrite(Blob&     blob,
                                 ByteOrder byteOrder,
                                 int32_t   offset,
                                 uint32_t  valueIdx,
                                 uint32_t  dataIdx,
                                 uint32_t& imageIdx);
        //@}
        //! @name Write support (Accessors)
        //@{
        /*!
          @brief This class does not really implement writeData(), it only has
                 write(). This method must not be called; it commits suicide.
         */
        virtual uint32_t doWriteData(Blob&     blob,
                                     ByteOrder byteOrder,
                                     int32_t   offset,
                                     uint32_t  dataIdx,
                                     uint32_t& imageIdx) const;
        /*!
          @brief Implements writeImage(). Write the image data of the IFD of
                 the Makernote. Return the number of bytes written.
         */
        virtual uint32_t doWriteImage(Blob&     blob,
                                      ByteOrder byteOrder) const;
        /*!
          @brief Implements size(). Return the size of the Makernote header,
                 TIFF directory, values and additional data.
         */
        virtual uint32_t doSize() const;
        /*!
          @brief Implements count(). Return the number of entries in the IFD
                 of the Makernote. Does not count entries which are marked as
                 deleted.
         */
        virtual uint32_t doCount() const;
        /*!
          @brief This class does not really implement sizeData(), it only has
                 size(). This method must not be called; it commits suicide.
         */
        virtual uint32_t doSizeData() const;
        /*!
          @brief Implements sizeImage(). Return the total image data size of the
                 makernote IFD.
         */
        virtual uint32_t doSizeImage() const;
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
        virtual uint32_t write(Blob& blob, ByteOrder byteOrder) const;
        virtual uint32_t ifdOffset() const { return size_; }
        //@}

    private:
        DataBuf header_;                //!< Data buffer for the makernote header
        static const byte signature_[]; //!< Olympus makernote header signature
        static const uint32_t size_;    //!< Size of the signature

    }; // class OlympusMnHeader

    //! Header of an Olympus II Makernote
    class Olympus2MnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        Olympus2MnHeader();
        //! Virtual destructor.
        virtual ~Olympus2MnHeader() {}
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
        virtual uint32_t write(Blob& blob, ByteOrder byteOrder) const;
        virtual uint32_t ifdOffset() const { return size_; }
        virtual uint32_t baseOffset(uint32_t mnOffset) const { return mnOffset; }
        //@}

    private:
        DataBuf header_;                //!< Data buffer for the makernote header
        static const byte signature_[]; //!< Olympus makernote header signature
        static const uint32_t size_;    //!< Size of the signature

    }; // class Olympus2MnHeader

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
        virtual uint32_t  write(Blob& blob, ByteOrder byteOrder) const;
        virtual uint32_t  ifdOffset() const { return start_; }
        virtual ByteOrder byteOrder() const { return byteOrder_; }
        virtual uint32_t  baseOffset(uint32_t mnOffset) const { return mnOffset; }
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
        virtual uint32_t write(Blob& blob, ByteOrder byteOrder) const;
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
        virtual uint32_t  write(Blob& blob, ByteOrder byteOrder) const;
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
        virtual uint32_t write(Blob& blob, ByteOrder byteOrder) const;
        virtual uint32_t ifdOffset() const { return start_; }
        //@}

    private:
        DataBuf buf_;                   //!< Raw header data
        uint32_t start_;                //!< Start of the mn IFD rel. to mn start
        static const byte signature_[]; //!< Panasonic makernote header signature
        static const uint32_t size_;    //!< Size of the signature

    }; // class PanasonicMnHeader

    //! Header of an Pentax Makernote
    class PentaxMnHeader : public MnHeader {
    public:
        //! @name Creators
        //@{
        //! Default constructor
        PentaxMnHeader();
        //! Virtual destructor.
        virtual ~PentaxMnHeader() {}
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
        virtual uint32_t write(Blob& blob, ByteOrder byteOrder) const;
        virtual uint32_t ifdOffset() const { return size_; }
        //@}

    private:
        DataBuf header_;                //!< Data buffer for the makernote header
        static const byte signature_[]; //!< Pentax makernote header signature
        static const uint32_t size_;    //!< Size of the signature

    }; // class PentaxMnHeader

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
        virtual uint32_t write(Blob& blob, ByteOrder byteOrder) const;
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
        virtual uint32_t write(Blob& blob, ByteOrder byteOrder) const;
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

    //! Function to create a simple IFD makernote (Canon, Minolta, Nikon1)
    TiffComponent* newIfdMn(uint16_t    tag,
                            uint16_t    group,
                            uint16_t    mnGroup,
                            const byte* pData,
                            uint32_t    size,
                            ByteOrder   byteOrder);

    //! Function to create a simple IFD makernote (Canon, Minolta, Nikon1)
    TiffComponent* newIfdMn2(uint16_t tag,
                             uint16_t group,
                             uint16_t mnGroup);

    //! Function to create an Olympus makernote
    TiffComponent* newOlympusMn(uint16_t    tag,
                                uint16_t    group,
                                uint16_t    mnGroup,
                                const byte* pData,
                                uint32_t    size,
                                ByteOrder   byteOrder);

    //! Function to create an Olympus makernote
    TiffComponent* newOlympusMn2(uint16_t tag,
                                 uint16_t group,
                                 uint16_t mnGroup);

    //! Function to create an Olympus II makernote
    TiffComponent* newOlympus2Mn2(uint16_t tag,
                                 uint16_t group,
                                 uint16_t mnGroup);

    //! Function to create a Fujifilm makernote
    TiffComponent* newFujiMn(uint16_t    tag,
                             uint16_t    group,
                             uint16_t    mnGroup,
                             const byte* pData,
                             uint32_t    size,
                             ByteOrder   byteOrder);

    //! Function to create a Fujifilm makernote
    TiffComponent* newFujiMn2(uint16_t tag,
                              uint16_t group,
                              uint16_t mnGroup);

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

    //! Function to create a Nikon2 makernote
    TiffComponent* newNikon2Mn2(uint16_t tag,
                                uint16_t group,
                                uint16_t mnGroup);

    //! Function to create a Nikon3 makernote
    TiffComponent* newNikon3Mn2(uint16_t tag,
                                uint16_t group,
                                uint16_t mnGroup);

    //! Function to create a Panasonic makernote
    TiffComponent* newPanasonicMn(uint16_t    tag,
                                  uint16_t    group,
                                  uint16_t    mnGroup,
                                  const byte* pData,
                                  uint32_t    size,
                                  ByteOrder   byteOrder);

    //! Function to create a Panasonic makernote
    TiffComponent* newPanasonicMn2(uint16_t tag,
                                   uint16_t group,
                                   uint16_t mnGroup);

    //! Function to create an Pentax makernote
    TiffComponent* newPentaxMn(uint16_t    tag,
                               uint16_t    group,
                               uint16_t    mnGroup,
                               const byte* pData,
                               uint32_t    size,
                               ByteOrder   byteOrder);

    //! Function to create an Pentax makernote
    TiffComponent* newPentaxMn2(uint16_t tag,
                                uint16_t group,
                                uint16_t mnGroup);

    //! Function to create a Sigma makernote
    TiffComponent* newSigmaMn(uint16_t    tag,
                              uint16_t    group,
                              uint16_t    mnGroup,
                              const byte* pData,
                              uint32_t    size,
                              ByteOrder   byteOrder);

    //! Function to create a Sigma makernote
    TiffComponent* newSigmaMn2(uint16_t tag,
                               uint16_t group,
                               uint16_t mnGroup);

    //! Function to create a Sony makernote
    TiffComponent* newSonyMn(uint16_t    tag,
                             uint16_t    group,
                             uint16_t    mnGroup,
                             const byte* pData,
                             uint32_t    size,
                             ByteOrder   byteOrder);

    //! Function to create a Sony1 makernote
    TiffComponent* newSony1Mn2(uint16_t tag,
                               uint16_t group,
                               uint16_t mnGroup);

    //! Function to create a Sony2 makernote
    TiffComponent* newSony2Mn2(uint16_t tag,
                               uint16_t group,
                               uint16_t mnGroup);

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef MAKERNOTE_INT_HPP_
