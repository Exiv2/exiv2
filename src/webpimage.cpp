// SPDX-License-Identifier: GPL-2.0-or-later

/*
  Google's WEBP container spec can be found at the link below:
  https://developers.google.com/speed/webp/docs/riff_container
*/

// included header files
#include "webpimage.hpp"

#include "basicio.hpp"
#include "config.h"
#include "convert.hpp"
#include "enforce.hpp"
#include "futils.hpp"
#include "image_int.hpp"
#include "safe_op.hpp"
#include "types.hpp"

#include <iostream>

namespace {
[[maybe_unused]] std::string binaryToHex(const uint8_t* data, size_t size) {
  std::stringstream hexOutput;

  auto tl = size / 16 * 16;
  auto tl_offset = size - tl;

  for (size_t loop = 0; loop < size; loop++) {
    if (data[loop] < 16) {
      hexOutput << "0";
    }
    hexOutput << std::hex << static_cast<int>(data[loop]);
    if ((loop % 8) == 7) {
      hexOutput << "  ";
    }
    if ((loop % 16) == 15 || loop == (tl + tl_offset - 1)) {
      int max = 15;
      if (loop >= tl) {
        max = static_cast<int>(tl_offset) - 1;
        for (int offset = 0; offset < static_cast<int>(16 - tl_offset); offset++) {
          if ((offset % 8) == 7) {
            hexOutput << "  ";
          }
          hexOutput << "   ";
        }
      }
      hexOutput << " ";
      for (int offset = max; offset >= 0; offset--) {
        if (offset == (max - 8)) {
          hexOutput << "  ";
        }
        uint8_t c = '.';
        if (data[loop - offset] >= 0x20 && data[loop - offset] <= 0x7E) {
          c = data[loop - offset];
        }
        hexOutput << static_cast<char>(c);
      }
      hexOutput << '\n';
    }
  }

  hexOutput << '\n' << '\n' << '\n';

  return hexOutput.str();
}
}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {

WebPImage::WebPImage(BasicIo::UniquePtr io) : Image(ImageType::webp, mdNone, std::move(io)) {
}  // WebPImage::WebPImage

std::string WebPImage::mimeType() const {
  return "image/webp";
}

void WebPImage::setIptcData(const IptcData& /*iptcData*/) {
  // not supported
  // just quietly ignore the request
  // throw(Error(ErrorCode::kerInvalidSettingForImage, "IPTC metadata", "WebP"));
}

void WebPImage::setComment(const std::string&) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "WebP"));
}

/* =========================================== */

void WebPImage::writeMetadata() {
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  MemIo tempIo;

  doWriteMetadata(tempIo);  // may throw
  io_->close();
  io_->transfer(tempIo);  // may throw
}  // WebPImage::writeMetadata

