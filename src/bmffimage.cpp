// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "bmffimage.hpp"

#include "basicio.hpp"
#include "config.h"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "safe_op.hpp"
#include "tiffimage.hpp"
#include "tiffimage_int.hpp"
#include "types.hpp"

#ifdef EXV_HAVE_BROTLI
#include <brotli/decode.h>  // for JXL brob
#endif

// + standard includes
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

#define TAG_ftyp 0x66747970 /**< "ftyp" File type box */
#define TAG_avif 0x61766966 /**< "avif" AVIF */
#define TAG_avio 0x6176696f /**< "avio" AVIF */
#define TAG_avis 0x61766973 /**< "avis" AVIF */
#define TAG_heic 0x68656963 /**< "heic" HEIC */
#define TAG_heif 0x68656966 /**< "heif" HEIF */
#define TAG_heim 0x6865696d /**< "heim" HEIC */
#define TAG_heis 0x68656973 /**< "heis" HEIC */
#define TAG_heix 0x68656978 /**< "heix" HEIC */
#define TAG_mif1 0x6d696631 /**< "mif1" HEIF */
#define TAG_crx 0x63727820  /**< "crx " Canon CR3 */
#define TAG_jxl 0x6a786c20  /**< "jxl " JPEG XL file type */
#define TAG_moov 0x6d6f6f76 /**< "moov" Movie */
#define TAG_meta 0x6d657461 /**< "meta" Metadata */
#define TAG_mdat 0x6d646174 /**< "mdat" Media data */
#define TAG_uuid 0x75756964 /**< "uuid" UUID */
#define TAG_dinf 0x64696e66 /**< "dinf" Data information */
#define TAG_iprp 0x69707270 /**< "iprp" Item properties */
#define TAG_ipco 0x6970636f /**< "ipco" Item property container */
#define TAG_iinf 0x69696e66 /**< "iinf" Item info */
#define TAG_iloc 0x696c6f63 /**< "iloc" Item location */
#define TAG_ispe 0x69737065 /**< "ispe" Image spatial extents */
#define TAG_infe 0x696e6665 /**< "infe" Item Info Extension */
#define TAG_ipma 0x69706d61 /**< "ipma" Item Property Association */
#define TAG_cmt1 0x434d5431 /**< "CMT1" ifd0Id */
#define TAG_cmt2 0x434D5432 /**< "CMD2" exifID */
#define TAG_cmt3 0x434D5433 /**< "CMT3" canonID */
#define TAG_cmt4 0x434D5434 /**< "CMT4" gpsID */
#define TAG_colr 0x636f6c72 /**< "colr" Colour information */
#define TAG_exif 0x45786966 /**< "Exif" Used by JXL */
#define TAG_xml 0x786d6c20  /**< "xml " Used by JXL */
#define TAG_brob 0x62726f62 /**< "brob" Used by JXL (brotli box) */
#define TAG_thmb 0x54484d42 /**< "THMB" Canon thumbnail */
#define TAG_prvw 0x50525657 /**< "PRVW" Canon preview image */

