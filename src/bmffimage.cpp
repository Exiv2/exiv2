// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "bmffimage.hpp"

#include "basicio.hpp"
#include "bmffbox_int.hpp"
#include "config.h"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "tags.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffimage_int.hpp"
#include "types.hpp"
#include "utils.hpp"

#ifdef EXV_HAVE_BROTLI
#include <brotli/decode.h>  // for JXL brob
#include "safe_op.hpp"
#endif

// + standard includes
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>
#include <string>

enum TAG {
  ftyp = 0x66747970U,  //!< "ftyp" File type box */
  avci = 0x61766369U,  //!< "avci" AVC */
  avcs = 0x61766373U,  //!< "avcs" AVC */
  avif = 0x61766966U,  //!< "avif" AVIF */
  avio = 0x6176696fU,  //!< "avio" AVIF */
  avis = 0x61766973U,  //!< "avis" AVIF */
  heic = 0x68656963U,  //!< "heic" HEIC */
  heif = 0x68656966U,  //!< "heif" HEIF */
  heim = 0x6865696dU,  //!< "heim" HEIC */
  heis = 0x68656973U,  //!< "heis" HEIC */
  heix = 0x68656978U,  //!< "heix" HEIC */
  j2is = 0x6a326973U,  //!< "j2is" HEJ2K */
  j2ki = 0x6a326b69U,  //!< "j2ki" HEJ2K */
  mif1 = 0x6d696631U,  //!< "mif1" HEIF */
  crx = 0x63727820U,   //!< "crx " Canon CR3 */
  jxl = 0x6a786c20U,   //!< "jxl " JPEG XL file type */
  moov = 0x6d6f6f76U,  //!< "moov" Movie */
  meta = 0x6d657461U,  //!< "meta" Metadata */
  mdat = 0x6d646174U,  //!< "mdat" Media data */
  uuid = 0x75756964U,  //!< "uuid" UUID */
  dinf = 0x64696e66U,  //!< "dinf" Data information */
  iprp = 0x69707270U,  //!< "iprp" Item properties */
  ipco = 0x6970636fU,  //!< "ipco" Item property container */
  iinf = 0x69696e66U,  //!< "iinf" Item info */
  iloc = 0x696c6f63U,  //!< "iloc" Item location */
  ispe = 0x69737065U,  //!< "ispe" Image spatial extents */
  infe = 0x696e6665U,  //!< "infe" Item Info Extension */
  ipma = 0x69706d61U,  //!< "ipma" Item Property Association */
  cmt1 = 0x434d5431U,  //!< "CMT1" ifd0Id */
  cmt2 = 0x434D5432U,  //!< "CMD2" exifID */
  cmt3 = 0x434D5433U,  //!< "CMT3" canonID */
  cmt4 = 0x434D5434U,  //!< "CMT4" gpsID */
  colr = 0x636f6c72U,  //!< "colr" Colour information */
  exif = 0x45786966U,  //!< "Exif" Used by JXL */
  xml = 0x786d6c20U,   //!< "xml " Used by JXL */
  brob = 0x62726f62U,  //!< "brob" Used by JXL (brotli box) */
  thmb = 0x54484d42U,  //!< "THMB" Canon thumbnail */
  prvw = 0x50525657U,  //!< "PRVW" Canon preview image */
};

// *****************************************************************************
// class member definitions
namespace Exiv2 {
bool enableBMFF(bool) {
#ifndef EXV_ENABLE_BMFF
  return false;
}
#else
  return true;
}

std::string Iloc::toString() const {
  return stringFormat("ID = {} from,length = {},{}", ID_, start_, length_);
}

BmffImage::BmffImage(BasicIo::UniquePtr io, const ImageCtorParams& params) :
    Image(ImageType::bmff, mdExif | mdIptc | mdXmp, std::move(io), params) {
}  // BmffImage::BmffImage

std::string BmffImage::toAscii(uint32_t n) {
  std::string result(sizeof(uint32_t), '\0');
  for (size_t i = 0; i < result.size(); ++i) {
    auto c = static_cast<unsigned char>(n >> (8 * (3 - i)));
    if (c == 0)
      result[i] = '_';
    else if (c < 32 || c > 126)
      result[i] = '.';
    else
      result[i] = static_cast<char>(c);
  }
  return result;
}

