// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2013 Andreas Huggel <ahuggel@gmx.net>
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
  @file    quicktimevideo.hpp
  @brief   An Image subclass to support Quick Time video files
  @version $Rev$
  @authors  Abhinav Badola for GSoC 2012
           <a href="mailto:mail.abu.to@gmail.com">mail.abu.to@gmail.com</a>
           Mahesh Hegde for GSoC 2013
           <a href="maheshmhegade at gmail dot com">maheshmhegade@gmail.com</a>
  @date    28-Jun-12, AB: created
 */
#ifndef QUICKTIMEVIDEO_HPP
#define QUICKTIMEVIDEO_HPP
#include <iostream>
using namespace std;
// *****************************************************************************
// included header files
#include "exif.hpp"
#include "image.hpp"
#include "tags_int.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

// Add qtime to the supported image formats
namespace ImageType {
const int qtime = 22; //!< Treating qtime as an image type>
}

/*!
      @brief Class to access QuickTime video files.
     */
class EXIV2API QuickTimeVideo:public Image
{
public:
    /*!
         * \brief The QuickAtom class:
         *Object of this class is intended to contain data about the premitive atom inside
         *any atom
         */
    class QuickAtom
    {
    public:
        QuickAtom(){}
        ~QuickAtom(){}
    public:
        byte m_AtomId[5];                     //!< Id of the Atom.
        unsigned long m_AtomLocation;         //!< Location of atom inside a file
        unsigned long m_AtomSize;             //!< Size of atom
    };
public:
    //! @name Creators
    //@{
    /*!
          @brief Constructor for a QuickTime video. Since the constructor
              can not return a result, callers should check the good() method
              after object construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method. Use the Image::io()
              method to get a temporary reference.
         */
    QuickTimeVideo(BasicIo::AutoPtr io);
    //@}

    ~QuickTimeVideo();
    //! @name Manipulators
    //@{
    void readMetadata();
    void writeMetadata();
    //@}

    //! @name Accessors
    //@{
    std::string mimeType() const;
    //@}

protected:
    /*!
          @brief Check for a valid tag and decode the block at the current IO
          position. Calls tagDecoder() or skips to next tag, if required.
         */
    void decodeBlock();
    /*!
          @brief Interpret tag information, and call the respective function
              to save it in the respective XMP container. Decodes a Tag
              Information and saves it in the respective XMP container, if
              the block size is small.
          @param buf Data buffer which cotains tag ID.
          @param size Size of the data block used to store Tag Information.
         */
    void tagDecoder(Exiv2::DataBuf & buf, uint32_t size);

private:
    /*!
          @brief Interpret file type of the video, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void fileTypeDecoder(uint32_t size);
    /*!
          @brief Interpret Media Header Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void mediaHeaderDecoder(uint32_t size);
    /*!
          @brief Interpret Video Header Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void videoHeaderDecoder(uint32_t size);
    /*!
          @brief Interpret Movie Header Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void movieHeaderDecoder(uint32_t size);
    /*!
          @brief Interpret Track Header Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void trackHeaderDecoder(uint32_t size);
    /*!
          @brief Interpret Handler Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void handlerDecoder(uint32_t size);
    /*!
          @brief Interpret Tag which contain other sub-tags,
              and save it in the respective XMP container.
         */
    void multipleEntriesDecoder(uint32_t iDummySize);
    /*!
          @brief Interpret Sample Description Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void sampleDesc(uint32_t size);
    /*!
          @brief Interpret Image Description Tag, and save it
              in the respective XMP container.
         */
    void imageDescDecoder();
    /*!
          @brief Interpret User Data Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void userDataDecoder(uint32_t size);
    /*!
          @brief Interpret Preview Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void previewTagDecoder(uint32_t size);
    /*!
          @brief Interpret Meta Keys Tags, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void keysTagDecoder(uint32_t size);
    /*!
          @brief Interpret Track Aperture Tags, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void trackApertureTagDecoder(uint32_t size);
    /*!
          @brief Interpret Nikon Tag, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void NikonTagsDecoder(uint32_t size);
    /*!
          @brief Interpret Tags from Different Camera make, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void CameraTagsDecoder(uint32_t size);
    /*!
          @brief Interpret Audio Description Tag, and save it
              in the respective XMP container.
         */
    void audioDescDecoder();
    /*!
          @brief Helps to calculate Frame Rate from timeToSample chunk,
              and save it in the respective XMP container.
         */
    void timeToSampleDecoder(uint32_t iDummySize);
    /*!
          @brief Recognizes which stream is currently under processing,
              and save its information in currentStream_ .
         */
    void setMediaStream(uint32_t iDummySize);
    /*!
          @brief Calculates Aspect Ratio of a video, and stores it in the
              respective XMP container.
         */
    void aspectRatio();
    /*!
         * \brief reverseTagDetails
         * \param inputTagVocabulary
         * \param outputTagVocabulary
         * \param size
         */
    void reverseTagDetails(const Internal::TagDetails inputTagVocabulary[],
                           Internal::RevTagDetails  outputTagVocabulary[] , int32_t size);

private:
    //! @name NOT Implemented
    //@{
    //! Copy constructor
    QuickTimeVideo(const QuickTimeVideo& rhs);
    //! Assignment operator
    QuickTimeVideo& operator=(const QuickTimeVideo& rhs);
    //@}
    /*!
              @brief Provides the main implementation of writeMetadata() by
                    writing all buffered metadata to the provided BasicIo.
              @param oIo BasicIo instance to write to (a temporary location).

              @return 4 if opening or writing to the associated BasicIo fails
             */
    EXV_DLLLOCAL void doWriteMetadata();
    /*!
         * \brief findAtomPositions
         * \param atomId
         * \return all the locations where atoms with atomId exist.
         */
    std::vector< pair<uint32_t,uint32_t> > findAtomPositions(const char *atomId);

