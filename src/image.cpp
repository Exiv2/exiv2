// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
  File:      image.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Brad Schick (brad) <brad@robotbattle.com>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
             19-Jul-04, brad: revamped to be more flexible and support Iptc
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#ifdef HAVE_CONFIG_H
# include <config.h>
#else
# ifdef _MSC_VER
#  include <config_win32.h>
# endif
#endif

#include "image.hpp"
#include "types.hpp"
#include "error.hpp"

// + standard includes
#include <cstdio>
#include <cstring>
#include <cstdio>                               // for rename, remove
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
# define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#endif
#ifdef HAVE_PROCESS_H
# include <process.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>                            // for getpid, stat
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    // Local functions. These could be static private functions on Image
    // subclasses but then ImageFactory needs to be made a friend. 
    /*!
      @brief Create a new ExvImage instance and return an auto-pointer to it. 
             Caller owns the returned object and the auto-pointer ensures that 
             it will be deleted.
     */
    Image::AutoPtr newExvInstance(const std::string& path, bool create);
    //! Check if the file ifp is an EXV file.
    bool isExvType(FILE* ifp, bool advance);
    /*!
      @brief Create a new JpegImage instance and return an auto-pointer to it.
             Caller owns the returned object and the auto-pointer ensures that 
             it will be deleted.
     */
    Image::AutoPtr newJpegInstance(const std::string& path, bool create);
    //! Check if the file ifp is a JPEG image.
    bool isJpegType(FILE* ifp, bool advance);

    ImageFactory* ImageFactory::pInstance_ = 0;

    ImageFactory& ImageFactory::instance()
    {
        if (0 == pInstance_) {
            pInstance_ = new ImageFactory;
        }
        return *pInstance_;
    } // ImageFactory::instance

    void ImageFactory::registerImage(Image::Type type, 
                NewInstanceFct newInst, IsThisTypeFct isType)
    {
        assert (newInst && isType);
        registry_[type] = ImageFcts(newInst, isType);
    } // ImageFactory::registerImage

    ImageFactory::ImageFactory()
    {
        // Register a prototype of each known image
        registerImage(Image::jpeg, newJpegInstance, isJpegType);
        registerImage(Image::exv, newExvInstance, isExvType);
    } // ImageFactory c'tor

    Image::Type ImageFactory::getType(const std::string& path) const
    {
        FileCloser closer(fopen(path.c_str(), "rb"));
        if (!closer.fp_) return Image::none;

        Image::Type type = Image::none;
        Registry::const_iterator b = registry_.begin();
        Registry::const_iterator e = registry_.end();
        for (Registry::const_iterator i = b; i != e; ++i)
        {
            if (i->second.isThisType(closer.fp_, false)) {
                type = i->first;
                break;
            }
        }
        return type;
    } // ImageFactory::getType

    Image::AutoPtr ImageFactory::open(const std::string& path) const
    {
        Image::AutoPtr image;
        FileCloser closer(fopen(path.c_str(), "rb"));
        if (!closer.fp_) return image;

        Registry::const_iterator b = registry_.begin();
        Registry::const_iterator e = registry_.end();
        for (Registry::const_iterator i = b; i != e; ++i)
        {
            if (i->second.isThisType(closer.fp_, false)) {
                image = i->second.newInstance(path, false);
                break;
            }
        }
        return image;
    } // ImageFactory::open

    Image::AutoPtr ImageFactory::create(Image::Type type, 
                                        const std::string& path) const
    {
        Registry::const_iterator i = registry_.find(type);
        if (i != registry_.end()) {
            return i->second.newInstance(path, true);
        }
        return Image::AutoPtr();
    } // ImageFactory::create


    const byte JpegBase::sos_    = 0xda;
    const byte JpegBase::eoi_    = 0xd9;
    const byte JpegBase::app0_   = 0xe0;
    const byte JpegBase::app1_   = 0xe1;
    const byte JpegBase::app13_  = 0xed;
    const byte JpegBase::com_    = 0xfe;
    const uint16_t JpegBase::iptc_ = 0x0404;
    const char JpegBase::exifId_[] = "Exif\0\0";
    const char JpegBase::jfifId_[] = "JFIF\0";
    const char JpegBase::ps3Id_[]  = "Photoshop 3.0\0";
    const char JpegBase::bimId_[]  = "8BIM";

    JpegBase::JpegBase(const std::string& path, bool create, 
                       const byte initData[], size_t dataSize) 
        : path_(path), sizeExifData_(0), pExifData_(0),
          sizeIptcData_(0), pIptcData_(0)
    {
        if (create) {
            FILE* fp = fopen(path.c_str(), "w+b");
            if (fp) {
                initFile(fp, initData, dataSize);
                fclose(fp);
            }
        }
    }

    int JpegBase::initFile(FILE* fp, const byte initData[], size_t dataSize)
    {
        if (!fp || ferror(fp)) return 4;
        if (fwrite(initData, 1, dataSize, fp) != dataSize) {
            return 4;
        }
        return 0;
    }

    JpegBase::~JpegBase()
    {
        delete[] pExifData_;
        delete[] pIptcData_;
    }

    bool JpegBase::good() const
    {
        FileCloser closer(fopen(path_.c_str(), "rb"));
        if (closer.fp_ == 0 ) return false;
        return isThisType(closer.fp_, false);
    }

    void JpegBase::clearMetadata()
    {
        clearIptcData();
        clearExifData();
        clearComment();
    }
    
    void JpegBase::clearIptcData()
    {
        delete[] pIptcData_;
        pIptcData_ = 0;
        sizeIptcData_ = 0;
    }

    void JpegBase::clearExifData()
    {
        delete[] pExifData_;
        pExifData_ = 0;
        sizeExifData_ = 0;
    }

    void JpegBase::clearComment()
    {
        comment_.erase();
    }

    void JpegBase::setExifData(const byte* buf, long size)
    {
        if (size > 0xfffd) throw Error("Exif data too large");
        clearExifData();
        if (size) {
            sizeExifData_ = size;
            pExifData_ = new byte[size];
            memcpy(pExifData_, buf, size);
        }
    }

    void JpegBase::setIptcData(const byte* buf, long size)
    {
        clearIptcData();
        if (size) {
            sizeIptcData_ = size;
            pIptcData_ = new byte[size];
            memcpy(pIptcData_, buf, size);
        }
    }

    void JpegBase::setComment(const std::string& comment)
    { 
        comment_ = comment; 
    }

    void JpegBase::setMetadata(const Image& image)
    {
        setIptcData(image.iptcData(), image.sizeIptcData());
        setExifData(image.exifData(), image.sizeExifData());
        setComment(image.comment());
    }

    int JpegBase::advanceToMarker(FILE *fp) const
    {
        int c = -1;
        // Skips potential padding between markers
        while ((c=fgetc(fp)) != 0xff) {
            if (c == EOF) return -1;
        }
            
        // Markers can start with any number of 0xff
        while ((c=fgetc(fp)) == 0xff) {
            if (c == EOF) return -1;
        }
        return c;
    }

    int JpegBase::readMetadata()
    {
        FileCloser closer(fopen(path_.c_str(), "rb"));
        if (!closer.fp_) return 1;

        // Ensure that this is the correct image type
        if (!isThisType(closer.fp_, true)) {
            if (ferror(closer.fp_) || feof(closer.fp_)) return 1;
            return 2;
        }
        clearMetadata();
        int search = 3;
        const long bufMinSize = 16;
        long bufRead = 0;
        DataBuf buf(bufMinSize);

        // Read section marker
        int marker = advanceToMarker(closer.fp_);
        if (marker < 0) return 2;
        
        while (marker != sos_ && marker != eoi_ && search > 0) {
            // Read size and signature (ok if this hits EOF)
            bufRead = (long)fread(buf.pData_, 1, bufMinSize, closer.fp_);
            if (ferror(closer.fp_)) return 1;
            uint16_t size = getUShort(buf.pData_, bigEndian);

            if (marker == app1_ && memcmp(buf.pData_ + 2, exifId_, 6) == 0) {
                if (size < 8) return 2;
                // Seek to begining and read the Exif data
                fseek(closer.fp_, 8-bufRead, SEEK_CUR); 
                long sizeExifData = size - 8;
                pExifData_ = new byte[sizeExifData];
                fread(pExifData_, 1, sizeExifData, closer.fp_);
                if (ferror(closer.fp_) || feof(closer.fp_)) {
                    delete[] pExifData_;
                    pExifData_ = 0;
                    return 1;
                }
                // Set the size and offset of the Exif data buffer
                sizeExifData_ = sizeExifData;
                --search;
            }
            else if (marker == app13_ && memcmp(buf.pData_ + 2, ps3Id_, 14) == 0) {
                if (size < 16) return 2;
                // Read the rest of the APP13 segment
                // needed if bufMinSize!=16: fseek(closer.fp_, 16-bufRead, SEEK_CUR);
                DataBuf psData(size - 16);
                fread(psData.pData_, 1, psData.size_, closer.fp_);
                if (ferror(closer.fp_) || feof(closer.fp_)) return 1;
                const byte *record = 0;
                uint16_t sizeIptc = 0;
                uint16_t sizeHdr = 0;
                // Find actual Iptc data within the APP13 segment
                if (!locateIptcData(psData.pData_, psData.size_, &record,
                            &sizeHdr, &sizeIptc)) {
                    assert(sizeIptc);
                    sizeIptcData_ = sizeIptc;
                    pIptcData_ = new byte[sizeIptc];
                    memcpy( pIptcData_, record + sizeHdr, sizeIptc );
                }
                --search;
            }
            else if (marker == com_ && comment_.empty())
            {
                if (size < 2) return 2;
                // Jpegs can have multiple comments, but for now only read
                // the first one (most jpegs only have one anyway). Comments
                // are simple single byte ISO-8859-1 strings.
                fseek(closer.fp_, 2-bufRead, SEEK_CUR);
                buf.alloc(size-2);
                fread(buf.pData_, 1, size-2, closer.fp_);
                if (ferror(closer.fp_) || feof(closer.fp_)) return 1;
                comment_.assign(reinterpret_cast<char*>(buf.pData_), size-2);
                while (   comment_.length()
                       && comment_.at(comment_.length()-1) == '\0') {
                    comment_.erase(comment_.length()-1);
                }
                --search;
            }
            else {
                if (size < 2) return 2;
                // Skip the remainder of the unknown segment
                if (fseek(closer.fp_, size-bufRead, SEEK_CUR)) return 2;
            }
            // Read the beginning of the next segment
            marker = advanceToMarker(closer.fp_);
            if (marker < 0) return 2;
        }
        return 0;
    } // JpegBase::readMetadata


    // Operates on raw data (rather than file streams) to simplify reuse
    int JpegBase::locateIptcData(const byte *pPsData, 
                                 long sizePsData,
                                 const byte **record, 
                                 uint16_t *const sizeHdr,
                                 uint16_t *const sizeIptc) const
    {
        assert(record);
        assert(sizeHdr);
        assert(sizeIptc);
        // Used for error checking
        long position = 0;

        // Data should follow Photoshop format, if not exit
        while (position <= (sizePsData - 14) &&
                memcmp(pPsData + position, bimId_, 4)==0) {
            const byte *hrd = pPsData + position;
            position += 4;
            uint16_t type = getUShort(pPsData+ position, bigEndian);
            position += 2;
           
            // Pascal string is padded to have an even size (including size byte)
            byte psSize = pPsData[position] + 1;
            psSize += (psSize & 1);
            position += psSize;
            if (position >= sizePsData) return -2;

            // Data is also padded to be even
            long dataSize = getULong(pPsData + position, bigEndian);
            position += 4;
            if (dataSize > sizePsData - position) return -2;
           
            if (type == iptc_) {
                *sizeIptc = static_cast<uint16_t>(dataSize);
                *sizeHdr = psSize + 10;
                *record = hrd;
                return 0;
            }
            position += dataSize + (dataSize & 1);
        }
        return 3;
    } // JpegBase::locateIptcData

    int JpegBase::writeMetadata()
    {
        FileCloser reader(fopen(path_.c_str(), "rb"));
        if (!reader.fp_) return 1;

        // Write the output to a temporary file
        pid_t pid = getpid();
        std::string tmpname = path_ + toString(pid);
        FileCloser writer(fopen(tmpname.c_str(), "wb"));
        if (!writer.fp_) return -3;

        int rc = doWriteMetadata(reader.fp_, writer.fp_);
        writer.close();
        reader.close();
        if (rc == 0) {
            // Workaround for MSVCRT rename that does not overwrite existing files
            if (remove(path_.c_str()) != 0) rc = -4;
        }
        if (rc == 0) {
            // rename temporary file
            if (rename(tmpname.c_str(), path_.c_str()) == -1) rc = -4;
        }
        if (rc != 0) {
            // remove temporary file
            remove(tmpname.c_str());
        }
        return rc;
    } // JpegBase::writeMetadata

    int JpegBase::doWriteMetadata(FILE *ifp, FILE* ofp) const
    {
        if (!ifp) return 1;
        if (!ofp) return 4;

        // Ensure that this is the correct image type
        if (!isThisType(ifp, true)) {
            if (ferror(ifp) || feof(ifp)) return 1;
            return 2;
        }
        
        const long bufMinSize = 16;
        long bufRead = 0;
        DataBuf buf(bufMinSize);
        const long seek = ftell(ifp);
        int count = 0;
        int search = 0;
        int insertPos = 0;
        int skipApp1Exif = -1;
        int skipApp13Ps3 = -1;
        int skipCom = -1;
        DataBuf psData;

        // Write image header
        if (writeHeader(ofp)) return 4;

        // Read section marker
        int marker = advanceToMarker(ifp);
        if (marker < 0) return 2;
        
        // First find segments of interest. Normally app0 is first and we want
        // to insert after it. But if app0 comes after com, app1 and app13 then
        // don't bother.
        while (marker != sos_ && marker != eoi_ && search < 3) {
            // Read size and signature (ok if this hits EOF)
            bufRead = (long)fread(buf.pData_, 1, bufMinSize, ifp);
            if (ferror(ifp)) return 1;
            uint16_t size = getUShort(buf.pData_, bigEndian);

            if (marker == app0_) {
                if (size < 2) return 2;
                insertPos = count + 1;
                if (fseek(ifp, size-bufRead, SEEK_CUR)) return 2;
            }
            else if (marker == app1_ && memcmp(buf.pData_ + 2, exifId_, 6) == 0) {
                if (size < 8) return 2;
                skipApp1Exif = count;
                ++search;
                if (fseek(ifp, size-bufRead, SEEK_CUR)) return 2;
            }
            else if (marker == app13_ && memcmp(buf.pData_ + 2, ps3Id_, 14) == 0) {
                if (size < 16) return 2;
                skipApp13Ps3 = count;
                ++search;
                // needed if bufMinSize!=16: fseek(ifp, 16-bufRead, SEEK_CUR);
                psData.alloc(size - 16);
                // Load PS data now to allow reinsertion at any point
                fread(psData.pData_, 1, psData.size_, ifp);
                if (ferror(ifp) || feof(ifp)) return 1;
            }
            else if (marker == com_ && skipCom == -1) {
                if (size < 2) return 2;
                // Jpegs can have multiple comments, but for now only handle
                // the first one (most jpegs only have one anyway).
                skipCom = count;
                ++search;
                if (fseek(ifp, size-bufRead, SEEK_CUR)) return 2;
            }
            else {
                if (size < 2) return 2;
                if (fseek(ifp, size-bufRead, SEEK_CUR)) return 2;
            }
            marker = advanceToMarker(ifp);
            if (marker < 0) return 2;
            ++count;
        }

        if (pExifData_) ++search;
        if (pIptcData_) ++search;
        if (!comment_.empty()) ++search;

        fseek(ifp, seek, SEEK_SET);
        count = 0;
        marker = advanceToMarker(ifp);
        if (marker < 0) return 2;
        
        // To simplify this a bit, new segments are inserts at either the start
        // or right after app0. This is standard in most jpegs, but has the
        // potential to change segment ordering (which is allowed).
        // Segments are erased if there is no assigned metadata.
        while (marker != sos_ && search > 0) {
            // Read size and signature (ok if this hits EOF)
            bufRead = (long)fread(buf.pData_, 1, bufMinSize, ifp);
            if (ferror(ifp)) return 1;
            // Careful, this can be a meaningless number for empty
            // images with only an eoi_ marker
            uint16_t size = getUShort(buf.pData_, bigEndian);

            if (insertPos == count) {
                byte tmpBuf[18];
                if (!comment_.empty()) {
                    // Write COM marker, size of comment, and string
                    tmpBuf[0] = 0xff;
                    tmpBuf[1] = com_;
                    us2Data(tmpBuf + 2, 
                            static_cast<uint16_t>(comment_.length()+3), bigEndian);
                    if (fwrite(tmpBuf, 1, 4, ofp) != 4) return 4;
                    if (   fwrite(comment_.data(), 1, comment_.length(), ofp)
                        != comment_.length()) return 4;
                    if (fputc(0, ofp)==EOF) return 4;
                    if (ferror(ofp)) return 4;
                    --search;
                }
                if (pExifData_) {
                    // Write APP1 marker, size of APP1 field, Exif id and Exif data
                    tmpBuf[0] = 0xff;
                    tmpBuf[1] = app1_;
                    us2Data(tmpBuf + 2, 
                            static_cast<uint16_t>(sizeExifData_+8), 
                            bigEndian);
                    memcpy(tmpBuf + 4, exifId_, 6);
                    if (fwrite(tmpBuf, 1, 10, ofp) != 10) return 4;
                    if (   fwrite(pExifData_, 1, sizeExifData_, ofp) 
                        != (size_t)sizeExifData_) return 4;
                    if (ferror(ofp)) return 4;
                    --search;
                }
                
                const byte *record = psData.pData_;
                uint16_t sizeIptc = 0;
                uint16_t sizeHdr = 0;
                // Safe to call with zero psData.size_
                locateIptcData(psData.pData_, psData.size_, &record, &sizeHdr, &sizeIptc);

                // Data is rounded to be even
                const int sizeOldData = sizeHdr + sizeIptc + (sizeIptc & 1);
                if (psData.size_ > sizeOldData || pIptcData_) {
                    // write app13 marker, new size, and ps3Id
                    tmpBuf[0] = 0xff;
                    tmpBuf[1] = app13_;
                    const int sizeNewData = sizeIptcData_ ? 
                            sizeIptcData_+(sizeIptcData_&1)+12 : 0;
                    us2Data(tmpBuf + 2, 
                            static_cast<uint16_t>(psData.size_-sizeOldData+sizeNewData+16),
                            bigEndian);
                    memcpy(tmpBuf + 4, ps3Id_, 14);
                    if (fwrite(tmpBuf, 1, 18, ofp) != 18) return 4;
                    if (ferror(ofp)) return 4;

                    const long sizeFront = (long)(record - psData.pData_);
                    const long sizeEnd = psData.size_ - sizeFront - sizeOldData;
                    // write data before old record.
                    if (fwrite(psData.pData_, 1, sizeFront, ofp) != (size_t)sizeFront) return 4;

                    // write new iptc record if we have it
                    if (pIptcData_) {
                        memcpy(tmpBuf, bimId_, 4);
                        us2Data(tmpBuf+4, iptc_, bigEndian);
                        tmpBuf[6] = 0;
                        tmpBuf[7] = 0;
                        ul2Data(tmpBuf + 8, sizeIptcData_, bigEndian);
                        if (fwrite(tmpBuf, 1, 12, ofp) != 12) return 4;
                        if (   fwrite(pIptcData_, 1, sizeIptcData_ , ofp) 
                            != (size_t)sizeIptcData_) return 4;
                        // data is padded to be even (but not included in size)
                        if (sizeIptcData_ & 1) {
                            if (fputc(0, ofp)==EOF) return 4;
                        }
                        if (ferror(ofp)) return 4;
                        --search;
                    }
                    
                    // write existing stuff after record
                    if (   fwrite(record+sizeOldData, 1, sizeEnd, ofp) 
                        != (size_t)sizeEnd) return 4;
                    if (ferror(ofp)) return 4;
                }
            }
            if (marker == eoi_) {
                break;
            }
            else if (skipApp1Exif==count || skipApp13Ps3==count || skipCom==count) {
                --search;
                fseek(ifp, size-bufRead, SEEK_CUR);
            }
            else {
                if (size < 2) return 2;
                buf.alloc(size+2);
                fseek(ifp, -bufRead-2, SEEK_CUR);
                fread(buf.pData_, 1, size+2, ifp);
                if (ferror(ifp) || feof(ifp)) return 1;
                if (fwrite(buf.pData_, 1, size+2, ofp) != (size_t)size+2) return 4;
                if (ferror(ofp)) return 4;
            }

            // Next marker
            marker = advanceToMarker(ifp);
            if (marker < 0) return 2;
            ++count;
        }

        // Copy rest of the stream
        fseek(ifp, -2, SEEK_CUR);
        fflush( ofp );
        buf.alloc(4096);
        size_t readSize = 0;
        while ((readSize=fread(buf.pData_, 1, buf.size_, ifp))) {
            if (fwrite(buf.pData_, 1, readSize, ofp) != readSize) return 4;
        }
        if (ferror(ofp)) return 4;
        
        return 0;
    }// JpegBase::doWriteMetadata


    const byte JpegImage::soi_ = 0xd8;
    const byte JpegImage::blank_[] = {
        0xFF,0xD8,0xFF,0xDB,0x00,0x84,0x00,0x10,0x0B,0x0B,0x0B,0x0C,0x0B,0x10,0x0C,0x0C,
        0x10,0x17,0x0F,0x0D,0x0F,0x17,0x1B,0x14,0x10,0x10,0x14,0x1B,0x1F,0x17,0x17,0x17,
        0x17,0x17,0x1F,0x1E,0x17,0x1A,0x1A,0x1A,0x1A,0x17,0x1E,0x1E,0x23,0x25,0x27,0x25,
        0x23,0x1E,0x2F,0x2F,0x33,0x33,0x2F,0x2F,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
        0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x01,0x11,0x0F,0x0F,0x11,0x13,0x11,0x15,0x12,
        0x12,0x15,0x14,0x11,0x14,0x11,0x14,0x1A,0x14,0x16,0x16,0x14,0x1A,0x26,0x1A,0x1A,
        0x1C,0x1A,0x1A,0x26,0x30,0x23,0x1E,0x1E,0x1E,0x1E,0x23,0x30,0x2B,0x2E,0x27,0x27,
        0x27,0x2E,0x2B,0x35,0x35,0x30,0x30,0x35,0x35,0x40,0x40,0x3F,0x40,0x40,0x40,0x40,
        0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0xFF,0xC0,0x00,0x11,0x08,0x00,0x01,0x00,
        0x01,0x03,0x01,0x22,0x00,0x02,0x11,0x01,0x03,0x11,0x01,0xFF,0xC4,0x00,0x4B,0x00,
        0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x07,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x10,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xDA,0x00,0x0C,0x03,0x01,0x00,0x02,
        0x11,0x03,0x11,0x00,0x3F,0x00,0xA0,0x00,0x0F,0xFF,0xD9 };

    JpegImage::JpegImage(const std::string& path, bool create) 
        : JpegBase(path, create, blank_, sizeof(blank_))
    {
    }

    int JpegImage::writeHeader(FILE* ofp) const
    {
        // Jpeg header
        byte tmpBuf[2];
        tmpBuf[0] = 0xff;
        tmpBuf[1] = soi_;
        if (fwrite(tmpBuf, 1, 2, ofp) != 2) return 4;
        if (ferror(ofp)) return 4;
        return 0;
    }

    bool JpegImage::isThisType(FILE* ifp, bool advance) const
    {
        return isJpegType(ifp, advance);
    }

    Image::AutoPtr newJpegInstance(const std::string& path, bool create)
    {
        Image::AutoPtr image;
        if (create) {
            image = Image::AutoPtr(new JpegImage(path, true));
        }
        else {
            image = Image::AutoPtr(new JpegImage(path, false));
        }
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isJpegType(FILE* ifp, bool advance)
    {
        bool result = true;
        byte tmpBuf[2];
        fread(tmpBuf, 1, 2, ifp);
        if (ferror(ifp) || feof(ifp)) return false;

        if (0xff!=tmpBuf[0] || JpegImage::soi_!=tmpBuf[1]) {
            result = false;
        }
        if (!advance || !result ) fseek(ifp, -2, SEEK_CUR);
        return result;
    }
   
    const char ExvImage::exiv2Id_[] = "Exiv2";
    const byte ExvImage::blank_[] = { 0xff,0x01,'E','x','i','v','2',0xff,0xd9 };

    ExvImage::ExvImage(const std::string& path, bool create) 
        : JpegBase(path, create, blank_, sizeof(blank_))
    {
    }

    int ExvImage::writeHeader(FILE* ofp) const
    {
        // Exv header
        byte tmpBuf[7];
        tmpBuf[0] = 0xff;
        tmpBuf[1] = 0x01;
        memcpy(tmpBuf + 2, exiv2Id_, 5);
        if (fwrite(tmpBuf, 1, 7, ofp) != 7) return 4;
        if (ferror(ofp)) return 4;
        return 0;
    }

    bool ExvImage::isThisType(FILE* ifp, bool advance) const
    {
        return isExvType(ifp, advance);
    }

    Image::AutoPtr newExvInstance(const std::string& path, bool create)
    {
        Image::AutoPtr image;
        if (create) {
            image = Image::AutoPtr(new ExvImage(path, true));
        }
        else {
            image = Image::AutoPtr(new ExvImage(path, false));
        }
        if (!image->good()) image.reset();
        return image;
    }

    bool isExvType(FILE* ifp, bool advance)
    {
        bool result = true;
        byte tmpBuf[7];
        fread(tmpBuf, 1, 7, ifp);
        if (ferror(ifp) || feof(ifp)) return false;

        if (0xff!=tmpBuf[0] || 0x01!=tmpBuf[1] || 
                    memcmp(tmpBuf + 2, ExvImage::exiv2Id_, 5) != 0) {
            result = false;
        }
        if (!advance || !result ) fseek(ifp, -7, SEEK_CUR);
        return result;
    }

    TiffHeader::TiffHeader(ByteOrder byteOrder) 
        : byteOrder_(byteOrder), tag_(0x002a), offset_(0x00000008)
    {
    }

    int TiffHeader::read(const byte* buf)
    {
        if (buf[0] == 0x49 && buf[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (buf[0] == 0x4d && buf[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            return 1;
        }
        tag_ = getUShort(buf+2, byteOrder_);
        offset_ = getULong(buf+4, byteOrder_);
        return 0;
    }

    long TiffHeader::copy(byte* buf) const
    {
        switch (byteOrder_) {
        case littleEndian:
            buf[0] = 0x49;
            buf[1] = 0x49;
            break;
        case bigEndian:
            buf[0] = 0x4d;
            buf[1] = 0x4d;
            break;
        case invalidByteOrder:
            // do nothing
            break;
        }
        us2Data(buf+2, 0x002a, byteOrder_);
        ul2Data(buf+4, 0x00000008, byteOrder_);
        return size();
    } // TiffHeader::copy

// *****************************************************************************
// free functions

    bool fileExists(const std::string& path, bool ct)
    {
        struct stat buf;
        int ret = stat(path.c_str(), &buf);
        if (0 != ret)                    return false;
        if (ct && !S_ISREG(buf.st_mode)) return false;
        return true;
    } // fileExists

}                                       // namespace Exiv2