bool BmffImage::superBox(uint32_t box) {
  return box == TAG::moov || box == TAG::dinf || box == TAG::iprp || box == TAG::ipco || box == TAG::meta ||
         box == TAG::iinf || box == TAG::iloc;
}

bool BmffImage::fullBox(uint32_t box) {
  return box == TAG::meta || box == TAG::iinf || box == TAG::iloc || box == TAG::thmb || box == TAG::prvw;
}

static bool skipBox(uint32_t box) {
  // Allows boxHandler() to optimise the reading of files by identifying
  // box types that we're not interested in. Box types listed here must
  // not appear in the legacy adapter's switch (box_type).
  return box == 0 || box == TAG::mdat;  // mdat is where the main image lives and can be huge
}

std::string BmffImage::mimeType() const {
  switch (fileType_) {
    case TAG::avci:
      return "image/avci";
    case TAG::avcs:
      return "image/avcs";
    case TAG::avif:
    case TAG::avio:
    case TAG::avis:
      return "image/avif";
    case TAG::heic:
    case TAG::heim:
    case TAG::heis:
    case TAG::heix:
      return "image/heic";
    case TAG::heif:
    case TAG::mif1:
      return "image/heif";
    case TAG::j2is:
      return "image/j2is";
    case TAG::j2ki:
      return "image/hej2k";
    case TAG::crx:
      return "image/x-canon-cr3";
    case TAG::jxl:
      return "image/jxl";  // https://github.com/novomesk/qt-jpegxl-image-plugin/issues/1
    default:
      return "image/generic";
  }
}

uint32_t BmffImage::pixelWidth() const {
  auto imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
  if (imageWidth == exifData_.end() || imageWidth->count() == 0)
    return pixelWidth_;
  return imageWidth->toUint32();
}