void WebPImage::doWriteMetadata(BasicIo& outIo) {
  if (!io_->isopen())
    throw Error(ErrorCode::kerInputDataReadFailed);
  if (!outIo.isopen())
    throw Error(ErrorCode::kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Writing metadata" << '\n';
#endif

  byte data[WEBP_TAG_SIZE * 3];
  DataBuf chunkId(WEBP_TAG_SIZE + 1);
  chunkId.write_uint8(WEBP_TAG_SIZE, '\0');

  io_->readOrThrow(data, WEBP_TAG_SIZE * 3, Exiv2::ErrorCode::kerCorruptedMetadata);
  uint64_t filesize = Exiv2::getULong(data + WEBP_TAG_SIZE, littleEndian);

  /* Set up header */
  if (outIo.write(data, WEBP_TAG_SIZE * 3) != WEBP_TAG_SIZE * 3)
    throw Error(ErrorCode::kerImageWriteFailed);

  /* Parse Chunks */
  bool has_size = false;
  bool has_xmp = false;
  bool has_exif = false;
  bool has_vp8x = false;
  bool has_alpha = false;
  bool has_icc = iccProfileDefined();

  uint32_t width = 0;
  uint32_t height = 0;

  byte size_buff[WEBP_TAG_SIZE];
  Blob blob;

  if (!exifData_.empty()) {
    ExifParser::encode(blob, littleEndian, exifData_);
    if (!blob.empty()) {
      has_exif = true;
    }
  }

  if (!xmpData_.empty() && !writeXmpFromPacket()) {
    XmpParser::encode(xmpPacket_, xmpData_, XmpParser::useCompactFormat | XmpParser::omitAllFormatting);
  }
  has_xmp = !xmpPacket_.empty();
  std::string xmp(xmpPacket_);

  /* Verify for a VP8X Chunk First before writing in
   case we have any exif or xmp data, also check
   for any chunks with alpha frame/layer set */
  while (!io_->eof() && io_->tell() < filesize) {
    io_->readOrThrow(chunkId.data(), WEBP_TAG_SIZE, Exiv2::ErrorCode::kerCorruptedMetadata);
    io_->readOrThrow(size_buff, WEBP_TAG_SIZE, Exiv2::ErrorCode::kerCorruptedMetadata);
    const uint32_t size_u32 = Exiv2::getULong(size_buff, littleEndian);

    DataBuf payload(size_u32);
    if (!payload.empty()) {
      io_->readOrThrow(payload.data(), payload.size(), Exiv2::ErrorCode::kerCorruptedMetadata);
      if (payload.size() % 2) {
        byte c = 0;
        io_->readOrThrow(&c, 1, Exiv2::ErrorCode::kerCorruptedMetadata);
      }
    }

    /* Chunk with information about features
     used in the file. */
    if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8X) && !has_vp8x) {
      has_vp8x = true;
    }
    if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8X) && !has_size) {
      Internal::enforce(size_u32 >= 10, Exiv2::ErrorCode::kerCorruptedMetadata);
      has_size = true;
      byte size_buf[WEBP_TAG_SIZE];

      // Fetch width - stored in 24bits
      memcpy(&size_buf, payload.c_data(4), 3);
      size_buf[3] = 0;
      width = Exiv2::getULong(size_buf, littleEndian) + 1;

      // Fetch height - stored in 24bits
      memcpy(&size_buf, payload.c_data(7), 3);
      size_buf[3] = 0;
      height = Exiv2::getULong(size_buf, littleEndian) + 1;
    }

    /* Chunk with animation control data. */
#ifdef __CHECK_FOR_ALPHA__  // Maybe in the future
    if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ANIM) && !has_alpha) {
      has_alpha = true;
    }
#endif

    /* Chunk with lossy image data. */
#ifdef __CHECK_FOR_ALPHA__  // Maybe in the future
    if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8) && !has_alpha) {
      has_alpha = true;
    }
