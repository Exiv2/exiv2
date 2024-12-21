// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "jp2image.hpp"

#include "config.h"

#include "basicio.hpp"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "jp2image_int.hpp"
#include "safe_op.hpp"
#include "tiffimage.hpp"
#include "types.hpp"

#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>

namespace Exiv2 {
namespace {
// JPEG-2000 box types
constexpr uint32_t kJp2BoxTypeSignature = 0x6a502020;    // signature box, required,
constexpr uint32_t kJp2BoxTypeFileTypeBox = 0x66747970;  // File type box, required
constexpr uint32_t kJp2BoxTypeHeader = 0x6a703268;       // Jp2 Header Box, required, Superbox
constexpr uint32_t kJp2BoxTypeImageHeader = 0x69686472;  // Image Header Box ('ihdr'), required,
constexpr uint32_t kJp2BoxTypeColorSpec = 0x636f6c72;    // Color Specification box ('colr'), required
constexpr uint32_t kJp2BoxTypeUuid = 0x75756964;         // 'uuid'
constexpr uint32_t kJp2BoxTypeClose = 0x6a703263;        // 'jp2c'

// JPEG-2000 UUIDs for embedded metadata
//
// See http://www.jpeg.org/public/wg1n2600.doc for information about embedding IPTC-NAA data in JPEG-2000 files
// See http://www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf for information about embedding XMP data in JPEG-2000
// files
constexpr unsigned char kJp2UuidExif[] = "JpgTiffExif->JP2";
constexpr unsigned char kJp2UuidIptc[] = "\x33\xc7\xa4\xd2\xb8\x1d\x47\x23\xa0\xba\xf1\xa3\xe0\x97\xad\x38";
constexpr unsigned char kJp2UuidXmp[] = "\xbe\x7a\xcf\xcb\x97\xa9\x42\xe8\x9c\x71\x99\x94\x91\xe3\xaf\xac";

// See section B.1.1 (JPEG 2000 Signature box) of JPEG-2000 specification
constexpr std::array<byte, 12> Jp2Signature{
    0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a,
};

constexpr std::array<byte, 249> Jp2Blank{
    0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a, 0x00, 0x00, 0x00, 0x14, 0x66, 0x74,
    0x79, 0x70, 0x6a, 0x70, 0x32, 0x20, 0x00, 0x00, 0x00, 0x00, 0x6a, 0x70, 0x32, 0x20, 0x00, 0x00, 0x00, 0x2d,
    0x6a, 0x70, 0x32, 0x68, 0x00, 0x00, 0x00, 0x16, 0x69, 0x68, 0x64, 0x72, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x01, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x63, 0x6f, 0x6c, 0x72, 0x01, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x6a, 0x70, 0x32, 0x63, 0xff, 0x4f, 0xff, 0x51, 0x00,
    0x29, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x07, 0x01, 0x01, 0xff, 0x64, 0x00, 0x23, 0x00, 0x01, 0x43, 0x72, 0x65, 0x61, 0x74, 0x6f, 0x72, 0x3a,
    0x20, 0x4a, 0x61, 0x73, 0x50, 0x65, 0x72, 0x20, 0x56, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x31, 0x2e,
    0x39, 0x30, 0x30, 0x2e, 0x31, 0xff, 0x52, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x05, 0x04, 0x04, 0x00,
    0x01, 0xff, 0x5c, 0x00, 0x13, 0x40, 0x40, 0x48, 0x48, 0x50, 0x48, 0x48, 0x50, 0x48, 0x48, 0x50, 0x48, 0x48,
    0x50, 0x48, 0x48, 0x50, 0xff, 0x90, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x01, 0xff, 0x5d,
    0x00, 0x14, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0xff, 0x93, 0xcf, 0xb4, 0x04, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0xff, 0xd9,
};

const size_t boxHSize = sizeof(Internal::Jp2BoxHeader);

void lf(std::ostream& out, bool& bLF) {
  if (bLF) {
    out << '\n';
    out.flush();
    bLF = false;
  }
}

void boxes_check(size_t b, size_t m) {
  if (b > m) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cout << "Exiv2::Jp2Image::readMetadata box maximum exceeded" << '\n';
#endif
    throw Error(ErrorCode::kerCorruptedMetadata);
  }
}

}  // namespace

Jp2Image::Jp2Image(BasicIo::UniquePtr io, bool create) : Image(ImageType::jp2, mdExif | mdIptc | mdXmp, std::move(io)) {
  if (create && io_->open() == 0) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Exiv2::Jp2Image:: Creating JPEG2000 image to memory" << '\n';
#endif
    IoCloser closer(*io_);
    if (io_->write(Jp2Blank.data(), Jp2Blank.size()) != Jp2Blank.size()) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "Exiv2::Jp2Image:: Failed to create JPEG2000 image on memory" << '\n';
#endif
    }
  }
}

