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
  File:      jp2image.cpp
  Version:   $Rev$
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id$")

// included header files
#include "config.h"

#include "jp2image.hpp"
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

// #define DEBUG

// JPEG-2000 box types
const uint32_t kJp2BoxTypeJp2Header   = 0x6a703268; // 'jp2h'
const uint32_t kJp2BoxTypeImageHeader = 0x69686472; // 'ihdr'
const uint32_t kJp2BoxTypeColorHeader = 0x636f6c72; // 'colr'
const uint32_t kJp2BoxTypeUuid        = 0x75756964; // 'uuid'

// from openjpeg-2.1.2/src/lib/openjp2/jp2.h
/*#define JPIP_JPIP 0x6a706970*/

#define     JP2_JP   0x6a502020    /**< JPEG 2000 signature box */
#define     JP2_FTYP 0x66747970    /**< File type box */
#define     JP2_JP2H 0x6a703268    /**< JP2 header box (super-box) */
#define     JP2_IHDR 0x69686472    /**< Image header box */
#define     JP2_COLR 0x636f6c72    /**< Colour specification box */
#define     JP2_JP2C 0x6a703263    /**< Contiguous codestream box */
#define     JP2_URL  0x75726c20    /**< Data entry URL box */
#define     JP2_PCLR 0x70636c72    /**< Palette box */
#define     JP2_CMAP 0x636d6170    /**< Component Mapping box */
#define     JP2_CDEF 0x63646566    /**< Channel Definition box */
#define     JP2_DTBL 0x6474626c    /**< Data Reference box */
#define     JP2_BPCC 0x62706363    /**< Bits per component box */
#define     JP2_JP2  0x6a703220    /**< File type fields */

/* For the future */
/* #define JP2_RES 0x72657320 */  /**< Resolution box (super-box) */
/* #define JP2_JP2I 0x6a703269 */  /**< Intellectual property box */
/* #define JP2_XML  0x786d6c20 */  /**< XML box */
/* #define JP2_UUID 0x75756994 */  /**< UUID box */
/* #define JP2_UINF 0x75696e66 */  /**< UUID info box (super-box) */
/* #define JP2_ULST 0x756c7374 */  /**< UUID list box */

// JPEG-2000 UUIDs for embedded metadata
//
// See http://www.jpeg.org/public/wg1n2600.doc for information about embedding IPTC-NAA data in JPEG-2000 files
// See http://www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf for information about embedding XMP data in JPEG-2000 files
const unsigned char kJp2UuidExif[] = "JpgTiffExif->JP2";
const unsigned char kJp2UuidIptc[] = "\x33\xc7\xa4\xd2\xb8\x1d\x47\x23\xa0\xba\xf1\xa3\xe0\x97\xad\x38";
const unsigned char kJp2UuidXmp[]  = "\xbe\x7a\xcf\xcb\x97\xa9\x42\xe8\x9c\x71\x99\x94\x91\xe3\xaf\xac";

// See section B.1.1 (JPEG 2000 Signature box) of JPEG-2000 specification
const unsigned char Jp2Signature[12] = { 0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50, 0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a };

