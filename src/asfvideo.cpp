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
/*
  File:      asfvideo.cpp
  Author(s): Abhinav Badola for GSoC 2012 (AB) <mail.abu.to@gmail.com>
  History:   08-Aug-12, AB: created
  Credits:   See header file
  Spec: https://exse.eyewated.com/fls/54b3ed95bbfb1a92.pdf
 */
// *****************************************************************************
// included header files
#include <iostream>
#include "config.h"

#include "asfvideo.hpp"
#include "basicio.hpp"
#include "convert.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "helper_functions.hpp"
#include "tags.hpp"
#include "tags_int.hpp"
#include "types.hpp"

// + standard includes
#include <cassert>
#include <cctype>
#include <cstring>

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {

/*!
  TagVocabulary Look-up list for ASF Type Video Files
  Associates the GUID of a TagVocabulary with its TagVocabulary Name(i.e. Human Readable Form)
  Tags have been diferentiated into Various Categories.
  The categories have been listed above the TagVocabulary Groups
  see :
  - https://fr.wikipedia.org/wiki/Advanced_Systems_Format
  - https://exse.eyewated.com/fls/54b3ed95bbfb1a92.pdf
 */
constexpr const TagVocabulary GUIDReferenceTags[] = {
    /// Top-level ASF object GUIDS
    {"75B22630-668E-11CF-A6D9-00AA0062CE6C", "Header"},
    {"75B22636-668E-11CF-A6D9-00AA0062CE6C", "Data"},
    {"33000890-E5B1-11CF-89F4-00A0C90349CB", "Simple_Index"},
    {"D6E229D3-35DA-11D1-9034-00A0C90349BE", "Index"},
    {"FEB103F8-12AD-4C64-840F-2A1D2F7AD48C", "Media_Index"},
    {"3CB73FD0-0C4A-4803-953D-EDF7B6228F0C", "Timecode_Index"},

    /// Header Object GUIDs
    {"8CABDCA1-A947-11CF-8EE4-00C00C205365", "File_Properties"},
    {"B7DC0791-A9B7-11CF-8EE6-00C00C205365", "Stream_Properties"},
    {"5FBF03B5-A92E-11CF-8EE3-00C00C205365", "Header_Extension"},
    {"86D15240-311D-11D0-A3A4-00A0C90348F6", "Codec_List"},
    {"1EFB1A30-0B62-11D0-A39B-00A0C90348F6", "Script_Command"},
    {"F487CD01-A951-11CF-8EE6-00C00C205365", "Marker"},
    {"D6E229DC-35DA-11D1-9034-00A0C90349BE", "Bitrate_Mutual_Exclusion"},
    {"75B22635-668E-11CF-A6D9-00AA0062CE6C", "Error_Correction"},
    {"75B22633-668E-11CF-A6D9-00AA0062CE6C", "Content_Description"},
    {"D2D0A440-E307-11D2-97F0-00A0C95EA850", "Extended_Content_Description"},
    {"2211B3FA-BD23-11D2-B4B7-00A0C955FC6E", "Content_Branding"},
    {"7BF875CE-468D-11D1-8D82-006097C9A2B2", "Stream_Bitrate_Properties"},
    {"2211B3FB-BD23-11D2-B4B7-00A0C955FC6E", "Content_Encryption"},
    {"298AE614-2622-4C17-B935-DAE07EE9289C", "Extended_Content_Encryption"},
    {"2211B3FC-BD23-11D2-B4B7-00A0C955FC6E", "Digital_Signature"},
    {"1806D474-CADF-4509-A4BA-9AABCB96AAE8", "Padding"},

    /// Header Extension Object GUIDs
    {"14E6A5CB-C672-4332-8399-A96952065B5A", "Extended_Stream_Properties"},
    {"A08649CF-4775-4670-8A16-6E35357566CD", "Advanced_Mutual_Exclusion"},
    {"D1465A40-5A79-4338-B71B-E36B8FD6C249", "Group_Mutual_Exclusion"},
    {"D4FED15B-88D3-454F-81F0-ED5C45999E24", "Stream_Prioritization"},
    {"A69609E6-517B-11D2-B6AF-00C04FD908E9", "Bandwidth_Sharing"},
    {"7C4346A9-EFE0-4BFC-B229-393EDE415C85", "Language_List"},
    {"C5F8CBEA-5BAF-4877-8467-AA8C44FA4CCA", "Metadata"},
    {"44231C94-9498-49D1-A141-1D134E457054", "Metadata_Library"},
    {"D6E229DF-35DA-11D1-9034-00A0C90349BE", "Index_Parameters"},
    {"6B203BAD-3F11-48E4-ACA8-D7613DE2CFA7", "Media_Index_Parameters"},
    {"F55E496D-9797-4B5D-8C8B-604DFE9BFB24", "Timecode_Index_Parameters"},
    {"26F18B5D-4584-47EC-9F5F-0E651F0452C9", "Compatibility"},
    {"43058533-6981-49E6-9B74-AD12CB86D58C", "Advanced_Content_Encryption"},

    /// Stream Properties Object Stream Type GUIDs
    {"F8699E40-5B4D-11CF-A8FD-00805F5C442B", "Audio_Media"},
    {"BC19EFC0-5B4D-11CF-A8FD-00805F5C442B", "Video_Media"},
    {"59DACFC0-59E6-11D0-A3AC-00A0C90348F6", "Command_Media"},
    {"B61BE100-5B4E-11CF-A8FD-00805F5C442B", "JFIF_Media"},
    {"35907DE0-E415-11CF-A917-00805F5C442B", "Degradable_JPEG_Media"},
    {"91BD222C-F21C-497A-8B6D-5AA86BFC0185", "File_Transfer_Media"},
    {"3AFB65E2-47EF-40F2-AC2C-70A90D71D343", "Binary_Media"},

    /// Web stream Type-Specific Data GUIDs
    {"776257D4-C627-41CB-8F81-7AC7FF1C40CC", "Web_Stream_Media_Subtype"},
    {"DA1E6B13-8359-4050-B398-388E965BF00C", "Web_Stream_Format"},

    /// Stream Properties Object Error Correction Type GUIDs
    {"20FB5700-5B55-11CF-A8FD-00805F5C442B", "No_Error_Correction"},
    {"BFC3CD50-618F-11CF-8BB2-00AA00B4E220", "Audio_Spread"},

    /// Header Extension Object GUIDs
    {"ABD3D211-A9BA-11cf-8EE6-00C00C205365", "Reserved_1"},

    /// Advanced Content Encryption Object System ID GUIDs
    {"7A079BB6-DAA4-4e12-A5CA-91D38DC11A8D", "Content_Encryption_System_Windows_Media_DRM_Network_Devices"},

    /// Codec List Object GUIDs
    {"86D15241-311D-11D0-A3A4-00A0C90348F6", "Reserved_2"},

    /// Script Command Object GUIDs
    {"4B1ACBE3-100B-11D0-A39B-00A0C90348F6", "Reserved_3"},

    /// Marker Object GUIDs
    {"4CFEDB20-75F6-11CF-9C0F-00A0C90349CB", "Reserved_4"},

    /// Mutual Exclusion Object Exclusion Type GUIDs
    {"D6E22A00-35DA-11D1-9034-00A0C90349BE", "Mutex_Language"},
    {"D6E22A01-35DA-11D1-9034-00A0C90349BE", "Mutex_Bitrate"},
    {"D6E22A02-35DA-11D1-9034-00A0C90349BE", "Mutex_Unknown"},

    /// Bandwidth Sharing Object GUIDs
    {"AF6060AA-5197-11D2-B6AF-00C04FD908E9", "Bandwidth_Sharing_Exclusive"},
    {"AF6060AB-5197-11D2-B6AF-00C04FD908E9", "Bandwidth_Sharing_Partial"},

    /// Standard Payload Extension System GUIDs
    {"399595EC-8667-4E2D-8FDB-98814CE76C1E", "Payload_Extension_System_Timecode"},
    {"E165EC0E-19ED-45D7-B4A7-25CBD1E28E9B", "Payload_Extension_System_File_Name"},
    {"D590DC20-07BC-436C-9CF7-F3BBFBF1A4DC", "Payload_Extension_System_Content_Type"},
    {"1B1EE554-F9EA-4BC8-821A-376B74E4C4B8", "Payload_Extension_System_Pixel_Aspect_Ratio"},
    {"C6BD9450-867F-4907-83A3-C77921B733AD", "Payload_Extension_System_Sample_Duration"},
    {"6698B84E-0AFA-4330-AEB2-1C0A98D7A44D", "Payload_Extension_System_Encryption_Sample_ID"},
    {"00E1AF06-7BEC-11D1-A582-00C04FC29CFB", "Payload_Extension_System_Degradable_JPEG"}};

/*!
  @brief Function used to check equality of two Tags (ignores case).
  @param str1 char* Pointer to First TagVocabulary
  @param str2 char* Pointer to Second TagVocabulary
  @return Returns true if both are equal.
 */
bool compareTag(const char* str1, const char* str2) {
  if (strlen(str1) != strlen(str2))
    return false;

  for (uint64_t i = 0; i < strlen(str1); ++i)
    if (tolower(str1[i]) != tolower(str2[i]))
      return false;

  return true;
}

/*!
  @brief Function used to calculate GUID, Tags comprises of 16 bytes.
      The Buffer contains the TagVocabulary in Binary Form. The information is then
      parsed into a character array GUID.
 */
void getGUID(byte buf[], char GUID[]) {
  int i;
  for (i = 0; i < 4; ++i) {
    GUID[(3 - i) * 2] = Util::returnHEX(buf[i] / 0x10);
    GUID[(3 - i) * 2 + 1] = Util::returnHEX(buf[i] % 0x10);
  }
  for (i = 4; i < 6; ++i) {
    GUID[(9 - i) * 2 + 1] = Util::returnHEX(buf[i] / 0x10);
    GUID[(9 - i) * 2 + 2] = Util::returnHEX(buf[i] % 0x10);
  }
  for (i = 6; i < 8; ++i) {
    GUID[(14 - i) * 2] = Util::returnHEX(buf[i] / 0x10);
    GUID[(14 - i) * 2 + 1] = Util::returnHEX(buf[i] % 0x10);
  }
  for (i = 8; i < 10; ++i) {
    GUID[i * 2 + 3] = Util::returnHEX(buf[i] / 0x10);
    GUID[i * 2 + 4] = Util::returnHEX(buf[i] % 0x10);
  }
  for (i = 10; i < 16; ++i) {
    GUID[i * 2 + 4] = Util::returnHEX(buf[i] / 0x10);
    GUID[i * 2 + 5] = Util::returnHEX(buf[i] % 0x10);
  }
  GUID[36] = '\0';
  GUID[8] = GUID[13] = GUID[18] = GUID[23] = '-';
}

/*!
  @brief Function used to check if data stored in buf is equivalent to
      ASF Header TagVocabulary's GUID.
  @param buf Exiv2 byte buffer
  @return Returns true if the buffer data is equivalent to Header GUID.
 */
bool isASFType(byte buf[]) {
  return buf[0] == 0x30 && buf[1] == 0x26 && buf[2] == 0xb2 && buf[3] == 0x75 && buf[4] == 0x8e && buf[5] == 0x66 &&
         buf[6] == 0xcf && buf[7] == 0x11 && buf[8] == 0xa6 && buf[9] == 0xd9 && buf[10] == 0x00 && buf[11] == 0xaa &&
         buf[12] == 0x00 && buf[13] == 0x62 && buf[14] == 0xce && buf[15] == 0x6c;
}

}  // namespace Exiv2::Internal