// Obtains the ascii version from the box.type
std::string Jp2Image::toAscii(uint32_t n) {
  const auto p = reinterpret_cast<const char*>(&n);
  std::string result(p, p + 4);
  if (isBigEndianPlatform())
    return result;
  std::reverse(result.begin(), result.end());
  return result;
}

std::string Jp2Image::mimeType() const {
  if (brand_ == Internal::brandJph)
    return "image/jph";
  return "image/jp2";
}

void Jp2Image::setComment(const std::string&) {
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "JP2"));
}

void Jp2Image::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Exiv2::Jp2Image::readMetadata: Reading JPEG-2000 file " << io_->path() << '\n';
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  if (!isJp2Type(*io_, false)) {
    throw Error(ErrorCode::kerNotAnImage, "JPEG-2000");
  }

  Internal::Jp2BoxHeader box = {0, 0};
  Internal::Jp2BoxHeader subBox = {0, 0};
  Internal::Jp2ImageHeaderBox ihdr = {0, 0, 0, 0, 0, 0, 0};
  Internal::Jp2UuidBox uuid = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
  size_t boxesCount = 0;
  const size_t boxem = 1000;  // boxes max
  uint32_t lastBoxTypeRead = 0;
  bool boxSignatureFound = false;
  bool boxFileTypeFound = false;

  while (io_->read(reinterpret_cast<byte*>(&box), boxHSize) == boxHSize) {
    boxes_check(boxesCount++, boxem);
    const size_t position = io_->tell();
    box.length = getULong(reinterpret_cast<byte*>(&box.length), bigEndian);
    box.type = getULong(reinterpret_cast<byte*>(&box.type), bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
    std::cout << "Exiv2::Jp2Image::readMetadata: Position: " << position << " box type: " << toAscii(box.type)
              << " length: " << box.length << '\n';
#endif
    Internal::enforce(box.length <= boxHSize + io_->size() - io_->tell(), ErrorCode::kerCorruptedMetadata);

    if (box.length == 0)
      return;

    if (box.length == 1) {
      /// \todo In this case, the real box size is given in XLBox (bytes 8-15)
    }

    switch (box.type) {
      case kJp2BoxTypeSignature: {
        if (boxSignatureFound)  // Only one is allowed
          throw Error(ErrorCode::kerCorruptedMetadata);
        boxSignatureFound = true;
        break;
      }
      case kJp2BoxTypeFileTypeBox: {
        // This box shall immediately follow the JPEG 2000 Signature box
        if (boxFileTypeFound || lastBoxTypeRead != kJp2BoxTypeSignature) {  // Only one is allowed
          throw Error(ErrorCode::kerCorruptedMetadata);
        }
        boxFileTypeFound = true;
        std::vector<byte> boxData(box.length - boxHSize);
        io_->readOrThrow(boxData.data(), boxData.size(), ErrorCode::kerCorruptedMetadata);
        if (!Internal::isValidBoxFileType(boxData))
          throw Error(ErrorCode::kerCorruptedMetadata);
        brand_ = getULong(boxData.data(), bigEndian);
        break;
      }
      case kJp2BoxTypeHeader: {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::Jp2Image::readMetadata: JP2Header box found\n";
#endif
        size_t restore = io_->tell();

        while (io_->read(reinterpret_cast<byte*>(&subBox), boxHSize) == boxHSize && subBox.length) {
          boxes_check(boxesCount++, boxem);
          subBox.length = getULong(reinterpret_cast<byte*>(&subBox.length), bigEndian);
          subBox.type = getULong(reinterpret_cast<byte*>(&subBox.type), bigEndian);
          if (subBox.length > io_->size()) {
            throw Error(ErrorCode::kerCorruptedMetadata);
          }
#ifdef EXIV2_DEBUG_MESSAGES
          std::cout << "Exiv2::Jp2Image::readMetadata: "
                    << "subBox = " << toAscii(subBox.type) << " length = " << subBox.length << '\n';
#endif
          if (subBox.type == kJp2BoxTypeColorSpec && subBox.length != 15) {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::Jp2Image::readMetadata: "
                      << "Color data found" << '\n';
#endif

            const size_t pad = 3;  // 3 padding bytes 2 0 0
            const size_t data_length = Safe::add(subBox.length, 8u);
            // data_length makes no sense if it is larger than the rest of the file
            if (data_length > io_->size() - io_->tell()) {
              throw Error(ErrorCode::kerCorruptedMetadata);
            }
            DataBuf data(data_length);
            io_->read(data.data(), data.size());
            const size_t iccLength = data.read_uint32(pad, bigEndian);
            // subtracting pad from data.size() is safe:
            // data.size() is at least 8 and pad = 3
            if (iccLength > data.size() - pad) {
              throw Error(ErrorCode::kerCorruptedMetadata);
            }
            DataBuf icc(iccLength);
            std::copy_n(data.c_data(pad), icc.size(), icc.begin());
#ifdef EXIV2_DEBUG_MESSAGES
            const char* iccPath = "/tmp/libexiv2_jp2.icc";
            if (auto f = std::ofstream(iccPath, std::ios::binary)) {
              f.write(reinterpret_cast<const char*>(icc.c_data()), static_cast<std::streamsize>(icc.size()));
              f.close();
              std::cout << "Exiv2::Jp2Image::readMetadata: wrote iccProfile " << icc.size() << " bytes to " << iccPath
                        << '\n';
            }
#endif
            setIccProfile(std::move(icc));
          }

          if (subBox.type == kJp2BoxTypeImageHeader) {
            io_->read(reinterpret_cast<byte*>(&ihdr), sizeof(ihdr));
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::Jp2Image::readMetadata: Ihdr data found" << '\n';
#endif
            ihdr.imageHeight = getULong(reinterpret_cast<byte*>(&ihdr.imageHeight), bigEndian);
            ihdr.imageWidth = getULong(reinterpret_cast<byte*>(&ihdr.imageWidth), bigEndian);
            ihdr.componentCount = getShort(reinterpret_cast<byte*>(&ihdr.componentCount), bigEndian);
            Internal::enforce(ihdr.c == 7, ErrorCode::kerCorruptedMetadata);

            pixelWidth_ = ihdr.imageWidth;
            pixelHeight_ = ihdr.imageHeight;
          }

          io_->seek(restore, BasicIo::beg);
          if (io_->seek(subBox.length, BasicIo::cur) != 0) {
            throw Error(ErrorCode::kerCorruptedMetadata);
          }
          restore = io_->tell();
        }
        break;
      }

      case kJp2BoxTypeUuid: {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::Jp2Image::readMetadata: UUID box found" << '\n';
#endif

        if (io_->read(reinterpret_cast<byte*>(&uuid), sizeof(uuid)) == sizeof(uuid)) {
          DataBuf rawData;
          size_t bufRead;
          bool bIsExif = memcmp(uuid.uuid, kJp2UuidExif, sizeof(uuid)) == 0;
          bool bIsIPTC = memcmp(uuid.uuid, kJp2UuidIptc, sizeof(uuid)) == 0;
          bool bIsXMP = memcmp(uuid.uuid, kJp2UuidXmp, sizeof(uuid)) == 0;

          if (bIsExif) {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::Jp2Image::readMetadata: Exif data found" << '\n';
#endif
            Internal::enforce(box.length >= boxHSize + sizeof(uuid), ErrorCode::kerCorruptedMetadata);
            rawData.alloc(box.length - (boxHSize + sizeof(uuid)));
            bufRead = io_->read(rawData.data(), rawData.size());
            if (io_->error())
              throw Error(ErrorCode::kerFailedToReadImageData);
            if (bufRead != rawData.size())
              throw Error(ErrorCode::kerInputDataReadFailed);

            if (rawData.size() > 8)  // "II*\0long"
            {
              // Find the position of Exif header in bytes array.
              const char a = rawData.read_uint8(0);
              const char b = rawData.read_uint8(1);
              const size_t notfound = std::numeric_limits<size_t>::max();
              size_t pos = (a == b && (a == 'I' || a == 'M')) ? 0 : notfound;

              // #1242  Forgive having Exif\0\0 in rawData.pData_
              std::array<byte, 6> exifHeader{0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
              for (size_t i = 0; pos == notfound && i < (rawData.size() - exifHeader.size()); i++) {
                if (rawData.cmpBytes(i, exifHeader.data(), exifHeader.size()) == 0) {
                  pos = i + sizeof(exifHeader);
#ifndef SUPPRESS_WARNINGS
                  EXV_WARNING << "Reading non-standard UUID-EXIF_bad box in " << io_->path() << '\n';
#endif
                }
              }

              // If found it, store only these data at from this place.
              if (pos != notfound) {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cout << "Exiv2::Jp2Image::readMetadata: Exif header found at position " << pos << '\n';
#endif
                ByteOrder bo =
                    TiffParser::decode(exifData(), iptcData(), xmpData(), rawData.c_data(pos), rawData.size() - pos);
                setByteOrder(bo);
              }
            } else {
#ifndef SUPPRESS_WARNINGS
              EXV_WARNING << "Failed to decode Exif metadata." << '\n';
#endif
              exifData_.clear();
            }
          }

          if (bIsIPTC) {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::Jp2Image::readMetadata: Iptc data found" << '\n';
#endif
            Internal::enforce(box.length >= boxHSize + sizeof(uuid), ErrorCode::kerCorruptedMetadata);
            rawData.alloc(box.length - (boxHSize + sizeof(uuid)));
            bufRead = io_->read(rawData.data(), rawData.size());
            if (io_->error())
              throw Error(ErrorCode::kerFailedToReadImageData);
            if (bufRead != rawData.size())
              throw Error(ErrorCode::kerInputDataReadFailed);

            if (IptcParser::decode(iptcData_, rawData.c_data(), rawData.size())) {
#ifndef SUPPRESS_WARNINGS
              EXV_WARNING << "Failed to decode IPTC metadata." << '\n';
#endif
              iptcData_.clear();
            }
          }

          if (bIsXMP) {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::Jp2Image::readMetadata: Xmp data found" << '\n';
#endif
            Internal::enforce(box.length >= boxHSize + sizeof(uuid), ErrorCode::kerCorruptedMetadata);
            rawData.alloc(box.length - (boxHSize + sizeof(uuid)));
            bufRead = io_->read(rawData.data(), rawData.size());
            if (io_->error())
              throw Error(ErrorCode::kerFailedToReadImageData);
            if (bufRead != rawData.size())
              throw Error(ErrorCode::kerInputDataReadFailed);
            xmpPacket_.assign(rawData.c_str(), rawData.size());

            if (auto idx = xmpPacket_.find_first_of('<'); idx != std::string::npos && idx > 0) {
#ifndef SUPPRESS_WARNINGS
              EXV_WARNING << "Removing " << static_cast<uint32_t>(idx)
                          << " characters from the beginning of the XMP packet" << '\n';
#endif
              xmpPacket_ = xmpPacket_.substr(idx);
            }

            if (!xmpPacket_.empty() && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
              EXV_WARNING << "Failed to decode XMP metadata." << '\n';
#endif
            }
          }
        }
        break;
      }

      default:
        break;
    }
    lastBoxTypeRead = box.type;

    // Move to the next box.
    io_->seek(static_cast<int64_t>(position - boxHSize + box.length), BasicIo::beg);
    if (io_->error())
      throw Error(ErrorCode::kerFailedToReadImageData);
  }

}  // Jp2Image::readMetadata

void Jp2Image::printStructure(std::ostream& out, PrintStructureOption option, size_t depth) {
  if (io_->open() != 0)
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());

  if (!isJp2Type(*io_, false)) {
    throw Error(ErrorCode::kerNotAJpeg);
  }

  // According to the JP2 standard: The start of the first box shall be the first byte of the file, and the
  // last byte of the last box shall be the last byte of the file.

  bool bPrint = option == kpsBasic || option == kpsRecursive;
  bool bRecursive = option == kpsRecursive;
  bool bICC = option == kpsIccProfile;
  bool bXMP = option == kpsXMP;
  bool bIPTCErase = option == kpsIptcErase;
  bool boxSignatureFound = false;

  if (bPrint) {
    out << "STRUCTURE OF JPEG2000 FILE: " << io_->path() << '\n';
    out << " address |   length | box       | data" << '\n';
  }

  if (bPrint || bXMP || bICC || bIPTCErase) {
    Internal::Jp2BoxHeader box = {1, 1};
    Internal::Jp2BoxHeader subBox = {1, 1};
    Internal::Jp2UuidBox uuid = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
    bool bLF = false;

    while (box.length && box.type != kJp2BoxTypeClose &&
           io_->read(reinterpret_cast<byte*>(&box), boxHSize) == boxHSize) {
      const size_t position = io_->tell();
      box.length = getULong(reinterpret_cast<byte*>(&box.length), bigEndian);
      box.type = getULong(reinterpret_cast<byte*>(&box.type), bigEndian);
      Internal::enforce(box.length <= boxHSize + io_->size() - io_->tell(), ErrorCode::kerCorruptedMetadata);

      if (bPrint) {
        out << stringFormat("{:8} | {:8} | {}      | ", position - boxHSize, box.length, toAscii(box.type));
        bLF = true;
        if (box.type == kJp2BoxTypeClose)
          lf(out, bLF);
      }
      if (box.type == kJp2BoxTypeClose)
        break;

      switch (box.type) {
        case kJp2BoxTypeSignature: {
          if (boxSignatureFound)  // Only one is allowed
            throw Error(ErrorCode::kerCorruptedMetadata);
          boxSignatureFound = true;
          break;
        }
        case kJp2BoxTypeFileTypeBox: {
          // This box shall immediately follow the JPEG 2000 Signature box
          /// \todo  All files shall contain one and only one File Type box.
          std::vector<byte> boxData(box.length - boxHSize);
          io_->readOrThrow(boxData.data(), boxData.size(), ErrorCode::kerCorruptedMetadata);
          if (!Internal::isValidBoxFileType(boxData))
            throw Error(ErrorCode::kerCorruptedMetadata);
          break;
        }
        case kJp2BoxTypeHeader: {
          lf(out, bLF);
          /// \todo  All files shall contain one and only one Header box.

          while (io_->read(reinterpret_cast<byte*>(&subBox), boxHSize) == boxHSize &&
                 io_->tell() < position + box.length)  // don't read beyond the box!
          {
            const size_t address = io_->tell() - boxHSize;
            subBox.length = getULong(reinterpret_cast<byte*>(&subBox.length), bigEndian);
            subBox.type = getULong(reinterpret_cast<byte*>(&subBox.type), bigEndian);

            if (subBox.length < boxHSize || subBox.length > io_->size() - io_->tell()) {
              throw Error(ErrorCode::kerCorruptedMetadata);
            }

            DataBuf data(subBox.length - boxHSize);
            io_->read(data.data(), data.size());
            if (bPrint) {
              out << stringFormat("{:8} | {:8} |  sub:{} | ", address, subBox.length, toAscii(subBox.type))
                  << Internal::binaryToString(makeSlice(data, 0, std::min<size_t>(30, data.size())));
              bLF = true;
            }

            if (subBox.type == kJp2BoxTypeImageHeader) {
              Internal::enforce(subBox.length == 22, ErrorCode::kerCorruptedMetadata);
              // height (4), width (4), componentsCount (2), bpc (1)
              auto compressionType = data.read_uint8(11);
              auto unkC = data.read_uint8(12);
              auto ipr = data.read_uint8(13);
              if (compressionType != 7 || unkC > 1 || ipr > 1) {
                throw Error(ErrorCode::kerCorruptedMetadata);
              }
            } else if (subBox.type == kJp2BoxTypeColorSpec) {
              const size_t pad = 3;  // don't know why there are 3 padding bytes

              // Bounds-check for the `getULong()` below, which reads 4 bytes, starting at `pad`.
              Internal::enforce(data.size() >= pad + 4, ErrorCode::kerCorruptedMetadata);

              /// \todo A conforming JP2 reader shall ignore all Colour Specification boxes after the first.
              auto METH = data.read_uint8(0);
              // auto PREC = data.read_uint8(1);
              // auto APPROX = data.read_uint8(2);
              if (METH == 1) {  // Enumerated Colourspace
                auto enumCS = data.read_uint32(3, bigEndian);
                if (enumCS != 16 && enumCS != 17) {
                  throw Error(ErrorCode::kerCorruptedMetadata);
                }
              } else {  // Restricted ICC Profile
                        // see the ICC Profile Format Specification, version ICC.1:1998-09
                const size_t iccLength = data.read_uint32(pad, bigEndian);
                if (bPrint) {
                  out << " | iccLength:" << iccLength;
                }
                Internal::enforce(iccLength <= data.size() - pad, ErrorCode::kerCorruptedMetadata);
                if (bICC) {
                  out.write(data.c_str(pad), iccLength);
                }
              }
            }
            lf(out, bLF);
          }
        } break;

        case kJp2BoxTypeUuid: {
          if (io_->read(reinterpret_cast<byte*>(&uuid), sizeof(uuid)) == sizeof(uuid)) {
            bool bIsExif = memcmp(uuid.uuid, kJp2UuidExif, sizeof(uuid)) == 0;
            bool bIsIPTC = memcmp(uuid.uuid, kJp2UuidIptc, sizeof(uuid)) == 0;
            bool bIsXMP = memcmp(uuid.uuid, kJp2UuidXmp, sizeof(uuid)) == 0;

            bool bUnknown = !(bIsExif || bIsIPTC || bIsXMP);

            if (bPrint) {
              if (bIsExif)
                out << "Exif: ";
              if (bIsIPTC)
                out << "IPTC: ";
              if (bIsXMP)
                out << "XMP : ";
              if (bUnknown)
                out << "????: ";
            }

            DataBuf rawData;
            Internal::enforce(box.length >= sizeof(uuid) + boxHSize, ErrorCode::kerCorruptedMetadata);
            rawData.alloc(box.length - sizeof(uuid) - boxHSize);
            const size_t bufRead = io_->read(rawData.data(), rawData.size());
            if (io_->error())
              throw Error(ErrorCode::kerFailedToReadImageData);
            if (bufRead != rawData.size())
              throw Error(ErrorCode::kerInputDataReadFailed);

            if (bPrint) {
              out << Internal::binaryToString(makeSlice(rawData, 0, rawData.size() > 40 ? 40 : rawData.size()));
              out.flush();
            }
            lf(out, bLF);

            if (bIsExif && bRecursive && rawData.size() > 8) {  // "II*\0long"
              const char a = rawData.read_uint8(0);
              const char b = rawData.read_uint8(1);
              if (a == b && (a == 'I' || a == 'M')) {
                MemIo p(rawData.c_data(), rawData.size());
                printTiffStructure(p, out, option, depth + 1);
              }
            }

            if (bIsIPTC && bRecursive) {
              IptcData::printStructure(out, makeSlice(rawData, 0, rawData.size()), depth);
            }

            if (bIsXMP && bXMP) {
              out.write(rawData.c_str(), rawData.size());
            }
          }
        } break;

        default:
          break;
      }

      // Move to the next box.
      io_->seek(static_cast<int64_t>(position - boxHSize + box.length), BasicIo::beg);
      if (io_->error())
        throw Error(ErrorCode::kerFailedToReadImageData);
      if (bPrint)
        lf(out, bLF);
    }
  }
}

