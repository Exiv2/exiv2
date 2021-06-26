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
#include "enforce.hpp"
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
#define TAG_avio 0x6176696f /**< "avio" AVIF */
#define TAG_avis 0x61766973 /**< "avis" AVIF */
#define TAG_heic 0x68656963 /**< "heic" HEIC */
#define TAG_heif 0x68656966 /**< "heif" HEIF */
#define TAG_heim 0x6865696d /**< "heim" HEIC */
#define TAG_heis 0x68656973 /**< "heis" HEIC */
#define TAG_heix 0x68656978 /**< "heix" HEIC */
#define TAG_mif1 0x6d696631 /**< "mif1" HEIF */
#define TAG_crx  0x63727820 /**< "crx " Canon CR3 */
#define TAG_jxl  0x4a584c20 /**< "JXL " JPEG XL   */  
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
#define TAG_exif 0x45786966 /**< "Exif" Used by JXL*/
#define TAG_xml  0x786d6c20 /**< "xml"  Used by JXL*/

// *****************************************************************************
// class member definitions
#ifdef EXV_ENABLE_BMFF
namespace Exiv2
{
    static bool   enabled = false;
    EXIV2API bool enableBMFF(bool enable)
    {
        enabled = enable ;
        return true ;
    }

    std::string Iloc::toString() const
    {
        return Internal::stringFormat("ID = %u from,length = %u,%u", ID_, start_, length_);
    }

    BmffImage::BmffImage(BasicIo::UniquePtr io, bool /* create */)
        : Image(ImageType::bmff, mdExif | mdIptc | mdXmp, std::move(io))
    {
    }  // BmffImage::BmffImage

    std::string BmffImage::toAscii(long n)
    {
        const auto p = reinterpret_cast<const char*>(&n);
        std::string result;
        for (int i = 0; i < 4; i++) {
            char c = p[isBigEndianPlatform() ? i : (3 - i)];
            result += (32<=c && c<127) ? c    // only allow 7-bit printable ascii
                    : c==0 ? '_'              // show 0 as _
                    : '.' ;                   // others .
        }
        return result;
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
            case TAG_avio:
            case TAG_avis:
                return "image/avif";
            case TAG_heic:
            case TAG_heim:
            case TAG_heis:
            case TAG_heix:
                return "image/heic";
            case TAG_heif:
            case TAG_mif1:
                return "image/heif";
            case TAG_crx:
                return "image/x-canon-cr3";
            case TAG_jxl:
                return "image/jxl"; // https://github.com/novomesk/qt-jpegxl-image-plugin/issues/1
            default:
                return "image/generic";
        }
    }

    int BmffImage::pixelWidth() const
    {
        auto imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            return imageWidth->toLong();
        }
        return pixelWidth_;
    }

    int BmffImage::pixelHeight() const
    {
        auto imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            return imageHeight->toLong();
        }
        return pixelHeight_;
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
        long result = static_cast<long>(io_->size());
        long address = io_->tell();
        // never visit a box twice!
        if ( depth == 0 ) visits_.clear();
        if (visits_.find(address) != visits_.end() || visits_.size() > visits_max_) {
            throw Error(kerCorruptedMetadata);
        }
        visits_.insert(address);

        bool bTrace    = option == kpsBasic || option == kpsRecursive ;
#ifdef EXIV2_DEBUG_MESSAGES
        bTrace = true ;