// *****************************************************************************
// class member definitions
#ifdef EXV_ENABLE_BMFF
namespace Exiv2 {
static bool enabled = false;
EXIV2API bool enableBMFF(bool enable) {
  enabled = enable;
  return true;
}

std::string Iloc::toString() const {
  return Internal::stringFormat("ID = %u from,length = %u,%u", ID_, start_, length_);
}

BmffImage::BmffImage(BasicIo::UniquePtr io, bool /* create */) :
    Image(ImageType::bmff, mdExif | mdIptc | mdXmp, std::move(io)) {
}  // BmffImage::BmffImage

std::string BmffImage::toAscii(uint32_t n) {
  const auto p = reinterpret_cast<const char*>(&n);
  std::string result;
  for (int i = 0; i < 4; i++) {
    char c = p[isBigEndianPlatform() ? i : (3 - i)];
    result += [c]() {
      if (32 <= c && c < 127)
        return c;  // only allow 7-bit printable ascii
      if (c == 0)
        return '_';  // show 0 as _
      return '.';    // others .
    }();
  }
  return result;
}

bool BmffImage::superBox(uint32_t box) {
  return box == TAG_moov || box == TAG_dinf || box == TAG_iprp || box == TAG_ipco || box == TAG_meta ||
         box == TAG_iinf || box == TAG_iloc;
}

bool BmffImage::fullBox(uint32_t box) {
  return box == TAG_meta || box == TAG_iinf || box == TAG_iloc || box == TAG_thmb || box == TAG_prvw;
}

static bool skipBox(uint32_t box) {
  // Allows boxHandler() to optimise the reading of files by identifying
  // box types that we're not interested in. Box types listed here must
  // not appear in the cases in switch (box_type) in boxHandler().
  return box == TAG_mdat;  // mdat is where the main image lives and can be huge
}

std::string BmffImage::mimeType() const {
  switch (fileType_) {
    case TAG_avif:
    case TAG_avio:
    case TAG_avis:
      return "image/avif";
    case TAG_heic:
    case TAG_heim:
    case TAG_heis:
    case TAG_heix:
      return "image/heic";
    case TAG_heif:
    case TAG_mif1:
      return "image/heif";
    case TAG_crx:
      return "image/x-canon-cr3";
    case TAG_jxl:
      return "image/jxl";  // https://github.com/novomesk/qt-jpegxl-image-plugin/issues/1
    default:
      return "image/generic";
  }
}

uint32_t BmffImage::pixelWidth() const {
  auto imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
  if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
    return imageWidth->toUint32();
  }
  return pixelWidth_;
}

uint32_t BmffImage::pixelHeight() const {
  auto imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
  if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
    return imageHeight->toUint32();
  }
  return pixelHeight_;
}

std::string BmffImage::uuidName(const Exiv2::DataBuf& uuid) {
  const char* uuidCano = "\x85\xC0\xB6\x87\x82\xF\x11\xE0\x81\x11\xF4\xCE\x46\x2B\x6A\x48";
  const char* uuidXmp = "\xBE\x7A\xCF\xCB\x97\xA9\x42\xE8\x9C\x71\x99\x94\x91\xE3\xAF\xAC";
  const char* uuidCanp = "\xEA\xF4\x2B\x5E\x1C\x98\x4B\x88\xB9\xFB\xB7\xDC\x40\x6E\x4D\x16";
  if (uuid.cmpBytes(0, uuidCano, 16) == 0)
    return "cano";
  if (uuid.cmpBytes(0, uuidXmp, 16) == 0)
    return "xmp";
  if (uuid.cmpBytes(0, uuidCanp, 16) == 0)
    return "canp";
  return "";
}

#ifdef EXV_HAVE_BROTLI

// Wrapper class for BrotliDecoderState that automatically calls
// BrotliDecoderDestroyInstance in its destructor.
class BrotliDecoderWrapper {
  BrotliDecoderState* decoder_;

 public:
  BrotliDecoderWrapper() : decoder_(BrotliDecoderCreateInstance(NULL, NULL, NULL)) {
    if (!decoder_) {
      throw Error(ErrorCode::kerMallocFailed);
    }
  }

  ~BrotliDecoderWrapper() {
    BrotliDecoderDestroyInstance(decoder_);
  }

  BrotliDecoderState* get() const {
    return decoder_;
  }
};

