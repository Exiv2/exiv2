// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
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
  File:    pngimage.cpp
  Version: $Rev$
  Author(s): Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
  History: 12-Jun-06, gc: submitted
  Credits: See header file
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id$")

// included header files
#include "config.h"

#ifdef   EXV_HAVE_LIBZ
#include "pngchunk_int.hpp"
#include "pngimage.hpp"
#include "jpgimage.hpp"
#include "tiffimage.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>

#include <zlib.h>     // To uncompress IccProfiles

// Signature from front of PNG file
const unsigned char pngSignature[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

const unsigned char pngBlank[] = { 0x89,0x50,0x4e,0x47,0x0d,0x0a,0x1a,0x0a,0x00,0x00,0x00,0x0d,0x49,0x48,0x44,0x52,
                                   0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x08,0x02,0x00,0x00,0x00,0x90,0x77,0x53,
                                   0xde,0x00,0x00,0x00,0x01,0x73,0x52,0x47,0x42,0x00,0xae,0xce,0x1c,0xe9,0x00,0x00,
                                   0x00,0x09,0x70,0x48,0x59,0x73,0x00,0x00,0x0b,0x13,0x00,0x00,0x0b,0x13,0x01,0x00,
                                   0x9a,0x9c,0x18,0x00,0x00,0x00,0x0c,0x49,0x44,0x41,0x54,0x08,0xd7,0x63,0xf8,0xff,
                                   0xff,0x3f,0x00,0x05,0xfe,0x02,0xfe,0xdc,0xcc,0x59,0xe7,0x00,0x00,0x00,0x00,0x49,
                                   0x45,0x4e,0x44,0xae,0x42,0x60,0x82
                                 };

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    PngImage::PngImage(BasicIo::AutoPtr io, bool create)
            : Image(ImageType::png, mdExif | mdIptc | mdXmp | mdComment, io)
    {
        if (create)
        {
            if (io_->open() == 0)
            {
#ifdef DEBUG
                std::cerr << "Exiv2::PngImage:: Creating PNG image to memory\n";
#endif
                IoCloser closer(*io_);
                if (io_->write(pngBlank, sizeof(pngBlank)) != sizeof(pngBlank))
                {
#ifdef DEBUG
                    std::cerr << "Exiv2::PngImage:: Failed to create PNG image on memory\n";
#endif
                }
            }
        }
    } // PngImage::PngImage

    std::string PngImage::mimeType() const
    {
        return "image/png";
    }

    static bool zlibToDataBuf(const byte* bytes,long length, DataBuf& result)
    {
        uLongf uncompressedLen = length * 2; // just a starting point
        int    zlibResult;

        do {
            result.alloc(uncompressedLen);
            zlibResult = uncompress((Bytef*)result.pData_,&uncompressedLen,bytes,length);
            if (zlibResult == Z_BUF_ERROR) {
                // the uncompressedArray needs to be larger
                result.release();

                // never bigger than 64k
                if  (uncompressedLen > 64*1024) zlibResult = Z_DATA_ERROR;
                uncompressedLen *= 2;
            }
        } while (zlibResult == Z_BUF_ERROR);

        return zlibResult == Z_OK ;
    }

    static bool tEXtToDataBuf(const byte* bytes,long length,DataBuf& result)
    {
        static const char* hexdigits = "0123456789ABCDEF";
        static int         value   [256] ;
        static bool        bFirst = true ;
        if ( bFirst ) {
            for ( int i = 0 ; i < 256 ; i++ )
                value[i] = 0;
            for ( int i = 0 ; i < 16 ; i++ ) {
                value[tolower(hexdigits[i])]=i+1;
                value[toupper(hexdigits[i])]=i+1;
            }
            bFirst = false;
        }

        // calculate length and allocate result;
        long        count=0;
        const byte* p = bytes ;
        // header is \nsomething\n number\n hex
        while ( count < 3 )
            if ( *p++ == '\n' )
                count++;
        for ( long i = 0 ; i < length ; i++ )
            if ( value[p[i]] )
                ++count;
        result.alloc((count+1)/2) ;

        // hex to binary
        count   = 0 ;
        byte* r = result.pData_;
        int   n = 0 ; // nibble
        for ( long i = 0 ; i < length ; i++ ) {
            if ( value[p[i]] ) {
                int v = value[p[i]]-1 ;
                if ( ++count % 2 ) n = v*16 ; // leading digit
                else *r++ =        n + v    ; // trailing
            }
        }
        return true;
    }

    static std::string indent(int depth)
    {
        std::string result;
        while ( depth -- ) result += "  ";
        return result;
    }

    void PngImage::printStructure(std::ostream& out, PrintStructureOption option, int depth)
    {
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isPngType(*io_, true)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "PNG");
        }

        char    chType[5];
        chType[0]=0;
        chType[4]=0;

        if ( option == kpsBasic || option == kpsXMP || option == kpsIccProfile || option == kpsRecursive ) {

            const std::string xmpKey  = "XML:com.adobe.xmp";
            const std::string exifKey = "Raw profile type exif";
            const std::string iptcKey = "Raw profile type iptc";
            const std::string iccKey  = "icc";
            const std::string softKey = "Software";

            if ( option == kpsBasic || option == kpsRecursive ) {
                out << "STRUCTURE OF PNG FILE: " << io_->path() << std::endl;
                out << " address | index | chunk_type |  length | data" << std::endl;
            }

            long       index   = 0;
            const long imgSize = io_->size();
            DataBuf    cheaderBuf(8);

            while( !io_->eof() && ::strcmp(chType,"IEND") ) {
                size_t address = io_->tell();

                std::memset(cheaderBuf.pData_, 0x0, cheaderBuf.size_);
                long bufRead = io_->read(cheaderBuf.pData_, cheaderBuf.size_);
                if (io_->error()) throw Error(14);
                if (bufRead != cheaderBuf.size_) throw Error(20);

                // Decode chunk data length.
                uint32_t dataOffset = Exiv2::getULong(cheaderBuf.pData_, Exiv2::bigEndian);
                for (int i = 4; i < 8; i++) {
                    chType[i-4]=cheaderBuf.pData_[i];
                }

                // test that we haven't hit EOF, or wanting to read excessive data
                long restore = io_->tell();
                if(  restore == -1
                ||  dataOffset > uint32_t(0x7FFFFFFF)
                ||  static_cast<long>(dataOffset) > imgSize - restore
                ){
                    throw Exiv2::Error(14);
                }

                // format output
                uint32_t    blen = dataOffset > 32 ? 32 : dataOffset ;
                std::string dataString ;
                DataBuf buff(blen);
                io_->read(buff.pData_,blen);
                io_->seek(restore, BasicIo::beg);
                dataString  = Internal::binaryToString(buff, blen);

                if ( option == kpsBasic || option == kpsRecursive )
                    out << Internal::stringFormat("%8d | %5d | %10s |%8d | "
                              ,(uint32_t)address, index++,chType,dataOffset)
                                    << dataString << std::endl;


                // chunk type
                bool tEXt  = std::strcmp(chType,"tEXt")== 0;
                bool zTXt  = std::strcmp(chType,"zTXt")== 0;
                bool iCCP  = std::strcmp(chType,"iCCP")== 0;
                bool iTXt  = std::strcmp(chType,"iTXt")== 0;

                // for XMP, ICC etc: read and format data
                bool bXMP  = option == kpsXMP        && dataString.find(xmpKey)==0;
                bool bICC  = option == kpsIccProfile && dataString.find(iccKey)==0;
                bool bExif = option == kpsRecursive  && dataString.find(exifKey)==0;
                bool bIptc = option == kpsRecursive  && dataString.find(iptcKey)==0;
                bool bSoft = option == kpsRecursive  && dataString.find(softKey)==0;
                bool bDump = bXMP || bICC || bExif || bIptc || bSoft ;

                if( bDump ) {
                    DataBuf   dataBuf;
                    byte*     data   = new byte[dataOffset];
                    io_->read(data,dataOffset);
                    io_->seek(restore, BasicIo::beg);
                    size_t     name_l = std::strlen((const char*)data)+1; // leading string length
                    size_t     start  = name_l;

                    // decode the chunk
                    bool bGood = false;
                    if ( tEXt ) {
                        bGood = tEXtToDataBuf(data+name_l,dataOffset-name_l,dataBuf);
                    }
                    if ( zTXt || iCCP ) {
                        name_l++ ; // +1 = 'compressed' flag
                        bGood = zlibToDataBuf(data+name_l,dataOffset-name_l,dataBuf);
                    }
                    if ( iTXt ) {
                        while ( data[start] == 0 && start < dataOffset ) start++; // crawl over the '\0' bytes between XML:....\0\0<xml stuff
                        data[dataOffset]=0;                 // ensure the XML is nul terminated
                        bGood = (start+3) < dataOffset ;    // good if not a nul chunk
                    }

                    // format is content dependent
                    if ( bGood ) {
                        if ( bXMP ) {
                            out <<  data+start;             // output the xml
                        }
                        if ( bExif ) {
                            const char* bytes = (const char*) dataBuf.pData_;
                            long        l     = std::strlen(bytes)+2;
                            // create a copy on write memio object with the data, then print the structure
                            BasicIo::AutoPtr p = BasicIo::AutoPtr(new MemIo(dataBuf.pData_+l,dataBuf.size_-l));
                            TiffImage::printTiffStructure(*p,out,option,depth);
                        }

                        if ( bSoft ) {
                            const char* bytes = (const char*) dataBuf.pData_;
                            long        l     = std::strlen(bytes)+2;
                            // create a copy on write memio object with the data, then print the structure
                            BasicIo::AutoPtr p = BasicIo::AutoPtr(new MemIo(dataBuf.pData_+l,dataBuf.size_-l));
                            out << indent(depth) << (const char*) buff.pData_ << ": " << (const char*) dataBuf.pData_ << std::endl;
                        }

                        if ( bICC ) {
                            out.write((const char*) dataBuf.pData_,dataBuf.size_);
                        }

                        if ( bIptc && bSoft ) { // we require a photoshop parser to recover IPTC data
                            const char* bytes = (const char*) dataBuf.pData_;
                            long        l     = std::strlen(bytes)+2;
                            // create a copy on write memio object with the data, then print the structure
                            BasicIo::AutoPtr p = BasicIo::AutoPtr(new MemIo(dataBuf.pData_+l,dataBuf.size_-l));
                            TiffImage::printTiffStructure(*p,out,option,depth);
                        }
                    }
                    delete [] data;
                }
                io_->seek(dataOffset + 4 , BasicIo::cur);
                if (io_->error()) throw Error(14);
            }
        }
    }

    void PngImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Exiv2::PngImage::readMetadata: Reading PNG file " << io_->path() << "\n";
