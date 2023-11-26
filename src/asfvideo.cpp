// SPDX-License-Identifier: GPL-2.0-or-later
// included header files
#include "asfvideo.hpp"

#include <cstring>
#include <iostream>
#include <sstream>

#include "basicio.hpp"
#include "config.h"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "helper_functions.hpp"
#include "image_int.hpp"
#include "utils.hpp"
// *****************************************************************************
// class member definitions
namespace Exiv2 {

/*!
  Look-up list for ASF Type Video Files
  Associates the GUID with its Name(i.e. Human Readable Form)
  Tags have been differentiated into Various Categories.
  The categories have been listed above Groups
  see :
  - https://fr.wikipedia.org/wiki/Advanced_Systems_Format
  - https://exse.eyewated.com/fls/54b3ed95bbfb1a92.pdf
 */
/*
 * @class GUID_struct
 *
 * @brief A class to represent a globally unique identifier (GUID) structure
 *
 * This class represents a globally unique identifier (GUID) structure which is used to identify objects in a
 * distributed environment. A GUID is a unique identifier that is generated on a computer and can be used to
 * identify an object across different systems. The GUID structure is comprised of four 32-bit values and an
 * array of 8 bytes.
 *
 * @note The byte order of the GUID structure is in little endian.
 *
 * @see https://en.wikipedia.org/wiki/Globally_unique_identifier
 *
 */

bool AsfVideo::GUIDTag::operator==(const AsfVideo::GUIDTag& other) const {
  return data1_ == other.data1_ && data2_ == other.data2_ && data3_ == other.data3_ && data4_ == other.data4_;
}

AsfVideo::GUIDTag::GUIDTag(const uint8_t* bytes) {
  std::memcpy(&data1_, bytes, DWORD);
  std::memcpy(&data2_, bytes + DWORD, WORD);
  std::memcpy(&data3_, bytes + DWORD + WORD, WORD);
  std::copy(bytes + QWORD, bytes + (2 * QWORD), data4_.begin());
  if (isBigEndianPlatform()) {
    data1_ = byteSwap(data1_, true);
    data2_ = byteSwap(data2_, true);
    data3_ = byteSwap(data3_, true);
  }
}

std::string AsfVideo::GUIDTag::to_string() const {
  // Concatenate all strings into a single string
  // Convert the string to uppercase
  // Example of output 399595EC-8667-4E2D-8FDB-98814CE76C1E
  return stringFormat("{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}", data1_, data2_, data3_,
                      data4_[0], data4_[1], data4_[2], data4_[3], data4_[4], data4_[5], data4_[6], data4_[7]);
}

bool AsfVideo::GUIDTag::operator<(const GUIDTag& other) const {
  if (data1_ != other.data1_)
    return data1_ < other.data1_;
  if (data2_ != other.data2_)
    return data2_ < other.data2_;
  if (data3_ != other.data3_)
    return data3_ < other.data3_;
  return std::lexicographical_compare(data4_.begin(), data4_.end(), other.data4_.begin(), other.data4_.end());
}

constexpr AsfVideo::GUIDTag Header(0x75B22630, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C});