void BmffImage::brotliUncompress(const byte* compressedBuf, size_t compressedBufSize, DataBuf& arr) {
  BrotliDecoderWrapper decoder;
  size_t uncompressedLen = compressedBufSize * 2;  // just a starting point
  BrotliDecoderResult result;
  int dos = 0;
  size_t available_in = compressedBufSize;
  const byte* next_in = compressedBuf;
  size_t available_out;
  byte* next_out;
  size_t total_out = 0;

  do {
    arr.alloc(uncompressedLen);
    available_out = uncompressedLen - total_out;
    next_out = arr.data() + total_out;
    result =
        BrotliDecoderDecompressStream(decoder.get(), &available_in, &next_in, &available_out, &next_out, &total_out);
    if (result == BROTLI_DECODER_RESULT_SUCCESS) {
      arr.resize(total_out);
    } else if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
      uncompressedLen *= 2;
      // DoS protection - can't be bigger than 128k
      if (uncompressedLen > 131072) {
        if (++dos > 1)
          break;
        uncompressedLen = 131072;
      }
    } else if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT) {
      // compressed input buffer in incomplete
      throw Error(ErrorCode::kerFailedToReadImageData);
    } else {
      // something bad happened
      throw Error(ErrorCode::kerErrorMessage, BrotliDecoderErrorString(BrotliDecoderGetErrorCode(decoder.get())));
    }
  } while (result != BROTLI_DECODER_RESULT_SUCCESS);

  if (result != BROTLI_DECODER_RESULT_SUCCESS) {
    throw Error(ErrorCode::kerFailedToReadImageData);
  }
}
#endif

