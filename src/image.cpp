// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "image.hpp"

#include "config.h"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image_int.hpp"
#include "safe_op.hpp"
#include "slice.hpp"

#ifdef EXV_ENABLE_BMFF
#include "bmffimage.hpp"
#endif  // EXV_ENABLE_BMFF

#include "cr2image.hpp"
#include "crwimage.hpp"
#include "epsimage.hpp"
#include "jpgimage.hpp"
#include "mrwimage.hpp"
#ifdef EXV_HAVE_LIBZ
#include "pngimage.hpp"
#endif  // EXV_HAVE_LIBZ
#include "bmpimage.hpp"
#include "gifimage.hpp"
#include "jp2image.hpp"
#include "nikonmn_int.hpp"
#include "orfimage.hpp"
#include "pgfimage.hpp"
#include "psdimage.hpp"
#include "rafimage.hpp"
#include "rw2image.hpp"
#include "tags_int.hpp"
#include "tgaimage.hpp"
#include "tiffimage.hpp"
#include "webpimage.hpp"
#include "xmpsidecar.hpp"
#ifdef EXV_ENABLE_VIDEO
#include "asfvideo.hpp"
#include "matroskavideo.hpp"
#include "quicktimevideo.hpp"
#include "riffvideo.hpp"
#endif  // EXV_ENABLE_VIDEO

// + standard includes
#include <array>
#include <cstdio>
#include <cstring>
#include <limits>
#include <set>

#ifdef __cpp_lib_endian
#include <bit>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

// *****************************************************************************
namespace {
using namespace Exiv2;

//! Struct for storing image types and function pointers.
struct Registry {
  //! Comparison operator to compare a Registry structure with an image type
  bool operator==(const ImageType& imageType) const {
    return imageType == imageType_;
  }