uint32_t BmffImage::pixelHeight() const {
  auto imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
  if (imageHeight == exifData_.end() || imageHeight->count() == 0)
    return pixelHeight_;
  return imageHeight->toUint32();
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
using BrotliDecoder = std::unique_ptr<BrotliDecoderState, decltype(&BrotliDecoderDestroyInstance)>;

void BmffImage::brotliUncompress(const byte* compressedBuf, size_t compressedBufSize, DataBuf& arr) {
  auto decoder = BrotliDecoder(BrotliDecoderCreateInstance(nullptr, nullptr, nullptr), BrotliDecoderDestroyInstance);
  size_t uncompressedLen = compressedBufSize * 2;  // just a starting point
  BrotliDecoderResult result = BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;
  int dos = 0;
  size_t available_in = compressedBufSize;
  const byte* next_in = compressedBuf;
  size_t available_out;
  byte* next_out;
  size_t total_out = 0;

  while (result != BROTLI_DECODER_RESULT_SUCCESS) {
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
        if (++dos > 1 || total_out > 131072)
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
  }

  if (result != BROTLI_DECODER_RESULT_SUCCESS) {
    throw Error(ErrorCode::kerFailedToReadImageData);
  }
}
#endif

namespace Internal {

/*!
  @brief Preserve legacy metadata interpretation while borrowing the shared box reader.

  The image and its open input must outlive this adapter. Friendship grants state
  access without adding BmffImage members or exported methods. Aggregate core
  budgets are unrestricted; legacy per-root visit and recursion limits apply.
 */
class BmffLegacyReader {
 public:
  //! @brief Borrow the image and retain its existing per-root visit and recursion limits.
  explicit BmffLegacyReader(BmffImage& image) :
      image_(image),
      reader_(*image.io_, {std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max(),
                           std::numeric_limits<unsigned>::max()}) {
  }

  //! @brief Visit all top-level boxes, preserving stream position at the end of the file.
  void all(std::ostream& out, PrintStructureOption option, size_t depth) {
    auto input = reader_.cursor({0, reader_.fileSize()});
    walk(input, out, option, depth);
  }

  //! @brief Preserve the exported boxHandler entry point for one subtree at the current position.
  uint64_t one(std::ostream& out, PrintStructureOption option, uint64_t parentEnd, size_t depth) {
    const auto start = image_.io_->tell();
    enforce(start <= parentEnd, ErrorCode::kerCorruptedMetadata);
    auto input = reader_.cursor({start, parentEnd - start});
    auto box = reader_.readBox(input);
    auto fields = reader_.cursor(box.payload());
    handle(box, fields, out, option, depth);
    return box.span.offset + box.span.size;
  }

 private:
  // Use the core's sibling traversal; counted iinf children may leave trailing bytes opaque.
  void walk(BmffCursor& input, std::ostream& out, PrintStructureOption option, size_t depth,
            uint64_t count = std::numeric_limits<uint64_t>::max()) {
    enforce(depth <= std::numeric_limits<unsigned>::max(), ErrorCode::kerCorruptedMetadata);
    uint64_t visited = 0;
    reader_.visit(
        input, static_cast<unsigned>(depth),
        [&](BmffBox& box, BmffCursor& fields) {
          handle(box, fields, out, option, depth);
          ++visited;
        },
        count);
    enforce(count == std::numeric_limits<uint64_t>::max() || visited == count, ErrorCode::kerCorruptedMetadata);
    image_.io_->seekOrThrow(static_cast<int64_t>(input.position()), BasicIo::beg, ErrorCode::kerInputDataReadFailed);
  }

  // Decode fields in the historical order; policy and compatibility heuristics stay here.
  void handle(const BmffBox& box, BmffCursor& fields, std::ostream& out, PrintStructureOption option, size_t depth) {
    const auto address = box.span.offset;
    if (depth == 0)
      image_.visits_.clear();
    if (image_.visits_.contains(address) || image_.visits_.size() > image_.visits_max_ ||
        depth >= image_.max_recursion_depth_)
      throw Error(ErrorCode::kerCorruptedMetadata);
    image_.visits_.insert(address);

#ifdef EXIV2_DEBUG_MESSAGES
    const bool bTrace = true;
#else
    const bool bTrace = option == kpsBasic || option == kpsRecursive;
#endif
    const auto box_type = box.type;
    const auto box_length = box.span.size;
    bool bLF = true;
    if (bTrace) {
      out << Internal::indent(depth) << "Exiv2::BmffImage::boxHandler: " << image_.toAscii(box_type)
          << stringFormat(" {:8}->{} ", address, box.size32);
    }

    // Legacy field and preview offsets exclude the size/type header, but include UUID bytes.
    const auto headerSize = box.headerSize - (box_type == TAG::uuid ? 16 : 0);
    const auto restore = box.span.offset + headerSize;
    const auto buffer_size = box.span.size - headerSize;
    if (skipBox(box_type)) {
      if (bTrace)
        out << '\n';
      return;
    }

    // Preserve legacy whole-box field buffers while using checked source reads.
    enforce(buffer_size <= std::numeric_limits<size_t>::max(), ErrorCode::kerCorruptedMetadata);
    DataBuf data(static_cast<size_t>(buffer_size));
    auto payload = reader_.cursor({restore, buffer_size});
    payload.read(data.data(), data.size());
    image_.io_->seekOrThrow(static_cast<int64_t>(restore), BasicIo::beg, ErrorCode::kerInputDataReadFailed);
    size_t skip = 0;  // read position in data.pData_
    uint8_t version = 0;

    if (image_.fullBox(box_type)) {
      Internal::enforce(data.size() - skip >= 4, Exiv2::ErrorCode::kerCorruptedMetadata);
      const auto full = decodeBmffFullBox(data.read_uint32(skip, BmffImage::endian_));
      version = full.version;
      skip += 4;
    }

    switch (box_type) {
      //  See notes in skipBox()
      case TAG::ftyp: {
        Internal::enforce(data.size() >= 4, Exiv2::ErrorCode::kerCorruptedMetadata);
        image_.fileType_ = data.read_uint32(0, BmffImage::endian_);
        if (bTrace) {
          out << "brand: " << image_.toAscii(image_.fileType_);
        }
      } break;

      // 8.11.6.1
      case TAG::iinf: {
        if (bTrace) {
          out << '\n';
          bLF = false;
        }

        Internal::enforce(data.size() - skip >= 2, Exiv2::ErrorCode::kerCorruptedMetadata);
        // The legacy reader uses a 16-bit count even for newer iinf versions.
        uint16_t n = data.read_uint16(skip, BmffImage::endian_);
        skip += 2;

        auto children = reader_.cursor({restore + skip, buffer_size - skip});
        walk(children, out, option, depth + 1, n);
      } break;

      // 8.11.6.2
      case TAG::infe: {  // .__._.__hvc1_ 2 0 0 1 0 1 0 0 104 118 99 49 0
        Internal::enforce(data.size() - skip >= 8, Exiv2::ErrorCode::kerCorruptedMetadata);
        /* getULong (data.pData_+skip,BmffImage::endian_) ; */ skip += 4;
        uint16_t ID = data.read_uint16(skip, BmffImage::endian_);
        skip += 2;
        /* getShort(data.pData_+skip,BmffImage::endian_) ; */ skip += 2;  // protection
        std::string id;
        // Check that the string has a '\0' terminator.
        const char* str = data.c_str(skip);
        const size_t maxlen = data.size() - skip;
        Internal::enforce(maxlen > 0 && strnlen(str, maxlen) < maxlen, Exiv2::ErrorCode::kerCorruptedMetadata);
        std::string name(str);
        if (Internal::contains(name, "Exif")) {  // "Exif" or "ExifExif"
          image_.exifID_ = ID;
          id = " *** Exif ***";
        } else if (Internal::contains(name, "mime\0xmp") || Internal::contains(name, "mime\0application/rdf+xml")) {
          image_.xmpID_ = ID;
          id = " *** XMP ***";
        }
        if (bTrace) {
          out << stringFormat("ID = {:3} {} {}", ID, name, id);
        }
      } break;

      case TAG::moov:
      case TAG::iprp:
      case TAG::ipco:
      case TAG::meta: {
        if (bTrace) {
          out << '\n';
          bLF = false;
        }
        auto children = reader_.cursor({restore + skip, buffer_size - skip});
        walk(children, out, option, depth + 1);
        // post-process meta box to recover Exif and XMP
        if (box_type == TAG::meta) {
          auto ilo = image_.ilocs_.find(image_.exifID_);
          if (ilo != image_.ilocs_.end()) {
            const Iloc& iloc = ilo->second;
            if (bTrace) {
              out << Internal::indent(depth) << "Exiv2::BMFF Exif: " << iloc.toString() << '\n';
            }
            image_.parseTiff(Internal::Tag::root, iloc.length_, iloc.start_);
          }
          ilo = image_.ilocs_.find(image_.xmpID_);
          if (ilo != image_.ilocs_.end()) {
            const Iloc& iloc = ilo->second;
            if (bTrace) {
              out << Internal::indent(depth) << "Exiv2::BMFF XMP: " << iloc.toString() << '\n';
            }
            image_.parseXmp(iloc.length_, iloc.start_);
          }
          image_.ilocs_.clear();
        }
      } break;

      // Retain legacy fixed-stride iloc heuristics separately from strict item validation.
      case TAG::iloc: {
        Internal::enforce(data.size() - skip >= 2, Exiv2::ErrorCode::kerCorruptedMetadata);
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
        Internal::enforce(data.size() - skip >= (version < 2u ? 2u : 4u), Exiv2::ErrorCode::kerCorruptedMetadata);
        uint32_t itemCount =
            version < 2 ? data.read_uint16(skip, BmffImage::endian_) : data.read_uint32(skip, BmffImage::endian_);
        skip += version < 2 ? 2 : 4;
        if (itemCount && itemCount < box_length / 14 && offsetSize == 4 && lengthSize == 4 &&
            ((box_length - 16) % itemCount) == 0) {
          if (bTrace) {
            out << '\n';
            bLF = false;
          }
          auto step = (static_cast<size_t>(box_length) - 16) / itemCount;  // length of data per item.
          size_t base = skip;
          for (uint32_t i = 0; i < itemCount; i++) {
            skip = base + (i * step);  // move in 14, 16 or 18 byte steps
            Internal::enforce(data.size() - skip >= (version > 2u ? 4u : 2u), Exiv2::ErrorCode::kerCorruptedMetadata);
            Internal::enforce(data.size() - skip >= step, Exiv2::ErrorCode::kerCorruptedMetadata);
            uint32_t ID =
                version > 2 ? data.read_uint32(skip, BmffImage::endian_) : data.read_uint16(skip, BmffImage::endian_);
            auto offset = [&data, skip, step] {
              if (step == 14 || step == 16)
                return data.read_uint32(skip + step - 8, BmffImage::endian_);
              if (step == 18)
                return data.read_uint32(skip + 4, BmffImage::endian_);
              return 0u;
            }();

            uint32_t ldata = data.read_uint32(skip + step - 4, BmffImage::endian_);
            if (bTrace) {
              out << Internal::indent(depth)
                  << stringFormat("{:8} | {:8} |   ID | {:4} | {:6},{:6}\n", address + skip, step, ID, offset, ldata);
            }
            // save data for post-processing in meta box
            if (offset && ldata && ID != image_.unknownID_) {
              image_.ilocs_[ID] = Iloc{ID, offset, ldata};
            }
          }
        }
      } break;

      case TAG::ispe: {
        Internal::enforce(data.size() - skip >= 12, Exiv2::ErrorCode::kerCorruptedMetadata);
        skip += 4;
        uint32_t width = data.read_uint32(skip, BmffImage::endian_);
        skip += 4;
        uint32_t height = data.read_uint32(skip, BmffImage::endian_);
        skip += 4;
        if (bTrace) {
          out << stringFormat("pixelWidth_, pixelHeight_ = {}, {}", width, height);
        }
        // HEIC files can have multiple ispe records
        // Store largest width/height
        if (width > image_.pixelWidth_ && height > image_.pixelHeight_) {
          image_.pixelWidth_ = width;
          image_.pixelHeight_ = height;
        }
      } break;

      // 12.1.5.2
      case TAG::colr: {
        if (data.size() >= (skip + 4 + 8)) {  // .____.HLino..__mntrR 2 0 0 0 0 12 72 76 105 110 111 2 16 ...
          // https://www.ics.uci.edu/~dan/class/267/papers/jpeg2000.pdf
          uint8_t meth = data.read_uint8(skip + 0);
          uint8_t prec = data.read_uint8(skip + 1);
          uint8_t approx = data.read_uint8(skip + 2);
          auto colour_type = std::string(data.c_str(), 4);
          skip += 4;
          if (colour_type == "rICC" || colour_type == "prof") {
            DataBuf profile(data.c_data(skip), data.size() - skip);
            image_.setIccProfile(std::move(profile));
          } else if (meth == 2 && prec == 0 && approx == 0) {
            // JP2000 files have a 3 byte head // 2 0 0 icc......
            skip -= 1;
            DataBuf profile(data.c_data(skip), data.size() - skip);
            image_.setIccProfile(std::move(profile));
          }
        }
      } break;

      case TAG::uuid: {
        DataBuf uuid(box.userType.data(), box.userType.size());
        image_.io_->seekOrThrow(static_cast<int64_t>(fields.position()), BasicIo::beg,
                                ErrorCode::kerInputDataReadFailed);
        std::string name = image_.uuidName(uuid);
        if (bTrace) {
          out << " uuidName " << name << '\n';
          bLF = false;
        }
        if (name == "cano" || name == "canp") {
          if (name == "canp") {
            // based on
            // https://github.com/lclevy/canon_cr3/blob/7be75d6/parse_cr3.py#L271
            fields.advance(8);
          }
          walk(fields, out, option, depth + 1);
        } else if (name == "xmp") {
          // Preserve the legacy UUID packet length and its existing decoding errors.
          image_.parseXmp(box_length, image_.io_->tell());
        }
      } break;

      case TAG::cmt1:
        image_.parseTiff(Internal::Tag::root, box_length);
        break;
      case TAG::cmt2:
        image_.parseTiff(Internal::Tag::cmt2, box_length);
        break;
      case TAG::cmt3:
        image_.parseTiff(Internal::Tag::cmt3, box_length);
        break;
      case TAG::cmt4:
        image_.parseTiff(Internal::Tag::cmt4, box_length);
        break;
      case TAG::exif:
        image_.parseTiff(Internal::Tag::root, buffer_size, image_.io_->tell());
        break;
      case TAG::xml:
        image_.parseXmp(buffer_size, image_.io_->tell());
        break;
      case TAG::brob: {
        Internal::enforce(data.size() >= 4, Exiv2::ErrorCode::kerCorruptedMetadata);
        uint32_t realType = data.read_uint32(0, BmffImage::endian_);
        if (bTrace) {
          out << "type: " << image_.toAscii(realType);
        }
#ifdef EXV_HAVE_BROTLI
        DataBuf arr;
        image_.brotliUncompress(data.c_data(4), data.size() - 4, arr);
        const DecodeParams dp(image_.max_recursion_depth_);
        if (realType == TAG::exif) {
          uint32_t offset = Safe::add(arr.read_uint32(0, BmffImage::endian_), 4u);
          Internal::enforce(Safe::add(offset, 4u) < arr.size(), Exiv2::ErrorCode::kerCorruptedMetadata);
          Internal::TiffParserWorker::decode(image_.exifData(), image_.iptcData(), image_.xmpData(), arr.c_data(offset),
                                             arr.size() - offset, Internal::Tag::root,
                                             Internal::TiffMapping::findDecoder, dp);
        } else if (realType == TAG::xml) {
          try {
            Exiv2::XmpParser::decode(image_.xmpData(), std::string(arr.c_str(), arr.size()), dp);
          } catch (...) {
            throw Error(ErrorCode::kerFailedToReadImageData);
          }
        }
#endif
      } break;
      case TAG::thmb:
        switch (version) {
          case 0:  // JPEG
            image_.parseCr3Preview(data, out, bTrace, version, skip, skip + 2, skip + 4, skip + 12);
            break;
          case 1:  // HDR
            image_.parseCr3Preview(data, out, bTrace, version, skip + 2, skip + 4, skip + 8, skip + 12);
            break;
          default:
            break;
        }
        break;
      case TAG::prvw:
        switch (version) {
          case 0:  // JPEG
          case 1:  // HDR
            image_.parseCr3Preview(data, out, bTrace, version, skip + 2, skip + 4, skip + 8, skip + 12);
            break;
          default:
            break;
        }
        break;

      default:
        break; /* do nothing */
    }
    if (bLF && bTrace)
      out << '\n';
  }

  BmffImage& image_;
  BmffReader reader_;
};
}  // namespace Internal

uint64_t BmffImage::boxHandler(std::ostream& out, PrintStructureOption option, uint64_t pbox_end, size_t depth) {
  return Internal::BmffLegacyReader(*this).one(out, option, pbox_end, depth);
}

void BmffImage::parseTiff(uint32_t root_tag, uint64_t length, uint64_t start) {
  Internal::enforce(start <= io_->size(), ErrorCode::kerCorruptedMetadata);
  Internal::enforce(length <= io_->size() - start, ErrorCode::kerCorruptedMetadata);
  Internal::enforce(start <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max()),
                    ErrorCode::kerCorruptedMetadata);
  Internal::enforce(length <= std::numeric_limits<size_t>::max(), ErrorCode::kerCorruptedMetadata);

  // read and parse exif data
  const size_t restore = io_->tell();
  DataBuf exif(static_cast<size_t>(length));
  io_->seek(static_cast<int64_t>(start), BasicIo::beg);
  if (exif.size() > 8 && io_->read(exif.data(), exif.size()) == exif.size()) {
    // hunt for "II" or "MM"
    const size_t eof = std::numeric_limits<size_t>::max();  // impossible value for punt
    size_t punt = eof;
    for (size_t i = 0; i < exif.size() - 9 && punt == eof; ++i) {
      auto charCurrent = exif.read_uint8(i);
      auto charNext = exif.read_uint8(i + 1);
      if (charCurrent == charNext && (charCurrent == 'I' || charCurrent == 'M'))
        punt = i;
    }
    if (punt != eof) {
      const DecodeParams dp(max_recursion_depth_);
      Internal::TiffParserWorker::decode(exifData(), iptcData(), xmpData(), exif.c_data(punt), exif.size() - punt,
                                         root_tag, Internal::TiffMapping::findDecoder, dp);
    }
  }
  io_->seek(restore, BasicIo::beg);
}

