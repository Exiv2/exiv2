// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "config.h"

#ifdef EXV_HAVE_LIBZ
#include <zlib.h>  // To uncompress IccProfiles

#include "basicio.hpp"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "jpgimage.hpp"
#include "photoshop.hpp"
#include "pngchunk_int.hpp"
#include "pngimage.hpp"
#include "tiffimage.hpp"
#include "types.hpp"
#include "utils.hpp"

#include <array>
#include <iostream>

namespace {
// Signature from front of PNG file
constexpr unsigned char pngSignature[] = {
    0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A,
};

constexpr unsigned char pngBlank[] = {
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d, 0x49, 0x48, 0x44, 0x52, 0x00, 0x00,
    0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x08, 0x02, 0x00, 0x00, 0x00, 0x90, 0x77, 0x53, 0xde, 0x00, 0x00, 0x00,
    0x01, 0x73, 0x52, 0x47, 0x42, 0x00, 0xae, 0xce, 0x1c, 0xe9, 0x00, 0x00, 0x00, 0x09, 0x70, 0x48, 0x59, 0x73,
    0x00, 0x00, 0x0b, 0x13, 0x00, 0x00, 0x0b, 0x13, 0x01, 0x00, 0x9a, 0x9c, 0x18, 0x00, 0x00, 0x00, 0x0c, 0x49,
    0x44, 0x41, 0x54, 0x08, 0xd7, 0x63, 0xf8, 0xff, 0xff, 0x3f, 0x00, 0x05, 0xfe, 0x02, 0xfe, 0xdc, 0xcc, 0x59,
    0xe7, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82,
};

const auto nullComp = reinterpret_cast<const Exiv2::byte*>("\0\0");
const auto typeExif = reinterpret_cast<const Exiv2::byte*>("eXIf");
const auto typeICCP = reinterpret_cast<const Exiv2::byte*>("iCCP");
bool compare(std::string_view str, const Exiv2::DataBuf& buf) {
  const auto minlen = std::min<size_t>(str.size(), buf.size());
  return buf.cmpBytes(0, str.data(), minlen) == 0;
}
}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {
using namespace Internal;

PngImage::PngImage(BasicIo::UniquePtr io, bool create) :
    Image(ImageType::png, mdExif | mdIptc | mdXmp | mdComment, std::move(io)) {
  if (create && io_->open() == 0) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << "Exiv2::PngImage:: Creating PNG image to memory\n";
#endif
    IoCloser closer(*io_);
    if (io_->write(pngBlank, sizeof(pngBlank)) != sizeof(pngBlank)) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << "Exiv2::PngImage:: Failed to create PNG image on memory\n";
#endif
    }
  }
}

std::string PngImage::mimeType() const {
  return "image/png";
}

static bool zlibToDataBuf(const byte* bytes, uLongf length, DataBuf& result) {
  uLongf uncompressedLen = length;  // just a starting point
  int zlibResult = Z_BUF_ERROR;

  while (zlibResult == Z_BUF_ERROR) {
    result.alloc(uncompressedLen);
    zlibResult = uncompress(result.data(), &uncompressedLen, bytes, length);
    // if result buffer is large than necessary, redo to fit perfectly.
    if (zlibResult == Z_OK && uncompressedLen < result.size()) {
      result.reset();

      result.alloc(uncompressedLen);
      zlibResult = uncompress(result.data(), &uncompressedLen, bytes, length);
    }
    if (zlibResult == Z_BUF_ERROR) {
      // the uncompressed buffer needs to be larger
      result.reset();

      // Sanity - never bigger than 16mb
      if (uncompressedLen > 16 * 1024 * 1024)
        zlibResult = Z_DATA_ERROR;
      else
        uncompressedLen *= 2;
    }
  };

  return zlibResult == Z_OK;
}

static bool zlibToCompressed(const byte* bytes, uLongf length, DataBuf& result) {
  uLongf compressedLen = length;  // just a starting point
  int zlibResult = Z_BUF_ERROR;

  while (zlibResult == Z_BUF_ERROR) {
    result.alloc(compressedLen);
    zlibResult = compress(result.data(), &compressedLen, bytes, length);
    if (zlibResult == Z_BUF_ERROR) {
      // the compressedArray needs to be larger
      result.reset();
      compressedLen *= 2;
    } else {
      result.reset();
      result.alloc(compressedLen);
      zlibResult = compress(result.data(), &compressedLen, bytes, length);
    }
  };

  return zlibResult == Z_OK;
}

