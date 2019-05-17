// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
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
  File:    pngchunk.cpp
 */
// *****************************************************************************
// included header files
#include "config.h"

#include "enforce.hpp"
#include "error.hpp"
#include "exif.hpp"
#include "helper_functions.hpp"
#include "image.hpp"
#include "iptc.hpp"
#include "jpgimage.hpp"
#include "pngchunk_int.hpp"
#include "safe_op.hpp"
#include "tiffimage.hpp"

// + standard includes
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include <zlib.h>  // To uncompress or compress text chunk

/*

URLs to find informations about PNG chunks :

tEXt and zTXt chunks : http://www.vias.org/pngguide/chapter11_04.html
iTXt chunk           : http://www.vias.org/pngguide/chapter11_05.html
PNG tags             : http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/PNG.html#TextualData

*/

// *****************************************************************************
// class member definitions
namespace Exiv2
{
    namespace Internal
    {
        void PngChunk::decodeIHDRChunk(const DataBuf& data, PngImageHeader& h)
        {
            enforce(data.size_ == 13, kerCorruptedMetadata);

            h.width = getLong((const byte*)data.pData_, bigEndian);
            h.height = getLong((const byte*)data.pData_ + 4, bigEndian);
            h.bitDepth = data.pData_[8];
            h.colorType = data.pData_[9];
            h.compressionMethod = data.pData_[10];
            h.filterMethod = data.pData_[11];
            h.interlaceMethod = data.pData_[12];

            enforce(h.colorType == 0 || h.colorType == 2 || h.colorType == 3 || h.colorType == 4 || h.colorType == 6,
                    kerCorruptedMetadata);
            switch (h.colorType) {
                case 0:
                    enforce(
                        h.bitDepth == 1 || h.bitDepth == 2 || h.bitDepth == 4 || h.bitDepth == 8 || h.bitDepth == 16,
                        kerCorruptedMetadata);
                    break;
                case 2:
                    enforce(h.bitDepth == 8 || h.bitDepth == 16, kerCorruptedMetadata);
                    break;
                case 3:
                    enforce(h.bitDepth == 1 || h.bitDepth == 2 || h.bitDepth == 4 || h.bitDepth == 8,
                            kerCorruptedMetadata);
                    break;
                case 4:
                    enforce(h.bitDepth == 8 || h.bitDepth == 16, kerCorruptedMetadata);
                    break;
                case 6:
                    enforce(h.bitDepth == 8 || h.bitDepth == 16, kerCorruptedMetadata);
                    break;
            }
            enforce(h.compressionMethod == 0, kerCorruptedMetadata);
            enforce(h.filterMethod == 0, kerCorruptedMetadata);
            enforce(h.interlaceMethod == 0 || h.interlaceMethod == 1, kerCorruptedMetadata);
        }

        void PngChunk::decodeTXTChunk(Image* pImage, const DataBuf& data, TxtChunkType type)
        {
            DataBuf key = keyTXTChunk(data);
            DataBuf arr = parseTXTChunk(data, key.size_, type);

#ifdef DEBUG
            std::cout << "Exiv2::PngChunk::decodeTXTChunk: TXT chunk data: "
                      << std::string((const char*)arr.pData_, arr.size_) << std::endl;
#endif
            parseChunkContent(pImage, key.pData_, key.size_, arr);

        }  // PngChunk::decodeTXTChunk

        DataBuf PngChunk::decodeTXTChunk(const DataBuf& data, TxtChunkType type)
        {
            DataBuf key = keyTXTChunk(data);

#ifdef DEBUG
            std::cout << "Exiv2::PngChunk::decodeTXTChunk: TXT chunk key: "
                      << std::string((const char*)key.pData_, key.size_) << std::endl;
#endif
            return parseTXTChunk(data, key.size_, type);

        }  // PngChunk::decodeTXTChunk

        DataBuf PngChunk::keyTXTChunk(const DataBuf& data, bool stripHeader)
        {
            // From a tEXt, zTXt, or iTXt chunk,
            // we get the key, it's a null terminated string at the chunk start
            const int offset = stripHeader ? 8 : 0;
            if (data.size_ <= offset)
                throw Error(kerFailedToReadImageData);
            const byte* key = data.pData_ + offset;

            // Find null string at end of key.
            int keysize = 0;
            while (key[keysize] != 0) {
                keysize++;
                // look if keysize is valid.
                if (keysize + offset >= data.size_)
                    throw Error(kerFailedToReadImageData);
            }

            return DataBuf(key, keysize);

        }  // PngChunk::keyTXTChunk

