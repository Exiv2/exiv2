// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  Version: $Rev: 823 $
  History: 12-Jun-06, gc: submitted
  Credits: See header file
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id: pngchunk.cpp 823 2006-06-23 07:35:00Z cgilles $")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

//#define DEBUG 1

// some defines to make it easier
#define PNG_CHUNK_TYPE(data, index)  &data[index+4]
#define PNG_CHUNK_DATA(data, index, offset) data[8+index+offset]
#define PNG_CHUNK_HEADER_SIZE 12

// To uncompress text chunck
#include <zlib.h>

#include "pngchunk.hpp"
#include "tiffparser.hpp"
#include "exif.hpp"
#include "iptc.hpp"
#include "image.hpp"
#include "error.hpp"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    void PngChunk::decode(Image*  pImage,
                          const   byte* pData,
                          long    size)
    {
        assert(pImage != 0);
        assert(pData != 0);

        // look for a tEXt chunk
        long index = 8;
        index += getLong(&pData[index], bigEndian) + PNG_CHUNK_HEADER_SIZE;

        while(index < size-PNG_CHUNK_HEADER_SIZE)
        {
            while (index < size-PNG_CHUNK_HEADER_SIZE &&
                   strncmp((char*)PNG_CHUNK_TYPE(pData, index), "tEXt", 4) &&
                   strncmp((char*)PNG_CHUNK_TYPE(pData, index), "zTXt", 4))
            {
                if (!strncmp((char*)PNG_CHUNK_TYPE(pData, index), "IEND", 4))
                    throw Error(14);

                index += getLong(&pData[index], bigEndian) + PNG_CHUNK_HEADER_SIZE;
            }

            if (index < size-PNG_CHUNK_HEADER_SIZE)
            {
                // we found a tEXt or zTXt field

                // get the key, it's a null terminated string at the chunk start
                const byte *key = &PNG_CHUNK_DATA(pData, index, 0);

                int keysize=0;
                for ( ; key[keysize] != 0 ; keysize++) 
                {
                    // look if we reached the end of the file (it might be corrupted)
                    if (8+index+keysize >= size)
                        throw Error(14);
                }

                DataBuf arr;

                if(!strncmp((char*)PNG_CHUNK_TYPE(pData, index), "zTXt", 4)) 
                {
                    // Extract a deflate compressed Latin-1 text chunk

#ifdef DEBUG
                    std::cerr << "Exiv2::PngChunk::decode: We found a zTXt field\n";
#endif
                    // we get the compression method after the key
                    const byte* compressionMethod = &PNG_CHUNK_DATA(pData, index, keysize+1);
                    if ( *compressionMethod != 0x00 ) 
                    {
                        // then it isn't zlib compressed and we are sunk
#ifdef DEBUG
                        std::cerr << "Exiv2::PngChunk::decode: Non-standard compression method.\n";
#endif
                        throw Error(14);
                    }

                    // compressed string after the compression technique spec
                    const byte* compressedText = &PNG_CHUNK_DATA(pData, index, keysize+2);
                    uint compressedTextSize    = getLong(&pData[index], bigEndian)-keysize-2;

                    // security check, also considering overflow wraparound from the addition --
                    // we may endup with a /smaller/ index if we wrap all the way around
                    long firstIndex       = (long)(compressedText - pData);
                    long onePastLastIndex = firstIndex + compressedTextSize;
                    if ( onePastLastIndex > size || onePastLastIndex <= firstIndex)
                        throw Error(14);

                    uLongf uncompressedLen = compressedTextSize * 2; // just a starting point
                    int zlibResult;

                    do
                    {
                        arr.alloc(uncompressedLen);
                        zlibResult = uncompress((Bytef*)arr.pData_, &uncompressedLen,
                                                compressedText, compressedTextSize);

                        if (Z_OK == zlibResult) 
                        {
                            // then it is all OK
                            arr.alloc(uncompressedLen);
                        }
                        else if (Z_BUF_ERROR == zlibResult) 
                        {
                            // the uncompressedArray needs to be larger
#ifdef DEBUG
                            std::cerr << "Exiv2::PngChunk::decode: doubling size for decompression.\n";
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
                    while (Z_BUF_ERROR == zlibResult);

                    if (zlibResult != Z_OK)
                        throw Error(14);
                }
                else if (!strncmp((char*)PNG_CHUNK_TYPE(pData, index), "tEXt", 4)) 
                {
                    // Extract a non-compressed Latin-1 text chunk
#ifdef DEBUG
                    std::cerr << "Exiv2::PngChunk::decode: We found a tEXt field\n";
#endif
                    // the text comes after the key, but isn't null terminated
                    const byte* text = &PNG_CHUNK_DATA(pData, index, keysize+1);
                    long textsize    = getLong(&pData[index], bigEndian)-keysize-1;

                    // security check, also considering overflow wraparound from the addition --
                    // we may endup with a /smaller/ index if we wrap all the way around
                    long firstIndex       = (long)(text - pData);
                    long onePastLastIndex = firstIndex + textsize;

                    if ( onePastLastIndex > size || onePastLastIndex <= firstIndex)
                        throw Error(14);

                    arr.alloc(textsize);
                    arr = DataBuf(text, textsize);
                }
                else
                {
                    // TODO : Add 'iTXt' chunk 'Description' tag support here

#ifdef DEBUG
                    std::cerr << "Exiv2::PngChunk::decode: We found a field, not expected though\n";
#endif
                    throw Error(14);
                }

#ifdef DEBUG
                std::cerr << "Exiv2::PngChunk::decode: Found PNG entry " << std::string((const char*)key) << " / " 
                          << std::string((const char*)arr.pData_, 64) << "\n";
#endif

                // We look if an EXIF raw profile exist.

                if ( memcmp("Raw profile type exif", key, 21) == 0 ||
                     memcmp("Raw profile type APP1", key, 21) == 0 )
                {
                    DataBuf exifData = readRawProfile(arr);
                    long length = exifData.size_;

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
                            std::cerr << "Exiv2::PngChunk::decode: Exif header found at position " << pos << "\n";
#endif
                            pos = pos + sizeof(exifHeader);
                            TiffParser::decode(pImage, exifData.pData_+pos, length-pos, TiffCreator::create);
                        }
                    }
                }

                // We look if an IPTC raw profile exist.

                if ( memcmp("Raw profile type iptc", key, 21) == 0 )
                {
                    DataBuf iptcData = readRawProfile(arr);
                    long length = iptcData.size_;

                    if (length > 0)
                        pImage->iptcData().load(iptcData.pData_, length);
                }

                index += getLong(&pData[index], bigEndian) + PNG_CHUNK_HEADER_SIZE;
            }
        }

    } // PngChunk::decode

    DataBuf PngChunk::readRawProfile(const DataBuf& text)
    {
        DataBuf                 info;
        register long           i;
        register unsigned char *dp;
        const char             *sp;
        uint                    nibbles;
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
            std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: invalid profile length\n";
            return DataBuf();
        }

        info.alloc(length);

        if (info.size_ != length)
        {
            std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: cannot allocate memory\n";
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
                    std::cerr << "Exiv2::PngChunk::readRawProfile: Unable To Copy Raw Profile: ran out of data\n";
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
}                                       // namespace Exiv2
