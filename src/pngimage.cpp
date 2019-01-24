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
  File:    pngimage.cpp
 */
// *****************************************************************************
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
#include "types.hpp"

// + standard includes
#include <string>
#include <iterator>
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

namespace
{
    inline bool compare(const char* str, const Exiv2::DataBuf& buf, size_t length)
    {
        // str & length should compile time constants => only running this in DEBUG mode is ok
        assert(strlen(str) <= length);
        return memcmp(str, buf.pData_, std::min(static_cast<long>(length), buf.size_)) == 0;
    }
}  // namespace

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
            // if result buffer is large than necessary, redo to fit perfectly.
            if (zlibResult == Z_OK && (long) uncompressedLen < result.size_ ) {
                result.free();

                result.alloc(uncompressedLen);
                zlibResult = uncompress((Bytef*)result.pData_,&uncompressedLen,bytes,length);
            }
            if (zlibResult == Z_BUF_ERROR) {
                // the uncompressed buffer needs to be larger
                result.free();

                // Sanity - never bigger than 16mb
                if  (uncompressedLen > 16*1024*1024) zlibResult = Z_DATA_ERROR;
                else uncompressedLen *= 2;
            }
        } while (zlibResult == Z_BUF_ERROR);

        return zlibResult == Z_OK ;
    }

    static bool zlibToCompressed(const byte* bytes,long length, DataBuf& result)
    {
        uLongf compressedLen = length; // just a starting point
        int    zlibResult;

        do {
            result.alloc(compressedLen);
            zlibResult = compress((Bytef*)result.pData_,&compressedLen,bytes,length);
            if (zlibResult == Z_BUF_ERROR) {
                // the compressedArray needs to be larger
                result.free();
                compressedLen *= 2;
            } else {
                result.free();
                result.alloc(compressedLen);
                zlibResult = compress((Bytef*)result.pData_,&compressedLen,bytes,length);
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
        // count: number of \n in the header
        long        count=0;
        // p points to the current position in the array bytes
        const byte* p = bytes ;

        // header is '\nsomething\n number\n hex'
        // => increment p until it points to the byte after the last \n
        //    p must stay within bounds of the bytes array!
        while ((count < 3) && (p - bytes < length)) {
            // length is later used for range checks of p => decrement it for each increment of p
            --length;
            if ( *p++ == '\n' ) {
                count++;
            }
        }
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

    std::string upper(const std::string& str)
    {
        std::string result;
        transform(str.begin(), str.end(), std::back_inserter(result), toupper);
        return result;
    }

    std::string::size_type findi(const std::string& str, const std::string& substr)
    {
        return upper(str).find(upper(substr) );
    }

    void PngImage::printStructure(std::ostream& out, PrintStructureOption option, int depth)
    {
        if (io_->open() != 0) {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        if (!isPngType(*io_, true)) {
            throw Error(kerNotAnImage, "PNG");
        }

        char    chType[5];
        chType[0]=0;
        chType[4]=0;

        if ( option == kpsBasic || option == kpsXMP || option == kpsIccProfile || option == kpsRecursive ) {

            const std::string xmpKey  = "XML:com.adobe.xmp";
            const std::string exifKey = "Raw profile type exif";
            const std::string app1Key = "Raw profile type APP1";
            const std::string iptcKey = "Raw profile type iptc";
            const std::string iccKey  = "icc";
            const std::string softKey = "Software";
            const std::string commKey = "Comment";
            const std::string descKey = "Description";

            bool bPrint = option == kpsBasic || option == kpsRecursive ;
            if ( bPrint ) {
                out << "STRUCTURE OF PNG FILE: " << io_->path() << std::endl;
                out << " address | chunk |  length | data                           | checksum" << std::endl;
            }

            const long imgSize = (long) io_->size();
            DataBuf    cheaderBuf(8);

            while( !io_->eof() && ::strcmp(chType,"IEND") ) {
                size_t address = io_->tell();

                std::memset(cheaderBuf.pData_, 0x0, cheaderBuf.size_);
                long bufRead = io_->read(cheaderBuf.pData_, cheaderBuf.size_);
                if (io_->error()) throw Error(kerFailedToReadImageData);
                if (bufRead != cheaderBuf.size_) throw Error(kerInputDataReadFailed);

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
                    throw Exiv2::Error(kerFailedToReadImageData);
                }

                DataBuf   buff(dataOffset);
                io_->read(buff.pData_,dataOffset);
                io_->seek(restore, BasicIo::beg);

                // format output
                const int    iMax = 30 ;
                const uint32_t blen = dataOffset > iMax ? iMax : dataOffset ;
                std::string dataString = "";
                // if blen == 0 => slice construction fails
                if (blen > 0) {
                    std::stringstream ss;
                    ss << Internal::binaryToString(makeSlice(buff, 0, blen));
                    dataString = ss.str();
                }
                while (      dataString.size() < iMax ) dataString += ' ';
                dataString = dataString.substr(0,iMax);

                if ( bPrint ) {
                    io_->seek(dataOffset, BasicIo::cur);// jump to checksum
                    byte checksum[4];
                    io_->read(checksum,4);
                    io_->seek(restore, BasicIo::beg)   ;// restore file pointer

                    out << Internal::stringFormat("%8d | %-5s |%8d | "
                                                  ,(uint32_t)address, chType,dataOffset)
                        << dataString
                        << Internal::stringFormat(" | 0x%02x%02x%02x%02x"
                                                   ,checksum[0],checksum[1],checksum[2],checksum[3])
                        << std::endl;
                }

                // chunk type
                bool tEXt  = std::strcmp(chType,"tEXt")== 0;
                bool zTXt  = std::strcmp(chType,"zTXt")== 0;
                bool iCCP  = std::strcmp(chType,"iCCP")== 0;
                bool iTXt  = std::strcmp(chType,"iTXt")== 0;

                // for XMP, ICC etc: read and format data
                bool bXMP  = option == kpsXMP        && findi(dataString,xmpKey)==0;
                bool bICC  = option == kpsIccProfile && findi(dataString,iccKey)==0;
                bool bExif = option == kpsRecursive  &&(findi(dataString,exifKey)==0 || findi(dataString,app1Key)==0);
                bool bIptc = option == kpsRecursive  && findi(dataString,iptcKey)==0;
                bool bSoft = option == kpsRecursive  && findi(dataString,softKey)==0;
                bool bComm = option == kpsRecursive  && findi(dataString,commKey)==0;
                bool bDesc = option == kpsRecursive  && findi(dataString,descKey)==0;
                bool bDump = bXMP || bICC || bExif || bIptc || bSoft || bComm || bDesc ;

                if( bDump ) {
                    DataBuf   dataBuf;
                    byte*     data   = new byte[dataOffset+1];
                    data[dataOffset] = 0;
                    io_->read(data,dataOffset);
                    io_->seek(restore, BasicIo::beg);
                    uint32_t  name_l = (uint32_t) std::strlen((const char*)data)+1; // leading string length
                    uint32_t  start  = name_l;
                    bool      bLF    = false;

                    // decode the chunk
                    bool bGood = false;
                    if ( tEXt ) {
                        bGood = tEXtToDataBuf(data+name_l,dataOffset-name_l,dataBuf);
                    }
                    if ( zTXt || iCCP ) {
                        bGood = zlibToDataBuf(data+name_l+1,dataOffset-name_l-1,dataBuf); // +1 = 'compressed' flag
                    }
                    if ( iTXt ) {
                        bGood = (start+3) < dataOffset ;    // good if not a nul chunk
                    }

                    // format is content dependent
                    if ( bGood ) {
                        if ( bXMP ) {
                            while ( !data[start] && start < dataOffset) start++; // skip leading nul bytes
                            out <<  data+start;             // output the xmp
                        }

                        if ( bExif || bIptc ) {
                            DataBuf parsedBuf = PngChunk::readRawProfile(dataBuf,tEXt);
#if DEBUG
                            std::cerr << Exiv2::Internal::binaryToString(parsedBuf.pData_, parsedBuf.size_>50?50:parsedBuf.size_,0) << std::endl;
#endif
                            if ( parsedBuf.size_ ) {
                                if ( bExif ) {
                                    // create memio object with the data, then print the structure
                                    BasicIo::AutoPtr p = BasicIo::AutoPtr(new MemIo(parsedBuf.pData_+6,parsedBuf.size_-6));
                                    printTiffStructure(*p,out,option,depth);
                                }
                                if ( bIptc ) {
                                    IptcData::printStructure(out, makeSlice(parsedBuf.pData_, 0, parsedBuf.size_), depth);
                                }
                            }
                        }

                        if ( bSoft && dataBuf.size_ > 0) {
                            DataBuf     s(dataBuf.size_+1);               // allocate buffer with an extra byte
                            memcpy(s.pData_,dataBuf.pData_,dataBuf.size_);// copy in the dataBuf
                            s.pData_[dataBuf.size_] = 0 ;                 // nul terminate it
                            const char* str = (const char*) s.pData_;     // give it name
                            out << Internal::indent(depth) << (const char*) buff.pData_ << ": " << str ;
                            bLF=true;
                        }

                        if ( bICC || bComm ) {
                            out.write((const char*) dataBuf.pData_,dataBuf.size_);
                            bLF = bComm ;
                        }

                        if ( bDesc && iTXt ) {
                            DataBuf decoded = PngChunk::decodeTXTChunk(buff,PngChunk::iTXt_Chunk );
                            out.write((const char*)decoded.pData_,decoded.size_);
                            bLF = true;
                        }

                        if ( bLF ) out << std::endl;
                    }
                    delete[] data;
                }
                io_->seek(dataOffset+4, BasicIo::cur);// jump past checksum
                if (io_->error()) throw Error(kerFailedToReadImageData);
            }
        }
    }

    void readChunk(DataBuf& buffer, BasicIo& io)
    {
#ifdef DEBUG
        std::cout << "Exiv2::PngImage::readMetadata: Position: " << io.tell() << std::endl;
#endif
        long bufRead = io.read(buffer.pData_, buffer.size_);
        if (io.error()) {
            throw Error(kerFailedToReadImageData);
        }
        if (bufRead != buffer.size_) {
            throw Error(kerInputDataReadFailed);
        }
    }

    void PngImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Exiv2::PngImage::readMetadata: Reading PNG file " << io_->path() << std::endl;
#endif
        if (io_->open() != 0)
        {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        if (!isPngType(*io_, true)) {
            throw Error(kerNotAnImage, "PNG");
        }
        clearMetadata();

        const long imgSize = (long) io_->size();
        DataBuf cheaderBuf(8);       // Chunk header: 4 bytes (data size) + 4 bytes (chunk type).

        while(!io_->eof())
        {
            std::memset(cheaderBuf.pData_, 0x0, cheaderBuf.size_);
            readChunk(cheaderBuf, *io_); // Read chunk header.

            // Decode chunk data length.
            uint32_t chunkLength = Exiv2::getULong(cheaderBuf.pData_, Exiv2::bigEndian);
            long pos = io_->tell();
            if (pos == -1 ||
                chunkLength > uint32_t(0x7FFFFFFF) ||
                static_cast<long>(chunkLength) > imgSize - pos) {
                throw Exiv2::Error(kerFailedToReadImageData);
            }

            std::string chunkType(reinterpret_cast<char *>(cheaderBuf.pData_) + 4, 4);
#ifdef DEBUG
            std::cout << "Exiv2::PngImage::readMetadata: chunk type: " << chunkType
                      << " length: " << chunkLength << std::endl;
#endif

            /// \todo analyse remaining chunks of the standard
            // Perform a chunk triage for item that we need.
            if (chunkType == "IEND" || chunkType == "IHDR" || chunkType == "tEXt" || chunkType == "zTXt" ||
                chunkType == "iTXt" || chunkType == "iCCP") {
                DataBuf chunkData(chunkLength);
                readChunk(chunkData, *io_);  // Extract chunk data.

                if (chunkType == "IEND") {
                    return;  // Last chunk found: we stop parsing.
                } else if (chunkType == "IHDR" && chunkData.size_ >= 8) {
                    PngChunk::decodeIHDRChunk(chunkData, &pixelWidth_, &pixelHeight_);
                } else if (chunkType == "tEXt") {
                    PngChunk::decodeTXTChunk(this, chunkData, PngChunk::tEXt_Chunk);
                } else if (chunkType == "zTXt") {
                    PngChunk::decodeTXTChunk(this, chunkData, PngChunk::zTXt_Chunk);
                } else if (chunkType == "iTXt") {
                    PngChunk::decodeTXTChunk(this, chunkData, PngChunk::iTXt_Chunk);
                } else if (chunkType == "iCCP") {
                    // The ICC profile name can vary from 1-79 characters.
                    uint32_t iccOffset = 0;
                    while (iccOffset < 80 && iccOffset < chunkLength) {
                         if (chunkData.pData_[iccOffset++] ==  0x00) {
                            break;
                         }
                    }
                    profileName_ = std::string(reinterpret_cast<char *>(chunkData.pData_), iccOffset-1);
                    ++iccOffset; // +1 = 'compressed' flag

                    zlibToDataBuf(chunkData.pData_ + iccOffset, chunkLength - iccOffset, iccProfile_);
#ifdef DEBUG
                    std::cout << "Exiv2::PngImage::readMetadata: profile name: " << profileName_ << std::endl;
                    std::cout << "Exiv2::PngImage::readMetadata: iccProfile.size_ (uncompressed) : "
                              << iccProfile_.size_ << std::endl;
#endif
                }

                // Set chunkLength to 0 in case we have read a supported chunk type. Otherwise, we need to seek the
                // file to the next chunk position.
                chunkLength = 0;
            }


            // Move to the next chunk: chunk data size + 4 CRC bytes.
#ifdef DEBUG
            std::cout << "Exiv2::PngImage::readMetadata: Seek to offset: " << chunkLength + 4 << std::endl;
#endif
            io_->seek(chunkLength + 4 , BasicIo::cur);
            if (io_->error() || io_->eof()) {
                throw Error(kerFailedToReadImageData);
            }
        }
    } // PngImage::readMetadata

    void PngImage::writeMetadata()
    {
        if (io_->open() != 0)
        {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        BasicIo::AutoPtr tempIo(new MemIo);
        assert (tempIo.get() != 0);

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw

    } // PngImage::writeMetadata

    void PngImage::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(kerInputDataReadFailed);
        if (!outIo.isopen()) throw Error(kerImageWriteFailed);

#ifdef DEBUG
        std::cout << "Exiv2::PngImage::doWriteMetadata: Writing PNG file " << io_->path() << "\n";
        std::cout << "Exiv2::PngImage::doWriteMetadata: tmp file created " << outIo.path() << "\n";
#endif

        if (!isPngType(*io_, true)) {
            throw Error(kerNoImageInInputData);
        }

        // Write PNG Signature.
        if (outIo.write(pngSignature, 8) != 8) throw Error(kerImageWriteFailed);

        DataBuf cheaderBuf(8);       // Chunk header : 4 bytes (data size) + 4 bytes (chunk type).

        while(!io_->eof())
        {
            // Read chunk header.

            std::memset(cheaderBuf.pData_, 0x00, cheaderBuf.size_);
            long bufRead = io_->read(cheaderBuf.pData_, cheaderBuf.size_);
            if (io_->error()) throw Error(kerFailedToReadImageData);
            if (bufRead != cheaderBuf.size_) throw Error(kerInputDataReadFailed);

            // Decode chunk data length.

            uint32_t dataOffset = getULong(cheaderBuf.pData_, bigEndian);
            if (dataOffset > 0x7FFFFFFF) throw Exiv2::Error(kerFailedToReadImageData);

            // Read whole chunk : Chunk header + Chunk data (not fixed size - can be null) + CRC (4 bytes).

            DataBuf chunkBuf(8 + dataOffset + 4);                     // Chunk header (8 bytes) + Chunk data + CRC (4 bytes).
            memcpy(chunkBuf.pData_, cheaderBuf.pData_, 8);            // Copy header.
            bufRead = io_->read(chunkBuf.pData_ + 8, dataOffset + 4); // Extract chunk data + CRC
            if (io_->error()) throw Error(kerFailedToReadImageData);
            if (bufRead != (long)(dataOffset + 4)) throw Error(kerInputDataReadFailed);

            char szChunk[5];
            memcpy(szChunk,cheaderBuf.pData_ + 4,4);
            szChunk[4]  = 0;

            if (!memcmp(cheaderBuf.pData_ + 4, "IEND", 4))
            {
                // Last chunk found: we write it and done.
#ifdef DEBUG
                std::cout << "Exiv2::PngImage::doWriteMetadata: Write IEND chunk (length: " << dataOffset << ")\n";
#endif
                if (outIo.write(chunkBuf.pData_, chunkBuf.size_) != chunkBuf.size_) throw Error(kerImageWriteFailed);
                return;
            }
            else if (!memcmp(cheaderBuf.pData_ + 4, "IHDR", 4))
            {
#ifdef DEBUG
                std::cout << "Exiv2::PngImage::doWriteMetadata: Write IHDR chunk (length: " << dataOffset << ")\n";
#endif
                if (outIo.write(chunkBuf.pData_, chunkBuf.size_) != chunkBuf.size_) throw Error(kerImageWriteFailed);

                // Write all updated metadata here, just after IHDR.
                if (!comment_.empty())
                {
                    // Update Comment data to a new PNG chunk
                    std::string chunk = PngChunk::makeMetadataChunk(comment_, mdComment);
                    if (outIo.write((const byte*)chunk.data(), static_cast<long>(chunk.size())) != (long)chunk.size())
                    {
                        throw Error(kerImageWriteFailed);
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
                            throw Error(kerImageWriteFailed);
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
                            throw Error(kerImageWriteFailed);
                        }
                    }
                }

                if ( iccProfileDefined() ) {
                    DataBuf compressed;
                    if ( zlibToCompressed(iccProfile_.pData_,iccProfile_.size_,compressed) ) {

                        const byte* nullComp = (const byte*) "\0\0";
                        const byte*  type    = (const byte*) "iCCP";
                        const long   nameLength  = profileName_.size();
                        const uint32_t chunkLength = profileName_.size() + 2 + compressed.size_ ;
                        byte     length[4];
                        ul2Data (length,chunkLength,bigEndian);

                        // calculate CRC
                        uLong   tmp = crc32(0L, Z_NULL, 0);
                        tmp         = crc32(tmp, (const Bytef*)type, 4);
                        tmp         = crc32(tmp, (const Bytef*)profileName_.data(), nameLength);
                        tmp         = crc32(tmp, (const Bytef*)nullComp, 2);
                        tmp         = crc32(tmp, (const Bytef*)compressed.pData_,compressed.size_);
                        byte    crc[4];
                        ul2Data(crc, tmp, bigEndian);

                        if( outIo.write(length, 4) != 4
                        ||  outIo.write(type, 4) != 4
                        ||  outIo.write(reinterpret_cast<const byte*>(profileName_.data()), nameLength) != nameLength
                        ||  outIo.write(nullComp,2) != 2
                        ||  outIo.write (compressed.pData_,compressed.size_) != compressed.size_
                        ||  outIo.write(crc,4)            != 4
                        ){
                            throw Error(kerImageWriteFailed);
                        }
#ifdef DEBUG
                        std::cout << "Exiv2::PngImage::doWriteMetadata: build iCCP"
                        << " chunk (length: " << compressed.size_ + chunkLength << ")" << std::endl;
#endif
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
                        throw Error(kerImageWriteFailed);
                    }
                }
            }
            else if (!memcmp(cheaderBuf.pData_ + 4, "tEXt", 4) ||
                     !memcmp(cheaderBuf.pData_ + 4, "zTXt", 4) ||
                     !memcmp(cheaderBuf.pData_ + 4, "iTXt", 4) ||
                     !memcmp(cheaderBuf.pData_ + 4, "iCCP", 4))
            {
                DataBuf key = PngChunk::keyTXTChunk(chunkBuf, true);
                if (compare("Raw profile type exif", key, 21) ||
                    compare("Raw profile type APP1", key, 21) ||
                    compare("Raw profile type iptc", key, 21) ||
                    compare("Raw profile type xmp",  key, 20) ||
                    compare("XML:com.adobe.xmp",     key, 17) ||
                    compare("icc",                   key,  3) || // see test/data/imagemagick.png
                    compare("ICC",                   key,  3) ||
                    compare("Description",           key, 11))
                {
#ifdef DEBUG
                    std::cout << "Exiv2::PngImage::doWriteMetadata: strip " << szChunk
                              << " chunk (length: " << dataOffset << ")" << std::endl;
#endif
                }
                else
                {
#ifdef DEBUG
                    std::cout << "Exiv2::PngImage::doWriteMetadata: write " << szChunk
                              << " chunk (length: " << dataOffset << ")" << std::endl;
#endif
                    if (outIo.write(chunkBuf.pData_, chunkBuf.size_) != chunkBuf.size_) throw Error(kerImageWriteFailed);
                }
            }
            else
            {
                // Write all others chunk as well.
#ifdef DEBUG
                std::cout << "Exiv2::PngImage::doWriteMetadata:  copy " << szChunk
                          << " chunk (length: " << dataOffset << ")" << std::endl;
#endif
                if (outIo.write(chunkBuf.pData_, chunkBuf.size_) != chunkBuf.size_) throw Error(kerImageWriteFailed);

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
        if (iIo.error() || iIo.eof()) {
            throw Error(kerInputDataReadFailed);
        }
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