namespace Exiv2 {

using namespace Exiv2::Internal;

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

  aspectRatio();
}  // AsfVideo::readMetadata

void AsfVideo::decodeBlock() {
  AsfObject obj(io_);

  char guid[GUID_SIZE] = "";
  getGUID(obj.getId(), guid);

  auto tv = find(GUIDReferenceTags, guid);
  if (tv) {
    if (compareTag(exvGettext(tv->label_), "Header")) {
      DataBuf nbHeadersBuf(DWORD + 1);
      io_->read(nbHeadersBuf.data(), DWORD);

      uint32_t nb_headers = Exiv2::getULong(nbHeadersBuf.data(), littleEndian);
      DataBuf reserved(BYTE + 1);
      io_->read(reserved.data(), BYTE);
      io_->read(reserved.data(), BYTE);
      for (uint32_t i = 0; i < nb_headers; i++) {
        AsfObject obj(io_);
        char guid[GUID_SIZE] = "";
        getGUID(obj.getId(), guid);
        auto tag = find(GUIDReferenceTags, guid);
        if (tag) {
          if (compareTag(exvGettext(tag->label_), "File_Properties"))
            fileProperties();
          else if (compareTag(exvGettext(tag->label_), "Stream_Properties"))
            streamProperties();
          else if (compareTag(exvGettext(tag->label_), "Header_Extension"))
            headerExtension();
          else if (compareTag(exvGettext(tag->label_), "Codec_List"))
            codecList();
          else if (compareTag(exvGettext(tag->label_), "Extended_Content_Description"))
            extendedContentDescription();
          else if (compareTag(exvGettext(tag->label_), "Content_Description"))
            contentDescription();
          else if (compareTag(exvGettext(tag->label_), "Extended_Stream_Properties"))
            extendedStreamProperties();
          else
            io_->seek(io_->tell() + obj.getRemainingSize(), BasicIo::beg);
        } else
          io_->seek(io_->tell() + obj.getRemainingSize(), BasicIo::beg);
      }
    } else
      io_->seek(io_->tell() + obj.getRemainingSize(), BasicIo::beg);

  } else
    io_->seek(io_->tell() + obj.getRemainingSize(), BasicIo::beg);
}  // AsfVideo::decodeBlock

