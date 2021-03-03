// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2021 Exiv2 authors
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

// *****************************************************************************

// #define EXIV2_DEBUG_MESSAGES

// included header files
#include "bmffimage.hpp"

#include "basicio.hpp"
#include "config.h"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "safe_op.hpp"
#include "tiffimage.hpp"
#include "tiffimage_int.hpp"
#include "types.hpp"
#include "unused.h"

// + standard includes
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>

struct BmffBoxHeader
{
    uint32_t length;
    uint32_t type;
};

#define TAG_ftyp 0x66747970 /**< "ftyp" File type box */
#define TAG_avif 0x61766966 /**< "avif" AVIF */
#define TAG_heic 0x68656963 /**< "heic" HEIC */
#define TAG_heif 0x68656966 /**< "heif" HEIF */
#define TAG_crx 0x63727820  /**< "crx " Canon CR3 */
#define TAG_moov 0x6d6f6f76 /**< "moov" Movie */
#define TAG_meta 0x6d657461 /**< "meta" Metadata */
#define TAG_mdat 0x6d646174 /**< "mdat" Media data */
#define TAG_uuid 0x75756964 /**< "uuid" UUID */
#define TAG_dinf 0x64696e66 /**< "dinf" Data information */
#define TAG_iprp 0x69707270 /**< "iprp" Item properties */
#define TAG_ipco 0x6970636f /**< "ipco" Item property container */
#define TAG_iinf 0x69696e66 /**< "iinf" Item info */
#define TAG_iloc 0x696c6f63 /**< "iloc" Item location */
#define TAG_ispe 0x69737065 /**< "ispe" Image spatial extents */
#define TAG_infe 0x696e6665 /**< "infe" Item Info Extention */
#define TAG_ipma 0x69706d61 /**< "ipma" Item Property Association */
#define TAG_cmt1 0x434d5431 /**< "CMT1" ifd0Id */
#define TAG_cmt2 0x434D5432 /**< "CMD2" exifID */
#define TAG_cmt3 0x434D5433 /**< "CMT3" canonID */
#define TAG_cmt4 0x434D5434 /**< "CMT4" gpsID */
#define TAG_colr 0x636f6c72 /**< "colr" */

// *****************************************************************************
// class member definitions
namespace Exiv2
{
    static bool enabled = false;

    EXIV2API bool enableBMFF(bool enable)
    {
#ifdef EXV_ENABLE_BMFF
        enabled = enable;
        return true;
#endif                   // EXV_ENABLE_BMFF
        enable = false;  // unused
        return enable;
    }

    std::string Iloc::toString() const
    {
        return Internal::stringFormat("ID = %u from,length = %u,%u", ID_, start_, length_);
    }

    BmffImage::BmffImage(BasicIo::AutoPtr io, bool /* create */) : Image(ImageType::bmff, mdExif | mdIptc | mdXmp, io)
    {
        pixelWidth_    = 0;
        pixelHeight_   = 0;
        bReadMetadata_ = false;
    }  // BmffImage::BmffImage

    std::string BmffImage::toAscii(long n)
    {
        const char* p = (const char*)&n;
        std::string result;
        bool bBigEndian = isBigEndianPlatform();
        for (int i = 0; i < 4; i++) {
            result += p[bBigEndian ? i : (3 - i)];
        }
        return result;
    }

    std::string BmffImage::boxName(uint32_t box)
    {
        char name[5];
        std::memcpy(name, &box, 4);
        name[4] = 0;
        return std::string(name);
    }

    bool BmffImage::superBox(uint32_t box)
    {
        return box == TAG_moov || box == TAG_dinf || box == TAG_iprp || box == TAG_ipco || box == TAG_meta ||
               box == TAG_iinf || box == TAG_iloc;
    }

    bool BmffImage::fullBox(uint32_t box)
    {
        return box == TAG_meta || box == TAG_iinf || box == TAG_iloc;
    }

    std::string BmffImage::mimeType() const
    {
        switch (fileType_) {
            case TAG_avif:
                return "image/avif";
            case TAG_heic:
            case TAG_heif:
                return "image/heif";
            case TAG_crx:
                return "image/x-canon-cr3";
            default:
                return "image/generic";
        }
    }