#endif
    if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8) && !has_size) {
      Internal::enforce(size_u32 >= 10, Exiv2::ErrorCode::kerCorruptedMetadata);
      has_size = true;
      byte size_buf[2];

      /* Refer to this https://tools.ietf.org/html/rfc6386
         for height and width reference for VP8 chunks */

      // Fetch width - stored in 16bits
      memcpy(&size_buf, payload.c_data(6), 2);
      width = Exiv2::getUShort(size_buf, littleEndian) & 0x3fff;

      // Fetch height - stored in 16bits
      memcpy(&size_buf, payload.c_data(8), 2);
      height = Exiv2::getUShort(size_buf, littleEndian) & 0x3fff;
    }

    /* Chunk with lossless image data. */
    if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8L) && !has_alpha) {
      Internal::enforce(size_u32 >= 5, Exiv2::ErrorCode::kerCorruptedMetadata);
      if ((payload.read_uint8(4) & WEBP_VP8X_ALPHA_BIT) == WEBP_VP8X_ALPHA_BIT) {
        has_alpha = true;
      }
    }
    if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8L) && !has_size) {
      Internal::enforce(size_u32 >= 5, Exiv2::ErrorCode::kerCorruptedMetadata);
      has_size = true;
      byte size_buf_w[2];
      byte size_buf_h[3];

      /* For VP8L chunks width & height are stored in 28 bits
         of a 32 bit field requires bitshifting to get actual
         sizes. Width and height are split even into 14 bits
         each. Refer to this https://goo.gl/bpgMJf */

      // Fetch width - 14 bits wide
      memcpy(&size_buf_w, payload.c_data(1), 2);
      size_buf_w[1] &= 0x3F;
      width = Exiv2::getUShort(size_buf_w, littleEndian) + 1;

      // Fetch height - 14 bits wide
      memcpy(&size_buf_h, payload.c_data(2), 3);
      size_buf_h[0] = ((size_buf_h[0] >> 6) & 0x3) | ((size_buf_h[1] & 0x3FU) << 0x2);
      size_buf_h[1] = ((size_buf_h[1] >> 6) & 0x3) | ((size_buf_h[2] & 0xFU) << 0x2);
      height = Exiv2::getUShort(size_buf_h, littleEndian) + 1;
    }

    /* Chunk with animation frame. */
    if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ANMF) && !has_alpha) {
      Internal::enforce(size_u32 >= 6, Exiv2::ErrorCode::kerCorruptedMetadata);
      if ((payload.read_uint8(5) & 0x2) == 0x2) {
        has_alpha = true;
      }
    }
    if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ANMF) && !has_size) {
      Internal::enforce(size_u32 >= 12, Exiv2::ErrorCode::kerCorruptedMetadata);
      has_size = true;
      byte size_buf[WEBP_TAG_SIZE];

      // Fetch width - stored in 24bits
      memcpy(&size_buf, payload.c_data(6), 3);
      size_buf[3] = 0;
      width = Exiv2::getULong(size_buf, littleEndian) + 1;

      // Fetch height - stored in 24bits
      memcpy(&size_buf, payload.c_data(9), 3);
      size_buf[3] = 0;
      height = Exiv2::getULong(size_buf, littleEndian) + 1;
    }

    /* Chunk with alpha data. */
    if (equalsWebPTag(chunkId, "ALPH") && !has_alpha) {
      has_alpha = true;
    }
  }

  /* Inject a VP8X chunk if one isn't available. */
  if (!has_vp8x) {
    inject_VP8X(outIo, has_xmp, has_exif, has_alpha, has_icc, width, height);
  }

  io_->seek(12, BasicIo::beg);
  while (!io_->eof() && io_->tell() < filesize) {
    io_->readOrThrow(chunkId.data(), 4, Exiv2::ErrorCode::kerCorruptedMetadata);
    io_->readOrThrow(size_buff, 4, Exiv2::ErrorCode::kerCorruptedMetadata);

    const uint32_t size_u32 = Exiv2::getULong(size_buff, littleEndian);
    DataBuf payload(size_u32);
    io_->readOrThrow(payload.data(), size_u32, Exiv2::ErrorCode::kerCorruptedMetadata);
    if (io_->tell() % 2)
      io_->seek(+1, BasicIo::cur);  // skip pad

    if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8X)) {
      Internal::enforce(size_u32 >= 1, Exiv2::ErrorCode::kerCorruptedMetadata);
      if (has_icc) {
        const uint8_t x = payload.read_uint8(0);
        payload.write_uint8(0, x | WEBP_VP8X_ICC_BIT);
      } else {
        const uint8_t x = payload.read_uint8(0);
        payload.write_uint8(0, x & ~WEBP_VP8X_ICC_BIT);
      }

      if (has_xmp) {
        const uint8_t x = payload.read_uint8(0);
        payload.write_uint8(0, x | WEBP_VP8X_XMP_BIT);
      } else {
        const uint8_t x = payload.read_uint8(0);
        payload.write_uint8(0, x & ~WEBP_VP8X_XMP_BIT);
      }

      if (has_exif) {
        const uint8_t x = payload.read_uint8(0);
        payload.write_uint8(0, x | WEBP_VP8X_EXIF_BIT);
      } else {
        const uint8_t x = payload.read_uint8(0);
        payload.write_uint8(0, x & ~WEBP_VP8X_EXIF_BIT);
      }

      if (outIo.write(chunkId.c_data(), WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
        throw Error(ErrorCode::kerImageWriteFailed);
      if (outIo.write(size_buff, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
        throw Error(ErrorCode::kerImageWriteFailed);
      if (outIo.write(payload.c_data(), payload.size()) != payload.size())
        throw Error(ErrorCode::kerImageWriteFailed);
      if (outIo.tell() % 2 && outIo.write(&WEBP_PAD_ODD, 1) != 1)
        throw Error(ErrorCode::kerImageWriteFailed);

      if (has_icc) {
        if (outIo.write(reinterpret_cast<const byte*>(WEBP_CHUNK_HEADER_ICCP), WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
          throw Error(ErrorCode::kerImageWriteFailed);
        ul2Data(data, static_cast<uint32_t>(iccProfile_.size()), littleEndian);
        if (outIo.write(data, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
          throw Error(ErrorCode::kerImageWriteFailed);
        if (outIo.write(iccProfile_.c_data(), iccProfile_.size()) != iccProfile_.size()) {
          throw Error(ErrorCode::kerImageWriteFailed);
        }
        has_icc = false;
      }
    } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ICCP)) {
      // Skip it altogether handle it prior to here :)
    } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_EXIF)) {
      // Skip and add new data afterwards
    } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_XMP)) {
      // Skip and add new data afterwards
    } else {
      if (outIo.write(chunkId.c_data(), WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
        throw Error(ErrorCode::kerImageWriteFailed);
      if (outIo.write(size_buff, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
        throw Error(ErrorCode::kerImageWriteFailed);
      if (outIo.write(payload.c_data(), payload.size()) != payload.size())
        throw Error(ErrorCode::kerImageWriteFailed);
    }

    // Encoder required to pad odd sized data with a null byte
    if (outIo.tell() % 2 && outIo.write(&WEBP_PAD_ODD, 1) != 1)
      throw Error(ErrorCode::kerImageWriteFailed);
  }

  if (has_exif) {
    if (outIo.write(reinterpret_cast<const byte*>(WEBP_CHUNK_HEADER_EXIF), WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
      throw Error(ErrorCode::kerImageWriteFailed);
    us2Data(data, static_cast<uint16_t>(blob.size()) + 8, bigEndian);
    ul2Data(data, static_cast<uint32_t>(blob.size()), littleEndian);
    if (outIo.write(data, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
      throw Error(ErrorCode::kerImageWriteFailed);
    if (outIo.write(blob.data(), blob.size()) != blob.size()) {
      throw Error(ErrorCode::kerImageWriteFailed);
    }
    if (outIo.tell() % 2 && outIo.write(&WEBP_PAD_ODD, 1) != 1)
      throw Error(ErrorCode::kerImageWriteFailed);
  }

  if (has_xmp) {
    if (outIo.write(reinterpret_cast<const byte*>(WEBP_CHUNK_HEADER_XMP), WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
      throw Error(ErrorCode::kerImageWriteFailed);
    ul2Data(data, static_cast<uint32_t>(xmpPacket().size()), littleEndian);
    if (outIo.write(data, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
      throw Error(ErrorCode::kerImageWriteFailed);
    if (outIo.write(reinterpret_cast<const byte*>(xmp.data()), xmp.size()) != xmp.size()) {
      throw Error(ErrorCode::kerImageWriteFailed);
    }
    if (outIo.tell() % 2 && outIo.write(&WEBP_PAD_ODD, 1) != 1)
      throw Error(ErrorCode::kerImageWriteFailed);
  }

  // Fix File Size Payload Data
  outIo.seek(0, BasicIo::beg);
  filesize = outIo.size() - 8;
  outIo.seek(4, BasicIo::beg);
  ul2Data(data, static_cast<uint32_t>(filesize), littleEndian);
  if (outIo.write(data, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
    throw Error(ErrorCode::kerImageWriteFailed);

}  // WebPImage::writeMetadata

/* =========================================== */

void WebPImage::printStructure(std::ostream& out, PrintStructureOption option, size_t depth) {
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  // Ensure this is the correct image type
  if (!isWebPType(*io_, true)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "WEBP");
  }

  bool bPrint = option == kpsBasic || option == kpsRecursive;
  if (bPrint || option == kpsXMP || option == kpsIccProfile || option == kpsIptcErase) {
    byte data[WEBP_TAG_SIZE * 2];
    io_->read(data, WEBP_TAG_SIZE * 2);
    uint64_t filesize = Exiv2::getULong(data + WEBP_TAG_SIZE, littleEndian);
    DataBuf chunkId(5);
    chunkId.write_uint8(4, '\0');

    if (bPrint) {
      out << Internal::indent(depth) << "STRUCTURE OF WEBP FILE: " << io().path() << '\n';
      out << Internal::indent(depth) << " Chunk |   Length |   Offset | Payload" << '\n';
    }

    io_->seek(0, BasicIo::beg);  // rewind
    while (!io_->eof() && io_->tell() < filesize) {
      auto offset = io_->tell();
      byte size_buff[WEBP_TAG_SIZE];
      io_->read(chunkId.data(), WEBP_TAG_SIZE);
      io_->read(size_buff, WEBP_TAG_SIZE);
      const uint32_t size = Exiv2::getULong(size_buff, littleEndian);
      DataBuf payload(offset ? size : WEBP_TAG_SIZE);  // header is different from chunks
      io_->read(payload.data(), payload.size());

      if (bPrint) {
        out << Internal::indent(depth) << stringFormat("  {} | {:8} | {:8} | ", chunkId.c_str(), size, offset)
            << Internal::binaryToString(makeSlice(payload, 0, payload.size() > 32 ? 32 : payload.size())) << '\n';
      }

      if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_EXIF) && option == kpsRecursive) {
        // create memio object with the payload, then print the structure
        MemIo p(payload.c_data(), payload.size());
        printTiffStructure(p, out, option, depth + 1);
      }

      bool bPrintPayload = (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_XMP) && option == kpsXMP) ||
                           (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ICCP) && option == kpsIccProfile);
      if (bPrintPayload) {
        out.write(payload.c_str(), payload.size());
      }

      if (offset && io_->tell() % 2)
        io_->seek(+1, BasicIo::cur);  // skip padding byte on sub-chunks
    }
  }
}

/* =========================================== */

void WebPImage::readMetadata() {
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isWebPType(*io_, true)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAJpeg);
  }
  clearMetadata();

  byte data[12];
  DataBuf chunkId(5);
  chunkId.write_uint8(4, '\0');

  io_->readOrThrow(data, WEBP_TAG_SIZE * 3, Exiv2::ErrorCode::kerCorruptedMetadata);

  const uint32_t filesize = Safe::add(Exiv2::getULong(data + WEBP_TAG_SIZE, littleEndian), 8U);
  Internal::enforce(filesize <= io_->size(), Exiv2::ErrorCode::kerCorruptedMetadata);

  WebPImage::decodeChunks(filesize);

}  // WebPImage::readMetadata

void WebPImage::decodeChunks(uint32_t filesize) {
  DataBuf chunkId(5);
  byte size_buff[WEBP_TAG_SIZE];
  bool has_canvas_data = false;

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Reading metadata" << '\n';
#endif

  chunkId.write_uint8(4, '\0');
  while (!io_->eof() && io_->tell() < filesize) {
    io_->readOrThrow(chunkId.data(), WEBP_TAG_SIZE, Exiv2::ErrorCode::kerCorruptedMetadata);
    io_->readOrThrow(size_buff, WEBP_TAG_SIZE, Exiv2::ErrorCode::kerCorruptedMetadata);

    const uint32_t size = Exiv2::getULong(size_buff, littleEndian);

    // Check that `size` is within bounds.
    Internal::enforce(io_->tell() <= filesize, Exiv2::ErrorCode::kerCorruptedMetadata);
    Internal::enforce(size <= (filesize - io_->tell()), Exiv2::ErrorCode::kerCorruptedMetadata);

    if (DataBuf payload(size); payload.empty()) {
      io_->seek(size, BasicIo::cur);
    } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8X) && !has_canvas_data) {
      Internal::enforce(size >= 10, Exiv2::ErrorCode::kerCorruptedMetadata);

      has_canvas_data = true;
      byte size_buf[WEBP_TAG_SIZE];

      io_->readOrThrow(payload.data(), payload.size(), Exiv2::ErrorCode::kerCorruptedMetadata);

      // Fetch width
      memcpy(&size_buf, payload.c_data(4), 3);
      size_buf[3] = 0;
      pixelWidth_ = Exiv2::getULong(size_buf, littleEndian) + 1;

      // Fetch height
      memcpy(&size_buf, payload.c_data(7), 3);
      size_buf[3] = 0;
      pixelHeight_ = Exiv2::getULong(size_buf, littleEndian) + 1;
    } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8) && !has_canvas_data) {
      Internal::enforce(size >= 10, Exiv2::ErrorCode::kerCorruptedMetadata);

      has_canvas_data = true;
      io_->readOrThrow(payload.data(), payload.size(), Exiv2::ErrorCode::kerCorruptedMetadata);
      byte size_buf[WEBP_TAG_SIZE];

      // Fetch width""
      memcpy(&size_buf, payload.c_data(6), 2);
      size_buf[2] = 0;
      size_buf[3] = 0;
      pixelWidth_ = Exiv2::getULong(size_buf, littleEndian) & 0x3fff;

      // Fetch height
      memcpy(&size_buf, payload.c_data(8), 2);
      size_buf[2] = 0;
      size_buf[3] = 0;
      pixelHeight_ = Exiv2::getULong(size_buf, littleEndian) & 0x3fff;
    } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_VP8L) && !has_canvas_data) {
      Internal::enforce(size >= 5, Exiv2::ErrorCode::kerCorruptedMetadata);

      has_canvas_data = true;
      byte size_buf_w[2];
      byte size_buf_h[3];

      io_->readOrThrow(payload.data(), payload.size(), Exiv2::ErrorCode::kerCorruptedMetadata);

      // Fetch width
      memcpy(&size_buf_w, payload.c_data(1), 2);
      size_buf_w[1] &= 0x3F;
      pixelWidth_ = Exiv2::getUShort(size_buf_w, littleEndian) + 1;

      // Fetch height
      memcpy(&size_buf_h, payload.c_data(2), 3);
      size_buf_h[0] = ((size_buf_h[0] >> 6) & 0x3) | ((size_buf_h[1] & 0x3FU) << 0x2);
      size_buf_h[1] = ((size_buf_h[1] >> 6) & 0x3) | ((size_buf_h[2] & 0xFU) << 0x2);
      pixelHeight_ = Exiv2::getUShort(size_buf_h, littleEndian) + 1;
    } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ANMF) && !has_canvas_data) {
      Internal::enforce(size >= 12, Exiv2::ErrorCode::kerCorruptedMetadata);

      has_canvas_data = true;
      byte size_buf[WEBP_TAG_SIZE];

      io_->readOrThrow(payload.data(), payload.size(), Exiv2::ErrorCode::kerCorruptedMetadata);

      // Fetch width
      memcpy(&size_buf, payload.c_data(6), 3);
      size_buf[3] = 0;
      pixelWidth_ = Exiv2::getULong(size_buf, littleEndian) + 1;

      // Fetch height
      memcpy(&size_buf, payload.c_data(9), 3);
      size_buf[3] = 0;
      pixelHeight_ = Exiv2::getULong(size_buf, littleEndian) + 1;
    } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_ICCP)) {
      io_->readOrThrow(payload.data(), payload.size(), Exiv2::ErrorCode::kerCorruptedMetadata);
      this->setIccProfile(std::move(payload));
    } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_EXIF)) {
      io_->readOrThrow(payload.data(), payload.size(), Exiv2::ErrorCode::kerCorruptedMetadata);

      byte size_buff2[2];
      // 4 meaningful bytes + 2 padding bytes
      byte exifLongHeader[] = {0xFF, 0x01, 0xFF, 0xE1, 0x00, 0x00};
      byte exifShortHeader[] = {0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
      const byte exifTiffLEHeader[] = {0x49, 0x49, 0x2A};        // "MM*"
      const byte exifTiffBEHeader[] = {0x4D, 0x4D, 0x00, 0x2A};  // "II\0*"
      size_t offset = 0;
      bool s_header = false;
      bool le_header = false;
      bool be_header = false;
      size_t pos = getHeaderOffset(payload.c_data(), payload.size(), exifLongHeader, 4);

      if (pos == std::string::npos) {
        pos = getHeaderOffset(payload.c_data(), payload.size(), exifLongHeader, 6);
        if (pos != std::string::npos) {
          s_header = true;
        }
      }
      if (pos == std::string::npos) {
        pos = getHeaderOffset(payload.c_data(), payload.size(), exifTiffLEHeader, 3);
        if (pos != std::string::npos) {
          le_header = true;
        }
      }
      if (pos == std::string::npos) {
        pos = getHeaderOffset(payload.c_data(), payload.size(), exifTiffBEHeader, 4);
        if (pos != std::string::npos) {
          be_header = true;
        }
      }

      if (s_header) {
        offset += 6;
      }
      if (be_header || le_header) {
        offset += 12;
      }

      const size_t sizePayload = Safe::add(payload.size(), offset);
      DataBuf rawExifData(sizePayload);

      if (s_header) {
        us2Data(size_buff2, static_cast<uint16_t>(sizePayload - 6), bigEndian);
        std::copy_n(reinterpret_cast<char*>(&exifLongHeader), 4, rawExifData.begin());
        std::copy_n(reinterpret_cast<char*>(&size_buff2), 2, rawExifData.begin() + 4);
      }

      if (be_header || le_header) {
        us2Data(size_buff2, static_cast<uint16_t>(sizePayload - 6), bigEndian);
        std::copy_n(reinterpret_cast<char*>(&exifLongHeader), 4, rawExifData.begin());
        std::copy_n(reinterpret_cast<char*>(&size_buff2), 2, rawExifData.begin() + 4);
        std::copy_n(reinterpret_cast<char*>(&exifShortHeader), 6, rawExifData.begin() + 6);
      }

      std::copy(payload.begin(), payload.end(), rawExifData.begin() + offset);

#ifdef EXIV2_DEBUG_MESSAGES
      std::cout << "Display Hex Dump [size:" << sizePayload << "]" << '\n';
      std::cout << binaryToHex(rawExifData.c_data(), sizePayload);
#endif

      if (pos != std::string::npos) {
        XmpData xmpData;
        ByteOrder bo = ExifParser::decode(exifData_, payload.c_data(pos), payload.size() - pos);
        setByteOrder(bo);
      } else {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Failed to decode Exif metadata." << '\n';
#endif
        exifData_.clear();
      }
    } else if (equalsWebPTag(chunkId, WEBP_CHUNK_HEADER_XMP)) {
      io_->readOrThrow(payload.data(), payload.size(), Exiv2::ErrorCode::kerCorruptedMetadata);
      xmpPacket_.assign(payload.c_str(), payload.size());
      if (!xmpPacket_.empty() && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Failed to decode XMP metadata." << '\n';
#endif
      } else {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Display Hex Dump [size:" << payload.size() << "]" << '\n';
        std::cout << binaryToHex(payload.c_data(), payload.size());
#endif
      }
    } else {
      io_->seek(size, BasicIo::cur);
    }

    if (io_->tell() % 2)
      io_->seek(+1, BasicIo::cur);
  }
}