const std::map<AsfVideo::GUIDTag, std::string> GUIDReferenceTags = {
    //!< Top-level ASF object GUIDS
    {Header, "Header"},
    {{0x75B22636, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}}, "Data"},
    {{0x33000890, 0xE5B1, 0x11CF, {0x89, 0xF4, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB}}, "Simple_Index"},
    {{0xD6E229D3, 0x35DA, 0x11D1, {0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE}}, "Index"},
    {{0xFEB103F8, 0x12AD, 0x4C64, {0x84, 0x0F, 0x2A, 0x1D, 0x2F, 0x7A, 0xD4, 0x8C}}, "Media_Index"},
    {{0x3CB73FD0, 0x0C4A, 0x4803, {0x95, 0x3D, 0xED, 0xF7, 0xB6, 0x22, 0x8F, 0x0C}}, "Timecode_Index"},

    //!< Header Object GUIDs
    {{0x8CABDCA1, 0xA947, 0x11CF, {0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}}, "File_Properties"},
    {{0xB7DC0791, 0xA9B7, 0x11CF, {0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}}, "Stream_Properties"},
    {{0x5FBF03B5, 0xA92E, 0x11CF, {0x8E, 0xE3, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}}, "Header_Extension"},
    {{0x86D15240, 0x311D, 0x11D0, {0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6}}, "Codec_List"},
    {{0x1EFB1A30, 0x0B62, 0x11D0, {0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6}}, "Script_Command"},
    {{0xF487CD01, 0xA951, 0x11CF, {0x8E, 0xE6, 0x00, 0xC0, 0x00, 0xC2, 0x05, 0x36}}, "Marker"},
    {{0xD6E229DC, 0x35DA, 0x11D1, {0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE}}, "Bitrate_Mutual_Exclusion"},
    {{0x75B22635, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}}, "Error_Correction"},
    {{0x75B22633, 0x668E, 0x11CF, {0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C}}, "Content_Description"},
    {{0xD2D0A440, 0xE307, 0x11D2, {0x97, 0xF0, 0x00, 0xA0, 0xC9, 0x5E, 0xA8, 0x50}}, "Extended_Content_Description"},
    {{0x2211B3FA, 0xBD23, 0x11D2, {0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E}}, "Content_Branding"},
    {{0x7BF875CE, 0x468D, 0x11D1, {0x8D, 0x82, 0x00, 0x60, 0x97, 0xC9, 0xA2, 0xB2}}, "Stream_Bitrate_Properties"},
    {{0x2211B3FB, 0xBD23, 0x11D2, {0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E}}, "Content_Encryption"},
    {{0x298AE614, 0x2622, 0x4C17, {0xB9, 0x35, 0xDA, 0xE0, 0x7E, 0xE9, 0x28, 0x9C}}, "Extended_Content_Encryption"},
    {{0x2211B3FC, 0xBD23, 0x11D2, {0xB4, 0xB7, 0x00, 0xA0, 0xC9, 0x55, 0xFC, 0x6E}}, "Digital_Signature"},
    {{0x1806D474, 0xCADF, 0x4509, {0xA4, 0xBA, 0x9A, 0xAB, 0xCB, 0x96, 0xAA, 0xE8}}, "Padding"},

    //!< Header Extension Object GUIDs
    {{0x14E6A5CB, 0xC672, 0x4332, {0x83, 0x99, 0xA9, 0x69, 0x52, 0x06, 0x5B, 0x5A}}, "Extended_Stream_Properties"},
    {{0xA08649CF, 0x4775, 0x4670, {0x8A, 0x16, 0x6E, 0x35, 0x35, 0x75, 0x66, 0xCD}}, "Advanced_Mutual_Exclusion"},
    {{0xD1465A40, 0x5A79, 0x4338, {0xB7, 0x1B, 0xE3, 0x6B, 0x8F, 0xD6, 0xC2, 0x49}}, "Group_Mutual_Exclusion"},
    {{0xD4FED15B, 0x88D3, 0x454F, {0x81, 0xF0, 0xED, 0x5C, 0x45, 0x99, 0x9E, 0x24}}, "Stream_Prioritization"},
    {{0xA69609E6, 0x517B, 0x11D2, {0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9}}, "Bandwidth_Sharing"},
    {{0x7C4346A9, 0xEFE0, 0x4BFC, {0xB2, 0x29, 0x39, 0x3E, 0xDE, 0x41, 0x5C, 0x85}}, "Language_List"},
    {{0xC5F8CBEA, 0x5BAF, 0x4877, {0x84, 0x67, 0xAA, 0x8C, 0x44, 0xFA, 0x4C, 0xCA}}, "Metadata"},
    {{0x44231C94, 0x9498, 0x49D1, {0xA1, 0x41, 0x1D, 0x13, 0x4E, 0x45, 0x70, 0x54}}, "Metadata_Library"},
    {{0xD6E229DF, 0x35DA, 0x11D1, {0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE}}, "Index_Parameters"},
    {{0x6B203BAD, 0x3F11, 0x48E4, {0xAC, 0xA8, 0xD7, 0x61, 0x3D, 0xE2, 0xCF, 0xA7}}, "Media_Index_Parameters"},
    {{0xF55E496D, 0x9797, 0x4B5D, {0x8C, 0x8B, 0x60, 0x4D, 0xFE, 0x9B, 0xFB, 0x24}}, "Timecode_Index_Parameters"},
    {{0x26F18B5D, 0x4584, 0x47EC, {0x9F, 0x5F, 0x0E, 0x65, 0x1F, 0x04, 0x52, 0xC9}}, "Compatibility"},
    {{0x43058533, 0x6981, 0x49E6, {0x9B, 0x74, 0xAD, 0x12, 0xCB, 0x86, 0xD5, 0x8C}}, "Advanced_Content_Encryption"},

    //!< Stream Properties Object Stream Type GUIDs
    {{0xF8699E40, 0x5B4D, 0x11CF, {0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}}, "Audio_Media"},
    {{0xBC19EFC0, 0x5B4D, 0x11CF, {0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}}, "Video_Media"},
    {{0x59DACFC0, 0x59E6, 0x11D0, {0xA3, 0xAC, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6}}, "Command_Media"},
    {{0xB61BE100, 0x5B4E, 0x11CF, {0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}}, "JFIF_Media"},
    {{0x35907DE0, 0xE415, 0x11CF, {0xA9, 0x17, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}}, "Degradable_JPEG_Media"},
    {{0x91BD222C, 0xF21C, 0x497A, {0x8B, 0x6D, 0x5A, 0xA8, 0x6B, 0xFC, 0x01, 0x85}}, "File_Transfer_Media"},
    {{0x3AFB65E2, 0x47EF, 0x40F2, {0xAC, 0x2C, 0x70, 0xA9, 0x0D, 0x71, 0xD3, 0x43}}, "Binary_Media"},

    //!< Web stream Type-Specific Data GUIDs
    {{0x776257D4, 0xC627, 0x41CB, {0x8F, 0x81, 0x7A, 0xC7, 0xFF, 0x1C, 0x40, 0xCC}}, "Web_Stream_Media_Subtype"},
    {{0xDA1E6B13, 0x8359, 0x4050, {0xB3, 0x98, 0x38, 0x8E, 0x96, 0x5B, 0xF0, 0x0C}}, "Web_Stream_Format"},

    //!< Stream Properties Object Error Correction Type GUIDs
    {{0x20FB5700, 0x5B55, 0x11CF, {0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B}}, "No_Error_Correction"},
    {{0xBFC3CD50, 0x618F, 0x11CF, {0x8B, 0xB2, 0x00, 0xAA, 0x00, 0xB4, 0xE2, 0x20}}, "Audio_Spread"},
    //!< Header Extension Object GUIDs
    {{0xABD3D211, 0xA9BA, 0x11CF, {0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65}}, "Reserved_1"},

    //!< Advanced Content Encryption Object System ID GUIDs
    {{0x7A079BB6, 0xDAA4, 0x4E12, {0xA5, 0xCA, 0x91, 0xD3, 0x8D, 0xC1, 0x1A, 0x8D}},
     "Content_Encryption_System_Windows_Media_DRM_Network_Devices"},

    //!< Codec List Object GUIDs
    {{0x86D15241, 0x311D, 0x11D0, {0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6}}, "Reserved_2"},

    //!< Script Command Object GUIDs
    {{0x4B1ACBE3, 0x100B, 0x11D0, {0xA3, 0x9B, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6}}, "Reserved_3"},

    //!< Marker Object GUIDs
    {{0x4CFEDB20, 0x75F6, 0x11CF, {0x9C, 0x0F, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xCB}}, "Reserved_4"},

    //!< Mutual Exclusion Object Exclusion Type GUIDs
    {{0xD6E22A00, 0x35DA, 0x11D1, {0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE}}, "Mutex_Language"},
    {{0xD6E22A01, 0x35DA, 0x11D1, {0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE}}, "Mutex_Bitrate"},
    {{0xD6E22A02, 0x35DA, 0x11D1, {0x90, 0x34, 0x00, 0xA0, 0xC9, 0x03, 0x49, 0xBE}}, "Mutex_Unknown"},
    //!< Bandwidth Sharing Object GUID
    {{0xAF6060AA, 0x5197, 0x11D2, {0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9}}, "Bandwidth_Sharing_Exclusive"},
    {{0xAF6060AB, 0x5197, 0x11D2, {0xB6, 0xAF, 0x00, 0xC0, 0x4F, 0xD9, 0x08, 0xE9}}, "Bandwidth_Sharing_Partial"},

    //!< Standard Payload Extension System GUIDs
    {{0x399595EC, 0x8667, 0x4E2D, {0x8F, 0xDB, 0x98, 0x81, 0x4C, 0xE7, 0x6C, 0x1E}},
     "Payload_Extension_System_Timecode"},
    {{0xE165EC0E, 0x19ED, 0x45D7, {0xB4, 0xA7, 0x25, 0xCB, 0xD1, 0xE2, 0x8E, 0x9B}},
     "Payload_Extension_System_File_Name"},
    {{0xD590DC20, 0x07BC, 0x436C, {0x9C, 0xF7, 0xF3, 0xBB, 0xFB, 0xF1, 0xA4, 0xDC}},
     "Payload_Extension_System_Content_Type"},
    {{0x1B1EE554, 0xF9EA, 0x4BC8, {0x82, 0x1A, 0x37, 0x6B, 0x74, 0xE4, 0xC4, 0xB8}},
     "Payload_Extension_System_Pixel_Aspect_Ratio"},
    {{0xC6BD9450, 0x867F, 0x4907, {0x83, 0xA3, 0xC7, 0x79, 0x21, 0xB7, 0x33, 0xAD}},
     "Payload_Extension_System_Sample_Duration"},
    {{0x6698B84E, 0x0AFA, 0x4330, {0xAE, 0xB2, 0x1C, 0x0A, 0x98, 0xD7, 0xA4, 0x4D}},
     "Payload_Extension_System_Encryption_Sample_ID"},
    {{0x00E1AF06, 0x7BEC, 0x11D1, {0xA5, 0x82, 0x00, 0xC0, 0x4F, 0xC2, 0x9C, 0xFB}},
     "Payload_Extension_System_Degradable_JPEG"},
};