  // DATA
  ImageType imageType_;
  NewInstanceFct newInstance_;
  IsThisTypeFct isThisType_;
  AccessMode exifSupport_;
  AccessMode iptcSupport_;
  AccessMode xmpSupport_;
  AccessMode commentSupport_;
};

/// \todo Use std::unordered_map for implementing the registry. Avoid to use ImageType::none
constexpr Registry registry[] = {
    // image type       creation fct     type check  Exif mode    IPTC mode    XMP mode     Comment mode
    //---------------  ---------------  ----------  -----------  -----------  -----------  ------------
    {ImageType::jpeg, newJpegInstance, isJpegType, amReadWrite, amReadWrite, amReadWrite, amReadWrite},
    {ImageType::exv, newExvInstance, isExvType, amReadWrite, amReadWrite, amReadWrite, amReadWrite},
    {ImageType::cr2, newCr2Instance, isCr2Type, amReadWrite, amReadWrite, amReadWrite, amNone},
    {ImageType::crw, newCrwInstance, isCrwType, amReadWrite, amNone, amNone, amReadWrite},
    {ImageType::mrw, newMrwInstance, isMrwType, amRead, amRead, amRead, amNone},
    {ImageType::tiff, newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone},
    {ImageType::webp, newWebPInstance, isWebPType, amReadWrite, amNone, amReadWrite, amNone},
    {ImageType::dng, newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone},
    {ImageType::nef, newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone},
    {ImageType::pef, newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone},
    {ImageType::arw, newTiffInstance, isTiffType, amRead, amRead, amRead, amNone},
    {ImageType::rw2, newRw2Instance, isRw2Type, amRead, amRead, amRead, amNone},
    {ImageType::sr2, newTiffInstance, isTiffType, amRead, amRead, amRead, amNone},
    {ImageType::srw, newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone},
    {ImageType::orf, newOrfInstance, isOrfType, amReadWrite, amReadWrite, amReadWrite, amNone},
#ifdef EXV_HAVE_LIBZ
    {ImageType::png, newPngInstance, isPngType, amReadWrite, amReadWrite, amReadWrite, amReadWrite},
#endif  // EXV_HAVE_LIBZ
    {ImageType::pgf, newPgfInstance, isPgfType, amReadWrite, amReadWrite, amReadWrite, amReadWrite},
    {ImageType::raf, newRafInstance, isRafType, amRead, amRead, amRead, amNone},
    {ImageType::eps, newEpsInstance, isEpsType, amNone, amNone, amReadWrite, amNone},
    {ImageType::xmp, newXmpInstance, isXmpType, amReadWrite, amReadWrite, amReadWrite, amNone},
    {ImageType::gif, newGifInstance, isGifType, amNone, amNone, amNone, amNone},
    {ImageType::psd, newPsdInstance, isPsdType, amReadWrite, amReadWrite, amReadWrite, amNone},
    {ImageType::tga, newTgaInstance, isTgaType, amNone, amNone, amNone, amNone},
    {ImageType::bmp, newBmpInstance, isBmpType, amNone, amNone, amNone, amNone},
    {ImageType::jp2, newJp2Instance, isJp2Type, amReadWrite, amReadWrite, amReadWrite, amNone},
// needs to be before bmff because some ftyp files are handled as qt and
// the rest should fall through to bmff
#ifdef EXV_ENABLE_VIDEO
    {ImageType::qtime, newQTimeInstance, isQTimeType, amRead, amNone, amRead, amNone},
    {ImageType::asf, newAsfInstance, isAsfType, amRead, amNone, amRead, amNone},
    {ImageType::riff, newRiffInstance, isRiffType, amRead, amNone, amRead, amNone},
    {ImageType::mkv, newMkvInstance, isMkvType, amRead, amNone, amRead, amNone},
#endif  // EXV_ENABLE_VIDEO
#ifdef EXV_ENABLE_BMFF
    {ImageType::bmff, newBmffInstance, isBmffType, amRead, amRead, amRead, amNone},
#endif  // EXV_ENABLE_BMFF
};

#ifdef EXV_ENABLE_FILESYSTEM
std::string pathOfFileUrl(const std::string& url) {
  std::string path = url.substr(7);
  size_t found = path.find('/');
  return (found == std::string::npos) ? path : path.substr(found);
}
#endif

}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {
Image::Image(ImageType type, uint16_t supportedMetadata, BasicIo::UniquePtr io) :
    io_(std::move(io)), imageType_(type), supportedMetadata_(supportedMetadata) {
}

void Image::printStructure(std::ostream&, PrintStructureOption, size_t /*depth*/) {
  throw Error(ErrorCode::kerUnsupportedImageType, io_->path());
}

bool Image::isStringType(uint16_t type) {
  return type == Exiv2::asciiString || type == Exiv2::unsignedByte || type == Exiv2::signedByte ||
         type == Exiv2::undefined;
}
bool Image::isShortType(uint16_t type) {
  return type == Exiv2::unsignedShort || type == Exiv2::signedShort;
}
bool Image::isLongType(uint16_t type) {
  return type == Exiv2::unsignedLong || type == Exiv2::signedLong;
}
bool Image::isLongLongType(uint16_t type) {
  return type == Exiv2::unsignedLongLong || type == Exiv2::signedLongLong;
}
bool Image::isRationalType(uint16_t type) {
  return type == Exiv2::unsignedRational || type == Exiv2::signedRational;
}
bool Image::is2ByteType(uint16_t type) {
  return isShortType(type);
}
bool Image::is4ByteType(uint16_t type) {
  return isLongType(type) || type == Exiv2::tiffFloat || type == Exiv2::tiffIfd;
}
bool Image::is8ByteType(uint16_t type) {
  return isRationalType(type) || isLongLongType(type) || type == Exiv2::tiffIfd8 || type == Exiv2::tiffDouble;
}
bool Image::isPrintXMP(uint16_t type, Exiv2::PrintStructureOption option) {
  return type == 700 && option == kpsXMP;
}
bool Image::isPrintICC(uint16_t type, Exiv2::PrintStructureOption option) {
  return type == 0x8773 && option == kpsIccProfile;
}

bool Image::isBigEndianPlatform() {
#ifdef __cpp_lib_endian
  return std::endian::native == std::endian::big;
#elif defined(__LITTLE_ENDIAN__)
  return false;
#elif defined(__BIG_ENDIAN__)
  return true;
#elif defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return true;
#else
  return false;
#endif
#else
  union {
    uint32_t i;
    char c[4];
  } e = {0x01000000};

  return e.c[0] != 0;
#endif
}
bool Image::isLittleEndianPlatform() {
#ifdef __cpp_lib_endian
  return std::endian::native == std::endian::little;
#elif defined(__LITTLE_ENDIAN__)
  return true;
#else
  return !isBigEndianPlatform();
#endif
}

uint64_t Image::byteSwap(uint64_t value, bool bSwap) {
#ifdef __cpp_lib_byteswap
  return bSwap ? std::byteswap(value) : value;
#else
  uint64_t result = 0;
  auto source_value = reinterpret_cast<const byte*>(&value);
  auto destination_value = reinterpret_cast<byte*>(&result);

  for (int i = 0; i < 8; i++)
    destination_value[i] = source_value[8 - i - 1];

  return bSwap ? result : value;
#endif
}

uint32_t Image::byteSwap(uint32_t value, bool bSwap) {
#ifdef __cpp_lib_byteswap
  return bSwap ? std::byteswap(value) : value;
#else
  uint32_t result = 0;
  result |= (value & 0x000000FFU) << 24;
  result |= (value & 0x0000FF00U) << 8;
  result |= (value & 0x00FF0000U) >> 8;
  result |= (value & 0xFF000000U) >> 24;
  return bSwap ? result : value;
#endif
}

uint16_t Image::byteSwap(uint16_t value, bool bSwap) {
#ifdef __cpp_lib_byteswap
  return bSwap ? std::byteswap(value) : value;
#else
  uint16_t result = 0;
  result |= (value & 0x00FFU) << 8;
  result |= (value & 0xFF00U) >> 8;
  return bSwap ? result : value;
#endif
}

uint16_t Image::byteSwap2(const DataBuf& buf, size_t offset, bool bSwap) {
  uint16_t v = 0;
  auto p = reinterpret_cast<char*>(&v);
  p[0] = buf.read_uint8(offset);
  p[1] = buf.read_uint8(offset + 1);
  return Image::byteSwap(v, bSwap);
}

uint32_t Image::byteSwap4(const DataBuf& buf, size_t offset, bool bSwap) {
  uint32_t v = 0;
  auto p = reinterpret_cast<char*>(&v);
  p[0] = buf.read_uint8(offset);
  p[1] = buf.read_uint8(offset + 1);
  p[2] = buf.read_uint8(offset + 2);
  p[3] = buf.read_uint8(offset + 3);
  return Image::byteSwap(v, bSwap);
}

/// \todo not used internally. At least we should test it
uint64_t Image::byteSwap8(const DataBuf& buf, size_t offset, bool bSwap) {
  uint64_t v = 0;
  auto p = reinterpret_cast<byte*>(&v);

  for (int i = 0; i < 8; i++)
    p[i] = buf.read_uint8(offset + i);

  return Image::byteSwap(v, bSwap);
}

const char* Image::typeName(uint16_t tag) {
  //! List of TIFF image tags
  const char* result = nullptr;
  switch (tag) {
    case Exiv2::unsignedByte:
      result = "BYTE";
      break;
    case Exiv2::asciiString:
      result = "ASCII";
      break;
    case Exiv2::unsignedShort:
      result = "SHORT";
      break;
    case Exiv2::unsignedLong:
      result = "LONG";
      break;
    case Exiv2::unsignedRational:
      result = "RATIONAL";
      break;
    case Exiv2::signedByte:
      result = "SBYTE";
      break;
    case Exiv2::undefined:
      result = "UNDEFINED";
      break;
    case Exiv2::signedShort:
      result = "SSHORT";
      break;
    case Exiv2::signedLong:
      result = "SLONG";
      break;
    case Exiv2::signedRational:
      result = "SRATIONAL";
      break;
    case Exiv2::tiffFloat:
      result = "FLOAT";
      break;
    case Exiv2::tiffDouble:
      result = "DOUBLE";
      break;
    case Exiv2::tiffIfd:
      result = "IFD";
      break;
    default:
      result = "unknown";
      break;
  }
  return result;
}

static bool typeValid(uint16_t type) {
  return type >= 1 && type <= 13;
}

static std::set<size_t> visits;  // #547

void Image::printIFDStructure(BasicIo& io, std::ostream& out, Exiv2::PrintStructureOption option, size_t start,
                              bool bSwap, char c, size_t depth) {
  if (depth == 1)
    visits.clear();
  bool bFirst = true;

  // buffer
  const size_t dirSize = 32;
  DataBuf dir(dirSize);
  bool bPrint = option == kpsBasic || option == kpsRecursive;

  do {
    // Read top of directory
    io.seekOrThrow(start, BasicIo::beg, ErrorCode::kerCorruptedMetadata);
    io.readOrThrow(dir.data(), 2, ErrorCode::kerCorruptedMetadata);
    uint16_t dirLength = byteSwap2(dir, 0, bSwap);
    // Prevent infinite loops. (GHSA-m479-7frc-gqqg)
    Internal::enforce(dirLength > 0, ErrorCode::kerCorruptedMetadata);

    if (dirLength > 500)  // tooBig
      throw Error(ErrorCode::kerTiffDirectoryTooLarge);

    if (bFirst && bPrint) {
      out << Internal::indent(depth) << stringFormat("STRUCTURE OF TIFF FILE ({}{}): {}\n", c, c, io.path());
    }

    // Read the dictionary
    for (int i = 0; i < dirLength; i++) {
      if (visits.find(io.tell()) != visits.end()) {  // #547
        throw Error(ErrorCode::kerCorruptedMetadata);
      }
      visits.insert(io.tell());

      if (bFirst && bPrint) {
        out << Internal::indent(depth) << " address |    tag                              |     "
            << " type |    count |    offset | value\n";
      }
      bFirst = false;

      io.readOrThrow(dir.data(), 12, ErrorCode::kerCorruptedMetadata);
      uint16_t tag = byteSwap2(dir, 0, bSwap);
      uint16_t type = byteSwap2(dir, 2, bSwap);
      uint32_t count = byteSwap4(dir, 4, bSwap);
      uint32_t offset = byteSwap4(dir, 8, bSwap);

      // Break for unknown tag types else we may segfault.
      if (!typeValid(type)) {
        EXV_ERROR << "invalid type in tiff structure" << type << '\n';
        throw Error(ErrorCode::kerInvalidTypeValue);
      }

      std::string sp;  // output spacer

      // prepare to print the value
      uint32_t kount = [=] {
        // haul in all the data
        if (isPrintXMP(tag, option))
          return count;
        // ditto
        if (isPrintICC(tag, option))
          return count;
        // restrict long arrays
        if (isStringType(type)) {
          return std::min(count, 32u);
        }
        return std::min(count, 5u);
      }();
      uint32_t pad = isStringType(type) ? 1 : 0;
      size_t size = [=] {
        if (isStringType(type))
          return 1;
        if (is2ByteType(type))
          return 2;
        if (is4ByteType(type))
          return 4;
        if (is8ByteType(type))
          return 8;
        return 1;
      }();

      // if ( offset > io.size() ) offset = 0; // Denial of service?

      // #55 and #56 memory allocation crash test/data/POC8
      const size_t allocate64 = (size * count) + pad + 20;
      if (allocate64 > io.size()) {
        throw Error(ErrorCode::kerInvalidMalloc);
      }
      DataBuf buf(allocate64);                     // allocate a buffer
      std::copy_n(dir.c_data(8), 4, buf.begin());  // copy dir[8:11] into buffer (short strings)

      // We have already checked that this multiplication cannot overflow.
      const size_t count_x_size = count * size;
      const bool bOffsetIsPointer = count_x_size > 4;

      if (bOffsetIsPointer) {                                                       // read into buffer
        const size_t restore = io.tell();                                           // save
        io.seekOrThrow(offset, BasicIo::beg, ErrorCode::kerCorruptedMetadata);      // position
        io.readOrThrow(buf.data(), count_x_size, ErrorCode::kerCorruptedMetadata);  // read
        io.seekOrThrow(restore, BasicIo::beg, ErrorCode::kerCorruptedMetadata);     // restore
      }

      if (bPrint) {
        const size_t address = start + 2 + (i * 12);
        const std::string offsetString = bOffsetIsPointer ? stringFormat("{:9}", offset) : "";

        out << Internal::indent(depth)
            << stringFormat("{:8} | {:#06x} {:<28} | {:>9} | {:>8} | {:9} | ", address, tag, tagName(tag).c_str(),
                            typeName(type), count, offsetString);
        if (isShortType(type)) {
          for (size_t k = 0; k < kount; k++) {
            out << sp << byteSwap2(buf, k * size, bSwap);
            sp = " ";
          }
        } else if (isLongType(type)) {
          for (size_t k = 0; k < kount; k++) {
            out << sp << byteSwap4(buf, k * size, bSwap);
            sp = " ";
          }

        } else if (isRationalType(type)) {
          for (size_t k = 0; k < kount; k++) {
            uint32_t a = byteSwap4(buf, (k * size) + 0, bSwap);
            uint32_t b = byteSwap4(buf, (k * size) + 4, bSwap);
            out << sp << a << "/" << b;
            sp = " ";
          }
        } else if (isStringType(type)) {
          out << sp << Internal::binaryToString(makeSlice(buf, 0, kount));
        }

        sp = kount == count ? "" : " ...";
        out << sp << '\n';

        if (option == kpsRecursive && (tag == 0x8769 /* ExifTag */ || tag == 0x014a /*SubIFDs*/ || type == tiffIfd)) {
          for (size_t k = 0; k < count; k++) {
            const size_t restore = io.tell();
            offset = byteSwap4(buf, k * size, bSwap);
            printIFDStructure(io, out, option, offset, bSwap, c, depth + 1);
            io.seekOrThrow(restore, BasicIo::beg, ErrorCode::kerCorruptedMetadata);
          }
        } else if (option == kpsRecursive && tag == 0x83bb /* IPTCNAA */) {
          if (count > 0) {
            if (static_cast<size_t>(Safe::add(count, offset)) > io.size()) {
              throw Error(ErrorCode::kerCorruptedMetadata);
            }

            const size_t restore = io.tell();
            io.seekOrThrow(offset, BasicIo::beg, ErrorCode::kerCorruptedMetadata);  // position
            std::vector<byte> bytes(count);                                         // allocate memory
            io.readOrThrow(bytes.data(), count, ErrorCode::kerCorruptedMetadata);
            io.seekOrThrow(restore, BasicIo::beg, ErrorCode::kerCorruptedMetadata);
            IptcData::printStructure(out, makeSliceUntil(bytes.data(), count), depth);
          }
        } else if (option == kpsRecursive && tag == 0x927c /* MakerNote */ && count > 10) {
          const size_t restore = io.tell();  // save

          uint32_t jump = 10;
          byte bytes[20];
          const auto chars = reinterpret_cast<const char*>(&bytes[0]);
          io.seekOrThrow(offset, BasicIo::beg, ErrorCode::kerCorruptedMetadata);  // position
          io.readOrThrow(bytes, jump, ErrorCode::kerCorruptedMetadata);           // read
          bytes[jump] = 0;

          bool bNikon = ::strcmp("Nikon", chars) == 0;
          bool bSony = ::strcmp("SONY DSC ", chars) == 0;

          if (bNikon) {
            // tag is an embedded tiff
            const long byteslen = count - jump;
            auto b = DataBuf(byteslen);                                           // allocate a buffer
            io.readOrThrow(b.data(), byteslen, ErrorCode::kerCorruptedMetadata);  // read
            MemIo memIo(b.c_data(), byteslen);                                    // create a file
            printTiffStructure(memIo, out, option, depth + 1);
          } else {
            // tag is an IFD
            uint32_t punt = bSony ? 12 : 0;
            io.seekOrThrow(0, BasicIo::beg, ErrorCode::kerCorruptedMetadata);  // position
            printIFDStructure(io, out, option, offset + punt, bSwap, c, depth + 1);
          }

          io.seekOrThrow(restore, BasicIo::beg, ErrorCode::kerCorruptedMetadata);  // restore
        }
      }

      if (isPrintXMP(tag, option)) {
        buf.write_uint8(count, 0);
        out << buf.c_str();
      }
      if (isPrintICC(tag, option)) {
        out.write(buf.c_str(), count);
      }
    }
    if (start) {
      io.readOrThrow(dir.data(), 4, ErrorCode::kerCorruptedMetadata);
      start = byteSwap4(dir, 0, bSwap);
    }
  } while (start);

  if (bPrint) {
    out << Internal::indent(depth) << "END " << io.path() << '\n';
  }
  out.flush();
}

void Image::printTiffStructure(BasicIo& io, std::ostream& out, Exiv2::PrintStructureOption option, size_t depth,
                               size_t offset /*=0*/) {
  if (option == kpsBasic || option == kpsXMP || option == kpsRecursive || option == kpsIccProfile) {
    // buffer
    const size_t dirSize = 32;
    DataBuf dir(dirSize);

    // read header (we already know for certain that we have a Tiff file)
    io.readOrThrow(dir.data(), 8, ErrorCode::kerCorruptedMetadata);
    auto c = dir.read_uint8(0);
    bool bSwap = (c == 'M' && isLittleEndianPlatform()) || (c == 'I' && isBigEndianPlatform());
    size_t start = byteSwap4(dir, 4, bSwap);
    printIFDStructure(io, out, option, start + offset, bSwap, c, depth);
  }
}

void Image::clearMetadata() {
  clearExifData();
  clearIptcData();
  clearXmpPacket();
  clearXmpData();
  clearComment();
  clearIccProfile();
}

ExifData& Image::exifData() {
  return exifData_;
}

IptcData& Image::iptcData() {
  return iptcData_;
}

XmpData& Image::xmpData() {
  return xmpData_;
}

std::string& Image::xmpPacket() {
  // Serialize the current XMP
  if (!xmpData_.empty() && !writeXmpFromPacket()) {
    XmpParser::encode(xmpPacket_, xmpData_, XmpParser::useCompactFormat | XmpParser::omitAllFormatting);
  }
  return xmpPacket_;
}

/// \todo not used internally. At least we should test it
void Image::setMetadata(const Image& image) {
  if (checkMode(mdExif) & amWrite) {
    setExifData(image.exifData());
  }
  if (checkMode(mdIptc) & amWrite) {
    setIptcData(image.iptcData());
  }
  if (checkMode(mdIccProfile) & amWrite) {
    setIccProfile(DataBuf(image.iccProfile()));
  }
  if (checkMode(mdXmp) & amWrite) {
    setXmpPacket(image.xmpPacket());
    setXmpData(image.xmpData());
  }
  if (checkMode(mdComment) & amWrite) {
    setComment(image.comment());
  }
}

void Image::clearExifData() {
  exifData_.clear();
}

void Image::setExifData(const ExifData& exifData) {
  exifData_ = exifData;
}

void Image::clearIptcData() {
  iptcData_.clear();
}

void Image::setIptcData(const IptcData& iptcData) {
  iptcData_ = iptcData;
}

void Image::clearXmpPacket() {
  xmpPacket_.clear();
  writeXmpFromPacket(true);
}

void Image::setXmpPacket(const std::string& xmpPacket) {
  if (XmpParser::decode(xmpData_, xmpPacket)) {
    throw Error(ErrorCode::kerInvalidXMP);
  }
  xmpPacket_ = xmpPacket;
}

void Image::clearXmpData() {
  xmpData_.clear();
  writeXmpFromPacket(false);
}

void Image::setXmpData(const XmpData& xmpData) {
  xmpData_ = xmpData;
  writeXmpFromPacket(false);
}

#ifdef EXV_HAVE_XMP_TOOLKIT
void Image::writeXmpFromPacket(bool flag) {
  writeXmpFromPacket_ = flag;
}
#else
void Image::writeXmpFromPacket(bool) {
}
#endif

void Image::clearComment() {
  comment_.erase();
}

void Image::setComment(const std::string& comment) {
  comment_ = comment;
}

void Image::setIccProfile(Exiv2::DataBuf&& iccProfile, bool bTestValid) {
  if (bTestValid) {
    if (iccProfile.size() < sizeof(long)) {
      throw Error(ErrorCode::kerInvalidIccProfile);
    }
    const size_t size = iccProfile.read_uint32(0, bigEndian);
    if (size != iccProfile.size()) {
      throw Error(ErrorCode::kerInvalidIccProfile);
    }
  }
  iccProfile_ = std::move(iccProfile);
}

void Image::clearIccProfile() {
  iccProfile_.reset();
}

void Image::setByteOrder(ByteOrder byteOrder) {
  byteOrder_ = byteOrder;
}

ByteOrder Image::byteOrder() const {
  return byteOrder_;
}

uint32_t Image::pixelWidth() const {
  return pixelWidth_;
}

uint32_t Image::pixelHeight() const {
  return pixelHeight_;
}

const ExifData& Image::exifData() const {
  return exifData_;
}

const IptcData& Image::iptcData() const {
  return iptcData_;
}

const XmpData& Image::xmpData() const {
  return xmpData_;
}

std::string Image::comment() const {
  return comment_;
}

const std::string& Image::xmpPacket() const {
  return xmpPacket_;
}

BasicIo& Image::io() const {
  return *io_;
}

bool Image::writeXmpFromPacket() const {
  return writeXmpFromPacket_;
}

const NativePreviewList& Image::nativePreviews() const {
  return nativePreviews_;
}

bool Image::good() const {
  if (io_->open() != 0)
    return false;
  IoCloser closer(*io_);
  return ImageFactory::checkType(imageType_, *io_, false);
}

/// \todo not used internally. At least we should test it
bool Image::supportsMetadata(MetadataId metadataId) const {
  return (supportedMetadata_ & metadataId) != 0;
}

AccessMode Image::checkMode(MetadataId metadataId) const {
  return ImageFactory::checkMode(imageType_, metadataId);
}

const std::string& Image::tagName(uint16_t tag) {
  if (init_) {
    int idx;
    const TagInfo* ti;
    for (ti = Internal::mnTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx)
      tags_[ti[idx].tag_] = ti[idx].name_;
    for (ti = Internal::iopTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx)
      tags_[ti[idx].tag_] = ti[idx].name_;
    for (ti = Internal::gpsTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx)
      tags_[ti[idx].tag_] = ti[idx].name_;
    for (ti = Internal::ifdTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx)
      tags_[ti[idx].tag_] = ti[idx].name_;
    for (ti = Internal::exifTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx)
      tags_[ti[idx].tag_] = ti[idx].name_;
    for (ti = Internal::mpfTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx)
      tags_[ti[idx].tag_] = ti[idx].name_;
    for (ti = Internal::Nikon1MakerNote::tagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx)
      tags_[ti[idx].tag_] = ti[idx].name_;
  }
  init_ = false;

  return tags_[tag];
}

AccessMode ImageFactory::checkMode(ImageType type, MetadataId metadataId) {
  auto r = Exiv2::find(registry, type);
  if (!r)
    throw Error(ErrorCode::kerUnsupportedImageType, static_cast<int>(type));
  if (metadataId == mdExif)
    return r->exifSupport_;
  if (metadataId == mdIptc)
    return r->iptcSupport_;
  if (metadataId == mdXmp)
    return r->xmpSupport_;
  if (metadataId == mdComment)
    return r->commentSupport_;
  return amNone;
}

bool ImageFactory::checkType(ImageType type, BasicIo& io, bool advance) {
  if (auto r = Exiv2::find(registry, type))
    return r->isThisType_(io, advance);
  return false;
}

ImageType ImageFactory::getType([[maybe_unused]] const std::string& path) {
#ifdef EXV_ENABLE_FILESYSTEM
  FileIo fileIo(path);
  return getType(fileIo);
#else
  return ImageType::none;
#endif
}

ImageType ImageFactory::getType(const byte* data, size_t size) {
  MemIo memIo(data, size);
  return getType(memIo);
}

ImageType ImageFactory::getType(BasicIo& io) {
  if (io.open() != 0)
    return ImageType::none;
  IoCloser closer(io);
  for (const auto& r : registry) {
    if (r.isThisType_(io, false)) {
      return r.imageType_;
    }
  }
  return ImageType::none;
}

BasicIo::UniquePtr ImageFactory::createIo(const std::string& path, [[maybe_unused]] bool useCurl) {
  Protocol fProt = fileProtocol(path);

#ifdef EXV_USE_CURL
  if (useCurl && (fProt == pHttp || fProt == pHttps || fProt == pFtp)) {
    return std::make_unique<CurlIo>(path);  // may throw
  }
#endif

#ifdef EXV_ENABLE_WEBREADY
  if (fProt == pHttp)
    return std::make_unique<HttpIo>(path);  // may throw
#endif
#ifdef EXV_ENABLE_FILESYSTEM
  if (fProt == pFileUri)
    return std::make_unique<FileIo>(pathOfFileUrl(path));
  if (fProt == pStdin || fProt == pDataUri)
    return std::make_unique<XPathIo>(path);  // may throw

  return std::make_unique<FileIo>(path);
#else
  throw Error(ErrorCode::kerFileAccessDisabled, path);
#endif
}  // ImageFactory::createIo

#ifdef _WIN32
BasicIo::UniquePtr ImageFactory::createIo(const std::wstring& path) {
#ifdef EXV_ENABLE_FILESYSTEM
  return std::make_unique<FileIo>(path);
#else
  return nullptr;
#endif
}
#endif

Image::UniquePtr ImageFactory::open(const std::string& path, bool useCurl) {
  auto image = open(ImageFactory::createIo(path, useCurl));  // may throw
  if (!image)
    throw Error(ErrorCode::kerFileContainsUnknownImageType, path);
  return image;
}

#ifdef _WIN32
Image::UniquePtr ImageFactory::open(const std::wstring& path) {
  auto image = open(ImageFactory::createIo(path));  // may throw
  if (!image) {
    char t[1024];
    WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, t, 1024, nullptr, nullptr);
    throw Error(ErrorCode::kerFileContainsUnknownImageType, t);
  }
  return image;
}
#endif