#endif
        if (io_->open() != 0)
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isPngType(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "PNG");
        }
        clearMetadata();

        const long imgSize = io_->size();
        DataBuf cheaderBuf(8);       // Chunk header size : 4 bytes (data size) + 4 bytes (chunk type).

        while(!io_->eof())
        {
            // Read chunk header.

#ifdef DEBUG
            std::cout << "Exiv2::PngImage::readMetadata: Position: " << io_->tell() << "\n";
#endif
            std::memset(cheaderBuf.pData_, 0x0, cheaderBuf.size_);
            long bufRead = io_->read(cheaderBuf.pData_, cheaderBuf.size_);
            if (io_->error()) throw Error(14);
            if (bufRead != cheaderBuf.size_) throw Error(20);

            // Decode chunk data length.
            uint32_t dataOffset = Exiv2::getULong(cheaderBuf.pData_, Exiv2::bigEndian);
            long pos = io_->tell();
            if (   pos == -1
                || dataOffset > uint32_t(0x7FFFFFFF)
                || static_cast<long>(dataOffset) > imgSize - pos) throw Exiv2::Error(14);

            // Perform a chunk triage for item that we need.

            if (!memcmp(cheaderBuf.pData_ + 4, "IEND", 4) ||
                !memcmp(cheaderBuf.pData_ + 4, "IHDR", 4) ||
                !memcmp(cheaderBuf.pData_ + 4, "tEXt", 4) ||
                !memcmp(cheaderBuf.pData_ + 4, "zTXt", 4) ||
                !memcmp(cheaderBuf.pData_ + 4, "iTXt", 4))
            {
                // Extract chunk data.

                DataBuf cdataBuf(dataOffset);
                bufRead = io_->read(cdataBuf.pData_, dataOffset);
                if (io_->error()) throw Error(14);
                if (bufRead != (long)dataOffset) throw Error(20);

                if (!memcmp(cheaderBuf.pData_ + 4, "IEND", 4))
                {
                    // Last chunk found: we stop parsing.
#ifdef DEBUG
                    std::cout << "Exiv2::PngImage::readMetadata: Found IEND chunk (length: " << dataOffset << ")\n";
#endif
                    return;
                }
                else if (!memcmp(cheaderBuf.pData_ + 4, "IHDR", 4))
                {
#ifdef DEBUG
                    std::cout << "Exiv2::PngImage::readMetadata: Found IHDR chunk (length: " << dataOffset << ")\n";
#endif
                    PngChunk::decodeIHDRChunk(cdataBuf, &pixelWidth_, &pixelHeight_);
                }
                else if (!memcmp(cheaderBuf.pData_ + 4, "tEXt", 4))
                {
#ifdef DEBUG
                    std::cout << "Exiv2::PngImage::readMetadata: Found tEXt chunk (length: " << dataOffset << ")\n";
#endif
                    PngChunk::decodeTXTChunk(this, cdataBuf, PngChunk::tEXt_Chunk);
                }
                else if (!memcmp(cheaderBuf.pData_ + 4, "zTXt", 4))
                {
#ifdef DEBUG
                    std::cout << "Exiv2::PngImage::readMetadata: Found zTXt chunk (length: " << dataOffset << ")\n";
#endif
                    PngChunk::decodeTXTChunk(this, cdataBuf, PngChunk::zTXt_Chunk);
                }
                else if (!memcmp(cheaderBuf.pData_ + 4, "iTXt", 4))
                {
#ifdef DEBUG
                    std::cout << "Exiv2::PngImage::readMetadata: Found iTXt chunk (length: " << dataOffset << ")\n";
#endif
                    PngChunk::decodeTXTChunk(this, cdataBuf, PngChunk::iTXt_Chunk);
                }

                // Set dataOffset to null like chunk data have been extracted previously.
                dataOffset = 0;
            }

            // Move to the next chunk: chunk data size + 4 CRC bytes.
#ifdef DEBUG
            std::cout << "Exiv2::PngImage::readMetadata: Seek to offset: " << dataOffset + 4 << "\n";
#endif
            io_->seek(dataOffset + 4 , BasicIo::cur);
            if (io_->error() || io_->eof()) throw Error(14);
        }

    } // PngImage::readMetadata

    void PngImage::writeMetadata()
    {
        if (io_->open() != 0)
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        BasicIo::AutoPtr tempIo(io_->temporary()); // may throw
        assert (tempIo.get() != 0);

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw

    } // PngImage::writeMetadata

    void PngImage::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(20);
        if (!outIo.isopen()) throw Error(21);

