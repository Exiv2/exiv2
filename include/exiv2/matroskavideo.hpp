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

/*!
  @brief Helper structure for the Matroska tags lookup table.
 */

// *****************************************************************************
// class definitions
namespace Internal {

enum matroskaTypeEnum : char {
  String = 's',
  Integer = 'i',
  UInteger = 'u',
  Date = 'd',
  InternalField = 'n',
  Boolean = 'o',
  Binary = 'b',
  Master = 'm',
  Float = 'f',
  Utf8 = '8',
  UndefinedType = 'z'

};

enum matroskaProcessEnum : char { Process = 'p', Skip = 's', Composite = 'c', Undefined = 'u' };

struct MatroskaTag {
  uint64_t _id;
  std::string _label;
  matroskaTypeEnum _type;
  matroskaProcessEnum _process;

  MatroskaTag(uint64_t id, const std::string& label, matroskaTypeEnum type, matroskaProcessEnum process) :
      _id(id), _label(label), _type(type), _process(process) {
  }

  MatroskaTag(uint64_t id, const std::string& label) :
      _id(id), _label(label), _type(matroskaTypeEnum::UndefinedType), _process(matroskaProcessEnum::Undefined) {
  }

  bool isSkipped() const {
    return _process == Skip;
  }
  bool isComposite() const {
    return _process == Composite;
  }
  void dump(std::ostream& os) const {
    os << " MatroskaTag "
       << " id: [0x" << std::hex << _id << "] label:[" << _label << "] type:[" << _type << "] process :[" << _process
       << "]\n";
  }
};

/// @brief  Utility function to search into std::array of pairs
/// @return the searched pair if exist,else nullptr
template <size_t N>
[[nodiscard]] const MatroskaTag* findTag(const std::array<MatroskaTag, N>& src, const uint64_t& key) {
  const auto rc = std::find_if(src.begin(), src.end(), [&key](const MatroskaTag& tag) { return tag._id == key; });
  // the return value is of type "const MatroskaTag*", so we return the adress of the content of the input
  // iterator return by find_if
  return rc == std::end(src) ? nullptr : &(*rc);
}

}  // namespace Internal

/*!
  @brief Class to access Matroska video files.
 */
class EXIV2API MatroskaVideo : public Image {
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
  explicit MatroskaVideo(BasicIo::UniquePtr io);

  //! Copy constructor
  MatroskaVideo(const MatroskaVideo&) = delete;
  //! Assignment operator
  MatroskaVideo& operator=(const MatroskaVideo&) = delete;
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
  /*!
    @brief Function used to calulate the size of a block.
        This information is only stored in one byte.
        The size of the block is calculated by counting
        the number of leading zeros in the binary code of the byte.
        Size = (No. of leading zeros + 1) bytes
    @param b The byte, which stores the information to calculate the size
    @return Return the size of the block.
   */
  [[nodiscard]] uint32_t findBlockSize(byte b);
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

  void decodeInternalTags(const Internal::MatroskaTag* tag, const byte* buf, size_t size);
  void decodeStringTags(const Internal::MatroskaTag* tag, const byte* buf);
  void decodeIntegerTags(const Internal::MatroskaTag* tag, const byte* buf, size_t size);
  void decodeBooleanTags(const Internal::MatroskaTag* tag, const byte* buf, size_t size);
  void decodeDateTags(const Internal::MatroskaTag* tag, const byte* buf, size_t size);
  void decodeFloatTags(const Internal::MatroskaTag* tag, const byte* buf, size_t size);
  /*!Internal::
    @brief Calculates Aspect Ratio of a video, and stores it in the
        respective XMP container.
   */
  void aspectRatio();

 private:
  //! Variable to check the end of metadata traversing.
  bool continueTraversing_;
  //! Variable to store height and width of a video frame.
  uint64_t height_;
  uint64_t width_;
  uint32_t track_count_;
  double time_code_scale_ = 1.0;
  uint64_t stream_ = 0;

  static constexpr double bytesMB = 1048576;

};  // class MatroskaVideo

// *****************************************************************************
/*!
  @brief Create a new MatroskaVideo instance and return an auto-pointer to it.
      Caller owns the returned object and the auto-pointer ensures that
      it will be deleted.
 */
EXIV2API Image::UniquePtr newMkvInstance(BasicIo::UniquePtr io, bool create);

//! Check if the file iIo is a Matroska Video.
EXIV2API bool isMkvType(BasicIo& iIo, bool advance);

}  // namespace Exiv2

#endif  // #ifndef MATROSKAVIDEO_HPP_
