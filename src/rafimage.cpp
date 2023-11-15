// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "rafimage.hpp"

#include "basicio.hpp"
#include "config.h"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "jpgimage.hpp"
#include "safe_op.hpp"
#include "tiffimage.hpp"

#include <cinttypes>
#include <iostream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
RafImage::RafImage(BasicIo::UniquePtr io, bool /*create*/) :
    Image(ImageType::raf, mdExif | mdIptc | mdXmp, std::move(io)) {
}  // RafImage::RafImage

std::string RafImage::mimeType() const {
  return "image/x-fuji-raf";
}

uint32_t RafImage::pixelWidth() const {
  auto widthIter = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
  if (widthIter == exifData_.end() || widthIter->count() == 0)
    return 0;
  return widthIter->toUint32();
}

uint32_t RafImage::pixelHeight() const {
  auto heightIter = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
  if (heightIter == exifData_.end() || heightIter->count() == 0)
    return 0;
  return heightIter->toUint32();
}

void RafImage::setExifData(const ExifData& /*exifData*/) {
  // Todo: implement me!
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Exif metadata", "RAF"));
}

void RafImage::setIptcData(const IptcData& /*iptcData*/) {
  // Todo: implement me!
  throw(Error(ErrorCode::kerInvalidSettingForImage, "IPTC metadata", "RAF"));
}

void RafImage::setComment(const std::string&) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "RAF"));
}

