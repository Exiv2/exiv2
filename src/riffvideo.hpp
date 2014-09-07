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
  @file    riffvideo.hpp
  @brief   An Image subclass to support RIFF video files
  @version $Rev$
  @authors Abhinav Badola for GSoC 2012
           <a href="mailto:mail.abu.to@gmail.com">mail.abu.to@gmail.com</a>

           Mahesh Hegde for GSoC 2013
           <b href="mailto:maheshmhegade@gmail.com">maheshmhegade@gmail.com</b>
  @date    18-Jun-12, AB: created
 */
#ifndef RIFFVIDEO_HPP
#define RIFFVIDEO_HPP

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

// Add RIFF to the supported image formats
namespace ImageType {
const int riff = 20; //!< Treating riff as an image type>
}

using namespace std;
/*!
      @brief Class to access RIFF video files.
     */
class EXIV2API RiffVideo:public Image
{
public:

    //List, Header,Primitive and Junk chunks inside m_riffFileSkeleton reveal entire RIFF structure
    /*!
     * \brief The PrimitiveChunk class Hold the information about a unit chunk.
     */
    class PrimitiveChunk
    {
    public:
        PrimitiveChunk(){}
        ~PrimitiveChunk(){}

    public:
        byte m_chunkId[5];                   //!< Primitive chunk ID.
        uint64_t m_chunkLocation;            //!< Chunk location in afile.
        uint64_t m_chunkSize;                //!< Chunk size.
    };

    /*!
     * \brief The HeaderChunk class Hold the information about Header chunk
     *        Containing multiple primitive chunks.
     */
    class HeaderChunk
    {
    public:
        HeaderChunk(){}
        ~HeaderChunk(){}

    public:
        byte m_headerId[5];                      //!< Chunk Header Id
        unsigned long m_headerLocation;          //!< Header Chunk Location in a file.
        unsigned long m_headerSize;              //!< Header Chunk size
    };

    /*!
     * \brief The RiffMetaSkeleton class Hold information about all the
     *        chunks present in a file.
     */
    class RiffMetaSkeleton
    {
    public:
        RiffMetaSkeleton(){}
        ~RiffMetaSkeleton(){}

    public:
        vector<HeaderChunk*> m_headerChunks;         //!< vector containing all Header chunks.
        vector<PrimitiveChunk*> m_primitiveChunks;   //!< vector Containing all the primitve chunks
    };

    /*!
     * \brief The IoPosition enum reveal the position,while traversing the file.
     */
    enum IoPosition
    {
        LastChunk,           //!< "MOVI" or "DATA"
        TraversingChunk,     //!< "AVI" "hdrl" etc which further have sub-chunks
        PremitiveChunk       //!< Remaining chunks which do not have subchunk
    };
    //! @name Creators
    //@{
    /*!
          @brief Constructor for a Riff video. Since the constructor
              can not return a result, callers should check the good() method
              after object construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method. Use the Image::io()
              method to get a temporary reference.
         */
    RiffVideo(BasicIo::AutoPtr io);
    //@}

    ~RiffVideo();

    //! @name Manipulators
    //@{
    void readMetadata();
    void writeMetadata();
    //@}