void BmffImage::parseTiff(uint32_t root_tag, uint64_t length) {
  if (length > 8) {
    Internal::enforce(length - 8 <= io_->size() - io_->tell(), ErrorCode::kerCorruptedMetadata);
    Internal::enforce(length - 8 <= std::numeric_limits<size_t>::max(), ErrorCode::kerCorruptedMetadata);
    DataBuf data(static_cast<size_t>(length - 8u));
    const size_t bufRead = io_->read(data.data(), data.size());

    if (io_->error())
      throw Error(ErrorCode::kerFailedToReadImageData);
    if (bufRead != data.size())
      throw Error(ErrorCode::kerInputDataReadFailed);

    const DecodeParams dp(max_recursion_depth_);
    Internal::TiffParserWorker::decode(exifData(), iptcData(), xmpData(), data.c_data(), data.size(), root_tag,
                                       Internal::TiffMapping::findDecoder, dp);
  }
}

void BmffImage::parseXmp(uint64_t length, uint64_t start) {
  Internal::enforce(start <= io_->size(), ErrorCode::kerCorruptedMetadata);
  Internal::enforce(length <= io_->size() - start, ErrorCode::kerCorruptedMetadata);

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
    const DecodeParams dp(max_recursion_depth_);
    Exiv2::XmpParser::decode(xmpData(), std::string(xmp.c_str()), dp);
  } catch (...) {
    throw Error(ErrorCode::kerFailedToReadImageData);
  }

  io_->seek(restore, BasicIo::beg);
}