void AsfVideo::extendedStreamProperties() {
  xmpData()["Xmp.video.StartTimecode"] = readQWORDTag();  // Start Time
  xmpData()["Xmp.video.EndTimecode"] = readWORDTag();     // End Time

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

  xmpData()["Xmp.video.FrameRate"] = readWORDTag();  // Average Time Per Frame
  uint16_t stream_name_count = readWORDTag();
  uint16_t payload_ext_sys_count = readWORDTag();

  for (uint16_t i = 0; i < stream_name_count; i++) {
    io_->seek(io_->tell() + WORD, BasicIo::beg);  // ignore Language ID Index
    uint16_t stream_length = readWORDTag();
    if (stream_length)
      io_->seek(io_->tell() + stream_length, BasicIo::beg);  // ignore Stream name
  }

  for (uint16_t i = 0; i < payload_ext_sys_count; i++) {
    io_->seek(io_->tell() + GUID, BasicIo::beg);  // ignore Extension System ID
    io_->seek(io_->tell() + WORD, BasicIo::beg);  // ignore Extension Data Size
    uint16_t ext_sys_info_length = readWORDTag();
    if (ext_sys_info_length)
      io_->seek(io_->tell() + ext_sys_info_length, BasicIo::beg);  // ignore Extension System Info
  }
}  // AsfVideo::extendedStreamProperties

