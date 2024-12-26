// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "config.h"

#ifdef EXV_HAVE_LIBZ
#include <zlib.h>  // To uncompress or compress text chunk

#include "enforce.hpp"
#include "error.hpp"
#include "exif.hpp"
#include "helper_functions.hpp"
#include "image.hpp"
#include "iptc.hpp"
#include "jpgimage.hpp"
#include "photoshop.hpp"
#include "pngchunk_int.hpp"
#include "safe_op.hpp"
#include "tiffimage.hpp"

// standard includes
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <string>

/*

URLs to find information about PNG chunks :

tEXt and zTXt chunks : http://www.vias.org/pngguide/chapter11_04.html
iTXt chunk           : http://www.vias.org/pngguide/chapter11_05.html
PNG tags             : http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/PNG.html#TextualData

*/
namespace {
constexpr size_t nullSeparators = 2;
}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2::Internal {
void PngChunk::decodeIHDRChunk(const DataBuf& data, uint32_t* outWidth, uint32_t* outHeight) {
  // Extract image width and height from IHDR chunk.
  *outWidth = data.read_uint32(0, bigEndian);
  *outHeight = data.read_uint32(4, bigEndian);
}

void PngChunk::decodeTXTChunk(Image* pImage, const DataBuf& data, TxtChunkType type) {
  DataBuf key = keyTXTChunk(data);
  DataBuf arr = parseTXTChunk(data, key.size(), type);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::PngChunk::decodeTXTChunk: TXT chunk data: " << std::string(arr.c_str(), arr.size()) << '\n';
#endif
  if (!key.empty())
    parseChunkContent(pImage, key.c_data(), key.size(), arr);
}

DataBuf PngChunk::decodeTXTChunk(const DataBuf& data, TxtChunkType type) {
  DataBuf key = keyTXTChunk(data);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::PngChunk::decodeTXTChunk: TXT chunk key: " << std::string(key.c_str(), key.size()) << '\n';
#endif
  return parseTXTChunk(data, key.size(), type);
}

DataBuf PngChunk::keyTXTChunk(const DataBuf& data, bool stripHeader) {
  // From a tEXt, zTXt, or iTXt chunk, we get the keyword which is null terminated.
  const size_t offset = stripHeader ? 8ul : 0ul;
  if (data.size() <= offset)
    throw Error(ErrorCode::kerFailedToReadImageData);

  auto it = std::find(data.cbegin() + offset, data.cend(), 0);
  if (it == data.cend())
    throw Error(ErrorCode::kerFailedToReadImageData);

  return {data.c_data() + offset, std::distance(data.cbegin(), it) - offset};
}

DataBuf PngChunk::parseTXTChunk(const DataBuf& data, size_t keysize, TxtChunkType type) {
  DataBuf arr;

  if (type == zTXt_Chunk) {
    enforce(data.size() >= Safe::add(keysize, nullSeparators), ErrorCode::kerCorruptedMetadata);

    // Extract a deflate compressed Latin-1 text chunk

    // we get the compression method after the key
    if (*data.c_data(keysize + 1) != 0x00) {
      // then it isn't zlib compressed and we are sunk
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "Exiv2::PngChunk::parseTXTChunk: Non-standard zTXt compression method.\n";
#endif
      throw Error(ErrorCode::kerFailedToReadImageData);
    }

    // compressed string after the compression technique spec
    size_t compressedTextSize = data.size() - keysize - nullSeparators;
    if (compressedTextSize) {
      const byte* compressedText = data.c_data(keysize + nullSeparators);
      enforce(compressedTextSize < data.size(), ErrorCode::kerCorruptedMetadata);

      zlibUncompress(compressedText, static_cast<uint32_t>(compressedTextSize), arr);
    }
  } else if (type == tEXt_Chunk) {
    enforce(data.size() >= Safe::add(keysize, static_cast<size_t>(1)), ErrorCode::kerCorruptedMetadata);
    // Extract a non-compressed Latin-1 text chunk

    // the text comes after the key, but isn't null terminated
    size_t textsize = data.size() - keysize - 1;
    if (textsize) {
      const byte* text = data.c_data(keysize + 1);

      arr = DataBuf(text, textsize);
    }
  } else if (type == iTXt_Chunk) {
    enforce(data.size() > Safe::add(keysize, static_cast<size_t>(3)), ErrorCode::kerCorruptedMetadata);
    const size_t nullCount = std::count(data.c_data(keysize + 3), data.c_data(data.size() - 1), '\0');
    enforce(nullCount >= nullSeparators, ErrorCode::kerCorruptedMetadata);

    // Extract a deflate compressed or uncompressed UTF-8 text chunk

    // we get the compression flag after the key
    const byte compressionFlag = data.read_uint8(keysize + 1);
    // we get the compression method after the compression flag
    const byte compressionMethod = data.read_uint8(keysize + 2);

    enforce(compressionFlag == 0x00 || compressionFlag == 0x01, ErrorCode::kerCorruptedMetadata);
    if (compressionFlag == 0x01)
      enforce(compressionMethod == 0x00, ErrorCode::kerFailedToReadImageData);

    // language description string after the compression technique spec
    const size_t languageTextMaxSize = data.size() - keysize - 3;
    std::string languageText = string_from_unterminated(data.c_str(keysize + 3), languageTextMaxSize);
    const size_t languageTextSize = languageText.size();

    enforce(data.size() >= Safe::add(Safe::add(keysize, static_cast<size_t>(4)), languageTextSize),
            ErrorCode::kerCorruptedMetadata);
    // translated keyword string after the language description
    std::string translatedKeyText = string_from_unterminated(data.c_str(keysize + 3 + languageTextSize + 1),
                                                             data.size() - (keysize + 3 + languageTextSize + 1));
    const size_t translatedKeyTextSize = translatedKeyText.size();

    enforce(Safe::add(keysize + 3 + languageTextSize + 1, Safe::add(translatedKeyTextSize, static_cast<size_t>(1))) <=
                data.size(),
            ErrorCode::kerCorruptedMetadata);

    const auto textsize =
        static_cast<long>(data.size() - (keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1));
    if (textsize) {
      const byte* text = data.c_data(keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1);

      if (compressionFlag == 0x00) {
        // then it's an uncompressed iTXt chunk
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PngChunk::parseTXTChunk: We found an uncompressed iTXt field\n";
#endif
        arr = DataBuf(text, textsize);
      } else {
        // then it's a zlib compressed iTXt chunk
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PngChunk::parseTXTChunk: We found a zlib compressed iTXt field\n";
#endif

        // the compressed text comes after the translated keyword, but isn't null terminated
        zlibUncompress(text, textsize, arr);
      }
    }
  } else {
#ifdef DEBUG
    std::cerr << "Exiv2::PngChunk::parseTXTChunk: We found a field, not expected though\n";
#endif
    throw Error(ErrorCode::kerFailedToReadImageData);
  }

  return arr;
}

void PngChunk::parseChunkContent(Image* pImage, const byte* key, size_t keySize, const DataBuf& arr) {
  // We look if an ImageMagick EXIF raw profile exist.

  if (keySize >= 21 &&
      (memcmp("Raw profile type exif", key, 21) == 0 || memcmp("Raw profile type APP1", key, 21) == 0) &&
      pImage->exifData().empty()) {
    DataBuf exifData = readRawProfile(arr, false);
    size_t length = exifData.size();

    if (length >= 6) {  // length should have at least the size of exifHeader
      // Find the position of Exif header in bytes array.
      const std::array<byte, 6> exifHeader{0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
      size_t pos = std::numeric_limits<size_t>::max();

      /// \todo Find substring inside an string
      for (size_t i = 0; i < length - exifHeader.size(); i++) {
        if (exifData.cmpBytes(i, exifHeader.data(), exifHeader.size()) == 0) {
          pos = i;
          break;
        }
      }

      // If found it, store only these data at from this place.

      if (pos != std::numeric_limits<size_t>::max()) {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PngChunk::parseChunkContent: Exif header found at position " << pos << "\n";
#endif
        pos = pos + sizeof(exifHeader);
        ByteOrder bo = TiffParser::decode(pImage->exifData(), pImage->iptcData(), pImage->xmpData(),
                                          exifData.c_data(pos), length - pos);
        pImage->setByteOrder(bo);
      } else {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Failed to decode Exif metadata.\n";
#endif
        pImage->exifData().clear();
      }
    }
  }

  // We look if an ImageMagick IPTC raw profile exist.

  if (keySize >= 21 && memcmp("Raw profile type iptc", key, 21) == 0 && pImage->iptcData().empty()) {
    DataBuf psData = readRawProfile(arr, false);
    if (!psData.empty()) {
      Blob iptcBlob;
      const byte* record = nullptr;
      uint32_t sizeIptc = 0;
      uint32_t sizeHdr = 0;

      const byte* pEnd = psData.c_data(psData.size() - 1);
      const byte* pCur = psData.c_data();
      while (pCur < pEnd && 0 == Photoshop::locateIptcIrb(pCur, pEnd - pCur, &record, sizeHdr, sizeIptc)) {
        if (sizeIptc) {
#ifdef EXIV2_DEBUG_MESSAGES
          std::cerr << "Found IPTC IRB, size = " << sizeIptc << "\n";
#endif
          append(iptcBlob, record + sizeHdr, sizeIptc);
        }
        pCur = record + sizeHdr + sizeIptc;
        pCur += (sizeIptc & 1);
      }
      if (!iptcBlob.empty() && IptcParser::decode(pImage->iptcData(), iptcBlob.data(), iptcBlob.size())) {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Failed to decode IPTC metadata.\n";
#endif
        pImage->clearIptcData();
      }
      // If there is no IRB, try to decode the complete chunk data
      if (iptcBlob.empty() && IptcParser::decode(pImage->iptcData(), psData.c_data(), psData.size())) {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Failed to decode IPTC metadata.\n";
#endif
        pImage->clearIptcData();
      }
    }  // if (psData.size() > 0)
  }

  // We look if an ImageMagick XMP raw profile exist.

  if (keySize >= 20 && memcmp("Raw profile type xmp", key, 20) == 0 && pImage->xmpData().empty()) {
    DataBuf xmpBuf = readRawProfile(arr, false);
    size_t length = xmpBuf.size();

    if (length > 0) {
      std::string& xmpPacket = pImage->xmpPacket();
      xmpPacket.assign(xmpBuf.c_str(), length);
      if (auto idx = xmpPacket.find_first_of('<'); idx != std::string::npos && idx > 0) {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Removing " << idx << " characters from the beginning of the XMP packet\n";
#endif
        xmpPacket = xmpPacket.substr(idx);
      }
      if (XmpParser::decode(pImage->xmpData(), xmpPacket)) {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
      }
    }
  }

  // We look if an Adobe XMP string exist.

  if (keySize >= 17 && memcmp("XML:com.adobe.xmp", key, 17) == 0 && pImage->xmpData().empty() && !arr.empty()) {
    std::string& xmpPacket = pImage->xmpPacket();
    xmpPacket.assign(arr.c_str(), arr.size());
    if (auto idx = xmpPacket.find_first_of('<'); idx != std::string::npos && idx > 0) {
#ifndef SUPPRESS_WARNINGS
      EXV_WARNING << "Removing " << idx << " characters "
                  << "from the beginning of the XMP packet\n";
#endif
      xmpPacket = xmpPacket.substr(idx);
    }
    if (XmpParser::decode(pImage->xmpData(), xmpPacket)) {
#ifndef SUPPRESS_WARNINGS
      EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
    }
  }

  // We look if a comments string exist. Note than we use only 'Description' keyword which
  // is dedicated to store long comments. 'Comment' keyword is ignored.

  if (keySize >= 11 && memcmp("Description", key, 11) == 0 && pImage->comment().empty()) {
    pImage->setComment(std::string(arr.c_str(), arr.size()));
  }

}  // PngChunk::parseChunkContent

std::string PngChunk::makeMetadataChunk(const std::string& metadata, MetadataId type) {
  std::string rawProfile;

  switch (type) {
    case mdComment:
      return makeUtf8TxtChunk("Description", metadata, true);
    case mdIptc:
      rawProfile = writeRawProfile(metadata, "iptc");
      return makeAsciiTxtChunk("Raw profile type iptc", rawProfile, true);
    case mdXmp:
      return makeUtf8TxtChunk("XML:com.adobe.xmp", metadata, false);
    case mdExif:
    case mdIccProfile:
    case mdNone:
      return {};
  }

  return {};

}  // PngChunk::makeMetadataChunk

void PngChunk::zlibUncompress(const byte* compressedText, unsigned int compressedTextSize, DataBuf& arr) {
  uLongf uncompressedLen = compressedTextSize * 2;  // just a starting point
  int zlibResult = Z_BUF_ERROR;
  int dos = 0;

  while (zlibResult == Z_BUF_ERROR) {
    arr.alloc(uncompressedLen);
    zlibResult = uncompress(arr.data(), &uncompressedLen, compressedText, compressedTextSize);
    if (zlibResult == Z_OK) {
      arr.resize(uncompressedLen);
    } else if (zlibResult == Z_BUF_ERROR) {
      // the uncompressedArray needs to be larger
      uncompressedLen *= 2;
      // DoS protection. can't be bigger than 64k
      if (uncompressedLen > 131072) {
        if (++dos > 1)
          break;
        uncompressedLen = 131072;
      }
    } else {
      // something bad happened
      throw Error(ErrorCode::kerFailedToReadImageData);
    }
  };

  if (zlibResult != Z_OK) {
    throw Error(ErrorCode::kerFailedToReadImageData);
  }
}  // PngChunk::zlibUncompress

std::string PngChunk::zlibCompress(const std::string& text) {
  auto compressedLen = static_cast<uLongf>(text.size() * 2);  // just a starting point
  int zlibResult = Z_BUF_ERROR;

  DataBuf arr;
  while (zlibResult == Z_BUF_ERROR) {
    arr.resize(compressedLen);
    zlibResult = compress2(arr.data(), &compressedLen, reinterpret_cast<const Bytef*>(text.data()),
                           static_cast<uLong>(text.size()), Z_BEST_COMPRESSION);

    switch (zlibResult) {
      case Z_OK:
        arr.resize(compressedLen);
        break;
      case Z_BUF_ERROR:
        // The compressed array needs to be larger
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PngChunk::parsePngChunk: doubling size for compression.\n";
#endif
        compressedLen *= 2;
        // DoS protection. Cap max compressed size
        if (compressedLen > 131072)
          throw Error(ErrorCode::kerFailedToReadImageData);
        break;
      default:
        // Something bad happened
        throw Error(ErrorCode::kerFailedToReadImageData);
    }
  };

  return {arr.c_str(), arr.size()};

}  // PngChunk::zlibCompress

std::string PngChunk::makeAsciiTxtChunk(const std::string& keyword, const std::string& text, bool compress) {
  // Chunk structure: length (4 bytes) + chunk type + chunk data + CRC (4 bytes)
  // Length is the size of the chunk data
  // CRC is calculated on chunk type + chunk data

  // Compressed text chunk using zlib.
  // Chunk data format : keyword + 0x00 + compression method (0x00) + compressed text

  // Not Compressed text chunk.
  // Chunk data format : keyword + 0x00 + text

  // Build chunk data, determine chunk type
  std::string chunkData = keyword + '\0';
  std::string chunkType;
  if (compress) {
    chunkData += '\0' + zlibCompress(text);
    chunkType = "zTXt";
  } else {
    chunkData += text;
    chunkType = "tEXt";
  }
  // Determine length of the chunk data
  byte length[4];
  ul2Data(length, static_cast<uint32_t>(chunkData.size()), bigEndian);
  // Calculate CRC on chunk type and chunk data
  std::string crcData = chunkType + chunkData;
  uLong tmp = crc32(0L, Z_NULL, 0);
  tmp = crc32(tmp, reinterpret_cast<const Bytef*>(crcData.data()), static_cast<uInt>(crcData.size()));
  byte crc[4];
  ul2Data(crc, tmp, bigEndian);
  // Assemble the chunk
  return std::string(reinterpret_cast<const char*>(length), 4) + chunkType + chunkData +
         std::string(reinterpret_cast<const char*>(crc), 4);

}  // PngChunk::makeAsciiTxtChunk

std::string PngChunk::makeUtf8TxtChunk(const std::string& keyword, const std::string& text, bool compress) {
  // Chunk structure: length (4 bytes) + chunk type + chunk data + CRC (4 bytes)
  // Length is the size of the chunk data
  // CRC is calculated on chunk type + chunk data

  // Chunk data format : keyword + 0x00 + compression flag (0x00: uncompressed - 0x01: compressed)
  //                     + compression method (0x00: zlib format) + language tag (null) + 0x00
  //                     + translated keyword (null) + 0x00 + text (compressed or not)

  // Build chunk data, determine chunk type
  std::string chunkData = keyword;
  if (compress) {
    static const char flags[] = {0x00, 0x01, 0x00, 0x00, 0x00};
    chunkData += std::string(flags, 5) + zlibCompress(text);
  } else {
    static const char flags[] = {0x00, 0x00, 0x00, 0x00, 0x00};
    chunkData += std::string(flags, 5) + text;
  }
  // Determine length of the chunk data
  byte length[4];
  ul2Data(length, static_cast<uint32_t>(chunkData.size()), bigEndian);
  // Calculate CRC on chunk type and chunk data
  std::string chunkType = "iTXt";
  std::string crcData = chunkType + chunkData;
  uLong tmp = crc32(0L, Z_NULL, 0);
  tmp = crc32(tmp, reinterpret_cast<const Bytef*>(crcData.data()), static_cast<uInt>(crcData.size()));
  byte crc[4];
  ul2Data(crc, tmp, bigEndian);
  // Assemble the chunk
  return std::string(reinterpret_cast<const char*>(length), 4) + chunkType + chunkData +
         std::string(reinterpret_cast<const char*>(crc), 4);

}  // PngChunk::makeUtf8TxtChunk

DataBuf PngChunk::readRawProfile(const DataBuf& text, bool iTXt) {
  DataBuf info;
  if (text.size() <= 1) {
    return info;
  }

  const unsigned char unhex[103] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15,
  };

  if (iTXt) {
    info.alloc(text.size());
    std::copy(text.cbegin(), text.cend(), info.begin());
    return info;
  }

  const char* sp = text.c_str(1);                 // current byte (space pointer)
  const char* eot = text.c_str(text.size() - 1);  // end of text

  if (sp >= eot) {
    return info;
  }

  // Look for newline
  while (*sp != '\n') {
    sp++;
    if (sp == eot) {
      return info;
    }
  }
  sp++;  // step over '\n'
  if (sp == eot) {
    return info;
  }

  // Look for length
  while (*sp == '\0' || *sp == ' ' || *sp == '\n') {
    sp++;
    if (sp == eot) {
      return info;
    }
  }

  // Parse the length.
  size_t length = 0;
  while ('0' <= *sp && *sp <= '9') {
    // Compute the new length using unsigned long, so that we can check for overflow.
    const size_t newlength = (10 * length) + (*sp - '0');
    length = newlength;
    sp++;
    if (sp == eot) {
      return info;
    }
  }
  sp++;  // step over '\n'
  if (sp == eot) {
    return info;
  }

  enforce(length <= static_cast<size_t>(eot - sp) / 2, Exiv2::ErrorCode::kerCorruptedMetadata);

  // Allocate space
  if (length == 0) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: invalid profile length\n";
#endif
  }
  info.alloc(length);
  if (info.size() != length) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: cannot allocate memory\n";
#endif
    return info;
  }

  if (info.empty())  // Early return
    return info;

  // Copy profile, skipping white space and column 1 "=" signs
  unsigned char* dp = info.data();  // decode pointer
  size_t nibbles = length * 2;

  for (size_t i = 0; i < nibbles; i++) {
    enforce(sp < eot, Exiv2::ErrorCode::kerCorruptedMetadata);
    while (*sp < '0' || (*sp > '9' && *sp < 'a') || *sp > 'f') {
      if (*sp == '\0') {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: ran out of data\n";
#endif
        return {};
      }

      sp++;
      enforce(sp < eot, Exiv2::ErrorCode::kerCorruptedMetadata);
    }

    if (i % 2 == 0)
      *dp = static_cast<unsigned char>(16 * unhex[static_cast<size_t>(*sp++)]);
    else
      (*dp++) += unhex[static_cast<size_t>(*sp++)];
  }

  return info;

}  // PngChunk::readRawProfile

std::string PngChunk::writeRawProfile(const std::string& profileData, const char* profileType) {
  static const byte hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

  std::ostringstream oss;
  oss << '\n' << profileType << '\n' << std::setw(8) << profileData.size();
  auto sp = reinterpret_cast<const byte*>(profileData.data());
  for (std::string::size_type i = 0; i < profileData.size(); ++i) {
    if (i % 36 == 0)
      oss << '\n';
    oss << hex[*sp >> 4 & 0x0fU];
    oss << hex[*sp++ & 0x0fU];
  }
  oss << '\n';
  return oss.str();

}  // PngChunk::writeRawProfile

}  // namespace Exiv2::Internal
#endif  // ifdef EXV_HAVE_LIBZ