uint64_t BmffImage::boxHandler(std::ostream& out /* = std::cout*/, Exiv2::PrintStructureOption option /* = kpsNone */,
                               uint64_t pbox_end, size_t depth) {
  const size_t address = io_->tell();
  // never visit a box twice!
  if (depth == 0)
    visits_.clear();
  if (visits_.find(address) != visits_.end() || visits_.size() > visits_max_) {
    throw Error(ErrorCode::kerCorruptedMetadata);
  }
  visits_.insert(address);

  bool bTrace = option == kpsBasic || option == kpsRecursive;
#ifdef EXIV2_DEBUG_MESSAGES
  bTrace = true;
#endif

  // 8-byte buffer for parsing the box length and type.
  byte hdrbuf[2 * sizeof(uint32_t)];

  size_t hdrsize = sizeof(hdrbuf);
  enforce(hdrsize <= static_cast<size_t>(pbox_end - address), Exiv2::ErrorCode::kerCorruptedMetadata);
  if (io_->read(reinterpret_cast<byte*>(&hdrbuf), sizeof(hdrbuf)) != sizeof(hdrbuf))
    return pbox_end;

  // The box length is encoded as a uint32_t by default, but the special value 1 means
  // that it's a uint64_t.
  uint64_t box_length = getULong(reinterpret_cast<byte*>(&hdrbuf[0]), endian_);
  uint32_t box_type = getULong(reinterpret_cast<byte*>(&hdrbuf[sizeof(uint32_t)]), endian_);
  bool bLF = true;

  if (bTrace) {
    bLF = true;
    out << Internal::indent(depth) << "Exiv2::BmffImage::boxHandler: " << toAscii(box_type)
        << Internal::stringFormat(" %8ld->%" PRIu64 " ", address, box_length);
  }

  if (box_length == 1) {
    // The box size is encoded as a uint64_t, so we need to read another 8 bytes.
    hdrsize += 8;
    enforce(hdrsize <= static_cast<size_t>(pbox_end - address), Exiv2::ErrorCode::kerCorruptedMetadata);
    DataBuf data(8);
    io_->read(data.data(), data.size());
    box_length = data.read_uint64(0, endian_);
  }

  // read data in box and restore file position
  const size_t restore = io_->tell();
  enforce(box_length >= hdrsize, Exiv2::ErrorCode::kerCorruptedMetadata);
  enforce(box_length - hdrsize <= pbox_end - restore, Exiv2::ErrorCode::kerCorruptedMetadata);

  const auto buffer_size = box_length - hdrsize;
  if (skipBox(box_type)) {
    if (bTrace) {
      out << std::endl;
    }
    // The enforce() above checks that restore + buffer_size won't
    // exceed pbox_end, and by implication, won't exceed LONG_MAX
    return restore + buffer_size;
  }

  DataBuf data(static_cast<size_t>(buffer_size));
  const size_t box_end = restore + data.size();
  io_->read(data.data(), data.size());
  io_->seek(restore, BasicIo::beg);

  size_t skip = 0;  // read position in data.pData_
  uint8_t version = 0;
  uint32_t flags = 0;

  if (fullBox(box_type)) {
    enforce(data.size() - skip >= 4, Exiv2::ErrorCode::kerCorruptedMetadata);
    flags = data.read_uint32(skip, endian_);  // version/flags
    version = static_cast<uint8_t>(flags >> 24);
    flags &= 0x00ffffff;
    skip += 4;
  }

  switch (box_type) {
    //  See notes in skipBox()
    case TAG_ftyp: {
      enforce(data.size() >= 4, Exiv2::ErrorCode::kerCorruptedMetadata);
      fileType_ = data.read_uint32(0, endian_);
      if (bTrace) {
        out << "brand: " << toAscii(fileType_);
      }
    } break;

    // 8.11.6.1
    case TAG_iinf: {
      if (bTrace) {
        out << std::endl;
        bLF = false;
      }

      enforce(data.size() - skip >= 2, Exiv2::ErrorCode::kerCorruptedMetadata);
      uint16_t n = data.read_uint16(skip, endian_);
      skip += 2;

      io_->seek(skip, BasicIo::cur);
      while (n-- > 0) {
        io_->seek(boxHandler(out, option, box_end, depth + 1), BasicIo::beg);
      }
    } break;

    // 8.11.6.2
    case TAG_infe: {  // .__._.__hvc1_ 2 0 0 1 0 1 0 0 104 118 99 49 0
      enforce(data.size() - skip >= 8, Exiv2::ErrorCode::kerCorruptedMetadata);
      /* getULong (data.pData_+skip,endian_) ; */ skip += 4;
      uint16_t ID = data.read_uint16(skip, endian_);
      skip += 2;
      /* getShort(data.pData_+skip,endian_) ; */ skip += 2;  // protection
      std::string id;
      // Check that the string has a '\0' terminator.
      const char* str = data.c_str(skip);
      const size_t maxlen = data.size() - skip;
      enforce(maxlen > 0 && strnlen(str, maxlen) < maxlen, Exiv2::ErrorCode::kerCorruptedMetadata);
      std::string name(str);
      if (name.find("Exif") != std::string::npos) {  // "Exif" or "ExifExif"
        exifID_ = ID;
        id = " *** Exif ***";
      } else if (name.find("mime\0xmp") != std::string::npos ||
                 name.find("mime\0application/rdf+xml") != std::string::npos) {
        xmpID_ = ID;
        id = " *** XMP ***";
      }
      if (bTrace) {
        out << Internal::stringFormat("ID = %3d ", ID) << name << " " << id;
      }
    } break;

    case TAG_moov:
    case TAG_iprp:
    case TAG_ipco:
    case TAG_meta: {
      if (bTrace) {
        out << std::endl;
        bLF = false;
      }
      io_->seek(skip, BasicIo::cur);
      while (io_->tell() < box_end) {
        io_->seek(boxHandler(out, option, box_end, depth + 1), BasicIo::beg);
      }
      // post-process meta box to recover Exif and XMP
      if (box_type == TAG_meta) {
        if (ilocs_.find(exifID_) != ilocs_.end()) {
          const Iloc& iloc = ilocs_.find(exifID_)->second;
          if (bTrace) {
            out << Internal::indent(depth) << "Exiv2::BMFF Exif: " << iloc.toString() << std::endl;
          }
          parseTiff(Internal::Tag::root, iloc.length_, iloc.start_);
        }
        if (ilocs_.find(xmpID_) != ilocs_.end()) {
          const Iloc& iloc = ilocs_.find(xmpID_)->second;
          if (bTrace) {
            out << Internal::indent(depth) << "Exiv2::BMFF XMP: " << iloc.toString() << std::endl;
          }
          parseXmp(iloc.length_, iloc.start_);
        }
        ilocs_.clear();
      }
    } break;

    // 8.11.3.1
    case TAG_iloc: {
      enforce(data.size() - skip >= 2, Exiv2::ErrorCode::kerCorruptedMetadata);
      uint8_t u = data.read_uint8(skip++);
      uint16_t offsetSize = u >> 4;
      uint16_t lengthSize = u & 0xF;
#if 0
                uint16_t indexSize  = 0       ;
                u             = data.read_uint8(skip++);
                if ( version == 1 || version == 2 ) {
                    indexSize = u & 0xF ;
                }
#else
      skip++;
#endif
      enforce(data.size() - skip >= (version < 2u ? 2u : 4u), Exiv2::ErrorCode::kerCorruptedMetadata);
      uint32_t itemCount = version < 2 ? data.read_uint16(skip, endian_) : data.read_uint32(skip, endian_);
      skip += version < 2 ? 2 : 4;
      if (itemCount && itemCount < box_length / 14 && offsetSize == 4 && lengthSize == 4 &&
          ((box_length - 16) % itemCount) == 0) {
        if (bTrace) {
          out << std::endl;
          bLF = false;
        }
        size_t step = (static_cast<size_t>(box_length) - 16) / itemCount;  // length of data per item.
        size_t base = skip;
        for (uint32_t i = 0; i < itemCount; i++) {
          skip = base + i * step;  // move in 14, 16 or 18 byte steps
          enforce(data.size() - skip >= (version > 2u ? 4u : 2u), Exiv2::ErrorCode::kerCorruptedMetadata);
          enforce(data.size() - skip >= step, Exiv2::ErrorCode::kerCorruptedMetadata);
          uint32_t ID = version > 2 ? data.read_uint32(skip, endian_) : data.read_uint16(skip, endian_);
          auto offset = [=] {
            if (step == 14 || step == 16)
              return data.read_uint32(skip + step - 8, endian_);
            if (step == 18)
              return data.read_uint32(skip + 4, endian_);
            return 0u;
          }();

          uint32_t ldata = data.read_uint32(skip + step - 4, endian_);
          if (bTrace) {
            out << Internal::indent(depth)
                << Internal::stringFormat("%8ld | %8ld |   ID | %4u | %6u,%6u", address + skip, step, ID, offset, ldata)
                << std::endl;
          }
          // save data for post-processing in meta box
          if (offset && ldata && ID != unknownID_) {
            ilocs_[ID] = Iloc(ID, offset, ldata);
          }
        }
      }
    } break;

    case TAG_ispe: {
      enforce(data.size() - skip >= 12, Exiv2::ErrorCode::kerCorruptedMetadata);
      skip += 4;
      uint32_t width = data.read_uint32(skip, endian_);
      skip += 4;
      uint32_t height = data.read_uint32(skip, endian_);
      skip += 4;
      if (bTrace) {
        out << "pixelWidth_, pixelHeight_ = " << Internal::stringFormat("%d, %d", width, height);
      }
      // HEIC files can have multiple ispe records
      // Store largest width/height
      if (width > pixelWidth_ && height > pixelHeight_) {
        pixelWidth_ = width;
        pixelHeight_ = height;
      }
    } break;

    // 12.1.5.2
    case TAG_colr: {
      if (data.size() >= (skip + 4 + 8)) {  // .____.HLino..__mntrR 2 0 0 0 0 12 72 76 105 110 111 2 16 ...
        // https://www.ics.uci.edu/~dan/class/267/papers/jpeg2000.pdf
        uint8_t meth = data.read_uint8(skip + 0);
        uint8_t prec = data.read_uint8(skip + 1);
        uint8_t approx = data.read_uint8(skip + 2);
        auto colour_type = std::string(data.c_str(), 4);
        skip += 4;
        if (colour_type == "rICC" || colour_type == "prof") {
          DataBuf profile(data.c_data(skip), data.size() - skip);
          setIccProfile(std::move(profile));
        } else if (meth == 2 && prec == 0 && approx == 0) {
          // JP2000 files have a 3 byte head // 2 0 0 icc......
          skip -= 1;
          DataBuf profile(data.c_data(skip), data.size() - skip);
          setIccProfile(std::move(profile));
        }
      }
    } break;

    case TAG_uuid: {
      DataBuf uuid(16);
      io_->read(uuid.data(), uuid.size());
      std::string name = uuidName(uuid);
      if (bTrace) {
        out << " uuidName " << name << std::endl;
        bLF = false;
      }
      if (name == "cano" || name == "canp") {
        if (name == "canp") {
          // based on
          // https://github.com/lclevy/canon_cr3/blob/7be75d6/parse_cr3.py#L271
          io_->seek(8, BasicIo::cur);
        }
        while (io_->tell() < box_end) {
          io_->seek(boxHandler(out, option, box_end, depth + 1), BasicIo::beg);
        }
      } else if (name == "xmp") {
        parseXmp(box_length, io_->tell());
      }
    } break;

    case TAG_cmt1:
      parseTiff(Internal::Tag::root, box_length);
      break;
    case TAG_cmt2:
      parseTiff(Internal::Tag::cmt2, box_length);
      break;
    case TAG_cmt3:
      parseTiff(Internal::Tag::cmt3, box_length);
      break;
    case TAG_cmt4:
      parseTiff(Internal::Tag::cmt4, box_length);
      break;
    case TAG_exif:
      parseTiff(Internal::Tag::root, buffer_size, io_->tell());
      break;
    case TAG_xml:
      parseXmp(buffer_size, io_->tell());
      break;
    case TAG_brob: {
      enforce(data.size() >= 4, Exiv2::ErrorCode::kerCorruptedMetadata);
      uint32_t realType = data.read_uint32(0, endian_);
      if (bTrace) {
        out << "type: " << toAscii(realType);
      }
#ifdef EXV_HAVE_BROTLI
      DataBuf arr;
      brotliUncompress(data.c_data(4), data.size() - 4, arr);
      if (realType == TAG_exif) {
        uint32_t offset = Safe::add(arr.read_uint32(0, endian_), 4u);
        enforce(Safe::add(offset, 4u) < arr.size(), Exiv2::ErrorCode::kerCorruptedMetadata);
        Internal::TiffParserWorker::decode(exifData(), iptcData(), xmpData(), arr.c_data(offset), arr.size() - offset,
                                           Internal::Tag::root, Internal::TiffMapping::findDecoder);
      } else if (realType == TAG_xml) {
        try {
          Exiv2::XmpParser::decode(xmpData(), std::string(arr.c_str(), arr.size()));
        } catch (...) {
          throw Error(ErrorCode::kerFailedToReadImageData);
        }
      }
#endif
    } break;
    case TAG_thmb:
      switch (version) {
        case 0:  // JPEG
          parseCr3Preview(data, out, bTrace, version, skip, skip + 2, skip + 4, skip + 12);
          break;
        case 1:  // HDR
          parseCr3Preview(data, out, bTrace, version, skip + 2, skip + 4, skip + 8, skip + 12);
          break;
        default:
          break;
      }
      break;
    case TAG_prvw:
      switch (version) {
        case 0:  // JPEG
        case 1:  // HDR
          parseCr3Preview(data, out, bTrace, version, skip + 2, skip + 4, skip + 8, skip + 12);
          break;
        default:
          break;
      }
      break;

    default:
      break; /* do nothing */
  }
  if (bLF && bTrace)
    out << std::endl;

  // return address of next box
  return box_end;
}