    int BmffImage::pixelWidth() const
    {
        ExifData::const_iterator imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            return imageWidth->toLong();
        }
        return 0;
    }

    int BmffImage::pixelHeight() const
    {
        ExifData::const_iterator imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            return imageHeight->toLong();
        }
        return 0;
    }

    std::string BmffImage::uuidName(Exiv2::DataBuf& uuid)
    {
        const char* uuidCano = "\x85\xC0\xB6\x87\x82\xF\x11\xE0\x81\x11\xF4\xCE\x46\x2B\x6A\x48";
        const char* uuidXmp = "\xBE\x7A\xCF\xCB\x97\xA9\x42\xE8\x9C\x71\x99\x94\x91\xE3\xAF\xAC";
        const char* uuidCanp = "\xEA\xF4\x2B\x5E\x1C\x98\x4B\x88\xB9\xFB\xB7\xDC\x40\x6E\x4D\x16";
        const char* result = std::memcmp(uuid.pData_, uuidCano, 16) == 0   ? "cano"
                             : std::memcmp(uuid.pData_, uuidXmp, 16) == 0  ? "xmp"
                             : std::memcmp(uuid.pData_, uuidCanp, 16) == 0 ? "canp"
                                                                           : "";
        return result;
    }

    long BmffImage::boxHandler(std::ostream& out /* = std::cout*/ , Exiv2::PrintStructureOption option /* = kpsNone */,int depth /* =0 */)
    {
        long result  = (long)io_->size();
        long address = (long)io_->tell();
        if (visits_.find(address) != visits_.end() || visits_.size() > visits_max_) {
            throw Error(kerCorruptedMetadata);
        }
        visits_.insert(address);

        bool bTrace    = option == kpsBasic || option == kpsRecursive ;
#ifdef EXIV2_DEBUG_MESSAGES
        bTrace = true ;
#endif

        BmffBoxHeader box = {0, 0};
        if (io_->read((byte*)&box, sizeof(box)) != sizeof(box))
            return result;

        box.length = getLong((byte*)&box.length, bigEndian);
        box.type = getLong((byte*)&box.type, bigEndian);
        bool bLF = true;

        if ( bTrace ) {
            bLF = true;
            out << indent(depth) << "Exiv2::BmffImage::boxHandler: " << toAscii(box.type)
                << Internal::stringFormat(" %8ld->%u ", address, box.length);
        }

        if (box.length == 1) {
            DataBuf data(8);
            io_->read(data.pData_, data.size_);
            result = address + (long)getULongLong(data.pData_, littleEndian);
            // sanity check
            if (result < 0 || result > (long)io_->size()) {
                result = (long)io_->size();
                box.length = result - address;
            }
            if ( bTrace ) {
                out << Internal::stringFormat(" (%lu)", result);
            }
        }

        // read data in box and restore file position
        long restore = io_->tell();
        DataBuf data(box.length - 8);
        io_->read(data.pData_, data.size_);
        io_->seek(restore, BasicIo::beg);

        uint32_t skip = 0;  // read position in data.pData_
        uint8_t version = 0;
        uint32_t flags = 0;

        if (fullBox(box.type)) {
            flags = getLong(data.pData_ + skip, bigEndian);  // version/flags
            version = (int8_t)flags >> 24;
            version &= 0x00ffffff;
            skip += 4;
        }

        switch (box.type) {
            case TAG_ftyp: {
                fileType_ = getLong(data.pData_, bigEndian);
                if ( bTrace ) {
                    out << "brand: " << toAscii(fileType_);
                }
            } break;

            // 8.11.6.1
            case TAG_iinf: {
                if ( bTrace ) {
                    out << std::endl;
                    bLF = false;
                }

                int n = getShort(data.pData_ + skip, bigEndian);
                skip += 2;

                io_->seek(skip, BasicIo::cur);
                while (n-- > 0) {
                    io_->seek(boxHandler(out,option,depth + 1), BasicIo::beg);
                }
            } break;

            // 8.11.6.2
            case TAG_infe: {  // .__._.__hvc1_ 2 0 0 1 0 1 0 0 104 118 99 49 0
                /* getLong (data.pData_+skip,bigEndian) ; */ skip += 4;
                uint16_t ID = getShort(data.pData_ + skip, bigEndian);
                skip += 2;
                /* getShort(data.pData_+skip,bigEndian) ; */ skip += 2;  // protection
                std::string id;
                std::string name((const char*)data.pData_ + skip);
                if ( !name.find("Exif") ) {  // "Exif" or "ExifExif"
                    exifID_ = ID;
                    id=" *** Exif ***";
                } else if ( !name.find("mime\0xmp") || !name.find("mime\0application/rdf+xml") ) {
                    xmpID_ = ID;
                    id=" *** XMP ***";
                }
                if ( bTrace ) {
                    out << Internal::stringFormat("ID = %3d ", ID) << name << " " << id;
                }
            } break;

            case TAG_moov:
            case TAG_iprp:
            case TAG_ipco:
            case TAG_meta: {
                if ( bTrace ) {
                    out << std::endl;
                    bLF = false;
                }
                io_->seek(skip, BasicIo::cur);
                while ((long)io_->tell() < (long)(address + box.length)) {
                    io_->seek(boxHandler(out,option,depth + 1), BasicIo::beg);
                }
                // post-process meta box to recover Exif and XMP
                if (box.type == TAG_meta) {
                    if ( ilocs_.find(exifID_) != ilocs_.end()) {
                        const Iloc& iloc = ilocs_.find(exifID_)->second;
                        if ( bTrace ) {
                            out << indent(depth) << "Exiv2::BMFF Exif: " << iloc.toString() << std::endl;
                        }
                        parseTiff(Internal::Tag::root,iloc.length_,iloc.start_);
                    }
                    if ( ilocs_.find(xmpID_) != ilocs_.end()) {
                        const Iloc& iloc = ilocs_.find(xmpID_)->second;
                        if ( bTrace ) {
                            out << indent(depth) << "Exiv2::BMFF XMP: " << iloc.toString() << std::endl;
                        }
                        parseXmp(iloc.length_,iloc.start_);
                    }
                    ilocs_.clear() ;
                }
            } break;

            // 8.11.3.1
            case TAG_iloc: {
                uint8_t u = data.pData_[skip++];
                uint16_t offsetSize = u >> 4;
                uint16_t lengthSize = u & 0xF;
#if 0
                uint16_t indexSize  = 0       ;
                u             = data.pData_[skip++];
                if ( version == 1 || version == 2 ) {
                    indexSize = u & 0xF ;
                }
#else
                skip++;
#endif
                uint32_t itemCount = version < 2 ? getShort(data.pData_ + skip, bigEndian)
                                                 : getLong(data.pData_ + skip, bigEndian);
                skip += version < 2 ? 2 : 4;
                if (itemCount && itemCount < box.length / 14 && offsetSize == 4 && lengthSize == 4 &&
                    ((box.length - 16) % itemCount) == 0) {
                    if ( bTrace ) {
                        out << std::endl;
                        bLF = false;
                    }
                    uint32_t step = (box.length - 16) / itemCount;  // length of data per item.
                    uint32_t base = skip;
                    for (uint32_t i = 0; i < itemCount; i++) {
                        skip = base + i * step;  // move in 14, 16 or 18 byte steps
                        uint32_t ID = version > 2 ? getLong(data.pData_ + skip, bigEndian)
                                                  : getShort(data.pData_ + skip, bigEndian);
                        uint32_t offset = step==14 || step==16 ? getLong(data.pData_ + skip + step - 8, bigEndian)
                                        : step== 18            ? getLong(data.pData_ + skip + 4, bigEndian)
                                        : 0 ;

                        uint32_t ldata = getLong(data.pData_ + skip + step - 4, bigEndian);
                        if ( bTrace ) {
                            out << indent(depth)
                                << Internal::stringFormat("%8ld | %8u |   ID | %4u | %6u,%6u", address + skip, step,
                                                            ID, offset, ldata)
                                << std::endl;
                        }
                        // save data for post-processing in meta box
                        if ( offset && ldata && ID != unknownID_ ) {
                            ilocs_[ID] = Iloc(ID, offset, ldata);
                        }
                    }
                }
            } break;

            case TAG_ispe: {
                skip += 4;
                int width = (int)getLong(data.pData_ + skip, bigEndian);
                skip += 4;
                int height = (int)getLong(data.pData_ + skip, bigEndian);
                skip += 4;
                if ( bTrace ) {
                    out << "pixelWidth_, pixelHeight_ = " << Internal::stringFormat("%d, %d", width, height);
                }
                // HEIC files can have multiple ispe records
                // Store largest width/height
                if (width > pixelWidth_ && height > pixelHeight_) {
                    pixelWidth_ = width;
                    pixelHeight_ = height;
                }
            } break;

            // 12.1.5.2
            case TAG_colr: {
                if ( data.size_ >= (long) (skip+4+8) /* 8 for safety */  ) { // .____.HLino..__mntrR 2 0 0 0 0 12 72 76 105 110 111 2 16 ...
                    // https://www.ics.uci.edu/~dan/class/267/papers/jpeg2000.pdf
                    uint8_t      meth        = data.pData_[skip+0];
                    uint8_t      prec        = data.pData_[skip+1];
                    uint8_t      approx      = data.pData_[skip+2];
                    uint32_t     colour_type = getLong(data.pData_+skip,littleEndian) ;
                    skip+=4;
                    if ( boxName(colour_type) == "rICC" || boxName(colour_type) == "prof" ) {
                        DataBuf    profile(box.length-skip);
                        ::memcpy(profile.pData_,data.pData_+skip,profile.size_-skip);
                        // fix length header bug in iOS files.
                        uint32_t                 iccLength = getLong((byte*)&profile.size_,bigEndian);
                        ::memcpy(profile.pData_,&iccLength,4);
                        setIccProfile(profile);
                    } else if ( meth == 2 && prec == 0 && approx == 0 ) {
                        // JP2000 files have a 3 byte head // 2 0 0 icc......
                        skip -= 1 ;
                        DataBuf    profile(box.length-skip);
                        ::memcpy(profile.pData_,data.pData_+skip,profile.size_-skip);
                        this->setIccProfile(profile);
                    }
                }
            } break;

            case TAG_uuid: {
                DataBuf   uuid(16);
                io_->read(uuid.pData_, uuid.size_);
                std::string name = uuidName(uuid);
                if ( bTrace ) {
                    out << " uuidName " << name << std::endl;
                    bLF = false;
                }
                if (name == "cano") {
                    while ((long)io_->tell() < (long)(address + box.length)) {
                        io_->seek(boxHandler(out,option,depth + 1), BasicIo::beg);
                    }
                } else if ( name == "xmp" ) {
                    parseXmp(box.length,io_->tell());
                }
            } break;

            case TAG_cmt1:
                parseTiff(Internal::Tag::root, box.length);
                break;
            case TAG_cmt2:
                parseTiff(Internal::Tag::cmt2, box.length);
                break;
            case TAG_cmt3:
                parseTiff(Internal::Tag::cmt3, box.length);
                break;
            case TAG_cmt4:
                parseTiff(Internal::Tag::cmt4, box.length);
                break;

            default: break ; /* do nothing */
        }
        if ( bLF&& bTrace) out << std::endl;

        // return address of next box
        if (box.length != 1)
            result = static_cast<long>(address + box.length);

        return result;
    }

    void BmffImage::parseTiff(uint32_t root_tag, uint32_t length,uint32_t start)
    {
        // read and parse exif data
        long    restore = io_->tell();
        DataBuf exif(length);
        io_->seek(start,BasicIo::beg);
        if ( exif.size_ > 8 && io_->read(exif.pData_,exif.size_) == exif.size_ ) {
            // hunt for "II" or "MM"
            long  eof  = 0xffffffff; // impossible value for punt
            long  punt = eof;
            for ( long i = 0 ; i < exif.size_ -8 && punt==eof ; i+=2) {
                if ( exif.pData_[i] == exif.pData_[i+1] )
                    if ( exif.pData_[i] == 'I' || exif.pData_[i] == 'M' )
                        punt = i;
            }
            if ( punt != eof ) {
                Internal::TiffParserWorker::decode(exifData(), iptcData(), xmpData(),
                  exif.pData_+punt, exif.size_-punt, root_tag,
                  Internal::TiffMapping::findDecoder);
            }
        }
        io_->seek(restore,BasicIo::beg);
    }

    void BmffImage::parseTiff(uint32_t root_tag, uint32_t length)
    {
        if (length > 8) {
            DataBuf data(length - 8);
            long bufRead = io_->read(data.pData_, data.size_);

            if (io_->error())
                throw Error(kerFailedToReadImageData);
            if (bufRead != data.size_)
                throw Error(kerInputDataReadFailed);

            Internal::TiffParserWorker::decode(exifData(), iptcData(), xmpData(),
                                               data.pData_, data.size_, root_tag,
                                               Internal::TiffMapping::findDecoder);
        }
    }

    void BmffImage::parseXmp(uint32_t length,uint32_t start)
    {
        if (length > 8) {
            long restore = io_->tell() ;
            io_->seek(start,BasicIo::beg);

            DataBuf  xmp(length+1);
            xmp.pData_[length]=0  ; // ensure xmp is null terminated!
            if ( io_->read(xmp.pData_, length) != length )
                throw Error(kerInputDataReadFailed);
            if ( io_->error() )
                throw Error(kerFailedToReadImageData);
            try {
                Exiv2::XmpParser::decode(xmpData(),std::string((char*)xmp.pData_));
            } catch (...) {
                throw Error(kerFailedToReadImageData);
            }

            io_->seek(restore,BasicIo::beg);
        }
    }

    void BmffImage::setComment(const std::string& /*comment*/)
    {
        // bmff files are read-only
        throw(Error(kerInvalidSettingForImage, "Image comment", "BMFF"));
    }  // BmffImage::setComment

    void BmffImage::openOrThrow()
    {
        if (io_->open() != 0) {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        // Ensure that this is the correct image type
        if (!isBmffType(*io_, false)) {
            if (io_->error() || io_->eof())
                throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage, "BMFF");
        }
    } // Bmff::openOrThrow();

    void BmffImage::readMetadata()
    {
        openOrThrow();
        IoCloser closer(*io_);

        clearMetadata();
        ilocs_.clear();
        visits_.clear();
        visits_max_ = io_->size() / 16;
        unknownID_ = 0xffff;
        exifID_    = unknownID_;

        long address = 0;
        while (address < (long)io_->size()) {
            io_->seek(address, BasicIo::beg);
            address = boxHandler(std::cout,kpsNone);
        }
        bReadMetadata_ = true;
    }  // BmffImage::readMetadata

    void BmffImage::printStructure(std::ostream& out, Exiv2::PrintStructureOption option,int depth)
    {
        if ( !bReadMetadata_ ) readMetadata();

        switch (option) {
            default: break; // do nothing

            case kpsIccProfile : {
                out.write((const char*)iccProfile_.pData_,iccProfile_.size_);
            } break;

#ifdef EXV_HAVE_XMP_TOOLKIT
            case kpsXMP : {
                std::string xmp;
                if ( Exiv2::XmpParser::encode(xmp, xmpData()) ) {
                    throw Exiv2::Error(Exiv2::kerErrorMessage, "Failed to serialize XMP data");
                }
                out << xmp;
            } break;
#endif
            case kpsBasic      : // drop
            case kpsRecursive  : {
                openOrThrow();
                IoCloser closer(*io_);
                visits_.clear();

                long   address = 0;
                while (address < (long)io_->size()) {
                    io_->seek(address, BasicIo::beg);
                    address = boxHandler(out,option,depth);
                }
            }; break;
        }
    }

    void BmffImage::writeMetadata()
    {
        // bmff files are read-only
        throw(Error(kerInvalidSettingForImage, "Image comment", "BMFF"));
    }  // BmffImage::writeMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newBmffInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new BmffImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isBmffType(BasicIo& iIo, bool advance)
    {
        if (!enabled) {
            return false;
        }
        const int32_t len = 12;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }

        bool matched = buf[4] == 'f' && buf[5] == 't' && buf[6] == 'y' && buf[7] == 'p';
        if (!advance || !matched) {
            iIo.seek(static_cast<long>(0), BasicIo::beg);
        }
        return matched;
    }
}  // namespace Exiv2
