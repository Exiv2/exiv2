// SPDX-License-Identifier: GPL-2.0-or-later
// included header files
#include "asfvideo.hpp"
#include <iostream>
#include "basicio.hpp"
#include "config.h"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "helper_functions.hpp"
// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {

/*!
  Look-up list for ASF Type Video Files
  Associates the GUID with its Name(i.e. Human Readable Form)
  Tags have been diferentiated into Various Categories.
  The categories have been listed above Groups
  see :
  - https://fr.wikipedia.org/wiki/Advanced_Systems_Format
  - https://exse.eyewated.com/fls/54b3ed95bbfb1a92.pdf
 */
const std::map<std::string, std::string> GUIDReferenceTags = {
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
  @brief Function used to calculate GUID, Tags comprises of 16 bytes.
      The Buffer contains the TagVocabulary in Binary Form. The information is then
      parsed into a character array GUID.
      https://fr.wikipedia.org/wiki/Globally_unique_identifier
 */

std::string getGUID(DataBuf& buf) {
  std::string GUID(36, '-');
  if (buf.size() >= 16) {
    GUID.at(0) = returnHex(buf.data()[3] / 0x10);
    GUID.at(1) = returnHex(buf.data()[3] % 0x10);
    GUID.at(2) = returnHex(buf.data()[2] / 0x10);
    GUID.at(3) = returnHex(buf.data()[2] % 0x10);
    GUID.at(4) = returnHex(buf.data()[1] / 0x10);
    GUID.at(5) = returnHex(buf.data()[1] % 0x10);
    GUID.at(6) = returnHex(buf.data()[0] / 0x10);
    GUID.at(7) = returnHex(buf.data()[0] % 0x10);

    GUID.at(9) = returnHex(buf.data()[5] / 0x10);
    GUID.at(10) = returnHex(buf.data()[5] % 0x10);
    GUID.at(11) = returnHex(buf.data()[4] / 0x10);
    GUID.at(12) = returnHex(buf.data()[4] % 0x10);

    GUID.at(14) = returnHex(buf.data()[7] / 0x10);
    GUID.at(15) = returnHex(buf.data()[7] % 0x10);
    GUID.at(16) = returnHex(buf.data()[6] / 0x10);
    GUID.at(17) = returnHex(buf.data()[6] % 0x10);

    GUID.at(19) = returnHex(buf.data()[8] / 0x10);
    GUID.at(20) = returnHex(buf.data()[8] % 0x10);
    GUID.at(21) = returnHex(buf.data()[9] / 0x10);
    GUID.at(22) = returnHex(buf.data()[9] % 0x10);

    GUID.at(24) = returnHex(buf.data()[10] / 0x10);
    GUID.at(25) = returnHex(buf.data()[10] % 0x10);
    GUID.at(26) = returnHex(buf.data()[11] / 0x10);
    GUID.at(27) = returnHex(buf.data()[11] % 0x10);
    GUID.at(28) = returnHex(buf.data()[12] / 0x10);
    GUID.at(29) = returnHex(buf.data()[12] % 0x10);
    GUID.at(30) = returnHex(buf.data()[13] / 0x10);
    GUID.at(31) = returnHex(buf.data()[13] % 0x10);
    GUID.at(32) = returnHex(buf.data()[14] / 0x10);
    GUID.at(33) = returnHex(buf.data()[14] % 0x10);
    GUID.at(34) = returnHex(buf.data()[15] / 0x10);
    GUID.at(35) = returnHex(buf.data()[15] % 0x10);
  }

  // Example of output 399595EC-8667-4E2D-8FDB-98814CE76C1E
  return GUID;
}

/*!
  @brief Function used to check if data stored in buf is equivalent to
      ASF Header TagVocabulary's GUID.
  @param buf Exiv2 byte buffer
  @return Returns true if the buffer data is equivalent to Header GUID.
 */
bool isASFType(const byte buf[]) {
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

AsfVideo::HeaderReader::HeaderReader(BasicIo::UniquePtr& io) : IdBuf_(GUID) {
  if (io->size() >= io->tell() + GUID + QWORD) {
    IdBuf_ = io->read(GUID);

    size_ = readQWORDTag(io);
    if (size_ >= GUID + QWORD)
      remaining_size_ = size_ - GUID - QWORD;
  }
}

void AsfVideo::decodeBlock() {
  HeaderReader header(io_);
  std::string guid = getGUID(header.getId());

  auto tv = GUIDReferenceTags.find(guid);
  if (tv != GUIDReferenceTags.end()) {
    if (tv->second == "Header") {
      DataBuf nbHeadersBuf(DWORD + 1);
      io_->read(nbHeadersBuf.data(), DWORD);

      uint32_t nb_headers = Exiv2::getULong(nbHeadersBuf.data(), littleEndian);
      io_->seekOrThrow(io_->tell() + BYTE * 2, BasicIo::beg,
                       ErrorCode::kerFailedToReadImageData);  // skip two reserved tags
      for (uint32_t i = 0; i < nb_headers; i++) {
        HeaderReader others(io_);
        auto guid = getGUID(others.getId());
        auto tag = GUIDReferenceTags.find(guid);
        if (tag != GUIDReferenceTags.end()) {
          if (tag->second == "File_Properties")
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
          else if (tag->second == "Degradable_JPEG_Media") {
            DegradableJPEGMedia();
          } else
            io_->seekOrThrow(io_->tell() + others.getRemainingSize(), BasicIo::beg,
                             ErrorCode::kerFailedToReadImageData);
        } else
          io_->seekOrThrow(io_->tell() + others.getRemainingSize(), BasicIo::beg, ErrorCode::kerFailedToReadImageData);
      }
    } else
      io_->seekOrThrow(io_->tell() + header.getRemainingSize(), BasicIo::beg, ErrorCode::kerFailedToReadImageData);

  } else
    io_->seekOrThrow(io_->tell() + header.getRemainingSize(), BasicIo::beg, ErrorCode::kerFailedToReadImageData);
}  // AsfVideo::decodeBlock

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

  io_->seek(io_->tell() + WORD * 3 /*3 Reserved*/, BasicIo::beg);

  uint32_t interchange_data_length = readWORDTag(io_);
  io_->seek(io_->tell() + interchange_data_length /*Interchange data*/, BasicIo::beg);
}
void AsfVideo::streamProperties() {
  DataBuf streamTypedBuf = io_->read(GUID);

  auto stream_type = getGUID(streamTypedBuf);

  enum streamTypeInfo { Audio = 1, Video = 2 };
  int stream = 0;

  auto tag_stream_type = GUIDReferenceTags.find(stream_type);
  if (tag_stream_type != GUIDReferenceTags.end()) {
    if (tag_stream_type->second == "Audio_Media")
      stream = Audio;
    else if (tag_stream_type->second == "Video_Media")
      stream = Video;

    io_->seek(io_->tell() + GUID, BasicIo::beg);  // ignore Error Correction Type

    uint64_t time_offset = readQWORDTag(io_);
    if (stream == Video)
      xmpData()["Xmp.video.TimeOffset"] = time_offset;
    else if (stream == Audio)
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

    uint16_t codec_name_length = readWORDTag(io_) * 2;
    if (codec_name_length)
      xmpData()[codec + std::string(".CodecName")] = readStringWcharTag(io_, codec_name_length);

    uint16_t codec_desc_length = readWORDTag(io_);
    if (codec_desc_length)
      xmpData()[codec + std::string(".CodecDescription")] = readStringWcharTag(io_, codec_desc_length);

    uint16_t codec_info_length = readWORDTag(io_);
    Internal::enforce(codec_info_length && codec_info_length + io_->tell() < io_->size(),
                      Exiv2::ErrorCode::kerCorruptedMetadata);
    xmpData()[codec + std::string(".CodecInfo")] = readStringTag(io_, codec_info_length);
  }
}  // AsfVideo::codecList

void AsfVideo::headerExtension() {
  io_->seek(io_->tell() + GUID /*reserved1*/ + WORD /*Reserved2*/, BasicIo::beg);
  auto header_ext_data_length = readDWORDTag(io_);
  io_->seek(io_->tell() + header_ext_data_length, BasicIo::beg);
}  // AsfVideo::headerExtension

void AsfVideo::extendedContentDescription() {
  uint16_t content_descriptor_count = readWORDTag(io_);
  std::string value;

  for (uint16_t i = 0; i < content_descriptor_count; i++) {
    uint16_t descriptor_name_length = readWORDTag(io_);
    if (descriptor_name_length)
      value += readStringWcharTag(io_, descriptor_name_length);  // Descriptor Name

    uint16_t descriptor_value_data_type = readWORDTag(io_);
    uint16_t descriptor_value_length = readWORDTag(io_);
    if (descriptor_value_length) {
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
          ;
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
  DataBuf FileIddBuf = io_->read(GUID);
  xmpData()["Xmp.video.FileID"] = getGUID(FileIddBuf);
  xmpData()["Xmp.video.FileLength"] = readQWORDTag(io_);
  xmpData()["Xmp.video.CreationDate"] = readQWORDTag(io_);
  xmpData()["Xmp.video.DataPackets"] = readQWORDTag(io_);
  xmpData()["Xmp.video.duration"] = readQWORDTag(io_);
  xmpData()["Xmp.video.SendDuration"] = readQWORDTag(io_);
  xmpData()["Xmp.video.Preroll"] = readQWORDTag(io_);

  io_->seek(io_->tell() + DWORD + DWORD + DWORD, BasicIo::beg);
  xmpData()["Xmp.video.MaxBitRate"] = readDWORDTag(io_);
}  // AsfVideo::fileProperties

void AsfVideo::aspectRatio() {
  // TODO - Make a better unified method to handle all cases of Aspect Ratio

  if (!height_)
    return;
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