const unsigned char Jp2Blank[] = { 0x00,0x00,0x00,0x0c,0x6a,0x50,0x20,0x20,0x0d,0x0a,0x87,0x0a,0x00,0x00,0x00,0x14,
                                   0x66,0x74,0x79,0x70,0x6a,0x70,0x32,0x20,0x00,0x00,0x00,0x00,0x6a,0x70,0x32,0x20,
                                   0x00,0x00,0x00,0x2d,0x6a,0x70,0x32,0x68,0x00,0x00,0x00,0x16,0x69,0x68,0x64,0x72,
                                   0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x01,0x07,0x07,0x00,0x00,0x00,0x00,
                                   0x00,0x0f,0x63,0x6f,0x6c,0x72,0x01,0x00,0x00,0x00,0x00,0x00,0x11,0x00,0x00,0x00,
                                   0x00,0x6a,0x70,0x32,0x63,0xff,0x4f,0xff,0x51,0x00,0x29,0x00,0x00,0x00,0x00,0x00,
                                   0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
                                   0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x07,
                                   0x01,0x01,0xff,0x64,0x00,0x23,0x00,0x01,0x43,0x72,0x65,0x61,0x74,0x6f,0x72,0x3a,
                                   0x20,0x4a,0x61,0x73,0x50,0x65,0x72,0x20,0x56,0x65,0x72,0x73,0x69,0x6f,0x6e,0x20,
                                   0x31,0x2e,0x39,0x30,0x30,0x2e,0x31,0xff,0x52,0x00,0x0c,0x00,0x00,0x00,0x01,0x00,
                                   0x05,0x04,0x04,0x00,0x01,0xff,0x5c,0x00,0x13,0x40,0x40,0x48,0x48,0x50,0x48,0x48,
                                   0x50,0x48,0x48,0x50,0x48,0x48,0x50,0x48,0x48,0x50,0xff,0x90,0x00,0x0a,0x00,0x00,
                                   0x00,0x00,0x00,0x2d,0x00,0x01,0xff,0x5d,0x00,0x14,0x00,0x40,0x40,0x00,0x00,0x00,
                                   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x93,0xcf,0xb4,
                                   0x04,0x00,0x80,0x80,0x80,0x80,0x80,0xff,0xd9
                                 };

//! @cond IGNORE
struct Jp2BoxHeader
{
    uint32_t length;
    uint32_t type;
};

struct Jp2ImageHeaderBox
{
    uint32_t imageHeight;
    uint32_t imageWidth;
    uint16_t componentCount;
    uint8_t  bitsPerComponent;
    uint8_t  compressionType;
    uint8_t  colorspaceIsUnknown;
    uint8_t  intellectualPropertyFlag;
    uint16_t compressionTypeProfile;
};

struct Jp2UuidBox
{
    uint8_t  uuid[16];
};
//! @endcond

// *****************************************************************************
// class member definitions
namespace Exiv2
{

    Jp2Image::Jp2Image(BasicIo::AutoPtr io, bool create)
            : Image(ImageType::jp2, mdExif | mdIptc | mdXmp, io)
    {
        if (create)
        {
            if (io_->open() == 0)
            {
#ifdef DEBUG
                std::cerr << "Exiv2::Jp2Image:: Creating JPEG2000 image to memory\n";
#endif
                IoCloser closer(*io_);
                if (io_->write(Jp2Blank, sizeof(Jp2Blank)) != sizeof(Jp2Blank))
                {
#ifdef DEBUG
                    std::cerr << "Exiv2::Jp2Image:: Failed to create JPEG2000 image on memory\n";
#endif
                }
            }
        }
    } // Jp2Image::Jp2Image

    std::string Jp2Image::mimeType() const
    {
        return "image/jp2";
    }

    void Jp2Image::setComment(const std::string& /*comment*/)
    {
        // Todo: implement me!
        throw(Error(32, "Image comment", "JP2"));
    } // Jp2Image::setComment

    static std::string toAscii(long n)
    {
        const char* p = (const char*) &n;
        std::string result;
        for ( int i = 0 ; i < 4 ; i++) result += p[3-i];
        return result;
    }