Image::UniquePtr ImageFactory::open(const byte* data, size_t size) {
  auto image = open(std::make_unique<MemIo>(data, size));  // may throw
  if (!image)
    throw Error(ErrorCode::kerMemoryContainsUnknownImageType);
  return image;
}

Image::UniquePtr ImageFactory::open(BasicIo::UniquePtr io) {
  if (io->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io->path(), strError());
  }
  for (const auto& r : registry) {
    if (r.isThisType_(*io, false)) {
      return r.newInstance_(std::move(io), false);
    }
  }
  return nullptr;
}

#ifdef EXV_ENABLE_FILESYSTEM
Image::UniquePtr ImageFactory::create(ImageType type, const std::string& path) {
  auto fileIo = std::make_unique<FileIo>(path);
  // Create or overwrite the file, then close it
  if (fileIo->open("w+b") != 0) {
    throw Error(ErrorCode::kerFileOpenFailed, path, "w+b", strError());
  }
  fileIo->close();

  BasicIo::UniquePtr io(std::move(fileIo));
  auto image = create(type, std::move(io));
  if (!image)
    throw Error(ErrorCode::kerUnsupportedImageType, static_cast<int>(type));
  return image;
}
#endif

Image::UniquePtr ImageFactory::create(ImageType type) {
  auto image = create(type, std::make_unique<MemIo>());
  if (!image)
    throw Error(ErrorCode::kerUnsupportedImageType, static_cast<int>(type));
  return image;
}

Image::UniquePtr ImageFactory::create(ImageType type, BasicIo::UniquePtr io) {
  // BasicIo instance does not need to be open
  if (type == ImageType::none)
    return {};
  if (auto r = Exiv2::find(registry, type))
    return r->newInstance_(std::move(io), true);
  return {};
}

// *****************************************************************************
// template, inline and free functions

void append(Blob& blob, const byte* buf, size_t len) {
  if (len != 0) {
    Blob::size_type size = blob.size();
    if (blob.capacity() - size < len) {
      blob.reserve(size + 65536);
    }
    blob.resize(size + len);
    std::copy_n(buf, len, &blob[size]);
  }
}  // append

}  // namespace Exiv2