static bool tEXtToDataBuf(const byte* bytes, size_t length, DataBuf& result) {
  static std::array<int, 256> value;
  static bool bFirst = true;
  if (bFirst) {
    value.fill(0);
    for (int i = 0; i < 10; i++) {
      value['0' + i] = i + 1;
    }
    for (int i = 0; i < 6; i++) {
      value['a' + i] = i + 10 + 1;
      value['A' + i] = i + 10 + 1;
    }
    bFirst = false;
  }

  // calculate length and allocate result;
  // count: number of \n in the header
  size_t count = 0;
  // p points to the current position in the array bytes
  const byte* p = bytes;

  // header is '\nsomething\n number\n hex'
  // => increment p until it points to the byte after the last \n
  //    p must stay within bounds of the bytes array!
  while (count < 3 && 0 < length) {
    // length is later used for range checks of p => decrement it for each increment of p
    --length;
    if (*p++ == '\n') {
      count++;
    }
  }
  for (size_t i = 0; i < length; i++)
    if (value[p[i]])
      ++count;
  result.alloc((count + 1) / 2);

  // hex to binary
  count = 0;
  byte* r = result.data();
  int n = 0;  // nibble
  for (size_t i = 0; i < length; i++) {
    if (value[p[i]]) {
      int v = value[p[i]] - 1;
      if (++count % 2)
        n = v * 16;  // leading digit
      else
        *r++ = n + v;  // trailing
    }
  }
  return true;
}

static std::string::size_type findi(const std::string& str, const std::string& substr) {
  return str.find(substr);
}