void BmffImage::parseTiff(uint32_t root_tag, uint64_t length, uint64_t start) {
  enforce(start <= io_->size(), ErrorCode::kerCorruptedMetadata);
  enforce(length <= io_->size() - start, ErrorCode::kerCorruptedMetadata);
  enforce(start <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()), ErrorCode::kerCorruptedMetadata);
  enforce(length <= std::numeric_limits<size_t>::max(), ErrorCode::kerCorruptedMetadata);

  // read and parse exif data
  const size_t restore = io_->tell();
  DataBuf exif(static_cast<size_t>(length));
  io_->seek(static_cast<int64_t>(start), BasicIo::beg);
  if (exif.size() > 8 && io_->read(exif.data(), exif.size()) == exif.size()) {
    // hunt for "II" or "MM"
    const size_t eof = std::numeric_limits<size_t>::max();  // impossible value for punt
    size_t punt = eof;
    for (size_t i = 0; i < exif.size() - 9 && punt == eof; ++i) {
      if (exif.read_uint8(i) == exif.read_uint8(i + 1))
        if (exif.read_uint8(i) == 'I' || exif.read_uint8(i) == 'M')
          punt = i;
    }
    if (punt != eof) {
      Internal::TiffParserWorker::decode(exifData(), iptcData(), xmpData(), exif.c_data(punt), exif.size() - punt,
                                         root_tag, Internal::TiffMapping::findDecoder);
    }
  }
  io_->seek(restore, BasicIo::beg);
}