        DataBuf PngChunk::parseTXTChunk(const DataBuf& data, int keysize, TxtChunkType type)
        {
            DataBuf arr;

            if (type == zTXt_Chunk) {
                enforce(data.size_ >= Safe::add(keysize, 2), Exiv2::kerCorruptedMetadata);

                // Extract a deflate compressed Latin-1 text chunk

                // we get the compression method after the key
                const byte* compressionMethod = data.pData_ + keysize + 1;
                if (*compressionMethod != 0x00) {
                    // then it isn't zlib compressed and we are sunk
#ifdef DEBUG
                    std::cerr << "Exiv2::PngChunk::parseTXTChunk: Non-standard zTXt compression method.\n";
#endif
                    throw Error(kerFailedToReadImageData);
                }

                // compressed string after the compression technique spec
                const byte* compressedText = data.pData_ + keysize + 2;
                long compressedTextSize = data.size_ - keysize - 2;
                enforce(compressedTextSize < data.size_, kerCorruptedMetadata);

                zlibUncompress(compressedText, compressedTextSize, arr);
            } else if (type == tEXt_Chunk) {
                enforce(data.size_ >= Safe::add(keysize, 1), Exiv2::kerCorruptedMetadata);
                // Extract a non-compressed Latin-1 text chunk

                // the text comes after the key, but isn't null terminated
                const byte* text = data.pData_ + keysize + 1;
                long textsize = data.size_ - keysize - 1;

                arr = DataBuf(text, textsize);
            } else if (type == iTXt_Chunk) {
                enforce(data.size_ >= Safe::add(keysize, 3), Exiv2::kerCorruptedMetadata);
                const size_t nullSeparators = std::count(&data.pData_[keysize + 3], &data.pData_[data.size_], '\0');
                enforce(nullSeparators >= 2, Exiv2::kerCorruptedMetadata);

                // Extract a deflate compressed or uncompressed UTF-8 text chunk

                // we get the compression flag after the key
                const byte compressionFlag = data.pData_[keysize + 1];
                // we get the compression method after the compression flag
                const byte compressionMethod = data.pData_[keysize + 2];

                enforce(compressionFlag == 0x00 || compressionFlag == 0x01, Exiv2::kerCorruptedMetadata);
                enforce(compressionMethod == 0x00, Exiv2::kerCorruptedMetadata);

                // language description string after the compression technique spec
                const size_t languageTextMaxSize = data.size_ - keysize - 3;
                std::string languageText =
                    string_from_unterminated((const char*)(data.pData_ + Safe::add(keysize, 3)), languageTextMaxSize);
                const size_t languageTextSize = languageText.size();

                enforce(static_cast<unsigned long>(data.size_) >=
                            Safe::add(static_cast<size_t>(Safe::add(keysize, 4)), languageTextSize),
                        Exiv2::kerCorruptedMetadata);
                // translated keyword string after the language description
                std::string translatedKeyText =
                    string_from_unterminated((const char*)(data.pData_ + keysize + 3 + languageTextSize + 1),
                                             data.size_ - (keysize + 3 + languageTextSize + 1));
                const unsigned int translatedKeyTextSize = static_cast<unsigned int>(translatedKeyText.size());

                if ((compressionFlag == 0x00) || (compressionFlag == 0x01 && compressionMethod == 0x00)) {
                    enforce(Safe::add(static_cast<unsigned int>(keysize + 3 + languageTextSize + 1),
                                      Safe::add(translatedKeyTextSize, 1u)) <= static_cast<unsigned int>(data.size_),
                            Exiv2::kerCorruptedMetadata);

                    const byte* text = data.pData_ + keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1;
                    const long textsize = static_cast<long>(
                        data.size_ - (keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1));

                    if (compressionFlag == 0x00) {
                        // then it's an uncompressed iTXt chunk
#ifdef DEBUG
                        std::cout << "Exiv2::PngChunk::parseTXTChunk: We found an uncompressed iTXt field\n";
#endif

                        arr.alloc(textsize);
                        arr = DataBuf(text, textsize);
                    } else if (compressionFlag == 0x01 && compressionMethod == 0x00) {
                        // then it's a zlib compressed iTXt chunk
#ifdef DEBUG
                        std::cout << "Exiv2::PngChunk::parseTXTChunk: We found a zlib compressed iTXt field\n";
#endif

                        // the compressed text comes after the translated keyword, but isn't null terminated
                        zlibUncompress(text, textsize, arr);
                    }
                } else {
                    // then it isn't zlib compressed and we are sunk
#ifdef DEBUG
                    std::cerr << "Exiv2::PngChunk::parseTXTChunk: Non-standard iTXt compression method.\n";
#endif
                    throw Error(kerFailedToReadImageData);
                }
            } else {
#ifdef DEBUG
                std::cerr << "Exiv2::PngChunk::parseTXTChunk: We found a field, not expected though\n";
#endif
                throw Error(kerFailedToReadImageData);
            }

            return arr;

        }  // PngChunk::parsePngChunk