#ifdef DEBUG
        std::cout << "Exiv2::PngImage::doWriteMetadata: Writing PNG file " << io_->path() << "\n";
        std::cout << "Exiv2::PngImage::doWriteMetadata: tmp file created " << outIo.path() << "\n";
#endif

        // Ensure that this is the correct image type
        if (!isPngType(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(20);
            throw Error(22);
        }

        // Write PNG Signature.
        if (outIo.write(pngSignature, 8) != 8) throw Error(21);

        DataBuf cheaderBuf(8);       // Chunk header : 4 bytes (data size) + 4 bytes (chunk type).

        while(!io_->eof())
        {
            // Read chunk header.

            std::memset(cheaderBuf.pData_, 0x00, cheaderBuf.size_);
            long bufRead = io_->read(cheaderBuf.pData_, cheaderBuf.size_);
            if (io_->error()) throw Error(14);
            if (bufRead != cheaderBuf.size_) throw Error(20);

            // Decode chunk data length.

            uint32_t dataOffset = getULong(cheaderBuf.pData_, bigEndian);
            if (dataOffset > 0x7FFFFFFF) throw Exiv2::Error(14);

            // Read whole chunk : Chunk header + Chunk data (not fixed size - can be null) + CRC (4 bytes).

            DataBuf chunkBuf(8 + dataOffset + 4);                     // Chunk header (8 bytes) + Chunk data + CRC (4 bytes).
            memcpy(chunkBuf.pData_, cheaderBuf.pData_, 8);            // Copy header.
            bufRead = io_->read(chunkBuf.pData_ + 8, dataOffset + 4); // Extract chunk data + CRC
            if (io_->error()) throw Error(14);
            if (bufRead != (long)(dataOffset + 4)) throw Error(20);

            if (!memcmp(cheaderBuf.pData_ + 4, "IEND", 4))
            {
                // Last chunk found: we write it and done.
#ifdef DEBUG
                std::cout << "Exiv2::PngImage::doWriteMetadata: Write IEND chunk (length: " << dataOffset << ")\n";
#endif
                if (outIo.write(chunkBuf.pData_, chunkBuf.size_) != chunkBuf.size_) throw Error(21);
                return;
            }
            else if (!memcmp(cheaderBuf.pData_ + 4, "IHDR", 4))
            {
#ifdef DEBUG
                std::cout << "Exiv2::PngImage::doWriteMetadata: Write IHDR chunk (length: " << dataOffset << ")\n";
#endif
                if (outIo.write(chunkBuf.pData_, chunkBuf.size_) != chunkBuf.size_) throw Error(21);

                // Write all updated metadata here, just after IHDR.
                if (!comment_.empty())
                {
                    // Update Comment data to a new PNG chunk
                    std::string chunk = PngChunk::makeMetadataChunk(comment_, mdComment);
                    if (outIo.write((const byte*)chunk.data(), static_cast<long>(chunk.size())) != (long)chunk.size())
                    {
                        throw Error(21);
                    }
                }

                if (exifData_.count() > 0)
                {
                    // Update Exif data to a new PNG chunk
                    Blob blob;
                    ExifParser::encode(blob, littleEndian, exifData_);
                    if (blob.size() > 0)
                    {
                        static const char exifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
                        std::string rawExif =   std::string(exifHeader, 6)
                                              + std::string((const char*)&blob[0], blob.size());
                        std::string chunk = PngChunk::makeMetadataChunk(rawExif, mdExif);
                        if (outIo.write((const byte*)chunk.data(), static_cast<long>(chunk.size())) != (long)chunk.size())
                        {
                            throw Error(21);
                        }
                    }
                }

                if (iptcData_.count() > 0)
                {
                    // Update IPTC data to a new PNG chunk
                    DataBuf newPsData = Photoshop::setIptcIrb(0, 0, iptcData_);
                    if (newPsData.size_ > 0)
                    {
                        std::string rawIptc((const char*)newPsData.pData_, newPsData.size_);
                        std::string chunk = PngChunk::makeMetadataChunk(rawIptc, mdIptc);
                        if (outIo.write((const byte*)chunk.data(), static_cast<long>(chunk.size())) != (long)chunk.size())
                        {
                            throw Error(21);
                        }
                    }
                }

                if (writeXmpFromPacket() == false) {
                    if (XmpParser::encode(xmpPacket_, xmpData_) > 1) {
#ifndef SUPPRESS_WARNINGS
                        EXV_ERROR << "Failed to encode XMP metadata.\n";
#endif
                    }
                }
                if (xmpPacket_.size() > 0) {
                    // Update XMP data to a new PNG chunk
                    std::string chunk = PngChunk::makeMetadataChunk(xmpPacket_, mdXmp);
                    if (outIo.write((const byte*)chunk.data(), static_cast<long>(chunk.size())) != (long)chunk.size()) {
                        throw Error(21);
                    }
                }
            }
            else if (!memcmp(cheaderBuf.pData_ + 4, "tEXt", 4) ||
                     !memcmp(cheaderBuf.pData_ + 4, "zTXt", 4) ||
                     !memcmp(cheaderBuf.pData_ + 4, "iTXt", 4))
            {
                DataBuf key = PngChunk::keyTXTChunk(chunkBuf, true);
                if (memcmp("Raw profile type exif", key.pData_, 21) == 0 ||
                    memcmp("Raw profile type APP1", key.pData_, 21) == 0 ||
                    memcmp("Raw profile type iptc", key.pData_, 21) == 0 ||
                    memcmp("Raw profile type xmp",  key.pData_, 20) == 0 ||
                    memcmp("XML:com.adobe.xmp",     key.pData_, 17) == 0 ||
                    memcmp("Description",           key.pData_, 11) == 0)
                {
#ifdef DEBUG
                    std::cout << "Exiv2::PngImage::doWriteMetadata: strip " << cheaderBuf.pData_ + 4
                              << " chunk (key: " << key.pData_ << ")\n";
#endif
                }
                else
                {
#ifdef DEBUG
                    std::cout << "Exiv2::PngImage::doWriteMetadata: write " << cheaderBuf.pData_ + 4
                              << " chunk (length: " << dataOffset << ")\n";
#endif
                    if (outIo.write(chunkBuf.pData_, chunkBuf.size_) != chunkBuf.size_) throw Error(21);
                }
            }
            else
            {
                // Write all others chunk as well.
#ifdef DEBUG
                std::cout << "Exiv2::PngImage::doWriteMetadata: write " << cheaderBuf.pData_ + 4
                          << " chunk (length: " << dataOffset << ")\n";
#endif
                if (outIo.write(chunkBuf.pData_, chunkBuf.size_) != chunkBuf.size_) throw Error(21);

            }
        }

    } // PngImage::doWriteMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newPngInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new PngImage(io, create));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isPngType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 8;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        int rc = memcmp(buf, pngSignature, 8);
        if (!advance || rc != 0)
        {
            iIo.seek(-len, BasicIo::cur);
        }

        return rc == 0;
    }
}                                       // namespace Exiv2
#endif