void AsfVideo::streamProperties() {
  byte streamTypedBuf[GUID];
  io_->read(streamTypedBuf,GUID);
  char stream_type[GUID_SIZE] = "";
  getGUID(streamTypedBuf, stream_type);

  enum streamTypeInfo { Audio = 1, Video = 2 };
  int stream = 0;

  auto tag_stream_type = find(GUIDReferenceTags, stream_type);
  if (tag_stream_type) {
    if (compareTag(exvGettext(tag_stream_type->label_), "Audio_Media"))
      stream = Audio;
    else if (compareTag(exvGettext(tag_stream_type->label_), "Video_Media"))
      stream = Video;

    io_->seek(io_->tell() + GUID, BasicIo::beg);  // ignore Error Correction Type

    uint64_t time_offset = readQWORDTag();
    if (stream == Video)
      xmpData()["Xmp.video.TimeOffset"] = time_offset;
    else if (stream == Audio)
      xmpData()["Xmp.audio.TimeOffset"] = time_offset;

    auto specific_data_length = readDWORDTag();
    auto correction_data_length = readDWORDTag();

    io_->seek(io_->tell() + WORD /*Flags*/ + DWORD /*Reserved*/ + specific_data_length + correction_data_length,
              BasicIo::beg);
  }

}  // AsfVideo::streamProperties