void BmffImage::parseTiff(uint32_t root_tag, uint64_t length) {
  if (length > 8) {
    enforce(length - 8 <= io_->size() - io_->tell(), ErrorCode::kerCorruptedMetadata);
    enforce(length - 8 <= std::numeric_limits<size_t>::max(), ErrorCode::kerCorruptedMetadata);
    DataBuf data(static_cast<size_t>(length - 8u));
    const size_t bufRead = io_->read(data.data(), data.size());

    if (io_->error())
      throw Error(ErrorCode::kerFailedToReadImageData);
    if (bufRead != data.size())
      throw Error(ErrorCode::kerInputDataReadFailed);

    Internal::TiffParserWorker::decode(exifData(), iptcData(), xmpData(), data.c_data(), data.size(), root_tag,
                                       Internal::TiffMapping::findDecoder);
  }
}

void BmffImage::parseXmp(uint64_t length, uint64_t start) {
  enforce(start <= io_->size(), ErrorCode::kerCorruptedMetadata);
  enforce(length <= io_->size() - start, ErrorCode::kerCorruptedMetadata);

  const size_t restore = io_->tell();
  io_->seek(static_cast<int64_t>(start), BasicIo::beg);

  auto lengthSizeT = static_cast<size_t>(length);
  DataBuf xmp(lengthSizeT + 1);
  xmp.write_uint8(lengthSizeT, 0);  // ensure xmp is null terminated!
  if (io_->read(xmp.data(), lengthSizeT) != lengthSizeT)
    throw Error(ErrorCode::kerInputDataReadFailed);
  if (io_->error())
    throw Error(ErrorCode::kerFailedToReadImageData);
  try {
    Exiv2::XmpParser::decode(xmpData(), std::string(xmp.c_str()));
  } catch (...) {
    throw Error(ErrorCode::kerFailedToReadImageData);
  }

  io_->seek(restore, BasicIo::beg);
}