        void PngChunk::parseChunkContent(Image* pImage, const byte* key, long keySize, const DataBuf arr)
        {
            // We look if an ImageMagick EXIF raw profile exist.

            if (keySize >= 21 &&
                (memcmp("Raw profile type exif", key, 21) == 0 || memcmp("Raw profile type APP1", key, 21) == 0) &&
                pImage->exifData().empty()) {
                DataBuf exifData = readRawProfile(arr, false);
                const long length = exifData.size_;

                if (length > 0) {
                    // Find the position of Exif header in bytes array.
                    bool foundPos{false};
                    size_t pos{0};
                    const std::array<byte, 6> exifHeader{0x45, 0x78, 0x69, 0x66, 0x00, 0x00};
                    const auto& it =
                        std::search(exifData.cbegin(), exifData.cend(), exifHeader.cbegin(), exifHeader.cend());
                    if (it != exifData.cend()) {
                        pos = it - exifData.cbegin() + exifHeader.size();
                        foundPos = true;
                    }

                    // If found it, store only these data at from this place.
                    if (foundPos) {
#ifdef DEBUG
                        std::cout << "Exiv2::PngChunk::parseChunkContent: Exif header found at position " << pos
                                  << "\n";
#endif
                        ByteOrder bo = TiffParser::decode(pImage->exifData(), pImage->iptcData(), pImage->xmpData(),
                                                          exifData.pData_ + pos, length - static_cast<uint32_t>(pos));
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
                if (psData.size_ > 0) {
                    Blob iptcBlob;
                    const byte* record = 0;
                    uint32_t sizeIptc = 0;
                    uint32_t sizeHdr = 0;

                    const byte* pEnd = psData.pData_ + psData.size_;
                    const byte* pCur = psData.pData_;
                    while (pCur < pEnd && 0 == Photoshop::locateIptcIrb(pCur, static_cast<long>(pEnd - pCur), &record,
                                                                        &sizeHdr, &sizeIptc)) {
                        if (sizeIptc) {
#ifdef DEBUG
                            std::cerr << "Found IPTC IRB, size = " << sizeIptc << "\n";
#endif
                            append(iptcBlob, record + sizeHdr, sizeIptc);
                        }
                        pCur = record + sizeHdr + sizeIptc;
                        pCur += (sizeIptc & 1);
                    }
                    if (iptcBlob.size() > 0 &&
                        IptcParser::decode(pImage->iptcData(), &iptcBlob[0], static_cast<uint32_t>(iptcBlob.size()))) {
#ifndef SUPPRESS_WARNINGS
                        EXV_WARNING << "Failed to decode IPTC metadata.\n";
#endif
                        pImage->clearIptcData();
                    }
                    // If there is no IRB, try to decode the complete chunk data
                    if (iptcBlob.empty() && IptcParser::decode(pImage->iptcData(), psData.pData_, psData.size_)) {
#ifndef SUPPRESS_WARNINGS
                        EXV_WARNING << "Failed to decode IPTC metadata.\n";
#endif
                        pImage->clearIptcData();
                    }
                }  // if (psData.size_ > 0)
            }

            // We look if an ImageMagick XMP raw profile exist.

            if (keySize >= 20 && memcmp("Raw profile type xmp", key, 20) == 0 && pImage->xmpData().empty()) {
                DataBuf xmpBuf = readRawProfile(arr, false);
                long length = xmpBuf.size_;

                if (length > 0) {
                    std::string& xmpPacket = pImage->xmpPacket();
                    xmpPacket.assign(reinterpret_cast<char*>(xmpBuf.pData_), length);
                    std::string::size_type idx = xmpPacket.find_first_of('<');
                    if (idx != std::string::npos && idx > 0) {
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

            if (keySize >= 17 && memcmp("XML:com.adobe.xmp", key, 17) == 0 && pImage->xmpData().empty()) {
                if (arr.size_ > 0) {
                    std::string& xmpPacket = pImage->xmpPacket();
                    xmpPacket.assign(reinterpret_cast<char*>(arr.pData_), arr.size_);
                    std::string::size_type idx = xmpPacket.find_first_of('<');
                    if (idx != std::string::npos && idx > 0) {
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
            }

            // We look if a comments string exist. Note than we use only 'Description' keyword which
            // is dedicaced to store long comments. 'Comment' keyword is ignored.

            if (keySize >= 11 && memcmp("Description", key, 11) == 0 && pImage->comment().empty()) {
                pImage->setComment(std::string(reinterpret_cast<char*>(arr.pData_), arr.size_));
            }

        }  // PngChunk::parseChunkContent

        std::string PngChunk::makeMetadataChunk(const std::string& metadata, MetadataId type)
        {
            std::string chunk;
            std::string rawProfile;

            switch (type) {
                case mdComment:
                    chunk = makeUtf8TxtChunk("Description", metadata, true);
                    break;
                case mdExif:
                    rawProfile = writeRawProfile(metadata, "exif");
                    chunk = makeAsciiTxtChunk("Raw profile type exif", rawProfile, true);
                    break;
                case mdIptc:
                    rawProfile = writeRawProfile(metadata, "iptc");
                    chunk = makeAsciiTxtChunk("Raw profile type iptc", rawProfile, true);
                    break;
                case mdXmp:
                    chunk = makeUtf8TxtChunk("XML:com.adobe.xmp", metadata, false);
                    break;
                case mdIccProfile:
                    break;
                case mdNone:
                    assert(false);
            }

            return chunk;

        }  // PngChunk::makeMetadataChunk

        void PngChunk::zlibUncompress(const byte* compressedText, unsigned int compressedTextSize, DataBuf& arr)
        {
            uLongf uncompressedLen = compressedTextSize * 2;  // just a starting point
            int zlibResult;
            int dos = 0;

            do {
                arr.alloc(uncompressedLen);
                zlibResult = uncompress((Bytef*)arr.pData_, &uncompressedLen, compressedText, compressedTextSize);
                if (zlibResult == Z_OK) {
                    assert((uLongf)arr.size_ >= uncompressedLen);
                    arr.size_ = uncompressedLen;
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
                    throw Error(kerFailedToReadImageData);
                }
            } while (zlibResult == Z_BUF_ERROR);

            if (zlibResult != Z_OK) {
                throw Error(kerFailedToReadImageData);
            }
        }  // PngChunk::zlibUncompress

        std::string PngChunk::zlibCompress(const std::string& text)
        {
            uLongf compressedLen = static_cast<uLongf>(text.size() * 2);  // just a starting point
            int zlibResult;

            DataBuf arr;
            do {
                arr.alloc(compressedLen);
                zlibResult = compress2((Bytef*)arr.pData_, &compressedLen, (const Bytef*)text.data(),
                                       static_cast<uLong>(text.size()), Z_BEST_COMPRESSION);

                switch (zlibResult) {
                    case Z_OK:
                        assert((uLongf)arr.size_ >= compressedLen);
                        arr.size_ = compressedLen;
                        break;
                    case Z_BUF_ERROR:
                        // The compressed array needs to be larger
#ifdef DEBUG
                        std::cout << "Exiv2::PngChunk::parsePngChunk: doubling size for compression.\n";
#endif
                        compressedLen *= 2;
                        // DoS protection. Cap max compressed size
                        if (compressedLen > 131072)
                            throw Error(kerFailedToReadImageData);
                        break;
                    default:
                        // Something bad happened
                        throw Error(kerFailedToReadImageData);
                }
            } while (zlibResult == Z_BUF_ERROR);

            return std::string((const char*)arr.pData_, arr.size_);

        }  // PngChunk::zlibCompress

        std::string PngChunk::makeAsciiTxtChunk(const std::string& keyword, const std::string& text, bool compress)
        {
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
            tmp = crc32(tmp, (const Bytef*)crcData.data(), static_cast<uInt>(crcData.size()));
            byte crc[4];
            ul2Data(crc, tmp, bigEndian);
            // Assemble the chunk
            return std::string((const char*)length, 4) + chunkType + chunkData + std::string((const char*)crc, 4);

        }  // PngChunk::makeAsciiTxtChunk

        std::string PngChunk::makeUtf8TxtChunk(const std::string& keyword, const std::string& text, bool compress)
        {
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
            tmp = crc32(tmp, (const Bytef*)crcData.data(), static_cast<uInt>(crcData.size()));
            byte crc[4];
            ul2Data(crc, tmp, bigEndian);
            // Assemble the chunk
            return std::string((const char*)length, 4) + chunkType + chunkData + std::string((const char*)crc, 4);

        }  // PngChunk::makeUtf8TxtChunk

        DataBuf PngChunk::readRawProfile(const DataBuf& text, bool iTXt)
        {
            DataBuf info;
            unsigned char unhex[103] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7,  8,  9,  0,  0,  0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0, 0, 0,
                                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15};
            if (text.size_ == 0) {
                return DataBuf();
            }

            if (iTXt) {
                info.alloc(text.size_);
                ::memcpy(info.pData_, text.pData_, text.size_);
                return info;
            }

            const char* sp = (char*)text.pData_ + 1;            // current byte (space pointer)
            const char* eot = (char*)text.pData_ + text.size_;  // end of text

            if (sp >= eot) {
                return DataBuf();
            }

            // Look for newline
            while (*sp != '\n') {
                sp++;
                if (sp == eot) {
                    return DataBuf();
                }
            }
            sp++;  // step over '\n'
            if (sp == eot) {
                return DataBuf();
            }

            // Look for length
            while (*sp == '\0' || *sp == ' ' || *sp == '\n') {
                sp++;
                if (sp == eot) {
                    return DataBuf();
                }
            }

            // Parse the length.
            long length = 0;
            while ('0' <= *sp && *sp <= '9') {
                // Compute the new length using unsigned long, so that we can
                // check for overflow.
                const unsigned long newlength = (10 * static_cast<unsigned long>(length)) + (*sp - '0');
                if (newlength > static_cast<unsigned long>(std::numeric_limits<long>::max())) {
                    return DataBuf(); // Integer overflow.
                }
                length = static_cast<long>(newlength);
                sp++;
                if (sp == eot ) {
                    return DataBuf();
                }
            }
            sp++;  // step over '\n'
            if (sp == eot) {
                return DataBuf();
            }

            enforce(length <= (eot - sp)/2, Exiv2::kerCorruptedMetadata);

            // Allocate space
            if (length == 0) {
#ifdef DEBUG
                std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: invalid profile length\n";
#endif
            }
            info.alloc(length);
            if (info.size_ != length) {
#ifdef DEBUG
                std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: cannot allocate memory\n";
#endif
                return DataBuf();
            }

            // Copy profile, skipping white space and column 1 "=" signs

            unsigned char* dp = (unsigned char*)info.pData_;  // decode pointer
            unsigned int nibbles = length * 2;

            for (long i = 0; i < (long)nibbles; i++) {
                enforce(sp < eot, Exiv2::kerCorruptedMetadata);
                while (*sp < '0' || (*sp > '9' && *sp < 'a') || *sp > 'f') {
                    if (*sp == '\0') {
#ifdef DEBUG
                        std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: ran out of data\n";
#endif
                        return DataBuf();
                    }

                    sp++;
                    enforce(sp < eot, Exiv2::kerCorruptedMetadata);
                }

                if (i%2 == 0)
                    *dp = (unsigned char) (16*unhex[(int) *sp++]);
                else
                    (*dp++) += unhex[(int) *sp++];
            }

            return info;

        }  // PngChunk::readRawProfile

        std::string PngChunk::writeRawProfile(const std::string& profileData, const char* profileType)
        {
            static byte hex[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

            std::ostringstream oss;
            oss << '\n' << profileType << '\n' << std::setw(8) << profileData.size();
            const char* sp = profileData.data();
            for (std::string::size_type i = 0; i < profileData.size(); ++i) {
                if (i % 36 == 0)
                    oss << '\n';
                oss << hex[((*sp >> 4) & 0x0f)];
                oss << hex[((*sp++) & 0x0f)];
            }
            oss << '\n';
            return oss.str();

        }  // PngChunk::writeRawProfile

    }  // namespace Internal
}  // namespace Exiv2