void AsfVideo::codecList() {
  io_->seek(io_->tell() + GUID /*reserved*/, BasicIo::beg);
  auto entries_count = readDWORDTag();
  for (uint32_t i = 0; i < entries_count; i++) {
    uint16_t codec_type = readWORDTag() * 2;
    std::string codec = (codec_type == 1) ? "Xmp.video" : "Xmp.audio";

    uint16_t codec_name_length = readWORDTag() * 2;
    if (codec_name_length)
      xmpData()[codec + std::string(".CodecName")] = readStringWCHAR(codec_name_length);

    uint16_t codec_desc_length = readWORDTag();
    if (codec_desc_length)
      xmpData()[codec + std::string(".CodecDescription")] = readStringWCHAR(codec_desc_length);

    uint16_t codec_info_length = readWORDTag();
    if (codec_info_length)
      xmpData()[codec + std::string(".CodecInfo")] = readString(codec_info_length);
  }
}  // AsfVideo::codecList

void AsfVideo::headerExtension() {
  io_->seek(io_->tell() + GUID /*reserved1*/ + WORD /*Reserved2*/, BasicIo::beg);
  auto header_ext_data_length = readDWORDTag();
  io_->seek(io_->tell() + header_ext_data_length, BasicIo::beg);
}  // AsfVideo::headerExtension

void AsfVideo::extendedContentDescription() {
  uint16_t content_descriptor_count = readWORDTag();
  std::string value;

  for (uint16_t i = 0; i < content_descriptor_count; i++) {
    uint16_t descriptor_name_length = readWORDTag();
    if (descriptor_name_length)
      value += readStringWCHAR(descriptor_name_length);  // Descriptor Name

    uint16_t descriptor_value_data_type = readWORDTag();
    uint16_t descriptor_value_length = readWORDTag();
    if (descriptor_value_length) {
      // Descriptor Value
      switch (descriptor_value_data_type) {
        case 0 /*Unicode string */:
          value += std::string(": ") + readStringWCHAR(descriptor_value_length);
          break;
        case 1 /*BYTE array  */:
          value += std::string(": ") + readString(descriptor_value_length);
          break;
        case 2 /*BOOL*/:
          value += std::string(": ") + std::to_string(readWORDTag());
          break;
        case 3 /*DWORD */:
          value += std::string(": ") + std::to_string(readDWORDTag());
          break;
        case 4 /*QWORD */:
          value += std::string(": ") + std::to_string(readQWORDTag());
          break;
        case 5 /*WORD*/:
          value += std::string(": ") + std::to_string(readWORDTag());
          ;
          break;
      }
    }
    value += std::string(", ");
  }

  xmpData()["Xmp.video.ExtendedContentDescription"] = value;
}  // AsfVideo::extendedContentDescription

void AsfVideo::contentDescription() {
  uint16_t title_length = readWORDTag();
  uint16_t author_length = readWORDTag();
  uint16_t copyright_length = readWORDTag();
  uint16_t desc_length = readWORDTag();
  uint16_t rating_length = readWORDTag();

  if (title_length)
    xmpData()["Xmp.video.Title"] = readStringWCHAR(title_length);

  if (author_length)
    xmpData()["Xmp.video.Author"] = readStringWCHAR(author_length);

  if (copyright_length)
    xmpData()["Xmp.video.Copyright"] = readStringWCHAR(copyright_length);

  if (desc_length)
    xmpData()["Xmp.video.Description"] = readStringWCHAR(desc_length);

  if (rating_length)
    xmpData()["Xmp.video.Rating"] = readStringWCHAR(rating_length);

}  // AsfVideo::extendedContentDescription

