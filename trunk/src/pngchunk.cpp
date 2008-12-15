// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
 *
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
  Version: $Rev$
  Author(s): Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
  History: 12-Jun-06, gc: submitted
  Credits: See header file
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

//#define DEBUG 1
#ifdef EXV_HAVE_LIBZ

extern "C" {
#include <zlib.h>     // To uncompress or compress text chunk
}

#include "pngchunk_int.hpp"
#include "tiffimage.hpp"
#include "exif.hpp"
#include "iptc.hpp"
#include "image.hpp"
#include "error.hpp"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <cstdarg>

/*

URLs to find informations about PNG chunks :

tEXt and zTXt chunks : http://www.vias.org/pngguide/chapter11_04.html
iTXt chunk           : http://www.vias.org/pngguide/chapter11_05.html
PNG tags             : http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/PNG.html#TextualData

*/

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    void PngChunk::decodeIHDRChunk(const DataBuf& data,
                                   int*           outWidth,
                                   int*           outHeight)
    {
        // Extract image width and height from IHDR chunk.

        *outWidth  = getLong((const byte*)data.pData_,     bigEndian);
        *outHeight = getLong((const byte*)data.pData_ + 4, bigEndian);

    } // PngChunk::decodeIHDRChunk

    void PngChunk::decodeTXTChunk(Image*         pImage,
                                  const DataBuf& data,
                                  TxtChunkType   type)
    {
        DataBuf key = keyTXTChunk(data);

#ifdef DEBUG
        std::cout << "Exiv2::PngChunk::decodeTXTChunk: TXT chunk key: "
                  << std::string((const char*)key.pData_) << "\n";
#endif
        DataBuf arr = parseTXTChunk(data, key.size_, type);

#ifdef DEBUG
        std::cout << "Exiv2::PngChunk::decodeTXTChunk: TXT chunk data: "
                  << std::string((const char*)arr.pData_, 32) << "\n";
#endif
        parseChunkContent(pImage, key.pData_, arr);

    } // PngChunk::decodeTXTChunk

    DataBuf PngChunk::keyTXTChunk(const DataBuf& data, bool stripHeader)
    {
        // From a tEXt, zTXt, or iTXt chunk,
        // we get the key, it's a null terminated string at the chunk start

        const byte *key = data.pData_ + (stripHeader ? 8 : 0);

        // Find null string at end of key.
        int keysize=0;
        for ( ; key[keysize] != 0 ; keysize++)
        {
            // look if keysize is valid.
            if (keysize >= data.size_)
                throw Error(14);
        }

        return DataBuf(key, keysize);

    } // PngChunk::keyTXTChunk

    DataBuf PngChunk::parseTXTChunk(const DataBuf& data,
                                    int            keysize,
                                    TxtChunkType   type)
    {
        DataBuf arr;

        if(type == zTXt_Chunk)
        {
            // Extract a deflate compressed Latin-1 text chunk

            // we get the compression method after the key
            const byte* compressionMethod = data.pData_ + keysize + 1;
            if ( *compressionMethod != 0x00 )
            {
                // then it isn't zlib compressed and we are sunk
#ifdef DEBUG
                std::cerr << "Exiv2::PngChunk::parseTXTChunk: Non-standard zTXt compression method.\n";
#endif
                throw Error(14);
            }

            // compressed string after the compression technique spec
            const byte* compressedText      = data.pData_ + keysize + 2;
            unsigned int compressedTextSize = data.size_  - keysize - 2;

            zlibUncompress(compressedText, compressedTextSize, arr);
        }
        else if(type == tEXt_Chunk)
        {
            // Extract a non-compressed Latin-1 text chunk

            // the text comes after the key, but isn't null terminated
            const byte* text = data.pData_ + keysize + 1;
            long textsize    = data.size_  - keysize - 1;

            arr.alloc(textsize);
            arr = DataBuf(text, textsize);
        }
        else if(type == iTXt_Chunk)
        {
            // Extract a deflate compressed or uncompressed UTF-8 text chunk

            // we get the compression flag after the key
            const byte* compressionFlag   = data.pData_ + keysize + 1;
            // we get the compression method after the compression flag
            const byte* compressionMethod = data.pData_ + keysize + 2;
            // language description string after the compression technique spec
            std::string languageText((const char*)(data.pData_ + keysize + 3));
            unsigned int languageTextSize = languageText.size();
            // translated keyword string after the language description
            std::string translatedKeyText((const char*)(data.pData_ + keysize + 3 + languageTextSize +1));
            unsigned int translatedKeyTextSize = translatedKeyText.size();

            if ( compressionFlag[0] == 0x00 )
            {
                // then it's an uncompressed iTXt chunk
#ifdef DEBUG
                std::cout << "Exiv2::PngChunk::parseTXTChunk: We found an uncompressed iTXt field\n";
#endif

                // the text comes after the translated keyword, but isn't null terminated
                const byte* text = data.pData_ + keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1;
                long textsize    = data.size_ - (keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1);

                arr.alloc(textsize);
                arr = DataBuf(text, textsize);
            }
            else if ( compressionFlag[0] == 0x01 && compressionMethod[0] == 0x00 )
            {
                // then it's a zlib compressed iTXt chunk
#ifdef DEBUG
                std::cout << "Exiv2::PngChunk::parseTXTChunk: We found a zlib compressed iTXt field\n";
#endif

                // the compressed text comes after the translated keyword, but isn't null terminated
                const byte* compressedText = data.pData_ + keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1;
                long compressedTextSize    = data.size_ - (keysize + 3 + languageTextSize + 1 + translatedKeyTextSize + 1);

                zlibUncompress(compressedText, compressedTextSize, arr);
            }
            else
            {
                // then it isn't zlib compressed and we are sunk
#ifdef DEBUG
                std::cerr << "Exiv2::PngChunk::parseTXTChunk: Non-standard iTXt compression method.\n";
#endif
                throw Error(14);
            }
        }
        else
        {
#ifdef DEBUG
            std::cerr << "Exiv2::PngChunk::parseTXTChunk: We found a field, not expected though\n";
#endif
            throw Error(14);
        }

        return arr;

    } // PngChunk::parsePngChunk

    void PngChunk::parseChunkContent(Image* pImage, const byte *key, const DataBuf arr)
    {
        // We look if an ImageMagick EXIF raw profile exist.

        if ( (memcmp("Raw profile type exif", key, 21) == 0 ||
              memcmp("Raw profile type APP1", key, 21) == 0) &&
             pImage->exifData().empty())
        {
            DataBuf exifData = readRawProfile(arr);
            long length      = exifData.size_;

            if (length > 0)
            {
                // Find the position of Exif header in bytes array.

                const byte exifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
                long pos = -1;

                for (long i=0 ; i < length-(long)sizeof(exifHeader) ; i++)
                {
                    if (memcmp(exifHeader, &exifData.pData_[i], sizeof(exifHeader)) == 0)
                    {
                        pos = i;
                        break;
                    }
                }

                // If found it, store only these data at from this place.

                if (pos !=-1)
                {
#ifdef DEBUG
                    std::cout << "Exiv2::PngChunk::parseChunkContent: Exif header found at position " << pos << "\n";
#endif
                    pos = pos + sizeof(exifHeader);
                    ByteOrder bo = TiffParser::decode(pImage->exifData(),
                                                      pImage->iptcData(),
                                                      pImage->xmpData(),
                                                      exifData.pData_ + pos,
                                                      length - pos);
                    pImage->setByteOrder(bo);
                }
                else
                {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Exiv2::PngChunk::parseChunkContent: Failed to decode Exif metadata.\n";
#endif
                    pImage->exifData().clear();
                }
            }
        }

        // We look if an ImageMagick IPTC raw profile exist.

        if ( memcmp("Raw profile type iptc", key, 21) == 0 &&
             pImage->iptcData().empty())
        {
            DataBuf iptcData = readRawProfile(arr);
            long length      = iptcData.size_;

            if (length > 0)
                IptcParser::decode(pImage->iptcData(), iptcData.pData_, length);
        }

        // We look if an ImageMagick XMP raw profile exist.

        if ( memcmp("Raw profile type xmp", key, 20) == 0 &&
             pImage->xmpData().empty())
        {
            DataBuf xmpBuf = readRawProfile(arr);
            long length    = xmpBuf.size_;

            if (length > 0)
            {
                std::string& xmpPacket = pImage->xmpPacket();
                xmpPacket.assign(reinterpret_cast<char*>(xmpBuf.pData_), length);
                std::string::size_type idx = xmpPacket.find_first_of('<');
                if (idx != std::string::npos && idx > 0)
                {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Exiv2::PngChunk::parseChunkContent: Removing " << idx
                              << " characters from the beginning of the XMP packet\n";
#endif
                    xmpPacket = xmpPacket.substr(idx);
                }
                if (XmpParser::decode(pImage->xmpData(), xmpPacket))
                {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Exiv2::PngChunk::parseChunkContent: Failed to decode XMP metadata.\n";
#endif
                }
            }
        }

        // We look if an Adobe XMP string exist.

        if ( memcmp("XML:com.adobe.xmp", key, 17) == 0 &&
             pImage->xmpData().empty())
        {
            if (arr.size_ > 0)
            {
                std::string& xmpPacket = pImage->xmpPacket();
                xmpPacket.assign(reinterpret_cast<char*>(arr.pData_), arr.size_);
                std::string::size_type idx = xmpPacket.find_first_of('<');
                if (idx != std::string::npos && idx > 0)
                {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: Removing " << idx << " characters "
                              << "from the beginning of the XMP packet\n";
#endif
                    xmpPacket = xmpPacket.substr(idx);
                }
                if (XmpParser::decode(pImage->xmpData(), xmpPacket))
                {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: Failed to decode XMP metadata.\n";
#endif
                }
            }
        }

        // We look if a comments string exist. Note than we use only 'Description' keyword which
        // is dedicaced to store long comments. 'Comment' keyword is ignored.

        if ( memcmp("Description", key, 11) == 0 &&
             pImage->comment().empty())
        {
            pImage->comment().assign(reinterpret_cast<char*>(arr.pData_), arr.size_);
        }

    } // PngChunk::parseChunkContent

    DataBuf PngChunk::makeMetadataChunk(const DataBuf& metadata, MetadataType type, bool compress)
    {
        if (type == comment_Data)
        {
            DataBuf key(11);
            memcpy(key.pData_, "Description", 11);
            DataBuf rawData = makeUtf8TxtChunk(key, metadata, compress);
            return rawData;
        }
        else if (type == exif_Data)
        {
            DataBuf tmp(4);
            memcpy(tmp.pData_, "exif", 4);
            DataBuf rawProfile = writeRawProfile(metadata, tmp);
            DataBuf key(17 + tmp.size_);
            memcpy(key.pData_,      "Raw profile type ", 17);
            memcpy(key.pData_ + 17, tmp.pData_, tmp.size_);
            DataBuf rawData = makeAsciiTxtChunk(key, rawProfile, compress);
            return rawData;
        }
        else if (type == iptc_Data)
        {
            DataBuf tmp(4);
            memcpy(tmp.pData_, "iptc", 4);
            DataBuf rawProfile = writeRawProfile(metadata, tmp);
            DataBuf key(17 + tmp.size_);
            memcpy(key.pData_,      "Raw profile type ", 17);
            memcpy(key.pData_ + 17, tmp.pData_, tmp.size_);
            DataBuf rawData = makeAsciiTxtChunk(key, rawProfile, compress);
            return rawData;
        }
        else if (type == xmp_Data)
        {
            DataBuf key(17);
            memcpy(key.pData_, "XML:com.adobe.xmp", 17);
            DataBuf rawData = makeUtf8TxtChunk(key, metadata, compress);
            return rawData;
        }

        return DataBuf();

    } // PngChunk::makeMetadataChunk

    void PngChunk::zlibUncompress(const byte*  compressedText,
                                  unsigned int compressedTextSize,
                                  DataBuf&     arr)
    {
        uLongf uncompressedLen = compressedTextSize * 2; // just a starting point
        int zlibResult;

        do
        {
            arr.alloc(uncompressedLen);
            zlibResult = uncompress((Bytef*)arr.pData_, &uncompressedLen,
                                    compressedText, compressedTextSize);

            if (zlibResult == Z_OK)
            {
                // then it is all OK
                arr.alloc(uncompressedLen);
            }
            else if (zlibResult == Z_BUF_ERROR)
            {
                // the uncompressedArray needs to be larger
#ifdef DEBUG
                std::cout << "Exiv2::PngChunk::parsePngChunk: doubling size for decompression.\n";
#endif
                uncompressedLen *= 2;

                // DoS protection. can't be bigger than 64k
                if ( uncompressedLen > 131072 )
                    break;
            }
            else
            {
                // something bad happened
                throw Error(14);
            }
        }
        while (zlibResult == Z_BUF_ERROR);

        if (zlibResult != Z_OK)
            throw Error(14);

    } // PngChunk::zlibUncompress

    void PngChunk::zlibCompress(const byte*  text,
                                unsigned int textSize,
                                DataBuf&     arr)
    {
        uLongf compressedLen = textSize * 2; // just a starting point
        int zlibResult;

        do
        {
            arr.alloc(compressedLen);
            zlibResult = compress2((Bytef*)arr.pData_, &compressedLen,
                                   text, textSize, Z_BEST_COMPRESSION);

            if (zlibResult == Z_OK)
            {
                // then it is all OK
                arr.alloc(compressedLen);
            }
            else if (zlibResult == Z_BUF_ERROR)
            {
                // the compressedArray needs to be larger
#ifdef DEBUG
                std::cout << "Exiv2::PngChunk::parsePngChunk: doubling size for compression.\n";
#endif
                compressedLen *= 2;

                // DoS protection. can't be bigger than 64k
                if ( compressedLen > 131072 )
                    break;
            }
            else
            {
                // something bad happened
                throw Error(14);
            }
        }
        while (zlibResult == Z_BUF_ERROR);

        if (zlibResult != Z_OK)
            throw Error(14);

    } // PngChunk::zlibCompress

    DataBuf PngChunk::makeAsciiTxtChunk(const DataBuf& key, const DataBuf& data, bool compress)
    {
        DataBuf type(4);
        DataBuf data4crc;
        DataBuf chunkData;
        byte    chunkDataSize[4];
        byte    chunkCRC[4];

        if (compress)
        {
            // Compressed text chunk using ZLib.
            // Data format    : key ("zTXt") + 0x00 + compression type (0x00) + compressed data
            // Chunk structure: data lenght (4 bytes) + chunk type (4 bytes) + compressed data + CRC (4 bytes)

            memcpy(type.pData_, "zTXt", 4);

            DataBuf compressedData;
            zlibCompress(data.pData_, data.size_, compressedData);

            data4crc.alloc(key.size_ + 1 + 1 + compressedData.size_);
            memcpy(data4crc.pData_,                 key.pData_,            key.size_);
            memcpy(data4crc.pData_ + key.size_,     "\0\0",                 2);
            memcpy(data4crc.pData_ + key.size_ + 2, compressedData.pData_, compressedData.size_);

            uLong crc = crc32(0L, Z_NULL, 0);
            crc       = crc32(crc, data4crc.pData_, data4crc.size_);

            ul2Data(chunkCRC, crc, Exiv2::bigEndian);
            ul2Data(chunkDataSize, data4crc.size_, Exiv2::bigEndian);

            chunkData.alloc(4 + type.size_ + data4crc.size_ + 4);
            memcpy(chunkData.pData_,                                   chunkDataSize,   4);
            memcpy(chunkData.pData_ + 4,                               type.pData_,     type.size_);
            memcpy(chunkData.pData_ + 4 + type.size_,                  data4crc.pData_, data4crc.size_);
            memcpy(chunkData.pData_ + 4 + type.size_ + data4crc.size_, chunkCRC,        4);
        }
        else
        {
            // Not Compressed text chunk.
            // Data Format    : key ("tEXt") + 0x00 + data
            // Chunk Structure: data lenght (4 bytes) + chunk type (4 bytes) + data + CRC (4 bytes)

            memcpy(type.pData_, "tEXt", 4);

            data4crc.alloc(key.size_ + 1 + data.size_);
            memcpy(data4crc.pData_,                 key.pData_,  key.size_);
            memcpy(data4crc.pData_ + key.size_,     "\0",        1);
            memcpy(data4crc.pData_ + key.size_ + 1, data.pData_, data.size_);

            uLong crc = crc32(0L, Z_NULL, 0);
            crc       = crc32(crc, data4crc.pData_, data4crc.size_);

            ul2Data(chunkCRC, crc, Exiv2::bigEndian);
            ul2Data(chunkDataSize, data4crc.size_, Exiv2::bigEndian);

            chunkData.alloc(4 + type.size_ + data4crc.size_ + 4);
            memcpy(chunkData.pData_,                                   chunkDataSize,   4);
            memcpy(chunkData.pData_ + 4,                               type.pData_,     type.size_);
            memcpy(chunkData.pData_ + 4 + type.size_,                  data4crc.pData_, data4crc.size_);
            memcpy(chunkData.pData_ + 4 + type.size_ + data4crc.size_, chunkCRC,        4);
        }

        return chunkData;

    } // PngChunk::makeAsciiTxtChunk

    DataBuf PngChunk::makeUtf8TxtChunk(const DataBuf& key, const DataBuf& data, bool compress)
    {
        DataBuf type(4);
        DataBuf textData;        // text compressed or not.
        DataBuf data4crc;
        DataBuf chunkData;
        byte    chunkDataSize[4];
        byte    chunkCRC[4];

        // Compressed text chunk using ZLib.
        // Data format    : key ("iTXt") + 0x00 + compression flag (0x00: uncompressed - 0x01: compressed) +
        //                  compression method (0x00) + language id (null) + 0x00 +
        //                  translated key (null) + 0x00 + text (compressed or not)
        // Chunk structure: data lenght (4 bytes) + chunk type (4 bytes) + data + CRC (4 bytes)

        memcpy(type.pData_, "iTXt", 4);

        if (compress)
        {
            const unsigned char flags[] = {0x00, 0x01, 0x00, 0x00, 0x00};

            zlibCompress(data.pData_, data.size_, textData);
            data4crc.alloc(key.size_ + 5 + textData.size_);
            memcpy(data4crc.pData_, key.pData_, key.size_);
            memcpy(data4crc.pData_ + key.size_, flags, 5);
        }
        else
        {
            const unsigned char flags[] = {0x00, 0x00, 0x00, 0x00, 0x00};

            textData = DataBuf(data.pData_, data.size_);
            data4crc.alloc(key.size_ + 5 + textData.size_);
            memcpy(data4crc.pData_, key.pData_, key.size_);
            memcpy(data4crc.pData_ + key.size_, flags, 5);
        }
        memcpy(data4crc.pData_ + key.size_ + 5, textData.pData_, textData.size_);

        uLong crc = crc32(0L, Z_NULL, 0);
        crc       = crc32(crc, data4crc.pData_, data4crc.size_);

        ul2Data(chunkCRC, crc, Exiv2::bigEndian);
        ul2Data(chunkDataSize, data4crc.size_, Exiv2::bigEndian);

        chunkData.alloc(4 + type.size_ + data4crc.size_ + 4);
        memcpy(chunkData.pData_,                                   chunkDataSize,   4);
        memcpy(chunkData.pData_ + 4,                               type.pData_,     type.size_);
        memcpy(chunkData.pData_ + 4 + type.size_,                  data4crc.pData_, data4crc.size_);
        memcpy(chunkData.pData_ + 4 + type.size_ + data4crc.size_, chunkCRC,        4);

        return chunkData;

    } // PngChunk::makeUtf8TxtChunk

    DataBuf PngChunk::readRawProfile(const DataBuf& text)
    {
        DataBuf                 info;
        register long           i;
        register unsigned char *dp;
        const char             *sp;
        unsigned int            nibbles;
        long                    length;
        unsigned char           unhex[103]={0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                                            0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                                            0,0,0,0,0,0,0,0,0,1, 2,3,4,5,6,7,8,9,0,0,
                                            0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0,
                                            0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,10,11,12,
                                            13,14,15};

        sp = (char*)text.pData_+1;

        // Look for newline

        while (*sp != '\n')
            sp++;

        // Look for length

        while (*sp == '\0' || *sp == ' ' || *sp == '\n')
            sp++;

        length = (long) atol(sp);

        while (*sp != ' ' && *sp != '\n')
            sp++;

        // Allocate space

        if (length == 0)
        {
#ifdef DEBUG
            std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: invalid profile length\n";
#endif
            return DataBuf();
        }

        info.alloc(length);

        if (info.size_ != length)
        {
#ifdef DEBUG
            std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: cannot allocate memory\n";
#endif
            return DataBuf();
        }

        // Copy profile, skipping white space and column 1 "=" signs

        dp      = (unsigned char*)info.pData_;
        nibbles = length * 2;

        for (i = 0; i < (long) nibbles; i++)
        {
            while (*sp < '0' || (*sp > '9' && *sp < 'a') || *sp > 'f')
            {
                if (*sp == '\0')
                {
#ifdef DEBUG
                    std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: ran out of data\n";
#endif
                    return DataBuf();
                }

                sp++;
            }

            if (i%2 == 0)
                *dp = (unsigned char) (16*unhex[(int) *sp++]);
            else
                (*dp++) += unhex[(int) *sp++];
        }

        return info;

    } // PngChunk::readRawProfile

    DataBuf PngChunk::writeRawProfile(const DataBuf& profile_data, const DataBuf& profile_type)
    {
        register long  i;

        char          *sp=0;
        char          *dp=0;
        char          *text=0;

        unsigned int   allocated_length, description_length, text_length;

        unsigned char  hex[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

        DataBuf        formatedData;

        description_length = profile_type.size_;
        allocated_length   = profile_data.size_*2 + (profile_data.size_ >> 5) + 20 + description_length;

        text = new char[allocated_length];

        sp = (char*)profile_data.pData_;
        dp = text;
        *dp++='\n';

        copyString(dp, (const char *)profile_type.pData_, allocated_length);

        dp += description_length;
        *dp++='\n';

        formatString(dp, allocated_length - strlen(text), "%8lu ", profile_data.size_);

        dp += 8;

        for (i=0; i < (long)profile_data.size_; i++)
        {
            if (i%36 == 0)
                *dp++='\n';

            *(dp++)=(char) hex[((*sp >> 4) & 0x0f)];
            *(dp++)=(char) hex[((*sp++)    & 0x0f)];
        }

        *dp++='\n';
        *dp='\0';

        text_length = (unsigned int)(dp-text);

        if (text_length <= allocated_length)
        {
            formatedData.alloc(text_length);
            memcpy(formatedData.pData_, text, text_length);
        }

        delete [] text;
        return formatedData;

    } // PngChunk::writeRawProfile

    size_t PngChunk::copyString(char* destination,
                                const char* source,
                                const size_t length)
    {
        register char       *q;

        register const char *p;

        register size_t      i;

        if ( !destination || !source || length == 0 )
            return 0;

        p = source;
        q = destination;
        i = length;

        if ((i != 0) && (--i != 0))
        {
            do
            {
                if ((*q++=(*p++)) == '\0')
                    break;
            }
            while (--i != 0);
        }

        if (i == 0)
        {
            if (length != 0)
                *q='\0';

            do
            {
            }
            while (*p++ != '\0');
        }

        return((size_t) (p-source-1));

    } // PngChunk::copyString

    long PngChunk::formatString(char*        string,
                                const size_t length,
                                const char*  format,
                                ...)
    {
        long n;

        va_list operands;

        va_start(operands,format);
        n = (long) formatStringList(string, length, format, operands);
        va_end(operands);
        return(n);

    } // PngChunk::formatString

    long PngChunk::formatStringList(char*        string,
                                    const size_t length,
                                    const char*  format,
                                    va_list      operands)
    {
        int n = vsnprintf(string, length, format, operands);

        if (n < 0)
            string[length-1] = '\0';

        return((long) n);

    } // PngChunk::formatStringList

}}                                      // namespace Internal, Exiv2
#endif // ifdef EXV_HAVE_LIBZ