    /*!
           * \brief writeStringData
           * \param xmpStringData
           * \param size
           * \param skipOffset
           */
    void writeStringData(Exiv2::Xmpdatum xmpStringData, int32_t size, int32_t skipOffset=0);

    /*!
           * \brief writeLongData
           * \param xmpIntData
           * \param size
           * \param skipOffset
           */
    void writeLongData(Exiv2::Xmpdatum xmpIntData, int32_t size=4, int32_t skipOffset=0);

    /*!
           * \brief writeShortData
           * \param xmpIntData
           * \param size
           * \param skipOffset
           */
    void writeShortData(Exiv2::Xmpdatum xmpIntData, int16_t size=2, int32_t skipOffset=0);

    /*!
     * \brief writeMultibyte
     * \param bRawData
     * \param size
     */
    bool writeMultibyte(Exiv2::byte * bRawData = NULL , int64_t iSize = 0, int64_t iOffset = 0);

    /*!
           * \brief writeApertureData
           * \param xmpIntData
           * \param size
           * \param skipOffset
           */
    void writeApertureData(Exiv2::Xmpdatum xmpIntData, int16_t size, int32_t skipOffset=0);

    /*!
     * \brief writeAudVidData
     * \param iTagType
     * \return
     */
    bool writeAudVidData(int64_t iTagType);

    /*!
     * \brief writeAudVidData
     * \param sXmpTag
     * \param iOffset
     * \return
     */
    bool writeAudVidData( std::string sXmpTag, double iMulFactor, int64_t iBytCnt, int64_t iOffset, int64_t iRetFuncCall);

    /*!
     * \brief readAudVidData
     * \param sTag
     * \param iNumRdBytes
     * \param dMulFact
     * \param iFunc2Call
     */
    void readAudVidData(std::string sTag, int32_t iNumRdBytes, double dMulFact ,int32_t iFunc2Call);

    void readAudVidStrData(std::string sTag, int32_t iNumRdBytes);
    void readStrData(std::string sTag, int32_t iNumRdBytes);

private:

    class Private;
    Private * const d;

}; //QuickTimeVideo End

// *****************************************************************************
// template, inline and free functions

// These could be static private functions on Image subclasses but then
// ImageFactory needs to be made a friend.
/*!
      @brief Create a new QuicktimeVideo instance and return an auto-pointer to it.
          Caller owns the returned object and the auto-pointer ensures that
          it will be deleted.
     */
EXIV2API Image::AutoPtr newQTimeInstance(BasicIo::AutoPtr io, bool create);

//! Check if the file iIo is a Quick Time Video.
EXIV2API bool isQTimeType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // QUICKTIMEVIDEO_HPP
