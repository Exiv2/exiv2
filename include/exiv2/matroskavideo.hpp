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
#ifndef MATROSKAVIDEO_HPP_
#define MATROSKAVIDEO_HPP_

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
  @brief Helper structure for the Matroska tags lookup table.
 */
using MatroskaTags = std::pair<uint64_t /*Tag value*/, const char* /*Translation of the tag value*/>;

/*!
  @brief Class to access Matroska video files.
 */
class MatroskaVideo : public Image {
 public:
  //! @name Creators
  //@{
  /*!
    @brief Constructor for a Matroska video. Since the constructor
        can not return a result, callers should check the good() method
        after object construction to determine success or failure.
    @param io An auto-pointer that owns a BasicIo instance used for
        reading and writing image metadata. \b Important: The constructor
        takes ownership of the passed in BasicIo instance through the
        auto-pointer. Callers should not continue to use the BasicIo
        instance after it is passed to this method. Use the Image::io()
        method to get a temporary reference.
   */
  MatroskaVideo(BasicIo::UniquePtr io);
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

 protected:
  /*!
    @brief Function used to calulate the size of a block.
        This information is only stored in one byte.
        The size of the block is calculated by counting
        the number of leading zeros in the binary code of the byte.
        Size = (No. of leading zeros + 1) bytes
    @param b The byte, which stores the information to calculate the size
    @return Return the size of the block.
   */
  uint32_t findBlockSize(byte b);
  /*!
    @brief Check for a valid tag and decode the block at the current IO position.
        Calls contentManagement() or skips to next tag, if required.
   */
  void decodeBlock();
  /*!
    @brief Interpret tag information, and save it in the respective XMP container.
    @param mt Pointer to current tag,
    @param buf Pointer to the memory area with the tag information.
    @param size Size of \em buf.
   */
  void contentManagement(const MatroskaTags* mt, const byte* buf, long size);
  /*!
    @brief Calculates Aspect Ratio of a video, and stores it in the
        respective XMP container.
   */
  void aspectRatio();

 private:
  //! Copy constructor
  MatroskaVideo(const MatroskaVideo& rhs);
  //! Assignment operator
  MatroskaVideo& operator=(const MatroskaVideo& rhs);
  //@}

 private:
  //! Variable to check the end of metadata traversing.
  bool continueTraversing_;
  //! Variable to store height and width of a video frame.
  uint64_t height_, width_;

};  // class MatroskaVideo

// *****************************************************************************
/*!
  @brief Create a new MatroskaVideo instance and return an auto-pointer to it.
      Caller owns the returned object and the auto-pointer ensures that
      it will be deleted.
 */
Image::UniquePtr newMkvInstance(BasicIo::UniquePtr io, bool create);

//! Check if the file iIo is a Matroska Video.
bool isMkvType(BasicIo& iIo, bool advance);

}  // namespace Exiv2

#endif  // #ifndef MATROSKAVIDEO_HPP_