    void Jp2Image::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Exiv2::Jp2Image::readMetadata: Reading JPEG-2000 file " << io_->path() << "\n";
#endif
        if (io_->open() != 0)
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isJp2Type(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "JPEG-2000");
        }

        long              position  = 0;
        Jp2BoxHeader      box       = {0,0};
        Jp2BoxHeader      subBox    = {0,0};
        Jp2ImageHeaderBox ihdr      = {0,0,0,0,0,0,0,0};
        Jp2UuidBox        uuid      = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

        while (io_->read((byte*)&box, sizeof(box)) == sizeof(box))
        {
            position   = io_->tell();
            box.length = getLong((byte*)&box.length, bigEndian);
            box.type   = getLong((byte*)&box.type, bigEndian);
#ifdef DEBUG
            std::cout << "Exiv2::Jp2Image::readMetadata: Position: " << position << "\n";
            std::cout << "Exiv2::Jp2Image::readMetadata: Find box type: " << toAscii(box.type)
                      << " length: " << box.length << "\n";
#endif

            if (box.length == 0)
            {
#ifdef DEBUG
                std::cout << "Exiv2::Jp2Image::readMetadata: Null Box size has been found. "
                             "This is the last box of file.\n";
#endif
                return;
            }
            if (box.length == 1)
            {
                // FIXME. Special case. the real box size is given in another place.
            }

            switch(box.type)
            {
                case kJp2BoxTypeJp2Header:
                {
#ifdef DEBUG
                    std::cout << "Exiv2::Jp2Image::readMetadata: JP2Header box found\n";
#endif
                    long restore = io_->tell();

                    while (io_->read((byte*)&subBox, sizeof(subBox)) == sizeof(subBox) && subBox.length )
                    {
                        subBox.length = getLong((byte*)&subBox.length, bigEndian);
                        subBox.type   = getLong((byte*)&subBox.type, bigEndian);
#ifdef DEBUG
                        std::cout << "subBox = " << toAscii(subBox.type) << " length = " << subBox.length << std::endl;
#endif
                        if(subBox.type == kJp2BoxTypeColorHeader)
                        {
#ifdef DEBUG
                            std::cout << "Exiv2::Jp2Image::readMetadata: Color data found\n";
#endif
                            long pad = 3 ; // don't know why there are 3 padding bytes
                            DataBuf data(subBox.length+8);
                            io_->read(data.pData_,data.size_);
                            long    iccLength = getULong(data.pData_+pad, bigEndian);
                            DataBuf icc(iccLength);
                            ::memcpy(icc.pData_,data.pData_+pad,icc.size_);
#ifdef DEBUG
                            const char* iccPath = "/tmp/libexiv2_jp2.icc";
                            FILE* f = fopen(iccPath,"wb");
                            if ( f ) {
                                fwrite(icc.pData_,icc.size_,1,f);
                                fclose(f);
                            }
                            std::cout << "Exiv2::Jp2Image::readMetadata: wrote iccProfile to " << iccPath << std::endl ;

#endif
                            setIccProfile(icc);
                        }

                        if( subBox.type == kJp2BoxTypeImageHeader)
                        {
                            io_->read((byte*)&ihdr, sizeof(ihdr));
#ifdef DEBUG
                           std::cout << "Exiv2::Jp2Image::readMetadata: Ihdr data found\n";
#endif

                            ihdr.imageHeight            = getLong((byte*)&ihdr.imageHeight, bigEndian);
                            ihdr.imageWidth             = getLong((byte*)&ihdr.imageWidth, bigEndian);
                            ihdr.componentCount         = getShort((byte*)&ihdr.componentCount, bigEndian);
                            ihdr.compressionTypeProfile = getShort((byte*)&ihdr.compressionTypeProfile, bigEndian);

                            pixelWidth_  = ihdr.imageWidth;
                            pixelHeight_ = ihdr.imageHeight;
                        }

                        io_->seek(restore,BasicIo::beg);
                        io_->seek(subBox.length, Exiv2::BasicIo::cur);
                        restore = io_->tell();
                    }
                    break;
                }
                case kJp2BoxTypeUuid:
                {
#ifdef DEBUG
                    std::cout << "Exiv2::Jp2Image::readMetadata: UUID box found\n";
#endif

                    if (io_->read((byte*)&uuid, sizeof(uuid)) == sizeof(uuid))
                    {
                        DataBuf rawData;
                        long    bufRead;
                        bool    bIsExif = memcmp(uuid.uuid, kJp2UuidExif, sizeof(uuid))==0;
                        bool    bIsIPTC = memcmp(uuid.uuid, kJp2UuidIptc, sizeof(uuid))==0;
                        bool    bIsXMP  = memcmp(uuid.uuid, kJp2UuidXmp , sizeof(uuid))==0;

                        if(bIsExif)
                        {
#ifdef DEBUG
                           std::cout << "Exiv2::Jp2Image::readMetadata: Exif data found\n";
#endif
                            rawData.alloc(box.length - (sizeof(box) + sizeof(uuid)));
                            bufRead = io_->read(rawData.pData_, rawData.size_);
                            if (io_->error()) throw Error(14);
                            if (bufRead != rawData.size_) throw Error(20);

                            if (rawData.size_ > 0)
                            {
                                // Find the position of Exif header in bytes array.
                                long pos = (     (rawData.pData_[0]      == rawData.pData_[1])
                                           &&    (rawData.pData_[0]=='I' || rawData.pData_[0]=='M')
                                           )  ? 0 : -1;

                                // #1242  Forgive having Exif\0\0 in rawData.pData_
                                const byte exifHeader[] = { 0x45, 0x78, 0x69, 0x66, 0x00, 0x00 };
                                for (long i=0 ; pos < 0 && i < rawData.size_-(long)sizeof(exifHeader) ; i++)
                                {
                                    if (memcmp(exifHeader, &rawData.pData_[i], sizeof(exifHeader)) == 0)
                                    {
                                        pos = i+sizeof(exifHeader);
                                    }
                                }

                                // If found it, store only these data at from this place.
                                if (pos >= 0 )
                                {
#ifdef DEBUG
                                    std::cout << "Exiv2::Jp2Image::readMetadata: Exif header found at position " << pos << std::endl;
#endif
                                    ByteOrder bo = TiffParser::decode(exifData(),
                                                                      iptcData(),
                                                                      xmpData(),
                                                                      rawData.pData_ + pos,
                                                                      rawData.size_ - pos);
                                    setByteOrder(bo);
                                }
                            }
                            else
                            {
#ifndef SUPPRESS_WARNINGS
                                EXV_WARNING << "Failed to decode Exif metadata.\n";
#endif
                                exifData_.clear();
                            }
                        }

                        if(bIsIPTC)
                        {
#ifdef DEBUG
                           std::cout << "Exiv2::Jp2Image::readMetadata: Iptc data found\n";
#endif
                            rawData.alloc(box.length - (sizeof(box) + sizeof(uuid)));
                            bufRead = io_->read(rawData.pData_, rawData.size_);
                            if (io_->error()) throw Error(14);
                            if (bufRead != rawData.size_) throw Error(20);

                            if (IptcParser::decode(iptcData_, rawData.pData_, rawData.size_))
                            {
#ifndef SUPPRESS_WARNINGS
                                EXV_WARNING << "Failed to decode IPTC metadata.\n";
#endif
                                iptcData_.clear();
                            }
                        }

                        if(bIsXMP)
                        {
#ifdef DEBUG
                           std::cout << "Exiv2::Jp2Image::readMetadata: Xmp data found\n";
#endif
                            rawData.alloc(box.length - (uint32_t)(sizeof(box) + sizeof(uuid)));
                            bufRead = io_->read(rawData.pData_, rawData.size_);
                            if (io_->error()) throw Error(14);
                            if (bufRead != rawData.size_) throw Error(20);
                            xmpPacket_.assign(reinterpret_cast<char *>(rawData.pData_), rawData.size_);

                            std::string::size_type idx = xmpPacket_.find_first_of('<');
                            if (idx != std::string::npos && idx > 0)
                            {
#ifndef SUPPRESS_WARNINGS
                                EXV_WARNING << "Removing " << static_cast<uint32_t>(idx)
                                            << " characters from the beginning of the XMP packet\n";
#endif
                                xmpPacket_ = xmpPacket_.substr(idx);
                            }

                            if (xmpPacket_.size() > 0 && XmpParser::decode(xmpData_, xmpPacket_))
                            {
#ifndef SUPPRESS_WARNINGS
                                EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
                            }
                        }
                    }
                    break;
                }

                default:
                {
                    break;
                }
            }

            // Move to the next box.
            io_->seek(static_cast<long>(position - sizeof(box) + box.length), BasicIo::beg);
            if (io_->error()/* || io_->eof()*/) throw Error(14);
        }

    } // Jp2Image::readMetadata

    static void lf(std::ostream& out,bool& bLF)
    {
        if ( bLF ) {
            out << std::endl;
            bLF = false ;
        }
    }
  /*
    static std::string printGUID(const uuid& guid)
    {
        long* data1 = (long*) &guid.uuid[0];
        return Internal::stringFormat("{%08X,%04X,%04X,%02X%02X%02X%02X%02X%02X%02X%02X}",
                +             *data1,*data2,*data3,*data4,
                +             id->Data2,
                +             id->Data3,
                +             id->Data4[0],
                +             id->Data4[1],
                +             id->Data4[2],
                +             id->Data4[3],
                +             id->Data4[4],
                +             id->Data4[5],
                +             id->Data4[6],
                +             id->Data4[7]);
    }
*/
    void Jp2Image::printStructure(std::ostream& out, PrintStructureOption option,int depth)
    {
        if (io_->open() != 0) throw Error(9, io_->path(), strError());

        // Ensure that this is the correct image type
        if (!isJp2Type(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(15);
        }

        out << "STRUCTURE OF JPEG2000 FILE: " << io_->path() << std::endl;
        out << " address |   length | box       | data" << std::endl ;

        bool bPrint     = option == kpsBasic || option==kpsRecursive;
        bool bRecursive = option == kpsRecursive;
        bool bICC       = option == kpsIccProfile;
        bool bXMP       = option == kpsXMP;
        bool bIPTCErase = option == kpsIptcErase;
        if ( bPrint || bXMP || bICC || bIPTCErase ) {

            long              position  = 0;
            Jp2BoxHeader      box       = {1,1};
            Jp2BoxHeader      subBox    = {1,1};
            Jp2UuidBox        uuid      = {{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};
            long              address   = io_->tell();
            bool              bLF       = false;

            while (box.length && io_->read((byte*)&box, sizeof(box)) == sizeof(box))
            {
                position   = io_->tell();
                box.length = getLong((byte*)&box.length, bigEndian);
                box.type   = getLong((byte*)&box.type, bigEndian);

                if ( bPrint ) {
                    out << Internal::stringFormat("%8ld | %8ld | ",address,box.length) << toAscii(box.type) << "      | " ;
                    bLF = true ;
                }

                if (box.length) switch(box.type)
                {
                    case kJp2BoxTypeJp2Header:
                    {
                        long restore = io_->tell();
                        lf(out,bLF);

                        while (io_->read((byte*)&subBox, sizeof(subBox)) == sizeof(subBox) && subBox.length )
                        {
                            subBox.length = getLong((byte*)&subBox.length, bigEndian);
                            subBox.type   = getLong((byte*)&subBox.type, bigEndian);

                            DataBuf data(subBox.length+8);
                            io_->read(data.pData_,data.size_);
                            if ( bPrint ) out << Internal::stringFormat("%8ld | %8ld |  sub:",restore,subBox.length) << toAscii(subBox.type)
                                              <<" | " << Internal::binaryToString(data.pData_,data.size_< 40 ? data.size_:40,sizeof(uuid)) << std::endl;

                            if(subBox.type == kJp2BoxTypeColorHeader)
                            {
                                long pad = 3 ; // don't know why there are 3 padding bytes
                                long    iccLength = getULong(data.pData_+pad, bigEndian);
                                DataBuf icc(iccLength);
                                if ( bICC ) out.write((const char*)icc.pData_,icc.size_);
                            }

                            io_->seek(restore,BasicIo::beg);
                            io_->seek(subBox.length, Exiv2::BasicIo::cur);
                            restore = io_->tell();
                        }
                        break;
                    }

                    case kJp2BoxTypeUuid:
                    {

                        if (io_->read((byte*)&uuid, sizeof(uuid)) == sizeof(uuid))
                        {
                            bool    bIsExif = memcmp(uuid.uuid, kJp2UuidExif, sizeof(uuid))==0;
                            bool    bIsIPTC = memcmp(uuid.uuid, kJp2UuidIptc, sizeof(uuid))==0;
                            bool    bIsXMP  = memcmp(uuid.uuid, kJp2UuidXmp , sizeof(uuid))==0;
                            bool    bUnknown= ! (bIsExif || bIsIPTC || bIsXMP);

                            if ( bPrint ) {
                                if ( bIsExif ) out << "Exif: " ;
                                if ( bIsIPTC ) out << "IPTC: " ;
                                if ( bIsXMP  ) out << "XMP : " ;
                                if ( bUnknown) out << "????: " ;
                            }

                            DataBuf rawData;
                                    rawData.alloc(box.length-sizeof(uuid)-sizeof(box));
                            long    bufRead = io_->read(rawData.pData_, rawData.size_);
                            if (io_->error()) throw Error(14);
                            if (bufRead != rawData.size_) throw Error(20);

                            if ( bPrint )out << Internal::binaryToString(rawData.pData_,rawData.size_< 40 ? rawData.size_:40,0);
                            lf(out,bLF);

                            if(bIsExif && bRecursive && rawData.size_ > 0)
                            {
                                if ( (rawData.pData_[0]      == rawData.pData_[1])
                                &&   (rawData.pData_[0]=='I' || rawData.pData_[0]=='M' )
                                ) {
                                    BasicIo::AutoPtr p = BasicIo::AutoPtr(new MemIo(rawData.pData_,rawData.size_));
                                    TiffImage::printTiffStructure(*p,out,option,depth);
                                }
                            }

                            if(bIsIPTC && bRecursive)
                            {
                                IptcData::printStructure(out,rawData.pData_,rawData.size_,depth);
                            }

                            if(bIsXMP && bXMP )
                            {
                                out.write((const char*)rawData.pData_,rawData.size_);
                            }
                        }
                        break;
                    }

                    default:
                    {
                        break;
                    }
                }
                address   = io_->tell();

                // Move to the next box.
                io_->seek(static_cast<long>(position - sizeof(box) + box.length), BasicIo::beg);
                if (io_->error()) throw Error(14);
                if ( bPrint ) lf(out,bLF);
            }
        }
    } // JpegBase::printStructure

    void Jp2Image::writeMetadata()
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

    } // Jp2Image::writeMetadata

    void Jp2Image::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(20);
        if (!outIo.isopen()) throw Error(21);

#ifdef DEBUG
        std::cout << "Exiv2::Jp2Image::doWriteMetadata: Writing JPEG-2000 file " << io_->path() << "\n";
        std::cout << "Exiv2::Jp2Image::doWriteMetadata: tmp file created " << outIo.path() << "\n";
#endif

        // Ensure that this is the correct image type
        if (!isJp2Type(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(20);
            throw Error(22);
        }

        // Write JPEG2000 Signature.
        if (outIo.write(Jp2Signature, 12) != 12) throw Error(21);

        Jp2BoxHeader box = {0,0};

        byte    boxDataSize[4];
        byte    boxUUIDtype[4];
        DataBuf bheaderBuf(8);     // Box header : 4 bytes (data size) + 4 bytes (box type).

        // FIXME: Andreas, why the loop do not stop when EOF is taken from _io. The loop go out by an exception
        // generated by a zero size data read.

        while(io_->tell() < io_->size())
        {
#ifdef DEBUG
            std::cout << "Exiv2::Jp2Image::doWriteMetadata: Position: " << io_->tell() << " / " << io_->size() << "\n";
#endif

            // Read chunk header.

            std::memset(bheaderBuf.pData_, 0x00, bheaderBuf.size_);
            long bufRead = io_->read(bheaderBuf.pData_, bheaderBuf.size_);
            if (io_->error()) throw Error(14);
            if (bufRead != bheaderBuf.size_) throw Error(20);

            // Decode box header.

            box.length = getLong(bheaderBuf.pData_,     bigEndian);
            box.type   = getLong(bheaderBuf.pData_ + 4, bigEndian);

#ifdef DEBUG
            std::cout << "Exiv2::Jp2Image::doWriteMetadata: Find box type: " << bheaderBuf.pData_ + 4
                      << " length: " << box.length << "\n";
#endif

            if (box.length == 0)
            {
#ifdef DEBUG
                std::cout << "Exiv2::Jp2Image::doWriteMetadata: Null Box size has been found. "
                             "This is the last box of file.\n";
#endif
                box.length = io_->size() - io_->tell() + 8;
            }
            if (box.length == 1)
            {
                // FIXME. Special case. the real box size is given in another place.
            }

            // Read whole box : Box header + Box data (not fixed size - can be null).

            DataBuf boxBuf(box.length);                             // Box header (8 bytes) + box data.
            memcpy(boxBuf.pData_, bheaderBuf.pData_, 8);               // Copy header.
            bufRead = io_->read(boxBuf.pData_ + 8, box.length - 8); // Extract box data.
            if (io_->error())
            {
#ifdef DEBUG
                std::cout << "Exiv2::Jp2Image::doWriteMetadata: Error reading source file\n";
#endif

                throw Error(14);
            }

            if (bufRead != (long)(box.length - 8))
            {
#ifdef DEBUG
                std::cout << "Exiv2::Jp2Image::doWriteMetadata: Cannot read source file data\n";
#endif
                throw Error(20);
            }

            switch(box.type)
            {
                case kJp2BoxTypeJp2Header:
                {

#ifdef DEBUG
                    std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write JP2Header box (length: " << box.length << ")\n";
#endif
                    if (outIo.write(boxBuf.pData_, boxBuf.size_) != boxBuf.size_) throw Error(21);

                    // Write all updated metadata here, just after JP2Header.

                    if (exifData_.count() > 0)
                    {
                        // Update Exif data to a new UUID box

                        Blob blob;
                        ExifParser::encode(blob, littleEndian, exifData_);
                        if (blob.size())
                        {
                            DataBuf rawExif(static_cast<long>(blob.size()));
                            memcpy(rawExif.pData_, &blob[0], blob.size());

                            DataBuf boxData(8 + 16 + rawExif.size_);
                            ul2Data(boxDataSize, boxData.size_, Exiv2::bigEndian);
                            ul2Data(boxUUIDtype, kJp2BoxTypeUuid, Exiv2::bigEndian);
                            memcpy(boxData.pData_,          boxDataSize,    4);
                            memcpy(boxData.pData_ + 4,      boxUUIDtype,    4);
                            memcpy(boxData.pData_ + 8,      kJp2UuidExif,   16);
                            memcpy(boxData.pData_ + 8 + 16, rawExif.pData_, rawExif.size_);

#ifdef DEBUG
                            std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write box with Exif metadata (length: "
                                      << boxData.size_ << ")\n";
#endif
                            if (outIo.write(boxData.pData_, boxData.size_) != boxData.size_) throw Error(21);
                        }
                    }

                    if (iptcData_.count() > 0)
                    {
                        // Update Iptc data to a new UUID box

                        DataBuf rawIptc = IptcParser::encode(iptcData_);
                        if (rawIptc.size_ > 0)
                        {
                            DataBuf boxData(8 + 16 + rawIptc.size_);
                            ul2Data(boxDataSize, boxData.size_, Exiv2::bigEndian);
                            ul2Data(boxUUIDtype, kJp2BoxTypeUuid, Exiv2::bigEndian);
                            memcpy(boxData.pData_,          boxDataSize,    4);
                            memcpy(boxData.pData_ + 4,      boxUUIDtype,    4);
                            memcpy(boxData.pData_ + 8,      kJp2UuidIptc,   16);
                            memcpy(boxData.pData_ + 8 + 16, rawIptc.pData_, rawIptc.size_);

#ifdef DEBUG
                            std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write box with Iptc metadata (length: "
                                      << boxData.size_ << ")\n";
#endif
                            if (outIo.write(boxData.pData_, boxData.size_) != boxData.size_) throw Error(21);
                        }
                    }

                    if (writeXmpFromPacket() == false)
                    {
                        if (XmpParser::encode(xmpPacket_, xmpData_) > 1)
                        {
#ifndef SUPPRESS_WARNINGS
                            EXV_ERROR << "Failed to encode XMP metadata.\n";
#endif
                        }
                    }
                    if (xmpPacket_.size() > 0)
                    {
                        // Update Xmp data to a new UUID box

                        DataBuf xmp(reinterpret_cast<const byte*>(xmpPacket_.data()), static_cast<long>(xmpPacket_.size()));
                        DataBuf boxData(8 + 16 + xmp.size_);
                        ul2Data(boxDataSize, boxData.size_, Exiv2::bigEndian);
                        ul2Data(boxUUIDtype, kJp2BoxTypeUuid, Exiv2::bigEndian);
                        memcpy(boxData.pData_,          boxDataSize,  4);
                        memcpy(boxData.pData_ + 4,      boxUUIDtype,  4);
                        memcpy(boxData.pData_ + 8,      kJp2UuidXmp,  16);
                        memcpy(boxData.pData_ + 8 + 16, xmp.pData_,   xmp.size_);

#ifdef DEBUG
                        std::cout << "Exiv2::Jp2Image::doWriteMetadata: Write box with XMP metadata (length: "
                                  << boxData.size_ << ")\n";
#endif
                        if (outIo.write(boxData.pData_, boxData.size_) != boxData.size_) throw Error(21);
                    }

                    break;
                }

                case kJp2BoxTypeUuid:
                {
                    if(memcmp(boxBuf.pData_ + 8, kJp2UuidExif, 16) == 0)
                    {
#ifdef DEBUG
                        std::cout << "Exiv2::Jp2Image::doWriteMetadata: strip Exif Uuid box\n";
#endif
                    }
                    else if(memcmp(boxBuf.pData_ + 8, kJp2UuidIptc, 16) == 0)
                    {
#ifdef DEBUG
                        std::cout << "Exiv2::Jp2Image::doWriteMetadata: strip Iptc Uuid box\n";
#endif
                    }
                    else if(memcmp(boxBuf.pData_ + 8, kJp2UuidXmp,  16) == 0)
                    {
#ifdef DEBUG
                        std::cout << "Exiv2::Jp2Image::doWriteMetadata: strip Xmp Uuid box\n";
#endif
                    }
                    else
                    {
#ifdef DEBUG
                        std::cout << "Exiv2::Jp2Image::doWriteMetadata: write Uuid box (length: " << box.length << ")\n";
#endif
                        if (outIo.write(boxBuf.pData_, boxBuf.size_) != boxBuf.size_) throw Error(21);
                    }
                    break;
                }

                default:
                {
#ifdef DEBUG
                    std::cout << "Exiv2::Jp2Image::doWriteMetadata: write box (length: " << box.length << ")\n";
#endif
                    if (outIo.write(boxBuf.pData_, boxBuf.size_) != boxBuf.size_) throw Error(21);

                    break;
                }
            }
        }

#ifdef DEBUG
        std::cout << "Exiv2::Jp2Image::doWriteMetadata: EOF\n";
#endif

    } // Jp2Image::doWriteMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newJp2Instance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new Jp2Image(io, create));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isJp2Type(BasicIo& iIo, bool advance)
    {
        const int32_t len = 12;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        bool matched = (memcmp(buf, Jp2Signature, len) == 0);
        if (!advance || !matched)
        {
            iIo.seek(-len, BasicIo::cur);
        }
        return matched;
    }
}                                       // namespace Exiv2