void PngImage::printStructure(std::ostream& out, PrintStructureOption option, size_t depth) {
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  if (!isPngType(*io_, true)) {
    throw Error(ErrorCode::kerNotAnImage, "PNG");
  }

  char chType[5];
  chType[0] = 0;
  chType[4] = 0;

  if (option == kpsBasic || option == kpsXMP || option == kpsIccProfile || option == kpsRecursive) {
    const auto xmpKey = upper("XML:com.adobe.xmp");
    const auto exifKey = upper("Raw profile type exif");
    const auto app1Key = upper("Raw profile type APP1");
    const auto iptcKey = upper("Raw profile type iptc");
    const auto softKey = upper("Software");
    const auto commKey = upper("Comment");
    const auto descKey = upper("Description");

    bool bPrint = option == kpsBasic || option == kpsRecursive;
    if (bPrint) {
      out << "STRUCTURE OF PNG FILE: " << io_->path() << '\n';
      out << " address | chunk |  length | data                           | checksum" << '\n';
    }

    const size_t imgSize = io_->size();
    DataBuf cheaderBuf(8);

    while (!io_->eof() && ::strcmp(chType, "IEND") != 0) {
      const size_t address = io_->tell();

      size_t bufRead = io_->read(cheaderBuf.data(), cheaderBuf.size());
      if (io_->error())
        throw Error(ErrorCode::kerFailedToReadImageData);
      if (bufRead != cheaderBuf.size())
        throw Error(ErrorCode::kerInputDataReadFailed);

      // Decode chunk data length.
      const uint32_t dataOffset = cheaderBuf.read_uint32(0, Exiv2::bigEndian);
      for (int i = 4; i < 8; i++) {
        chType[i - 4] = cheaderBuf.read_uint8(i);
      }

      // test that we haven't hit EOF, or wanting to read excessive data
      const size_t restore = io_->tell();
      if (dataOffset > imgSize - restore) {
        throw Exiv2::Error(ErrorCode::kerFailedToReadImageData);
      }

      DataBuf buff(dataOffset);
      if (dataOffset > 0) {
        bufRead = io_->read(buff.data(), dataOffset);
        enforce(bufRead == dataOffset, ErrorCode::kerFailedToReadImageData);
      }
      io_->seek(restore, BasicIo::beg);

      // format output
      const int iMax = 30;
      const uint32_t blen = dataOffset > iMax ? iMax : dataOffset;
      std::string dataString;
      // if blen == 0 => slice construction fails
      if (blen > 0) {
        std::stringstream ss;
        ss << Internal::binaryToString(makeSlice(buff, 0, blen));
        dataString = ss.str();
      }
      while (dataString.size() < iMax)
        dataString += ' ';
      dataString.resize(iMax);

      if (bPrint) {
        io_->seek(dataOffset, BasicIo::cur);  // jump to checksum
        byte checksum[4];
        bufRead = io_->read(checksum, 4);
        enforce(bufRead == 4, ErrorCode::kerFailedToReadImageData);
        io_->seek(restore, BasicIo::beg);  // restore file pointer

        out << stringFormat("{:8} | {:<5} |{:8} | {}", address, chType, dataOffset, dataString)
            << stringFormat(" | 0x{:02x}{:02x}{:02x}{:02x}\n", checksum[0], checksum[1], checksum[2], checksum[3]);
      }

      // chunk type
      bool tEXt = std::strcmp(chType, "tEXt") == 0;
      bool zTXt = std::strcmp(chType, "zTXt") == 0;
      bool iCCP = std::strcmp(chType, "iCCP") == 0;
      bool iTXt = std::strcmp(chType, "iTXt") == 0;
      bool eXIf = std::strcmp(chType, "eXIf") == 0;

      // for XMP, ICC etc: read and format data
      const auto dataStringU = upper(dataString);
      bool bXMP = option == kpsXMP && findi(dataStringU, xmpKey) == 0;
      bool bExif = option == kpsRecursive && (findi(dataStringU, exifKey) == 0 || findi(dataStringU, app1Key) == 0);
      bool bIptc = option == kpsRecursive && findi(dataStringU, iptcKey) == 0;
      bool bSoft = option == kpsRecursive && findi(dataStringU, softKey) == 0;
      bool bComm = option == kpsRecursive && findi(dataStringU, commKey) == 0;
      bool bDesc = option == kpsRecursive && findi(dataStringU, descKey) == 0;
      bool bDump = bXMP || bExif || bIptc || bSoft || bComm || bDesc || iCCP || eXIf;

      if (bDump) {
        DataBuf dataBuf;
        enforce(dataOffset < std::numeric_limits<uint32_t>::max(), ErrorCode::kerFailedToReadImageData);
        DataBuf data(dataOffset + 1ul);
        data.write_uint8(dataOffset, 0);
        bufRead = io_->read(data.data(), dataOffset);
        enforce(bufRead == dataOffset, ErrorCode::kerFailedToReadImageData);
        io_->seek(restore, BasicIo::beg);
        size_t name_l = std::strlen(data.c_str()) + 1;  // leading string length
        enforce(name_l < dataOffset, ErrorCode::kerCorruptedMetadata);

        auto start = static_cast<uint32_t>(name_l);
        bool bLF = false;

        // decode the chunk
        bool bGood = false;
        if (tEXt) {
          bGood = tEXtToDataBuf(data.c_data(name_l), dataOffset - name_l, dataBuf);
        }
        if (zTXt || iCCP) {
          enforce(dataOffset - name_l - 1 <= std::numeric_limits<uLongf>::max(), ErrorCode::kerCorruptedMetadata);
          bGood = zlibToDataBuf(data.c_data(name_l + 1), static_cast<uLongf>(dataOffset - name_l - 1),
                                dataBuf);  // +1 = 'compressed' flag
        }
        if (iTXt) {
          bGood = (3 <= dataOffset) && (start < dataOffset - 3);  // good if not a nul chunk
        }
        if (eXIf) {
          bGood = true;  // eXIf requires no pre-processing
        }

        // format is content dependent
        if (bGood) {
          if (bXMP) {
            while (start < dataOffset && !data.read_uint8(start))
              start++;                  // skip leading nul bytes
            out << data.c_data(start);  // output the xmp
          }

          if (bExif || bIptc) {
            DataBuf parsedBuf = PngChunk::readRawProfile(dataBuf, tEXt);
#ifdef EXIV2_DEBUG_MESSAGES
            std::cerr << Exiv2::Internal::binaryToString(
                             makeSlice(parsedBuf.c_data(), parsedBuf.size() > 50 ? 50 : parsedBuf.size(), 0))
                      << '\n';
#endif
            if (!parsedBuf.empty()) {
              if (bExif) {
                // create memio object with the data, then print the structure
                MemIo p(parsedBuf.c_data(6), parsedBuf.size() - 6);
                printTiffStructure(p, out, option, depth + 1);
              }
              if (bIptc) {
                IptcData::printStructure(out, makeSlice(parsedBuf, 0, parsedBuf.size()), depth);
              }
            }
          }

          if (bSoft && !dataBuf.empty()) {
            DataBuf s(dataBuf.size() + 1);                         // allocate buffer with an extra byte
            std::copy(dataBuf.begin(), dataBuf.end(), s.begin());  // copy in the dataBuf
            s.write_uint8(dataBuf.size(), 0);                      // nul terminate it
            const auto str = s.c_str();                            // give it name
            out << Internal::indent(depth) << buff.c_str() << ": " << str;
            bLF = true;
          }

          if ((iCCP && option == kpsIccProfile) || bComm) {
            out.write(dataBuf.c_str(), dataBuf.size());
            bLF = bComm;
          }

          if (bDesc && iTXt) {
            DataBuf decoded = PngChunk::decodeTXTChunk(buff, PngChunk::iTXt_Chunk);
            out.write(decoded.c_str(), decoded.size());
            bLF = true;
          }

          if (eXIf && option == kpsRecursive) {
            // create memio object with the data, then print the structure
            MemIo p(data.c_data(), dataOffset);
            printTiffStructure(p, out, option, depth + 1);
          }

          if (bLF)
            out << '\n';
        }
      }
      io_->seek(dataOffset + 4, BasicIo::cur);  // jump past checksum
      if (io_->error())
        throw Error(ErrorCode::kerFailedToReadImageData);
    }
  }
}