void Jp2Image::writeMetadata() {
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  MemIo tempIo;

  doWriteMetadata(tempIo);  // may throw
  io_->close();
  io_->transfer(tempIo);  // may throw

}  // Jp2Image::writeMetadata

void Jp2Image::encodeJp2Header(const DataBuf& boxBuf, DataBuf& outBuf) {
  DataBuf output(boxBuf.size() + iccProfile_.size() + 100);  // allocate sufficient space
  size_t outlen = boxHSize;                                  // now many bytes have we written to output?
  size_t inlen = boxHSize;                                   // how many bytes have we read from boxBuf?
  Internal::enforce(boxHSize <= output.size(), ErrorCode::kerCorruptedMetadata);
  uint32_t length = getULong(boxBuf.c_data(0), bigEndian);
  Internal::enforce(length <= output.size(), ErrorCode::kerCorruptedMetadata);
  uint32_t count = boxHSize;
  bool bWroteColor = false;

  while (count < length && !bWroteColor) {
    Internal::enforce(boxHSize <= length - count, ErrorCode::kerCorruptedMetadata);
    Internal::Jp2BoxHeader subBox;
    std::memcpy(&subBox, boxBuf.c_data(count), boxHSize);
    Internal::Jp2BoxHeader newBox = subBox;

    if (count < length) {
      subBox.length = getULong(boxBuf.c_data(count), bigEndian);
      subBox.type = getULong(boxBuf.c_data(count + 4), bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
      std::cout << "Jp2Image::encodeJp2Header subbox: " << toAscii(subBox.type) << " length = " << subBox.length
                << '\n';
#endif
      Internal::enforce(subBox.length > 0, ErrorCode::kerCorruptedMetadata);
      Internal::enforce(subBox.length <= length - count, ErrorCode::kerCorruptedMetadata);
      count += subBox.length;
      newBox.type = subBox.type;
    } else {
      subBox.length = 0;
      newBox.type = kJp2BoxTypeColorSpec;
      count = length;
    }

    size_t newlen = subBox.length;
    if (newBox.type == kJp2BoxTypeColorSpec) {
      bWroteColor = true;
      if (!iccProfileDefined()) {
        const char* pad = "\x01\x00\x00\x00\x00\x00\x10\x00\x00\x05\x1cuuid";
        uint32_t psize = 15;
        Internal::enforce(newlen <= output.size() - outlen, ErrorCode::kerCorruptedMetadata);
        ul2Data(reinterpret_cast<byte*>(&newBox.length), psize, bigEndian);
        ul2Data(reinterpret_cast<byte*>(&newBox.type), newBox.type, bigEndian);
        std::copy_n(reinterpret_cast<char*>(&newBox), sizeof(newBox), output.begin() + outlen);
        std::copy_n(pad, psize, output.begin() + outlen + sizeof(newBox));
      } else {
        const char* pad = "\x02\x00\x00";
        uint32_t psize = 3;
        newlen = sizeof(newBox) + psize + iccProfile_.size();
        Internal::enforce(newlen <= output.size() - outlen, ErrorCode::kerCorruptedMetadata);
        ul2Data(reinterpret_cast<byte*>(&newBox.length), static_cast<uint32_t>(newlen), bigEndian);
        ul2Data(reinterpret_cast<byte*>(&newBox.type), newBox.type, bigEndian);
        std::copy_n(reinterpret_cast<char*>(&newBox), sizeof(newBox), output.begin() + outlen);
        std::copy_n(pad, psize, output.begin() + outlen + sizeof(newBox));
        std::copy(iccProfile_.begin(), iccProfile_.end(), output.begin() + outlen + sizeof(newBox) + psize);
      }
    } else {
      Internal::enforce(newlen <= output.size() - outlen, ErrorCode::kerCorruptedMetadata);
      std::copy_n(boxBuf.c_data(inlen), subBox.length, output.begin() + outlen);
    }

    outlen += newlen;
    inlen += subBox.length;
  }

  // allocate the correct number of bytes, copy the data and update the box header
  outBuf.alloc(outlen);
  std::copy_n(output.c_data(), outlen, outBuf.begin());
  ul2Data(outBuf.data(0), static_cast<uint32_t>(outlen), bigEndian);
  ul2Data(outBuf.data(4), kJp2BoxTypeHeader, bigEndian);
}

void Jp2Image::doWriteMetadata(BasicIo& outIo) {
  if (!io_->isopen())
    throw Error(ErrorCode::kerInputDataReadFailed);
  if (!outIo.isopen())
    throw Error(ErrorCode::kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::Jp2Image::doWriteMetadata: Writing JPEG-2000 file " << io_->path() << '\n';
  std::cout << "Exiv2::Jp2Image::doWriteMetadata: tmp file created " << outIo.path() << '\n';
#endif

  // Ensure that this is the correct image type
  if (!isJp2Type(*io_, true)) {
    throw Error(ErrorCode::kerNoImageInInputData);
  }

  // Write JPEG2000 Signature (This is the 1st box)
  if (outIo.write(Jp2Signature.data(), Jp2Signature.size()) != 12)
    throw Error(ErrorCode::kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Jp2Image::doWriteMetadata: JPEG 2000 Signature box written" << '\n';
#endif

  Internal::Jp2BoxHeader box = {0, 0};

  byte boxDataSize[4];
  byte boxUUIDtype[4];
  DataBuf bheaderBuf(8);

  while (io_->tell() < io_->size()) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cout << "Exiv2::Jp2Image::doWriteMetadata: Position: " << io_->tell() << " / " << io_->size() << '\n';
#endif

    // Read chunk header.
    io_->readOrThrow(bheaderBuf.data(), bheaderBuf.size(), ErrorCode::kerInputDataReadFailed);

    // Decode box header.
    box.length = bheaderBuf.read_uint32(0, bigEndian);
    box.type = bheaderBuf.read_uint32(4, bigEndian);

#ifdef EXIV2_DEBUG_MESSAGES
    std::cout << "Exiv2::Jp2Image::doWriteMetadata: box type: " << toAscii(box.type) << " length: " << box.length
              << '\n';
#endif

    if (box.length == 0) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cout << "Exiv2::Jp2Image::doWriteMetadata: Null Box size has been found. "
                   "This is the last box of file."
                << '\n';
#endif
      box.length = static_cast<uint32_t>(io_->size() - io_->tell() + 8);
    }
    if (box.length < 8) {
      // box is broken, so there is nothing we can do here
      throw Error(ErrorCode::kerCorruptedMetadata);
    }

    // Prevent a malicious file from causing a large memory allocation.
    Internal::enforce(box.length - 8 <= io_->size() - io_->tell(), ErrorCode::kerCorruptedMetadata);

    // Read whole box : Box header + Box data (not fixed size - can be null).
    DataBuf boxBuf(box.length);                          // Box header (8 bytes) + box data.
    std::copy_n(bheaderBuf.begin(), 8, boxBuf.begin());  // Copy header.
    io_->readOrThrow(boxBuf.data(8), box.length - 8, ErrorCode::kerInputDataReadFailed);  // Extract box data.

    switch (box.type) {
      case kJp2BoxTypeHeader: {
        DataBuf newBuf;
        encodeJp2Header(boxBuf, newBuf);
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write JP2Header box (length: " << box.length << ")\n";
#endif
        if (outIo.write(newBuf.data(), newBuf.size()) != newBuf.size())
          throw Error(ErrorCode::kerImageWriteFailed);

        // Write all updated metadata here, just after JP2Header.

        if (!exifData_.empty()) {
          // Update Exif data to a new UUID box

          Blob blob;
          ExifParser::encode(blob, littleEndian, exifData_);
          if (!blob.empty()) {
            DataBuf rawExif(blob.size());
            std::copy(blob.begin(), blob.end(), rawExif.begin());

            DataBuf boxData(8 + 16 + rawExif.size());
            ul2Data(boxDataSize, static_cast<uint32_t>(boxData.size()), bigEndian);
            ul2Data(boxUUIDtype, kJp2BoxTypeUuid, bigEndian);
            std::copy_n(boxDataSize, 4, boxData.begin());
            std::copy_n(boxUUIDtype, 4, boxData.begin() + 4);
            std::copy_n(kJp2UuidExif, 16, boxData.begin() + 8);
            std::copy(rawExif.begin(), rawExif.end(), boxData.begin() + 8 + 16);

#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write box with Exif metadata (length: " << boxData.size()
                      << '\n';
#endif
            if (outIo.write(boxData.c_data(), boxData.size()) != boxData.size())
              throw Error(ErrorCode::kerImageWriteFailed);
          }
        }

        if (!iptcData_.empty()) {
          // Update Iptc data to a new UUID box

          DataBuf rawIptc = IptcParser::encode(iptcData_);
          if (!rawIptc.empty()) {
            DataBuf boxData(8 + 16 + rawIptc.size());
            ul2Data(boxDataSize, static_cast<uint32_t>(boxData.size()), bigEndian);
            ul2Data(boxUUIDtype, kJp2BoxTypeUuid, bigEndian);
            std::copy_n(boxDataSize, 4, boxData.begin());
            std::copy_n(boxUUIDtype, 4, boxData.begin() + 4);
            std::copy_n(kJp2UuidIptc, 16, boxData.begin() + 8);
            std::copy(rawIptc.begin(), rawIptc.end(), boxData.begin() + 8 + 16);

#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write box with Iptc metadata (length: " << boxData.size()
                      << '\n';
#endif
            if (outIo.write(boxData.c_data(), boxData.size()) != boxData.size())
              throw Error(ErrorCode::kerImageWriteFailed);
          }
        }

        if (!writeXmpFromPacket() && XmpParser::encode(xmpPacket_, xmpData_) > 1) {
#ifndef SUPPRESS_WARNINGS
          EXV_ERROR << "Failed to encode XMP metadata." << '\n';
#endif
        }
        if (!xmpPacket_.empty()) {
          // Update Xmp data to a new UUID box

          DataBuf xmp(reinterpret_cast<const byte*>(xmpPacket_.data()), xmpPacket_.size());
          DataBuf boxData(8 + 16 + xmp.size());
          ul2Data(boxDataSize, static_cast<uint32_t>(boxData.size()), bigEndian);
          ul2Data(boxUUIDtype, kJp2BoxTypeUuid, bigEndian);
          std::copy_n(boxDataSize, 4, boxData.begin());
          std::copy_n(boxUUIDtype, 4, boxData.begin() + 4);
          std::copy_n(kJp2UuidXmp, 16, boxData.begin() + 8);
          std::copy(xmp.begin(), xmp.end(), boxData.begin() + 8 + 16);

#ifdef EXIV2_DEBUG_MESSAGES
          std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write box with XMP metadata (length: " << boxData.size()
                    << ")" << '\n';
#endif
          if (outIo.write(boxData.c_data(), boxData.size()) != boxData.size())
            throw Error(ErrorCode::kerImageWriteFailed);
        }

        break;
      }

      case kJp2BoxTypeUuid: {
        Internal::enforce(boxBuf.size() >= 24, ErrorCode::kerCorruptedMetadata);
        if (boxBuf.cmpBytes(8, kJp2UuidExif, 16) == 0) {
#ifdef EXIV2_DEBUG_MESSAGES
          std::cout << "Exiv2::Jp2Image::doWriteMetadata: strip Exif Uuid box" << '\n';
#endif
        } else if (boxBuf.cmpBytes(8, kJp2UuidIptc, 16) == 0) {
#ifdef EXIV2_DEBUG_MESSAGES
          std::cout << "Exiv2::Jp2Image::doWriteMetadata: strip Iptc Uuid box" << '\n';
#endif
        } else if (boxBuf.cmpBytes(8, kJp2UuidXmp, 16) == 0) {
#ifdef EXIV2_DEBUG_MESSAGES
          std::cout << "Exiv2::Jp2Image::doWriteMetadata: strip Xmp Uuid box" << '\n';
#endif
        } else {
#ifdef EXIV2_DEBUG_MESSAGES
          std::cout << "Exiv2::Jp2Image::doWriteMetadata: write Uuid box (length: " << box.length << ")" << '\n';
#endif
          if (outIo.write(boxBuf.c_data(), boxBuf.size()) != boxBuf.size())
            throw Error(ErrorCode::kerImageWriteFailed);
        }
        break;
      }

      default: {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::Jp2Image::doWriteMetadata: write box (length: " << box.length << ")" << '\n';
#endif
        if (outIo.write(boxBuf.c_data(), boxBuf.size()) != boxBuf.size())
          throw Error(ErrorCode::kerImageWriteFailed);

        break;
      }
    }
  }

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::Jp2Image::doWriteMetadata: EOF" << '\n';
#endif

}  // Jp2Image::doWriteMetadata

// *************************************************************************
// free functions
Image::UniquePtr newJp2Instance(BasicIo::UniquePtr io, bool create) {
  auto image = std::make_unique<Jp2Image>(std::move(io), create);
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isJp2Type(BasicIo& iIo, bool advance) {
  byte buf[Jp2Signature.size()];
  const size_t bytesRead = iIo.read(buf, Jp2Signature.size());
  if (iIo.error() || iIo.eof() || bytesRead != Jp2Signature.size()) {
    return false;
  }
  bool matched = (memcmp(buf, Jp2Signature.data(), Jp2Signature.size()) == 0);
  if (!advance || !matched) {
    iIo.seek(-static_cast<int64_t>(Jp2Signature.size()), BasicIo::cur);  // Return to original position
  }
  return matched;
}
}  // namespace Exiv2