/*!
  @brief Function used to check if data stored in buf is equivalent to
      ASF Header TagVocabulary's GUID.
  @param buf Exiv2 byte buffer
  @return Returns true if the buffer data is equivalent to Header GUID.
 */
static bool isASFType(const byte buf[]) {
  return Header == AsfVideo::GUIDTag(buf);
}

AsfVideo::AsfVideo(BasicIo::UniquePtr io) : Image(ImageType::asf, mdNone, std::move(io)) {
}  // AsfVideo::AsfVideo

std::string AsfVideo::mimeType() const {
  return "video/asf";
}

void AsfVideo::writeMetadata() {
}

void AsfVideo::readMetadata() {
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());

  // Ensure that this is the correct image type
  if (!isAsfType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "ASF");
  }

  IoCloser closer(*io_);
  clearMetadata();
  io_->seek(0, BasicIo::beg);
  height_ = width_ = 1;

  xmpData()["Xmp.video.FileSize"] = io_->size() / 1048576.;
  xmpData()["Xmp.video.MimeType"] = mimeType();

  decodeBlock();

  xmpData_["Xmp.video.AspectRatio"] = getAspectRatio(width_, height_);
}  // AsfVideo::readMetadata

AsfVideo::HeaderReader::HeaderReader(const BasicIo::UniquePtr& io) : IdBuf_(GUID) {
  if (io->size() >= io->tell() + GUID + QWORD) {
    io->readOrThrow(IdBuf_.data(), IdBuf_.size(), Exiv2::ErrorCode::kerCorruptedMetadata);

    size_ = readQWORDTag(io);
    if (size_ >= GUID + QWORD)
      remaining_size_ = size_ - GUID - QWORD;
  }
}