static void readChunk(DataBuf& buffer, BasicIo& io) {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::PngImage::readMetadata: Position: " << io.tell() << '\n';
#endif
  const size_t bufRead = io.read(buffer.data(), buffer.size());
  if (io.error()) {
    throw Error(ErrorCode::kerFailedToReadImageData);
  }
  if (bufRead != buffer.size()) {
    throw Error(ErrorCode::kerInputDataReadFailed);
  }
}

void PngImage::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Exiv2::PngImage::readMetadata: Reading PNG file " << io_->path() << '\n';
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  if (!isPngType(*io_, true)) {
    throw Error(ErrorCode::kerNotAnImage, "PNG");
  }
  clearMetadata();

  const size_t imgSize = io_->size();
  DataBuf cheaderBuf(8);  // Chunk header: 4 bytes (data size) + 4 bytes (chunk type).

  while (!io_->eof()) {
    readChunk(cheaderBuf, *io_);  // Read chunk header.

    // Decode chunk data length.
    uint32_t chunkLength = cheaderBuf.read_uint32(0, Exiv2::bigEndian);
    if (chunkLength > imgSize - io_->tell()) {
      throw Exiv2::Error(ErrorCode::kerFailedToReadImageData);
    }

    std::string chunkType(cheaderBuf.c_str(4), 4);
#ifdef EXIV2_DEBUG_MESSAGES
    std::cout << "Exiv2::PngImage::readMetadata: chunk type: " << chunkType << " length: " << chunkLength << '\n';
#endif

    /// \todo analyse remaining chunks of the standard
    // Perform a chunk triage for item that we need.
    if (chunkType == "IEND" || chunkType == "IHDR" || chunkType == "tEXt" || chunkType == "zTXt" ||
        chunkType == "eXIf" || chunkType == "iTXt" || chunkType == "iCCP") {
      DataBuf chunkData(chunkLength);
      if (chunkLength > 0) {
        readChunk(chunkData, *io_);  // Extract chunk data.
      }

      if (chunkType == "IEND") {
        return;  // Last chunk found: we stop parsing.
      }
      if (chunkType == "IHDR" && chunkData.size() >= 8) {
        PngChunk::decodeIHDRChunk(chunkData, &pixelWidth_, &pixelHeight_);
      } else if (chunkType == "tEXt") {
        PngChunk::decodeTXTChunk(this, chunkData, PngChunk::tEXt_Chunk);
      } else if (chunkType == "zTXt") {
        PngChunk::decodeTXTChunk(this, chunkData, PngChunk::zTXt_Chunk);
      } else if (chunkType == "iTXt") {
        PngChunk::decodeTXTChunk(this, chunkData, PngChunk::iTXt_Chunk);
      } else if (chunkType == "eXIf") {
        ByteOrder bo = TiffParser::decode(exifData(), iptcData(), xmpData(), chunkData.c_data(), chunkData.size());
        setByteOrder(bo);
      } else if (chunkType == "iCCP") {
        // The ICC profile name can vary from 1-79 characters.
        uint32_t iccOffset = 0;
        do {
          enforce(iccOffset < 80 && iccOffset < chunkLength, Exiv2::ErrorCode::kerCorruptedMetadata);
        } while (chunkData.read_uint8(iccOffset++) != 0x00);

        profileName_ = std::string(chunkData.c_str(), iccOffset - 1);
        ++iccOffset;  // +1 = 'compressed' flag
        enforce(iccOffset <= chunkLength, Exiv2::ErrorCode::kerCorruptedMetadata);

        zlibToDataBuf(chunkData.c_data(iccOffset), static_cast<uLongf>(chunkLength - iccOffset), iccProfile_);
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PngImage::readMetadata: profile name: " << profileName_ << '\n';
        std::cout << "Exiv2::PngImage::readMetadata: iccProfile.size_ (uncompressed) : " << iccProfile_.size() << '\n';
#endif
      }

      // Set chunkLength to 0 in case we have read a supported chunk type. Otherwise, we need to seek the
      // file to the next chunk position.
      chunkLength = 0;
    }

    // Move to the next chunk: chunk data size + 4 CRC bytes.
#ifdef EXIV2_DEBUG_MESSAGES
    std::cout << "Exiv2::PngImage::readMetadata: Seek to offset: " << chunkLength + 4 << '\n';
#endif
    io_->seek(chunkLength + 4, BasicIo::cur);
    if (io_->error() || io_->eof()) {
      throw Error(ErrorCode::kerFailedToReadImageData);
    }
  }
}  // PngImage::readMetadata