uint64_t AsfVideo::readQWORDTag() {
  DataBuf FieldBuf(QWORD);
  io_->read(FieldBuf.data(), QWORD);
  uint64_t field = Util::getUint64_t(FieldBuf);
  return field;
}

uint32_t AsfVideo::readDWORDTag() {
  DataBuf FieldBuf(DWORD);
  io_->read(FieldBuf.data(), DWORD);
  uint32_t field = Exiv2::getULong(FieldBuf.data(), littleEndian);
  return field;
}

uint16_t AsfVideo::readWORDTag() {
  DataBuf FieldBuf(WORD);
  io_->read(FieldBuf.data(), WORD);
  uint16_t field = Exiv2::getUShort(FieldBuf.data(), littleEndian);
  return field;
}

std::string AsfVideo::readStringWCHAR(uint16_t length) {
  DataBuf FieldBuf(length);
  io_->read(FieldBuf.data(), length);
  return Util::toString16(FieldBuf);
}

std::string AsfVideo::readString(uint16_t length) {
  DataBuf FieldBuf(length);
  io_->read(FieldBuf.data(), length);
  return Exiv2::toString(FieldBuf.data());
}

void AsfVideo::fileProperties() {
  byte FileIddBuf[GUID];
  io_->read(FileIddBuf, GUID);
  char fileId[GUID_SIZE] = "";
  getGUID(FileIddBuf, fileId);
  xmpData()["Xmp.video.FileID"] = fileId;
  xmpData()["Xmp.video.FileLength"] = readQWORDTag();
  xmpData()["Xmp.video.CreationDate"] = readQWORDTag();
  xmpData()["Xmp.video.DataPackets"] = readQWORDTag();
  xmpData()["Xmp.video.Duration"] = readQWORDTag();
  xmpData()["Xmp.video.SendDuration"] = readQWORDTag();
  xmpData()["Xmp.video.Preroll"] = readQWORDTag();

  io_->seek(io_->tell() + DWORD + DWORD + DWORD, BasicIo::beg);
  xmpData()["Xmp.video.MaxBitRate"] = readDWORDTag();
}  // AsfVideo::fileProperties

void AsfVideo::aspectRatio() {
  // TODO - Make a better unified method to handle all cases of Aspect Ratio

  double aspectRatio = static_cast<double>(width_) / height_;
  aspectRatio = floor(aspectRatio * 10) / 10;
  xmpData()["Xmp.video.AspectRatio"] = aspectRatio;

  auto aR = static_cast<int>((aspectRatio * 10.0) + 0.1);

  switch (aR) {
    case 13:
      xmpData()["Xmp.video.AspectRatio"] = "4:3";
      break;
    case 17:
      xmpData()["Xmp.video.AspectRatio"] = "16:9";
      break;
    case 10:
      xmpData()["Xmp.video.AspectRatio"] = "1:1";
      break;
    case 16:
      xmpData()["Xmp.video.AspectRatio"] = "16:10";
      break;
    case 22:
      xmpData()["Xmp.video.AspectRatio"] = "2.21:1";
      break;
    case 23:
      xmpData()["Xmp.video.AspectRatio"] = "2.35:1";
      break;
    case 12:
      xmpData()["Xmp.video.AspectRatio"] = "5:4";
      break;
    default:
      xmpData()["Xmp.video.AspectRatio"] = aspectRatio;
      break;
  }
}  // AsfVideo::aspectRatio

Image::UniquePtr newAsfInstance(BasicIo::UniquePtr io, bool /*create*/) {
  auto image = std::make_unique<AsfVideo>(std::move(io));
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isAsfType(BasicIo& iIo, bool advance) {
  const int32_t len = 16;
  byte buf[len];
  iIo.read(buf, len);

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