/* =========================================== */

Image::UniquePtr newWebPInstance(BasicIo::UniquePtr io, bool /*create*/) {
  auto image = std::make_unique<WebPImage>(std::move(io));
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isWebPType(BasicIo& iIo, bool /*advance*/) {
  if (iIo.size() < 12) {
    return false;
  }
  const int32_t len = 4;
  const unsigned char RiffImageId[4] = {'R', 'I', 'F', 'F'};
  const unsigned char WebPImageId[4] = {'W', 'E', 'B', 'P'};
  byte webp[len];
  byte data[len];
  byte riff[len];
  iIo.readOrThrow(riff, len, Exiv2::ErrorCode::kerCorruptedMetadata);
  iIo.readOrThrow(data, len, Exiv2::ErrorCode::kerCorruptedMetadata);
  iIo.readOrThrow(webp, len, Exiv2::ErrorCode::kerCorruptedMetadata);
  bool matched_riff = (memcmp(riff, RiffImageId, len) == 0);
  bool matched_webp = (memcmp(webp, WebPImageId, len) == 0);
  iIo.seek(-12, BasicIo::cur);
  return matched_riff && matched_webp;
}

/*!
 @brief Function used to check equality of a Tags with a
 particular string (ignores case while comparing).
 @param buf Data buffer that will contain Tag to compare
 @param str char* Pointer to string
 @return Returns true if the buffer value is equal to string.
 */
bool WebPImage::equalsWebPTag(const Exiv2::DataBuf& buf, const char* str) {
  for (int i = 0; i < 4; i++)
    if (toupper(buf.read_uint8(i)) != str[i])
      return false;
  return true;
}

/*!
 @brief Function used to add missing EXIF & XMP flags
 to the feature section.
 @param  iIo get BasicIo pointer to inject data
 @param has_xmp Verify if we have xmp data and set required flag
 @param has_exif Verify if we have exif data and set required flag
 */
void WebPImage::inject_VP8X(BasicIo& iIo, bool has_xmp, bool has_exif, bool has_alpha, bool has_icc, uint32_t width,
                            uint32_t height) const {
  const byte size[4] = {0x0A, 0x00, 0x00, 0x00};
  byte data[10] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  iIo.write(reinterpret_cast<const byte*>(WEBP_CHUNK_HEADER_VP8X), WEBP_TAG_SIZE);
  iIo.write(size, WEBP_TAG_SIZE);

  if (has_alpha) {
    data[0] |= WEBP_VP8X_ALPHA_BIT;
  }

  if (has_icc) {
    data[0] |= WEBP_VP8X_ICC_BIT;
  }

  if (has_xmp) {
    data[0] |= WEBP_VP8X_XMP_BIT;
  }

  if (has_exif) {
    data[0] |= WEBP_VP8X_EXIF_BIT;
  }

  /* set width - stored in 24bits*/
  Internal::enforce(width > 0, Exiv2::ErrorCode::kerCorruptedMetadata);
  uint32_t w = width - 1;
  data[4] = w & 0xFF;
  data[5] = (w >> 8) & 0xFF;
  data[6] = (w >> 16) & 0xFF;

  /* set height - stored in 24bits */
  Internal::enforce(width > 0, Exiv2::ErrorCode::kerCorruptedMetadata);
  uint32_t h = height - 1;
  data[7] = h & 0xFF;
  data[8] = (h >> 8) & 0xFF;
  data[9] = (h >> 16) & 0xFF;

  iIo.write(data, 10);

  /* Handle inject an icc profile right after VP8X chunk */
  if (has_icc) {
    byte size_buff[WEBP_TAG_SIZE];
    ul2Data(size_buff, static_cast<uint32_t>(iccProfile_.size()), littleEndian);
    if (iIo.write(reinterpret_cast<const byte*>(WEBP_CHUNK_HEADER_ICCP), WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
      throw Error(ErrorCode::kerImageWriteFailed);
    if (iIo.write(size_buff, WEBP_TAG_SIZE) != WEBP_TAG_SIZE)
      throw Error(ErrorCode::kerImageWriteFailed);
    if (iIo.write(iccProfile_.c_data(), iccProfile_.size()) != iccProfile_.size())
      throw Error(ErrorCode::kerImageWriteFailed);
    if (iIo.tell() % 2 && iIo.write(&WEBP_PAD_ODD, 1) != 1)
      throw Error(ErrorCode::kerImageWriteFailed);
  }
}

size_t WebPImage::getHeaderOffset(const byte* data, size_t data_size, const byte* header, size_t header_size) {
  size_t pos = std::string::npos;  // error value
  if (data_size < header_size) {
    return pos;
  }
  for (size_t i = 0; i < data_size - header_size; i++) {
    if (memcmp(header, &data[i], header_size) == 0) {
      pos = i;
      break;
    }
  }
  return pos;
}

}  // namespace Exiv2
