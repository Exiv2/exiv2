// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "exif.hpp"
#include "exiv2lib_export.h"
#include "image.hpp"

namespace Exiv2 {

// *****************************************************************************
// class definitions

/*!
  @brief Class to access RIFF video files.
 */
class EXIV2API RiffVideo : public Image {
 public:
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
  explicit RiffVideo(BasicIo::UniquePtr io);
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

 protected:
  class HeaderReader {
    std::string id_;
    uint64_t size_ = 0;

   public:
    explicit HeaderReader(const BasicIo::UniquePtr& io);

    [[nodiscard]] uint64_t getSize() const {
      return size_;
    }

    [[nodiscard]] const std::string& getId() const {
      return id_;
    }
  };

  void readList(const HeaderReader& header_);

  void readChunk(const HeaderReader& header_);

  void decodeBlocks();

 private:
  static bool equal(const std::string& str1, const std::string& str2);

  /*!
  @brief Interpret MainAVIHeader (avih) structure, and save it in the respective XMP container.
  */
  void readAviHeader();

  /*!
  @brief Interpret stream header list element (strh), and save it in the respective XMP container.
  */
  void readStreamHeader();

  /*!
  @brief Interpret stream header list element (strf), and save it in the respective XMP container.
  @param size_ Size of the data block used to store Tag Information.
  */
  void readStreamFormat(uint64_t size_);

  /*!
  @brief Interpret Additional header data (strd), and save it in the respective XMP container.
  @param size_ Size of the data block used to store Tag Information.
 */
  void readStreamData(uint64_t size_) const;

  /*!
  @brief Interpret stream header list element (strn) , and save it in the respective XMP container.
  @param size_ Size of the data block used to store Tag Information.
 */
  void StreamName(uint64_t size_) const;
  /*!
  @brief Interpret INFO List Chunk, and save it in the respective XMP container.
  @param size_ Size of the data block used to store Tag Information.
 */
  void readInfoListChunk(uint64_t size_);

  /*!
  @brief Interpret Riff Stream Data tag information, and save it in the respective XMP container.
  The Movi - Lists contain Video, Audio, Subtitle and (secondary) index data. Those can be grouped into rec - Lists.
  @param size_ Size of the data block used to store Tag Information.
 */
  void readMoviList(uint64_t size_) const;
  /*!
  @brief Interpret Video Properties Header chunk, and save it in the respective XMP container.
  The video properties header identifies video signal properties associated with a digital video stream in an AVI file
  @param size_ Size of the data block used to store Tag Information.
 */
  void readVPRPChunk(uint64_t size_) const;
  /*!
  @brief Interpret Riff INdex Chunk, and save it in the respective XMP container.
  @param size_ Size of the data block used to store Tag Information.
 */
  void readIndexChunk(uint64_t size_) const;
  /*!
  @brief Interpret Riff Stream Chunk, and save it in the respective XMP container.
  @param size_ Size of the data block used to store Tag Information.
 */
  void readDataChunk(uint64_t size_) const;
  /*!
  @brief Interpret Junk Chunk and save it in the respective XMP container.
  @param size_ Size of the data block used to store Tag Information.
 */
  void readJunk(uint64_t size_) const;

  static std::string getStreamType(uint32_t stream);
  /*!
   @brief Calculates Duration of a video, and stores it in the respective XMP container.
   @param frame_rate Frame rate of the video.
   @param frame_count Total number of frames present in the video.
  */
  void fillDuration(double frame_rate, size_t frame_count);

  /*!
   @brief Calculates Aspect Ratio of a video, and stores it in the respective XMP container.
   @param width Width of the video.
   @param height Height of the video.
  */
  void fillAspectRatio(size_t width, size_t height);

  static constexpr auto CHUNK_HEADER_ICCP = "ICCP";
  static constexpr auto CHUNK_HEADER_EXIF = "EXIF";
  static constexpr auto CHUNK_HEADER_XMP = "XMP ";

  /* Chunk header names */
  static constexpr auto CHUNK_ID_MOVI = "MOVI";
  static constexpr auto CHUNK_ID_DATA = "DATA";
  static constexpr auto CHUNK_ID_HDRL = "HDRL";
  static constexpr auto CHUNK_ID_STRL = "STRL";
  static constexpr auto CHUNK_ID_LIST = "LIST";
  static constexpr auto CHUNK_ID_JUNK = "JUNK";
  static constexpr auto CHUNK_ID_AVIH = "AVIH";
  static constexpr auto CHUNK_ID_STRH = "STRH";
  static constexpr auto CHUNK_ID_STRF = "STRF";
  static constexpr auto CHUNK_ID_FMT = "FMT ";
  static constexpr auto CHUNK_ID_STRN = "STRN";
  static constexpr auto CHUNK_ID_STRD = "STRD";
  static constexpr auto CHUNK_ID_IDIT = "IDIT";
  static constexpr auto CHUNK_ID_INFO = "INFO";
  static constexpr auto CHUNK_ID_NCDT = "NCDT";
  static constexpr auto CHUNK_ID_ODML = "ODML";
  static constexpr auto CHUNK_ID_VPRP = "VPRP";
  static constexpr auto CHUNK_ID_IDX1 = "IDX1";

  int streamType_{};

};  // Class RiffVideo

/*
  @brief Create a new RiffVideo instance and return an auto-pointer to it.
      Caller owns the returned object and the auto-pointer ensures that
      it will be deleted.
 */
EXIV2API Image::UniquePtr newRiffInstance(BasicIo::UniquePtr io, bool create);

//! Check if the file iIo is a Riff Video.
EXIV2API bool isRiffType(BasicIo& iIo, bool advance);

}  // namespace Exiv2