/// \todo instead of passing the last 4 parameters, pass just one and build the different offsets inside
void BmffImage::parseCr3Preview(const DataBuf& data, std::ostream& out, bool bTrace, uint8_t version,
                                size_t width_offset, size_t height_offset, size_t size_offset,
                                size_t relative_position) {
  // Derived from https://github.com/lclevy/canon_cr3
  const size_t here = io_->tell();
  Internal::enforce(here <= std::numeric_limits<size_t>::max() - relative_position, ErrorCode::kerCorruptedMetadata);
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
  if (bTrace) {
    out << stringFormat("width,height,size = {},{},{}", nativePreview.width_, nativePreview.height_,
                        nativePreview.size_);
  }
  nativePreviews_.push_back(std::move(nativePreview));
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

void BmffImage::openOrThrow() const {
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  // Ensure that this is the correct image type
  if (!isBmffType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "BMFF");
  }
}

void BmffImage::readMetadata() {
  openOrThrow();
  IoCloser closer(*io_);

  clearMetadata();
  ilocs_.clear();
  visits_max_ = io_->size() / 16;
  unknownID_ = 0xffff;
  exifID_ = unknownID_;
  xmpID_ = unknownID_;

  Internal::BmffLegacyReader(*this).all(std::cout, kpsNone, 0);
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

      Internal::BmffLegacyReader(*this).all(out, option, depth);
    } break;
  }
}

void BmffImage::writeMetadata() {
  // bmff files are read-only
  throw(Error(ErrorCode::kerWritingImageFormatUnsupported, "BMFF"));
}  // BmffImage::writeMetadata

// *************************************************************************
// free functions
Image::UniquePtr newBmffInstance(BasicIo::UniquePtr io, const ImageCtorParams& params) {
  auto image = std::make_unique<BmffImage>(std::move(io), params);
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isBmffType(BasicIo& iIo, bool advance) {
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
#endif  // EXV_ENABLE_BMFF
}  // namespace Exiv2