    //! @name Accessors
    //@{
    std::string mimeType() const;
    const char* printAudioEncoding(uint64_t i);
    //@}

protected:
    /*!
          @brief Check for a valid tag and decode the block at the current IO
          position. Calls tagDecoder() or skips to next tag, if required.
         */
    void decodeBlock();
    /*!
     * \brief tagDecoder Main method to call corresponding handler methos for decoding tags.
     */
    void tagDecoder();
    /*!
          @brief Interpret Junk tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void junkHandler(int32_t size);

    /*! find whether stream is audio or video
        */
    void setStreamType();
    /*!
          @brief Interpret Stream tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void streamHandler(int32_t size);
    /*!
          @brief Interpret Stream Format tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void streamFormatHandler(int32_t size);
    /*!
          @brief Interpret Riff Header tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void aviHeaderTagsHandler(int32_t size);
    /*!
          @brief Interpret Riff List tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void listHandler(long size);
    /*!
          @brief Interpret Riff Stream Data tag information, and save it
              in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
         */
    void streamDataTagHandler(int32_t size);
    /*!
          @brief Interpret INFO tag information, and save it
              in the respective XMP container.
         */
    void infoTagsHandler();
    /*!
          @brief Interpret Nikon Tags related to Video information, and
              save it in the respective XMP container.
         */
    void nikonTagsHandler();
    /*!
          @brief Interpret OpenDML tag information, and save it
              in the respective XMP container.
         */
    void odmlTagsHandler();
    //! @brief Skips Particular Blocks of Metadata List.
    void skipListData();
    /*!
          @brief Interprets DateTimeOriginal tag or stream name tag
              information, and save it in the respective XMP container.
          @param size Size of the data block used to store Tag Information.
          @param i parameter used to overload function
         */
    void dateTimeOriginal(int32_t size, int32_t i = 0);
    /*!
          @brief Calculates Sample Rate of a particular stream.
          @param buf Data buffer with the dividend.
          @param divisor The Divisor required to calculate sample rate.
          @return Return the sample rate of the stream.
         */
    double returnSampleRate(Exiv2::DataBuf& buf, int32_t divisor = 1);
    /*!
          @brief Calculates Aspect Ratio of a video, and stores it in the
              respective XMP container.
          @param width Width of the video.
          @param height Height of the video.
         */
    void fillAspectRatio(int32_t width = 1, int32_t height = 1);
    /*!
          @brief Calculates Duration of a video, and stores it in the
              respective XMP container.
          @param frame_rate Frame rate of the video.
          @param frame_count Total number of frames present in the video.
         */
    void fillDuration(double frame_rate, int32_t frame_count);

    /*!
     * \brief copyRestOfTheFile copy rest of the file content as it is
     * \param oldSavedData saved data from file, where data is already written into the file.
     * \return
     */
    bool copyRestOfTheFile(DataBuf oldSavedData);

    /*!
     * \brief reverseTagDetails reverse the position of members of a structure
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
    RiffVideo(const RiffVideo& rhs);
    //! Assignment operator
    RiffVideo& operator=(const RiffVideo& rhs);
    //@}
    /*!
          @brief Provides the main implementation of writeMetadata() by
                writing all buffered metadata to the provided BasicIo.
          @param oIo BasicIo instance to write to (a temporary location).

          @return 4 if opening or writing to the associated BasicIo fails
         */
    EXV_DLLLOCAL void doWriteMetadata();

    /*!
     * \brief findChunkPositions Finds the primitive shunk position in a file.
     * \param chunkId Id of chunk to be searched
     * \return
     */
    std::vector<int32_t> findChunkPositions(const char *chunkId);

    /*!
     * \brief findHeaderPositions Finds the location of Header chunk in a file.
     * \param headerId Id of Header Chunk to be searched.
     * \return
     */
    std::vector<int32_t> findHeaderPositions(const char* headerId);

    /*!
     * \brief writeNewSubChunks Writes the new chunk,also hold the data from overwritten
     *                          location in a file.
     * \param chunkData vector containing chunk ID and chunk Data
     * \return
     */
    bool writeNewSubChunks(std::vector<std::pair<std::string,std::string> > chunkData);

    /*!
     * \brief writeStringData
     * \param stringData
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
     * \brief writeFloatData
     * \param xmpIntData
     * \param size
     * \param skipOffset
     */
    void writeFloatData(Exiv2::Xmpdatum xmpIntData, int32_t size=4, int32_t skipOffset=0);

    /*!
     * \brief writeShortData
     * \param xmpIntData
     * \param size
     * \param skipOffset
     */
    void writeShortData(Exiv2::Xmpdatum xmpIntData, int16_t size=2, int32_t skipOffset=0);

private:

    // Good idea to use private(Refer KDE documentaion https://techbase.kde.org/Policies/Binary_Compatibility_Issues_With_C++ )
    class Private;
    Private* const d;

}; //Class RiffVideo

// *****************************************************************************
// template, inline and free functions

// These could be static private functions on Image subclasses but then
// ImageFactory needs to be made a friend.
/*!
      @brief Create a new RiffVideo instance and return an auto-pointer to it.
          Caller owns the returned object and the auto-pointer ensures that
          it will be deleted.
     */
EXIV2API Image::AutoPtr newRiffInstance(BasicIo::AutoPtr io, bool create);

//! Check if the file iIo is a Riff Video.
EXIV2API bool isRiffType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // RIFFVIDEO_HPP