#endif

        BmffBoxHeader box = {0, 0};
        if (io_->read(reinterpret_cast<byte*>(&box), sizeof(box)) != sizeof(box))
            return result;

        box.length = getLong(reinterpret_cast<byte*>(&box.length), endian_);
        box.type = getLong(reinterpret_cast<byte*>(&box.type), endian_);
        bool bLF = true;

        if ( bTrace ) {
            bLF = true;
            out << indent(depth) << "Exiv2::BmffImage::boxHandler: " << toAscii(box.type)
                << Internal::stringFormat(" %8ld->%u ", address, box.length);
        }

        if (box.length == 1) {
            DataBuf data(8);
            io_->read(data.pData_, data.size_);
            const uint64_t sz = getULongLong(data.pData_, endian_);
            // Check that `sz` is safe to cast to `long`.
            enforce(sz <= static_cast<uint64_t>(std::numeric_limits<long>::max()),
                    Exiv2::kerCorruptedMetadata);
            result = static_cast<long>(sz);
            // sanity check
            if (result < 8 || result > static_cast<long>(io_->size()) - address) {
                result = static_cast<long>(io_->size());
                box.length = result;
            } else {
                box.length = static_cast<long>(io_->size() - address);
            }
        }

        // read data in box and restore file position
        long restore = io_->tell();
        enforce(box.length >= 8, Exiv2::kerCorruptedMetadata);
        DataBuf data(box.length - 8);
        io_->read(data.pData_, data.size_);
        io_->seek(restore, BasicIo::beg);

        long skip = 0;  // read position in data.pData_
        uint8_t version = 0;
        uint32_t flags = 0;

        if (fullBox(box.type)) {
            enforce(data.size_ - skip >= 4, Exiv2::kerCorruptedMetadata);
            flags = getLong(data.pData_ + skip, endian_);  // version/flags
            version = static_cast<int8_t>(flags) >> 24;
            version &= 0x00ffffff;
            skip += 4;
        }

        switch (box.type) {
            case TAG_ftyp: {
                enforce(data.size_ >= 4, Exiv2::kerCorruptedMetadata);
                fileType_ = getLong(data.pData_, endian_);
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

                enforce(data.size_ - skip >= 2, Exiv2::kerCorruptedMetadata);
                int n = getShort(data.pData_ + skip, endian_);
                skip += 2;

                io_->seek(skip, BasicIo::cur);
                while (n-- > 0) {
                    io_->seek(boxHandler(out,option,depth + 1), BasicIo::beg);
                }
            } break;

            // 8.11.6.2
            case TAG_infe: {  // .__._.__hvc1_ 2 0 0 1 0 1 0 0 104 118 99 49 0
                enforce(data.size_ - skip >= 8, Exiv2::kerCorruptedMetadata);
                /* getLong (data.pData_+skip,endian_) ; */ skip += 4;
                uint16_t ID = getShort(data.pData_ + skip, endian_);
                skip += 2;
                /* getShort(data.pData_+skip,endian_) ; */ skip += 2;  // protection
                std::string id;
                // Check that the string has a '\0' terminator.
                const char* str = reinterpret_cast<const char*>(data.pData_) + skip;
                const auto maxlen = static_cast<size_t>(data.size_ - skip);
                enforce(strnlen(str, maxlen) < maxlen, Exiv2::kerCorruptedMetadata);
                std::string name(str);
                if (name.find("Exif") != std::string::npos) {  // "Exif" or "ExifExif"
                    exifID_ = ID;
                    id=" *** Exif ***";
                } else if (name.find("mime\0xmp") != std::string::npos ||
                           name.find("mime\0application/rdf+xml") != std::string::npos) {
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
                while (io_->tell() < static_cast<long>((address + box.length))) {
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
                enforce(data.size_ - skip >= 2, Exiv2::kerCorruptedMetadata);
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
                enforce(data.size_ - skip >= (version < 2 ? 2 : 4), Exiv2::kerCorruptedMetadata);
                uint32_t itemCount = version < 2 ? getShort(data.pData_ + skip, endian_)
                                                 : getLong(data.pData_ + skip, endian_);
                skip += version < 2 ? 2 : 4;
                if (itemCount && itemCount < box.length / 14 && offsetSize == 4 && lengthSize == 4 &&
                    ((box.length - 16) % itemCount) == 0) {
                    if ( bTrace ) {
                        out << std::endl;
                        bLF = false;
                    }
                    long step = (box.length - 16) / itemCount;  // length of data per item.
                    long base = skip;
                    for (uint32_t i = 0; i < itemCount; i++) {
                        skip = base + i * step;  // move in 14, 16 or 18 byte steps
                        enforce(data.size_ - skip >= (version > 2 ? 4 : 2), Exiv2::kerCorruptedMetadata);
                        enforce(data.size_ - skip >= step, Exiv2::kerCorruptedMetadata);
                        uint32_t ID = version > 2 ? getLong(data.pData_ + skip, endian_)
                                                  : getShort(data.pData_ + skip, endian_);
                        uint32_t offset = step==14 || step==16 ? getLong(data.pData_ + skip + step - 8, endian_)
                                        : step== 18            ? getLong(data.pData_ + skip + 4, endian_)
                                        : 0 ;

                        uint32_t ldata = getLong(data.pData_ + skip + step - 4, endian_);
                        if ( bTrace ) {
                            out << indent(depth)
                                << Internal::stringFormat("%8ld | %8ld |   ID | %4u | %6u,%6u", address + skip, step,
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
                enforce(data.size_ - skip >= 12, Exiv2::kerCorruptedMetadata);
                skip += 4;
                int width = getLong(data.pData_ + skip, endian_);
                skip += 4;
                int height = getLong(data.pData_ + skip, endian_);
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
                if (data.size_ >=
                    static_cast<long>(skip + 4 +
                                      sizeof(box))) {  // .____.HLino..__mntrR 2 0 0 0 0 12 72 76 105 110 111 2 16 ...
                    // https://www.ics.uci.edu/~dan/class/267/papers/jpeg2000.pdf
                    uint8_t      meth        = data.pData_[skip+0];
                    uint8_t      prec        = data.pData_[skip+1];
                    uint8_t      approx      = data.pData_[skip+2];
                    std::string colour_type = std::string(reinterpret_cast<char*>(data.pData_), 4);
                    skip+=4;
                    if ( colour_type == "rICC" || colour_type == "prof" ) {
                        DataBuf       profile(data.pData_+skip,data.size_-skip);
                        setIccProfile(profile);
                    } else if ( meth == 2 && prec == 0 && approx == 0 ) {
                        // JP2000 files have a 3 byte head // 2 0 0 icc......
                        skip -= 1 ;
                        DataBuf       profile(data.pData_+skip,data.size_-skip);
                        setIccProfile(profile);
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
                    while (io_->tell() < static_cast<long>(address + box.length)) {
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
            case TAG_exif:
                parseTiff(Internal::Tag::root, box.length,address+8);
                break;
            case TAG_xml:
                parseXmp(box.length,io_->tell());
                break;

            default: break ; /* do nothing */
        }
        if ( bLF&& bTrace) out << std::endl;

        // return address of next box
        result = (address + box.length);

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
                Exiv2::XmpParser::decode(xmpData(), std::string(reinterpret_cast<char*>(xmp.pData_)));
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
        visits_max_ = io_->size() / 16;
        unknownID_ = 0xffff;
        exifID_    = unknownID_;
        xmpID_     = unknownID_;

        long address = 0;
        while (address < static_cast<long>(io_->size())) {
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
                out.write(reinterpret_cast<const char*>(iccProfile_.pData_), iccProfile_.size_);
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

                long   address = 0;
                while (address < static_cast<long>(io_->size())) {
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
    Image::UniquePtr newBmffInstance(BasicIo::UniquePtr io, bool create)
    {
        Image::UniquePtr image(new BmffImage(std::move(io), create));
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

        bool matched = (buf[4] == 'f' && buf[5] == 't' && buf[6] == 'y' && buf[7] == 'p')
                     ||(buf[4] == 'J' && buf[5] == 'X' && buf[6] == 'L' && buf[7] == ' ');
        if (!advance || !matched) {
            iIo.seek(static_cast<long>(0), BasicIo::beg);
        }
        return matched;
    }
}  // namespace Exiv2
#else  // ifdef EXV_ENABLE_BMFF
namespace Exiv2
{
    EXIV2API bool enableBMFF(bool)
    {
        return false ;
    }
}
#endif