void AsfVideo::decodeBlock() {
  Internal::enforce(GUID + QWORD <= io_->size() - io_->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  HeaderReader objectHeader(io_);
#ifdef EXIV2_DEBUG_MESSAGES
  EXV_INFO << "decodeBlock = " << GUIDTag(objectHeader.getId().data()).to_string()
           << "\tsize= " << objectHeader.getSize() << "\t " << io_->tell() << "/" << io_->size() << '\n';
#endif
  Internal::enforce(objectHeader.getSize() <= io_->size() - io_->tell(), Exiv2::ErrorCode::kerCorruptedMetadata);
  auto tag = GUIDReferenceTags.find(GUIDTag(objectHeader.getId().data()));

  if (tag != GUIDReferenceTags.end()) {
    if (tag->second == "Header")
      decodeHeader();
    else if (tag->second == "File_Properties")
      fileProperties();
    else if (tag->second == "Stream_Properties")
      streamProperties();
    else if (tag->second == "Header_Extension")
      headerExtension();
    else if (tag->second == "Codec_List")
      codecList();
    else if (tag->second == "Extended_Content_Description")
      extendedContentDescription();
    else if (tag->second == "Content_Description")
      contentDescription();
    else if (tag->second == "Extended_Stream_Properties")
      extendedStreamProperties();
    else if (tag->second == "Degradable_JPEG_Media")
      DegradableJPEGMedia();
    else {  // tag found but not processed
      // Make sure that the remaining size is non-zero, so that we won't
      // keep revisiting the same location in the file.
      const uint64_t remaining_size = objectHeader.getRemainingSize();
      Internal::enforce(remaining_size > 0, Exiv2::ErrorCode::kerCorruptedMetadata);
      io_->seekOrThrow(io_->tell() + remaining_size, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
    }
  } else {  // tag not found
    // Make sure that the remaining size is non-zero, so that we won't keep
    // revisiting the same location in the file.
    const uint64_t remaining_size = objectHeader.getRemainingSize();
    Internal::enforce(remaining_size > 0, Exiv2::ErrorCode::kerCorruptedMetadata);
    io_->seekOrThrow(io_->tell() + remaining_size, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
  }

}  // AsfVideo::decodeBlock

void AsfVideo::decodeHeader() {
  DataBuf nbHeadersBuf(DWORD + 1);
  io_->readOrThrow(nbHeadersBuf.data(), DWORD, Exiv2::ErrorCode::kerCorruptedMetadata);

  uint32_t nb_headers = Exiv2::getULong(nbHeadersBuf.data(), littleEndian);
  Internal::enforce(nb_headers < std::numeric_limits<uint32_t>::max(), Exiv2::ErrorCode::kerCorruptedMetadata);
  io_->seekOrThrow(io_->tell() + (BYTE * 2), BasicIo::beg,
                   ErrorCode::kerFailedToReadImageData);  // skip two reserved tags
  for (uint32_t i = 0; i < nb_headers; i++) {
    decodeBlock();
  }
}

void AsfVideo::extendedStreamProperties() {
  xmpData()["Xmp.video.StartTimecode"] = readQWORDTag(io_);  // Start Time
  xmpData()["Xmp.video.EndTimecode"] = readWORDTag(io_);     // End Time

  io_->seek(io_->tell() + DWORD, BasicIo::beg);  // ignore Data Bitrate
  io_->seek(io_->tell() + DWORD, BasicIo::beg);  // ignore Buffer Size
  io_->seek(io_->tell() + DWORD, BasicIo::beg);  // ignore Initial Buffer Fullness
  io_->seek(io_->tell() + DWORD, BasicIo::beg);  // ignore Alternate Data Bitrate
  io_->seek(io_->tell() + DWORD, BasicIo::beg);  // ignore Alternate Buffer Size
  io_->seek(io_->tell() + DWORD, BasicIo::beg);  // ignore Alternate Initial Buffer Fullness
  io_->seek(io_->tell() + DWORD, BasicIo::beg);  // ignore Maximum Object Size
  io_->seek(io_->tell() + DWORD, BasicIo::beg);  // ignore Flags Buffer Size
  io_->seek(io_->tell() + WORD, BasicIo::beg);   // ignore Flags Stream Number
  io_->seek(io_->tell() + WORD, BasicIo::beg);   // ignore Stream Language ID Index

  xmpData()["Xmp.video.FrameRate"] = readWORDTag(io_);  // Average Time Per Frame
  uint16_t stream_name_count = readWORDTag(io_);
  uint16_t payload_ext_sys_count = readWORDTag(io_);

  for (uint16_t i = 0; i < stream_name_count; i++) {
    io_->seek(io_->tell() + WORD, BasicIo::beg);  // ignore Language ID Index
    uint16_t stream_length = readWORDTag(io_);
    if (stream_length)
      io_->seek(io_->tell() + stream_length, BasicIo::beg);  // ignore Stream name
  }

  for (uint16_t i = 0; i < payload_ext_sys_count; i++) {
    io_->seek(io_->tell() + GUID, BasicIo::beg);  // ignore Extension System ID
    io_->seek(io_->tell() + WORD, BasicIo::beg);  // ignore Extension Data Size
    uint16_t ext_sys_info_length = readWORDTag(io_);
    if (ext_sys_info_length)
      io_->seek(io_->tell() + ext_sys_info_length, BasicIo::beg);  // ignore Extension System Info
  }
}  // AsfVideo::extendedStreamProperties

void AsfVideo::DegradableJPEGMedia() {
  uint32_t width = readDWORDTag(io_);
  width_ = width;
  xmpData_["Xmp.video.Width"] = width;

  uint32_t height = readDWORDTag(io_);
  height_ = height;
  xmpData_["Xmp.video.Height"] = height;

  io_->seek(io_->tell() + (WORD * 3) /*3 Reserved*/, BasicIo::beg);

  uint32_t interchange_data_length = readWORDTag(io_);
  io_->seek(io_->tell() + interchange_data_length /*Interchange data*/, BasicIo::beg);
}

void AsfVideo::streamProperties() {
  DataBuf streamTypedBuf(GUID);
  io_->readOrThrow(streamTypedBuf.data(), streamTypedBuf.size(), Exiv2::ErrorCode::kerCorruptedMetadata);

  enum class streamTypeInfo { Audio = 1, Video = 2 };
  auto stream = static_cast<streamTypeInfo>(0);

  auto tag_stream_type = GUIDReferenceTags.find(GUIDTag(streamTypedBuf.data()));
  if (tag_stream_type != GUIDReferenceTags.end()) {
    if (tag_stream_type->second == "Audio_Media")
      stream = streamTypeInfo::Audio;
    else if (tag_stream_type->second == "Video_Media")
      stream = streamTypeInfo::Video;

    io_->seek(io_->tell() + GUID, BasicIo::beg);  // ignore Error Correction Type

    uint64_t time_offset = readQWORDTag(io_);
    if (stream == streamTypeInfo::Video)
      xmpData()["Xmp.video.TimeOffset"] = time_offset;
    else if (stream == streamTypeInfo::Audio)
      xmpData()["Xmp.audio.TimeOffset"] = time_offset;

    auto specific_data_length = readDWORDTag(io_);
    auto correction_data_length = readDWORDTag(io_);

    io_->seek(io_->tell() + WORD /*Flags*/ + DWORD /*Reserved*/ + specific_data_length + correction_data_length,
              BasicIo::beg);
  }

}  // AsfVideo::streamProperties

void AsfVideo::codecList() {
  io_->seek(io_->tell() + GUID /*reserved*/, BasicIo::beg);
  auto entries_count = readDWORDTag(io_);
  for (uint32_t i = 0; i < entries_count; i++) {
    uint16_t codec_type = readWORDTag(io_) * 2;
    std::string codec = (codec_type == 1) ? "Xmp.video" : "Xmp.audio";

    if (uint16_t codec_name_length = readWORDTag(io_) * 2)
      xmpData()[codec + std::string(".CodecName")] = readStringWcharTag(io_, codec_name_length);

    if (uint16_t codec_desc_length = readWORDTag(io_))
      xmpData()[codec + std::string(".CodecDescription")] = readStringWcharTag(io_, codec_desc_length);

    uint16_t codec_info_length = readWORDTag(io_);
    Internal::enforce(codec_info_length && codec_info_length < io_->size() - io_->tell(),
                      Exiv2::ErrorCode::kerCorruptedMetadata);
    xmpData()[codec + std::string(".CodecInfo")] = readStringTag(io_, codec_info_length);
  }
}  // AsfVideo::codecList

void AsfVideo::headerExtension() const {
  io_->seek(io_->tell() + GUID /*reserved1*/ + WORD /*Reserved2*/, BasicIo::beg);
  auto header_ext_data_length = readDWORDTag(io_);
  io_->seek(io_->tell() + header_ext_data_length, BasicIo::beg);
}  // AsfVideo::headerExtension

void AsfVideo::extendedContentDescription() {
  uint16_t content_descriptor_count = readWORDTag(io_);
  std::string value;

  for (uint16_t i = 0; i < content_descriptor_count; i++) {
    if (uint16_t descriptor_name_length = readWORDTag(io_))
      value += readStringWcharTag(io_, descriptor_name_length);  // Descriptor Name

    uint16_t descriptor_value_data_type = readWORDTag(io_);
    if (uint16_t descriptor_value_length = readWORDTag(io_)) {
      // Descriptor Value
      switch (descriptor_value_data_type) {
        case 0 /*Unicode string */:
          value += std::string(": ") + readStringWcharTag(io_, descriptor_value_length);
          break;
        case 1 /*BYTE array  */:
          value += std::string(": ") + readStringTag(io_, descriptor_value_length);
          break;
        case 2 /*BOOL*/:
          value += std::string(": ") + std::to_string(readWORDTag(io_));
          break;
        case 3 /*DWORD */:
          value += std::string(": ") + std::to_string(readDWORDTag(io_));
          break;
        case 4 /*QWORD */:
          value += std::string(": ") + std::to_string(readQWORDTag(io_));
          break;
        case 5 /*WORD*/:
          value += std::string(": ") + std::to_string(readWORDTag(io_));
          break;
      }
    }
    value += std::string(", ");
  }

  xmpData()["Xmp.video.ExtendedContentDescription"] = value;
}  // AsfVideo::extendedContentDescription

void AsfVideo::contentDescription() {
  uint16_t title_length = readWORDTag(io_);
  uint16_t author_length = readWORDTag(io_);
  uint16_t copyright_length = readWORDTag(io_);
  uint16_t desc_length = readWORDTag(io_);
  uint16_t rating_length = readWORDTag(io_);

  if (title_length)
    xmpData()["Xmp.video.Title"] = readStringWcharTag(io_, title_length);

  if (author_length)
    xmpData()["Xmp.video.Author"] = readStringWcharTag(io_, author_length);

  if (copyright_length)
    xmpData()["Xmp.video.Copyright"] = readStringWcharTag(io_, copyright_length);

  if (desc_length)
    xmpData()["Xmp.video.Description"] = readStringWcharTag(io_, desc_length);

  if (rating_length)
    xmpData()["Xmp.video.Rating"] = readStringWcharTag(io_, rating_length);

}  // AsfVideo::extendedContentDescription

void AsfVideo::fileProperties() {
  DataBuf FileIddBuf(GUID);
  io_->readOrThrow(FileIddBuf.data(), FileIddBuf.size(), Exiv2::ErrorCode::kerCorruptedMetadata);
  xmpData()["Xmp.video.FileID"] = GUIDTag(FileIddBuf.data()).to_string();
  xmpData()["Xmp.video.FileLength"] = readQWORDTag(io_);
  xmpData()["Xmp.video.CreationDate"] = readQWORDTag(io_);
  xmpData()["Xmp.video.DataPackets"] = readQWORDTag(io_);
  xmpData()["Xmp.video.duration"] = readQWORDTag(io_);
  xmpData()["Xmp.video.SendDuration"] = readQWORDTag(io_);
  xmpData()["Xmp.video.Preroll"] = readQWORDTag(io_);

  io_->seek(io_->tell() + DWORD + DWORD + DWORD,
            BasicIo::beg);  // ignore Flags, Minimum Data Packet Size and Maximum Data Packet Size
  xmpData()["Xmp.video.MaxBitRate"] = readDWORDTag(io_);
}  // AsfVideo::fileProperties

Image::UniquePtr newAsfInstance(BasicIo::UniquePtr io, bool /*create*/) {
  auto image = std::make_unique<AsfVideo>(std::move(io));
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isAsfType(BasicIo& iIo, bool advance) {
  byte buf[GUID];
  iIo.read(buf, GUID);

  if (iIo.error() || iIo.eof()) {
    return false;
  }

  bool matched = isASFType(buf);
  if (!advance || !matched) {
    iIo.seek(0, BasicIo::beg);
  }

  return matched;
}

}  // namespace Exiv2