/// \todo instead of passing the last 4 parameters, pass just one and build the different offsets inside
void BmffImage::parseCr3Preview(DataBuf& data, std::ostream& out, bool bTrace, uint8_t version, size_t width_offset,
                                size_t height_offset, size_t size_offset, size_t relative_position) {
  // Derived from https://github.com/lclevy/canon_cr3
  const size_t here = io_->tell();
  enforce(here <= std::numeric_limits<size_t>::max() - relative_position, ErrorCode::kerCorruptedMetadata);
  NativePreview nativePreview;
  nativePreview.position_ = here + relative_position;
  nativePreview.width_ = data.read_uint16(width_offset, endian_);
  nativePreview.height_ = data.read_uint16(height_offset, endian_);
  nativePreview.size_ = data.read_uint32(size_offset, endian_);
  nativePreview.filter_ = "";
  nativePreview.mimeType_ = [version] {
    if (version == 0)
      return "image/jpeg";
    return "application/octet-stream";
  }();
  nativePreviews_.push_back(nativePreview);

  if (bTrace) {
    out << Internal::stringFormat("width,height,size = %zu,%zu,%zu", nativePreview.width_, nativePreview.height_,
                                  nativePreview.size_);
  }
}

void BmffImage::setExifData(const ExifData& /*exifData*/) {
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Exif metadata", "BMFF"));
}