void PngImage::writeMetadata() {
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  MemIo tempIo;

  doWriteMetadata(tempIo);  // may throw
  io_->close();
  io_->transfer(tempIo);  // may throw

}  // PngImage::writeMetadata

void PngImage::doWriteMetadata(BasicIo& outIo) {
  if (!io_->isopen())
    throw Error(ErrorCode::kerInputDataReadFailed);
  if (!outIo.isopen())
    throw Error(ErrorCode::kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::PngImage::doWriteMetadata: Writing PNG file " << io_->path() << "\n";
  std::cout << "Exiv2::PngImage::doWriteMetadata: tmp file created " << outIo.path() << "\n";
#endif

  if (!isPngType(*io_, true)) {
    throw Error(ErrorCode::kerNoImageInInputData);
  }

  // Write PNG Signature.
  if (outIo.write(pngSignature, 8) != 8)
    throw Error(ErrorCode::kerImageWriteFailed);

  DataBuf cheaderBuf(8);  // Chunk header : 4 bytes (data size) + 4 bytes (chunk type).

  while (!io_->eof()) {
    // Read chunk header.
    size_t bufRead = io_->read(cheaderBuf.data(), 8);
    if (io_->error())
      throw Error(ErrorCode::kerFailedToReadImageData);
    if (bufRead != 8)
      throw Error(ErrorCode::kerInputDataReadFailed);

    // Decode chunk data length.

    uint32_t dataOffset = cheaderBuf.read_uint32(0, Exiv2::bigEndian);
    if (dataOffset > 0x7FFFFFFF)
      throw Exiv2::Error(ErrorCode::kerFailedToReadImageData);

    // Read whole chunk : Chunk header + Chunk data (not fixed size - can be null) + CRC (4 bytes).

    DataBuf chunkBuf(8 + dataOffset + 4);                   // Chunk header (8 bytes) + Chunk data + CRC (4 bytes).
    std::copy_n(cheaderBuf.begin(), 8, chunkBuf.begin());   // Copy header.
    bufRead = io_->read(chunkBuf.data(8), dataOffset + 4);  // Extract chunk data + CRC
    if (io_->error())
      throw Error(ErrorCode::kerFailedToReadImageData);
    if (bufRead != dataOffset + 4)
      throw Error(ErrorCode::kerInputDataReadFailed);

    char szChunk[5];
    std::copy_n(cheaderBuf.c_data(4), 4, szChunk);
    szChunk[4] = 0;

    if (!strcmp(szChunk, "IEND")) {
      // Last chunk found: we write it and done.
#ifdef EXIV2_DEBUG_MESSAGES
      std::cout << "Exiv2::PngImage::doWriteMetadata: Write IEND chunk (length: " << dataOffset << ")\n";
#endif
      if (outIo.write(chunkBuf.data(), chunkBuf.size()) != chunkBuf.size())
        throw Error(ErrorCode::kerImageWriteFailed);
      return;
    }
    if (!strcmp(szChunk, "eXIf") || !strcmp(szChunk, "iCCP")) {
      // do nothing (strip): Exif metadata is written following IHDR
      // together with the ICC profile as fresh eXIf and iCCP chunks
#ifdef EXIV2_DEBUG_MESSAGES
      std::cout << "Exiv2::PngImage::doWriteMetadata: strip " << szChunk << " chunk (length: " << dataOffset << ")"
                << '\n';
#endif
    } else if (!strcmp(szChunk, "IHDR")) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cout << "Exiv2::PngImage::doWriteMetadata: Write IHDR chunk (length: " << dataOffset << ")\n";
#endif
      if (outIo.write(chunkBuf.data(), chunkBuf.size()) != chunkBuf.size())
        throw Error(ErrorCode::kerImageWriteFailed);

      // Write all updated metadata here, just after IHDR.
      if (!comment_.empty()) {
        // Update Comment data to a new PNG chunk
        std::string chunk = PngChunk::makeMetadataChunk(comment_, mdComment);
        if (outIo.write(reinterpret_cast<const byte*>(chunk.data()), chunk.size()) != chunk.size()) {
          throw Error(ErrorCode::kerImageWriteFailed);
        }
      }

      if (!exifData_.empty()) {
        // Update Exif data to a new PNG chunk
        Blob blob;
        ExifParser::encode(blob, littleEndian, exifData_);
        if (!blob.empty()) {
          byte length[4];
          ul2Data(length, static_cast<uint32_t>(blob.size()), bigEndian);

          // calculate CRC
          uLong tmp = crc32(0L, Z_NULL, 0);
          tmp = crc32(tmp, typeExif, 4);
          tmp = crc32(tmp, blob.data(), static_cast<uint32_t>(blob.size()));
          byte crc[4];
          ul2Data(crc, tmp, bigEndian);

          if (outIo.write(length, 4) != 4 || outIo.write(typeExif, 4) != 4 ||
              outIo.write(blob.data(), blob.size()) != blob.size() || outIo.write(crc, 4) != 4) {
            throw Error(ErrorCode::kerImageWriteFailed);
          }
#ifdef EXIV2_DEBUG_MESSAGES
          std::cout << "Exiv2::PngImage::doWriteMetadata: build eXIf"
                    << " chunk (length: " << blob.size() << ")" << '\n';
#endif
        }
      }

      if (!iptcData_.empty()) {
        // Update IPTC data to a new PNG chunk
        DataBuf newPsData = Photoshop::setIptcIrb(nullptr, 0, iptcData_);
        if (!newPsData.empty()) {
          std::string rawIptc(newPsData.c_str(), newPsData.size());
          std::string chunk = PngChunk::makeMetadataChunk(rawIptc, mdIptc);
          if (outIo.write(reinterpret_cast<const byte*>(chunk.data()), chunk.size()) != chunk.size()) {
            throw Error(ErrorCode::kerImageWriteFailed);
          }
        }
      }

      if (iccProfileDefined()) {
        DataBuf compressed;
        enforce(iccProfile_.size() <= std::numeric_limits<uLongf>::max(), ErrorCode::kerCorruptedMetadata);
        if (zlibToCompressed(iccProfile_.c_data(), static_cast<uLongf>(iccProfile_.size()), compressed)) {
          const auto nameLength = static_cast<uint32_t>(profileName_.size());
          const uint32_t chunkLength = nameLength + 2 + static_cast<uint32_t>(compressed.size());
          byte length[4];
          ul2Data(length, chunkLength, bigEndian);

          // calculate CRC
          uLong tmp = crc32(0L, Z_NULL, 0);
          tmp = crc32(tmp, typeICCP, 4);
          tmp = crc32(tmp, reinterpret_cast<const Bytef*>(profileName_.data()), nameLength);
          tmp = crc32(tmp, nullComp, 2);
          tmp = crc32(tmp, compressed.c_data(), static_cast<uint32_t>(compressed.size()));
          byte crc[4];
          ul2Data(crc, tmp, bigEndian);

          if (outIo.write(length, 4) != 4 || outIo.write(typeICCP, 4) != 4 ||
              outIo.write(reinterpret_cast<const byte*>(profileName_.data()), nameLength) != nameLength ||
              outIo.write(nullComp, 2) != 2 ||
              outIo.write(compressed.c_data(), compressed.size()) != compressed.size() || outIo.write(crc, 4) != 4) {
            throw Error(ErrorCode::kerImageWriteFailed);
          }
#ifdef EXIV2_DEBUG_MESSAGES
          std::cout << "Exiv2::PngImage::doWriteMetadata: build iCCP"
                    << " chunk (length: " << chunkLength << ")" << '\n';
#endif
        }
      }

      if (!writeXmpFromPacket() && XmpParser::encode(xmpPacket_, xmpData_) > 1) {
#ifndef SUPPRESS_WARNINGS
        EXV_ERROR << "Failed to encode XMP metadata.\n";
#endif
      }
      if (!xmpPacket_.empty()) {
        // Update XMP data to a new PNG chunk
        std::string chunk = PngChunk::makeMetadataChunk(xmpPacket_, mdXmp);
        if (outIo.write(reinterpret_cast<const byte*>(chunk.data()), chunk.size()) != chunk.size()) {
          throw Error(ErrorCode::kerImageWriteFailed);
        }
      }
    } else if (!strcmp(szChunk, "tEXt") || !strcmp(szChunk, "zTXt") || !strcmp(szChunk, "iTXt")) {
      DataBuf key = PngChunk::keyTXTChunk(chunkBuf, true);
      if (!key.empty() && (compare("Raw profile type exif", key) || compare("Raw profile type APP1", key) ||
                           compare("Raw profile type iptc", key) || compare("Raw profile type xmp", key) ||
                           compare("XML:com.adobe.xmp", key) || compare("Description", key))) {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PngImage::doWriteMetadata: strip " << szChunk << " chunk (length: " << dataOffset << ")"
                  << '\n';
#endif
      } else {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PngImage::doWriteMetadata: write " << szChunk << " chunk (length: " << dataOffset << ")"
                  << '\n';
#endif
        if (outIo.write(chunkBuf.c_data(), chunkBuf.size()) != chunkBuf.size())
          throw Error(ErrorCode::kerImageWriteFailed);
      }
    } else {
      // Write all others chunk as well.
#ifdef EXIV2_DEBUG_MESSAGES
      std::cout << "Exiv2::PngImage::doWriteMetadata:  copy " << szChunk << " chunk (length: " << dataOffset << ")"
                << '\n';
#endif
      if (outIo.write(chunkBuf.c_data(), chunkBuf.size()) != chunkBuf.size())
        throw Error(ErrorCode::kerImageWriteFailed);
    }
  }

}  // PngImage::doWriteMetadata

// *************************************************************************
// free functions
Image::UniquePtr newPngInstance(BasicIo::UniquePtr io, bool create) {
  auto image = std::make_unique<PngImage>(std::move(io), create);
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isPngType(BasicIo& iIo, bool advance) {
  if (iIo.error() || iIo.eof()) {
    throw Error(ErrorCode::kerInputDataReadFailed);
  }
  const int32_t len = 8;
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  int rc = memcmp(buf, pngSignature, 8);
  if (!advance || rc != 0) {
    iIo.seek(-len, BasicIo::cur);
  }

  return rc == 0;
}
}  // namespace Exiv2
#endif
