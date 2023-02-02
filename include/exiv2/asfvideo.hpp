// SPDX-License-Identifier: GPL-2.0-or-later
// Spec : Advanced Systems Format (ASF) Specification : Revision 01.20.05 :
// https://exse.eyewated.com/fls/54b3ed95bbfb1a92.pdf
#pragma once

// *****************************************************************************
#include "exiv2lib_export.h"

// included header files
#include "image.hpp"

// *****************************************************************************
// namespace extensions
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
  //@}

  //! @name NOT Implemented
  //@{
  //! Copy constructor
  AsfVideo(const AsfVideo&) = delete;
  //! Assignment operator
  AsfVideo& operator=(const AsfVideo&) = delete;
  //@}

  //! @name Manipulators
  //@{
  void readMetadata() override;
  void writeMetadata() override;
  //@}

  //! @name Accessors
  //@{
  [[nodiscard]] std::string mimeType() const override;
  //@}
 private:
  static constexpr size_t CODEC_TYPE_VIDEO = 1;
  static constexpr size_t CODEC_TYPE_AUDIO = 2;

  class HeaderReader {
    DataBuf IdBuf_;
    uint64_t size_;
    uint64_t remaining_size_;

   public:
    explicit HeaderReader(BasicIo::UniquePtr& io);

    [[nodiscard]] uint64_t getSize() const {
      return size_;
    }

    [[nodiscard]] uint64_t getRemainingSize() const {
      return remaining_size_;
    }

    [[nodiscard]] DataBuf& getId() {
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

  void DegradableJPEGMedia();

  /*!
    @brief Calculates Aspect Ratio of a video, and stores it in the
        respective XMP container.
   */
  void aspectRatio();

 private:
  //! Variable to store height and width of a video frame.
  uint64_t height_, width_;

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
