// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2021 Exiv2 authors
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
  @file    asfvideo.hpp
  @brief   An Image subclass to support ASF video files
  @author  Abhinav Badola for GSoC 2012
           <a href="mailto:mail.abu.to@gmail.com">mail.abu.to@gmail.com</a>
  @date    08-Aug-12, AB: created
 */
#ifndef ASFVIDEO_HPP
#define ASFVIDEO_HPP

// *****************************************************************************
#include "exiv2lib_export.h"

// included header files
#include "image.hpp"

namespace Exiv2 {

// *****************************************************************************
// class definitions
/*!
  @brief Class to access ASF video files.
 */
class EXIV2API AsfVideo : public Image {
 public:
  //! @name Creators
  //@{
  /*!
    @brief Constructor for a ASF video. Since the constructor
        can not return a result, callers should check the good() method
        after object construction to determine success or failure.
    @param io An auto-pointer that owns a BasicIo instance used for
        reading and writing image metadata. \b Important: The constructor
        takes ownership of the passed in BasicIo instance through the
        auto-pointer. Callers should not continue to use the BasicIo
        instance after it is passed to this method. Use the Image::io()
        method to get a temporary reference.
   */
  explicit AsfVideo(BasicIo::UniquePtr io);

  //! Copy constructor
  AsfVideo(const AsfVideo& rhs) = delete;
  //! Assignment operator
  AsfVideo& operator=(const AsfVideo& rhs) = delete;
  //@}

  //! @name Manipulators
  //@{
  void readMetadata() override;
  void writeMetadata() override;
  //@}

  //! @name Accessors
  //@{
  std::string mimeType() const override;
  //@}
 private:
  static constexpr size_t GUID_SIZE = 37;
  static constexpr size_t CODEC_TYPE_VIDEO = 1;
  static constexpr size_t CODEC_TYPE_AUDIO = 2;

  static constexpr size_t BYTE = 1;
  static constexpr size_t WCHAR = 2;
  static constexpr size_t WORD = 2;
  static constexpr size_t DWORD = 4;
  static constexpr size_t QWORD = 8;
  static constexpr size_t GUID = 16;

  class AsfObject {
    byte IdBuf_[GUID + 1];
    uint64_t size_;
    uint64_t remaining_size_;

   public:
    explicit AsfObject(BasicIo::UniquePtr& io) {
      DataBuf SizeBuf(QWORD + 1);

      io->read(IdBuf_, GUID);
      io->read(SizeBuf.data(), QWORD);

      size_ = Exiv2::getULongLong(SizeBuf.data(), littleEndian);
      remaining_size_ = size_ - GUID - QWORD;
    }

    [[nodiscard]] uint64_t getSize() const {
      return size_;
    }

    [[nodiscard]] uint64_t getRemainingSize() const {
      return remaining_size_;
    }

    [[nodiscard]] byte* getId() {
      return IdBuf_;
    }
  };

 protected:
  /*!
    @brief Check for a valid tag and decode the block at the current IO
    position. Calls tagDecoder() or skips to next tag, if required.
   */
  void decodeBlock();
  /*!
    @brief Interpret File_Properties tag information, and save it in
        the respective XMP container.
   */
  void fileProperties();
  /*!
    @brief Interpret Stream_Properties tag information, and save it
        in the respective XMP container.
   */
  void streamProperties();
  /*!
    @brief Interpret Codec_List tag information, and save it in
        the respective XMP container.
   */
  void codecList();
  /*!
    @brief Interpret Content_Description tag information, and save it
        in the respective XMP container.
    @param size Size of the data block used to store Tag Data.
   */
  void contentDescription();
  /*!
    @brief Interpret Extended_Stream_Properties tag information, and
        save it in the respective XMP container.
   */
  void extendedStreamProperties();
  /*!
    @brief Interpret Header_Extension tag information, and save it in
        the respective XMP container.
   */
  void headerExtension();
  /*!
    @brief Interpret Metadata, Extended_Content_Description,
        Metadata_Library tag information, and save it in the respective
        XMP container.
   */
  void extendedContentDescription();

  /*!
    @brief Calculates Aspect Ratio of a video, and stores it in the
        respective XMP container.
   */
  void aspectRatio();

 private:
  //! Variable to store height and width of a video frame.
  uint64_t height_, width_;

  [[nodiscard]] uint64_t readQWORDTag();
  [[nodiscard]] uint32_t readDWORDTag();
  [[nodiscard]] uint16_t readWORDTag();
  [[nodiscard]] std::string readStringWCHAR(uint16_t length);
  [[nodiscard]] std::string readString(uint16_t length);

};  // Class AsfVideo

// *****************************************************************************
// template, inline and free functions

// These could be static private functions on Image subclasses but then
// ImageFactory needs to be made a friend.
/*!
  @brief Create a new AsfVideo instance and return an auto-pointer to it.
      Caller owns the returned object and the auto-pointer ensures that
      it will be deleted.
 */
EXIV2API Image::UniquePtr newAsfInstance(BasicIo::UniquePtr io, bool create);

//! Check if the file iIo is a Windows Asf Video.
EXIV2API bool isAsfType(BasicIo& iIo, bool advance);

}  // namespace Exiv2

#endif  // #ifndef ASFVIDEO_HPP_