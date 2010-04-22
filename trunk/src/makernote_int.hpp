// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2010 Andreas Huggel <ahuggel@gmx.net>
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
  @brief   Makernote factory and registry, IFD makernote header, and camera
           vendor specific makernote implementations.<BR>References:<BR>
  [1] <a href="http://www.sno.phy.queensu.ca/~phil/exiftool/">ExifTool</a> by Phil Harvey<BR>
  [2] <a href="http://www.cybercom.net/~dcoffin/dcraw/">Decoding raw digital photos in Linux</a> by Dave Coffin
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    11-Apr-06, ahu: created
 */
#ifndef MAKERNOTE_INT_HPP_
#define MAKERNOTE_INT_HPP_

// *****************************************************************************
// included header files
#include "tifffwd_int.hpp"
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
        const uint16_t olympeq   = 284; //!< Olympus equipment tags
        const uint16_t olymprd   = 285; //!< Olympus raw development tags
        const uint16_t olymprd2  = 286; //!< Olympus raw development 2 tags
        const uint16_t olympip   = 287; //!< Olympus image processing tags
        const uint16_t olympfi   = 288; //!< Olympus focus info tags
        const uint16_t olympfe1  = 289; //!< Olympus FE 1 tags
        const uint16_t olympfe2  = 290; //!< Olympus FE 2 tags
        const uint16_t olympfe3  = 291; //!< Olympus FE 3 tags
        const uint16_t olympfe4  = 292; //!< Olympus FE 4 tags
        const uint16_t olympfe5  = 293; //!< Olympus FE 5 tags
        const uint16_t olympfe6  = 294; //!< Olympus FE 6 tags
        const uint16_t olympfe7  = 295; //!< Olympus FE 7 tags
        const uint16_t olympfe8  = 296; //!< Olympus FE 8 tags
        const uint16_t olympfe9  = 297; //!< Olympus FE 9 tags
        const uint16_t olympri   = 298; //!< Olympus raw info tags
        const uint16_t nikonwt   = 299; //!< Nikon World Time tags
        const uint16_t nikonii   = 300; //!< Nikon ISO Info tags
        const uint16_t nikonld1  = 301; //!< Nikon Lens Data 1 tags
        const uint16_t nikonld2  = 302; //!< Nikon Lens Data 2 tags
        const uint16_t nikonld3  = 303; //!< Nikon Lens Data 3 tags
        const uint16_t nikoncb1  = 304; //!< Nikon Color Balance 1 Tags
        const uint16_t nikoncb2  = 305; //!< Nikon Color Balance 2 Tags
        const uint16_t nikoncb2a = 306; //!< Nikon Color Balance 2 Tags
        const uint16_t nikoncb2b = 307; //!< Nikon Color Balance 2 Tags
        const uint16_t nikoncb3  = 308; //!< Nikon Color Balance 3 Tags
        const uint16_t nikoncb4  = 309; //!< Nikon Color Balance 4 Tags
        const uint16_t nikonvr   = 310; //!< Nikon Vibration Reduction Tags
        const uint16_t nikonpc   = 311; //!< Nikon Picture Control Tags
        const uint16_t nikonaf   = 312; //!< Nikon Auto Focus Tags
        const uint16_t nikonsi1  = 313; //!< Nikon Shot Info D80 Tags
        const uint16_t nikonsi2  = 314; //!< Nikon Shot Info D40 Tags
        const uint16_t nikonsi3  = 315; //!< Nikon Shot Info D300 Tags (a)
        const uint16_t nikonsi4  = 316; //!< Nikon Shot Info D300 Tags (b)
        const uint16_t nikonsi5  = 317; //!< Nikon Shot Info v2.* Tags
        const uint16_t nikonsi6  = 318; //!< Nikon Shot Info v1.* Tags
        const uint16_t canonfi   = 320; //!< Canon File Info
        const uint16_t sonymltmn = 330; //!< Sony Minolta Makernotes
        const uint16_t sony1cs   = 331; //!< Sony Camera Settings (in Sony1 makernote)
        const uint16_t sony1cs2  = 332; //!< Sony Camera Settings 2 (in Sony1 makernote)
        const uint16_t sony1mcso = 333; //!< Minolta (old) Camera Settings (in Sony1 makernote)
        const uint16_t sony1mcsn = 334; //!< Minolta (new) Camera Settings (in Sony1 makernote)
        const uint16_t sony1mcs5 = 335; //!< Minolta D5 Camera Settings (in Sony1 makernote)
        const uint16_t sony1mcs7 = 336; //!< Minolta D7 Camera Settings (in Sony1 makernote)
        const uint16_t sony2cs   = 337; //!< Sony Camera Settings (in Sony2 makernote)
        const uint16_t sony2cs2  = 338; //!< Sony Camera Settings 2 (in Sony2 makernote)
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
        virtual ~MnHeader();
        //@}
        //! @name Manipulators
        //@{
        //! Read the header from a data buffer, return true if ok
        virtual bool read(const byte* pData,
                          uint32_t    size,
                          ByteOrder   byteOrder) =0;
        /*!
          @brief Set the byte order for the makernote.
         */
        virtual void setByteOrder(ByteOrder byteOrder);
        //@}
        //! @name Accessors
        //@{
        //! Return the size of the header (in bytes).
        virtual uint32_t size() const =0;
        //! Write the header to a data buffer, return the number of bytes written.
        virtual uint32_t write(IoWrapper& ioWrapper,
                               ByteOrder  byteOrder) const =0;
        /*!
          @brief Return the offset to the start of the Makernote IFD from
                 the start of the Makernote (= the start of the header).
         */
        virtual uint32_t ifdOffset() const;
        /*!
          @brief Return the byte order for the makernote. If the return value is
                 invalidByteOrder, this means that the byte order of the the
                 image should be used for the makernote.
         */
        virtual ByteOrder byteOrder() const;
        /*!
          @brief Return the base offset for the makernote IFD entries relative
                 to the start of the TIFF header. \em mnOffset is the offset
                 to the makernote from the start of the TIFF header.
         */
        virtual uint32_t baseOffset(uint32_t /*mnOffset*/) const;
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
        virtual ~OlympusMnHeader();
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData,
                          uint32_t    size,
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size() const;
        virtual uint32_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
        virtual uint32_t ifdOffset() const;
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
        virtual ~Olympus2MnHeader();
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData,
                          uint32_t    size,
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size() const;
        virtual uint32_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
        virtual uint32_t ifdOffset() const;
        virtual uint32_t baseOffset(uint32_t mnOffset) const;
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
        virtual ~FujiMnHeader();
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData,
                          uint32_t    size,
                          ByteOrder   byteOrder);
        // setByteOrder not implemented
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t  size() const;
        virtual uint32_t  write(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
        virtual uint32_t  ifdOffset() const;
        virtual ByteOrder byteOrder() const;
        virtual uint32_t  baseOffset(uint32_t mnOffset) const;
        //@}

    private:
        DataBuf header_;                //!< Data buffer for the makernote header
        static const byte signature_[]; //!< Fujifilm makernote header signature
        static const uint32_t size_;    //!< Size of the signature
        static const ByteOrder byteOrder_; //!< Byteorder for makernote (always II)
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
        virtual ~Nikon2MnHeader();
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData,
                          uint32_t    size,
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size() const;
        virtual uint32_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
        virtual uint32_t ifdOffset() const;
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
        virtual ~Nikon3MnHeader();
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData,
                          uint32_t    size,
                          ByteOrder   byteOrder);
        virtual void setByteOrder(ByteOrder byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t  size()      const;
        virtual uint32_t  write(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
        virtual uint32_t  ifdOffset() const;
        virtual ByteOrder byteOrder() const;
        virtual uint32_t  baseOffset(uint32_t mnOffset) const;
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
        virtual ~PanasonicMnHeader();
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData,
                          uint32_t    size,
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size() const;
        virtual uint32_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
        virtual uint32_t ifdOffset() const;
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
        virtual ~PentaxMnHeader();
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData,
                          uint32_t    size,
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size() const;
        virtual uint32_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
        virtual uint32_t ifdOffset() const;
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
        virtual ~SigmaMnHeader();
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData,
                          uint32_t    size,
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size() const;
        virtual uint32_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
        virtual uint32_t ifdOffset() const;
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
        virtual ~SonyMnHeader();
        //@}
        //! @name Manipulators
        //@{
        virtual bool read(const byte* pData,
                          uint32_t    size,
                          ByteOrder   byteOrder);
        //@}
        //! @name Accessors
        //@{
        virtual uint32_t size() const;
        virtual uint32_t write(IoWrapper& ioWrapper, ByteOrder byteOrder) const;
        virtual uint32_t ifdOffset() const;
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

    /*!
      @brief Function to select cfg + def of the Sony Camera Settings complex binary array.

      @param tag Tag number of the binary array
      @param pData Pointer to the raw array data.
      @param size Size of the array data.
      @param pRoot Pointer to the root component of the TIFF tree.
      @return An index into the array set, -1 if no match was found.
     */
    int sonyCsSelector(uint16_t tag, const byte* pData, uint32_t size, TiffComponent* const pRoot);

    /*!
      @brief Function to select cfg + def of a Nikon complex binary array.

      @param tag Tag number of the binary array
      @param pData Pointer to the raw array data.
      @param size Size of the array data.
      @param pRoot Pointer to the root component of the TIFF tree.
      @return An index into the array set, -1 if no match was found.
     */
    int nikonSelector(uint16_t tag, const byte* pData, uint32_t size, TiffComponent* const pRoot);

    /*!
      @brief Encrypt and decrypt Nikon data.

      Checks the version of the Nikon data array and en/decrypts (portions of) it as
      needed. (The Nikon encryption algorithm is symmetric.)

      @note This function requires access to other components of the composite, it
            should only be called after all other components are read.

      @param tag Tag number of the binary array
      @param pData Pointer to the start of the data to en/decrypt.
      @param size Size of the data buffer.
      @param pRoot Pointer to the root element of the composite.
      @return En/decrypted data. Ownership of the memory is passed to the caller.
              The buffer may be empty in case no decryption was needed.
     */
    DataBuf nikonCrypt(uint16_t tag, const byte* pData, uint32_t size, TiffComponent* const pRoot);

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef MAKERNOTE_INT_HPP_