void RafImage::printStructure(std::ostream& out, PrintStructureOption option, size_t depth) {
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  // Ensure this is the correct image type
  if (!isRafType(*io_, true)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "RAF");
  }

  const bool bPrint = option == kpsBasic || option == kpsRecursive;
  if (bPrint) {
    io_->seek(0, BasicIo::beg);  // rewind
    size_t address = io_->tell();
    constexpr auto format = " %9zu | %9" PRIu32 " | ";

    {
      out << Internal::indent(depth) << "STRUCTURE OF RAF FILE: " << io().path() << std::endl;
      out << Internal::indent(depth) << "   Address |    Length | Payload" << std::endl;
    }

    byte magicdata[17];
    io_->readOrThrow(magicdata, 16);
    magicdata[16] = 0;
    {
      out << Internal::indent(depth) << Internal::stringFormat(format, address, 16U)  // 0
          << "       magic : " << reinterpret_cast<char*>(magicdata) << std::endl;
    }

    address = io_->tell();
    byte data1[5];
    io_->read(data1, 4);
    data1[4] = 0;
    {
      out << Internal::indent(depth) << Internal::stringFormat(format, address, 4U)  // 16
          << "       data1 : " << std::string(reinterpret_cast<char*>(&data1)) << std::endl;
    }

    address = io_->tell();
    byte data2[9];
    io_->read(data2, 8);
    data2[8] = 0;
    {
      out << Internal::indent(depth) << Internal::stringFormat(format, address, 8U)  // 20
          << "       data2 : " << std::string(reinterpret_cast<char*>(&data2)) << std::endl;
    }

    address = io_->tell();
    byte camdata[33];
    io_->read(camdata, 32);
    camdata[32] = 0;
    {
      out << Internal::indent(depth) << Internal::stringFormat(format, address, 32U)  // 28
          << "      camera : " << std::string(reinterpret_cast<char*>(&camdata)) << std::endl;
    }

    address = io_->tell();
    byte dir_version[5];
    io_->read(dir_version, 4);
    dir_version[4] = 0;
    {
      out << Internal::indent(depth) << Internal::stringFormat(format, address, 4U)  // 60
          << "     version : " << std::string(reinterpret_cast<char*>(&dir_version)) << std::endl;
    }

    address = io_->tell();
    DataBuf unknown(20);
    io_->readOrThrow(unknown.data(), unknown.size());
    {
      out << Internal::indent(depth) << Internal::stringFormat(format, address, 20U)
          << "     unknown : " << Internal::binaryToString(makeSlice(unknown, 0, unknown.size())) << std::endl;
    }

    address = io_->tell();
    byte jpg_img_offset[4];
    io_->read(jpg_img_offset, 4);
    byte jpg_img_length[4];
    size_t address2 = io_->tell();
    io_->read(jpg_img_length, 4);

    uint32_t jpg_img_off = Exiv2::getULong(jpg_img_offset, bigEndian);
    uint32_t jpg_img_len = Exiv2::getULong(jpg_img_length, bigEndian);
    {
      std::stringstream j_off;
      std::stringstream j_len;
      j_off << jpg_img_off;
      j_len << jpg_img_len;
      out << Internal::indent(depth) << Internal::stringFormat(format, address, 4U) << " JPEG offset : " << j_off.str()
          << std::endl;
      out << Internal::indent(depth) << Internal::stringFormat(format, address2, 4U) << " JPEG length : " << j_len.str()
          << std::endl;
    }

    // RAFs can carry the payload in one or two parts
    uint32_t meta_off[2], meta_len[2];
    uint32_t cfa_off[2], cfa_len[2], cfa_skip[2], cfa_size[2], cfa_stride[2];
    for (size_t i = 0; i < 2; i++) {
      address = io_->tell();
      byte data[4];
      io_->readOrThrow(data, 4);
      meta_off[i] = Exiv2::getULong(data, bigEndian);
      address2 = io_->tell();
      io_->readOrThrow(data, 4);
      meta_len[i] = Exiv2::getULong(data, bigEndian);
      {
        std::stringstream c_off;
        std::stringstream c_len;
        c_off << meta_off[i];
        c_len << meta_len[i];
        out << Internal::indent(depth) << Internal::stringFormat(format, address, 4U) << "meta offset" << i + 1 << " : "
            << c_off.str() << std::endl;
        out << Internal::indent(depth) << Internal::stringFormat(format, address2, 4U) << "meta length" << i + 1
            << " : " << c_len.str() << std::endl;
      }

      address = io_->tell();
      io_->readOrThrow(data, 4);
      cfa_off[i] = Exiv2::getULong(data, bigEndian);
      address2 = io_->tell();
      io_->readOrThrow(data, 4);
      cfa_len[i] = Exiv2::getULong(data, bigEndian);
      size_t address3 = io_->tell();
      io_->readOrThrow(data, 4);
      cfa_skip[i] = Exiv2::getULong(data, bigEndian);
      size_t address4 = io_->tell();
      io_->readOrThrow(data, 4);
      cfa_size[i] = Exiv2::getULong(data, bigEndian);
      size_t address5 = io_->tell();
      io_->readOrThrow(data, 4);
      cfa_stride[i] = Exiv2::getULong(data, bigEndian);
      {
        std::stringstream c_off;
        std::stringstream c_len;
        std::stringstream c_skip;
        std::stringstream c_size;
        std::stringstream c_stride;
        c_off << cfa_off[i];
        c_len << cfa_len[i];
        c_skip << cfa_skip[i];
        c_size << cfa_size[i];
        c_stride << cfa_stride[i];
        out << Internal::indent(depth) << Internal::stringFormat(format, address, 4U) << " CFA offset" << i + 1 << " : "
            << c_off.str() << std::endl;
        out << Internal::indent(depth) << Internal::stringFormat(format, address2, 4U) << " CFA length" << i + 1
            << " : " << c_len.str() << std::endl;
        out << Internal::indent(depth) << Internal::stringFormat(format, address3, 4U) << "   CFA skip" << i + 1
            << " : " << c_skip.str() << std::endl;
        out << Internal::indent(depth) << Internal::stringFormat(format, address4, 4U) << "  CFA chunk" << i + 1
            << " : " << c_size.str() << std::endl;
        out << Internal::indent(depth) << Internal::stringFormat(format, address5, 4U) << " CFA stride" << i + 1
            << " : " << c_stride.str() << std::endl;
      }
    }

    io_->seek(jpg_img_off, BasicIo::beg);  // rewind
    address = io_->tell();
    DataBuf payload(16);  // header is different from chunks
    io_->readOrThrow(payload.data(), payload.size());
    {
      out << Internal::indent(depth) << Internal::stringFormat(format, address, jpg_img_len)
          << "   JPEG data : " << Internal::binaryToString(makeSlice(payload, 0, payload.size())) << std::endl;
    }

    io_->seek(meta_off[0], BasicIo::beg);  // rewind
    address = io_->tell();
    io_->readOrThrow(payload.data(), payload.size());
    {
      out << Internal::indent(depth) << Internal::stringFormat(format, address, meta_len[0])
          << "  meta data1 : " << Internal::binaryToString(makeSlice(payload, 0, payload.size())) << std::endl;
    }

    if (meta_off[1] && meta_len[1]) {
      io_->seek(meta_off[1], BasicIo::beg);  // rewind
      address = io_->tell();
      io_->readOrThrow(payload.data(), payload.size());
      {
        out << Internal::indent(depth) << Internal::stringFormat(format, address, meta_len[1])
            << "  meta data2 : " << Internal::binaryToString(makeSlice(payload, 0, payload.size())) << std::endl;
      }
    }

    io_->seek(cfa_off[0], BasicIo::beg);  // rewind
    address = io_->tell();
    io_->readOrThrow(payload.data(), payload.size());
    {
      out << Internal::indent(depth) << Internal::stringFormat(format, address, cfa_len[0])
          << "   CFA data1 : " << Internal::binaryToString(makeSlice(payload, 0, payload.size())) << std::endl;
    }

    if (cfa_off[1] && cfa_len[1]) {
      io_->seek(cfa_off[1], BasicIo::beg);  // rewind
      address = io_->tell();
      io_->readOrThrow(payload.data(), payload.size());
      {
        out << Internal::indent(depth) << Internal::stringFormat(format, address, cfa_len[1])  // cfa_off
            << "   CFA data2 : " << Internal::binaryToString(makeSlice(payload, 0, payload.size())) << std::endl;
      }
    }
  }
}  // RafImage::printStructure