void BmffImage::setIptcData(const IptcData& /*iptcData*/) {
  throw(Error(ErrorCode::kerInvalidSettingForImage, "IPTC metadata", "BMFF"));
}

void BmffImage::setXmpData(const XmpData& /*xmpData*/) {
  throw(Error(ErrorCode::kerInvalidSettingForImage, "XMP metadata", "BMFF"));
}

void BmffImage::setComment(const std::string&) {
  // bmff files are read-only
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "BMFF"));
}

void BmffImage::openOrThrow() {
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  // Ensure that this is the correct image type
  if (!isBmffType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "BMFF");
  }
}  // Bmff::openOrThrow();

void BmffImage::readMetadata() {
  openOrThrow();
  IoCloser closer(*io_);

  clearMetadata();
  ilocs_.clear();
  visits_max_ = io_->size() / 16;
  unknownID_ = 0xffff;
  exifID_ = unknownID_;
  xmpID_ = unknownID_;

  uint64_t address = 0;
  const auto file_end = io_->size();
  while (address < file_end) {
    io_->seek(address, BasicIo::beg);
    address = boxHandler(std::cout, kpsNone, file_end, 0);
  }
  bReadMetadata_ = true;
}  // BmffImage::readMetadata

void BmffImage::printStructure(std::ostream& out, Exiv2::PrintStructureOption option, size_t depth) {
  if (!bReadMetadata_)
    readMetadata();

  switch (option) {
    default:
      break;  // do nothing

    case kpsIccProfile: {
      out.write(iccProfile_.c_str(), iccProfile_.size());
    } break;

#ifdef EXV_HAVE_XMP_TOOLKIT
    case kpsXMP: {
      std::string xmp;
      if (Exiv2::XmpParser::encode(xmp, xmpData())) {
        throw Exiv2::Error(Exiv2::ErrorCode::kerErrorMessage, "Failed to serialize XMP data");
      }
      out << xmp;
    } break;
#endif
    case kpsBasic:  // drop
    case kpsRecursive: {
      openOrThrow();
      IoCloser closer(*io_);

      uint64_t address = 0;
      const auto file_end = io_->size();
      while (address < file_end) {
        io_->seek(address, BasicIo::beg);
        address = boxHandler(out, option, file_end, depth);
      }
    } break;
  }
}

void BmffImage::writeMetadata() {
  // bmff files are read-only
  throw(Error(ErrorCode::kerWritingImageFormatUnsupported, "BMFF"));
}  // BmffImage::writeMetadata

// *************************************************************************
// free functions
Image::UniquePtr newBmffInstance(BasicIo::UniquePtr io, bool create) {
  auto image = std::make_unique<BmffImage>(std::move(io), create);
  if (!image->good()) {
    image.reset();
  }
  return image;
}

bool isBmffType(BasicIo& iIo, bool advance) {
  if (!enabled) {
    return false;
  }
  const int32_t len = 12;
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }

  // bmff should start with "ftyp"
  bool const is_ftyp = (buf[4] == 'f' && buf[5] == 't' && buf[6] == 'y' && buf[7] == 'p');
  // jxl files have a special start indicator of "JXL "
  bool const is_jxl = (buf[4] == 'J' && buf[5] == 'X' && buf[6] == 'L' && buf[7] == ' ');

  bool matched = is_jxl || is_ftyp;
  if (!advance || !matched) {
    iIo.seek(0, BasicIo::beg);
  }
  return matched;
}
}  // namespace Exiv2
#else  // ifdef EXV_ENABLE_BMFF
namespace Exiv2 {
EXIV2API bool enableBMFF(bool) {
  return false;
}
}  // namespace Exiv2
#endif