void RafImage::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Reading RAF file " << io_->path() << "\n";
#endif
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isRafType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "RAF");
  }

  clearMetadata();

  if (io_->seek(84, BasicIo::beg) != 0)
    throw Error(ErrorCode::kerFailedToReadImageData);
  byte jpg_img_offset[4];
  if (io_->read(jpg_img_offset, 4) != 4)
    throw Error(ErrorCode::kerFailedToReadImageData);
  byte jpg_img_length[4];
  if (io_->read(jpg_img_length, 4) != 4)
    throw Error(ErrorCode::kerFailedToReadImageData);
  uint32_t jpg_img_off_u32 = Exiv2::getULong(jpg_img_offset, bigEndian);
  uint32_t jpg_img_len_u32 = Exiv2::getULong(jpg_img_length, bigEndian);

  Internal::enforce(Safe::add(jpg_img_off_u32, jpg_img_len_u32) <= io_->size(), ErrorCode::kerCorruptedMetadata);

  auto jpg_img_off = static_cast<long>(jpg_img_off_u32);
  auto jpg_img_len = static_cast<long>(jpg_img_len_u32);

  Internal::enforce(jpg_img_len >= 12, ErrorCode::kerCorruptedMetadata);

  DataBuf jpg_buf(jpg_img_len);
  if (io_->seek(jpg_img_off, BasicIo::beg) != 0)
    throw Error(ErrorCode::kerFailedToReadImageData);

  if (!jpg_buf.empty()) {
    io_->read(jpg_buf.data(), jpg_buf.size());
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
  }

  // Retrieve metadata from embedded JPEG preview image.
  try {
    auto jpg_io = std::make_unique<Exiv2::MemIo>(jpg_buf.data(), jpg_buf.size());
    auto jpg_img = JpegImage(std::move(jpg_io), false);
    jpg_img.readMetadata();
    setByteOrder(jpg_img.byteOrder());
    xmpData_ = jpg_img.xmpData();
    exifData_ = jpg_img.exifData();
    iptcData_ = jpg_img.iptcData();
    comment_ = jpg_img.comment();
  } catch (const Exiv2::Error&) {
  }

  exifData_["Exif.Image2.JPEGInterchangeFormat"] = getULong(jpg_img_offset, bigEndian);
  exifData_["Exif.Image2.JPEGInterchangeFormatLength"] = getULong(jpg_img_length, bigEndian);

  // parse the tiff
  byte readBuff[4];
  if (io_->seek(100, BasicIo::beg) != 0)
    throw Error(ErrorCode::kerFailedToReadImageData);
  if (io_->read(readBuff, 4) != 4)
    throw Error(ErrorCode::kerFailedToReadImageData);
  uint32_t tiffOffset = Exiv2::getULong(readBuff, bigEndian);

  if (io_->read(readBuff, 4) != 4)
    throw Error(ErrorCode::kerFailedToReadImageData);
  uint32_t tiffLength = Exiv2::getULong(readBuff, bigEndian);

  // sanity check.  Does tiff lie inside the file?
  Internal::enforce(Safe::add(tiffOffset, tiffLength) <= io_->size(), ErrorCode::kerCorruptedMetadata);

  if (io_->seek(tiffOffset, BasicIo::beg) != 0)
    throw Error(ErrorCode::kerFailedToReadImageData);

  // Check if this really is a tiff and then call the tiff parser.
  // Check is needed because some older models just embed a raw bitstream.
  // For those files we skip the parsing step.
  if (io_->read(readBuff, 4) != 4) {
    throw Error(ErrorCode::kerFailedToReadImageData);
  }
  io_->seek(-4, BasicIo::cur);
  if (memcmp(readBuff, "\x49\x49\x2A\x00", 4) == 0 || memcmp(readBuff, "\x4D\x4D\x00\x2A", 4) == 0) {
    DataBuf tiff(tiffLength);
    io_->read(tiff.data(), tiff.size());

    if (!io_->error() && !io_->eof()) {
      TiffParser::decode(exifData_, iptcData_, xmpData_, tiff.c_data(), tiff.size());
    }
  }
}

void RafImage::writeMetadata() {
  //! Todo: implement me!
  throw(Error(ErrorCode::kerWritingImageFormatUnsupported, "RAF"));
}  // RafImage::writeMetadata

// *************************************************************************
// free functions
Image::UniquePtr newRafInstance(BasicIo::UniquePtr io, bool create) {
  auto image = std::make_unique<RafImage>(std::move(io), create);
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isRafType(BasicIo& iIo, bool advance) {
  const int32_t len = 8;
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  int rc = memcmp(buf, "FUJIFILM", 8);
  if (!advance || rc != 0) {
    iIo.seek(-len, BasicIo::cur);
  }
  return rc == 0;
}

}  // namespace Exiv2
