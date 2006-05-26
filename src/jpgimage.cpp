// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      jpgimage.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Brad Schick (brad) <brad@robotbattle.com>
  History:   15-Jan-05, brad: split out from image.cpp

 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "jpgimage.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <cstring>
#include <cassert>

// *****************************************************************************
// class member definitions

namespace Exiv2 {

    const byte JpegBase::sos_    = 0xda;
    const byte JpegBase::eoi_    = 0xd9;
    const byte JpegBase::app0_   = 0xe0;
    const byte JpegBase::app1_   = 0xe1;
    const byte JpegBase::app13_  = 0xed;
    const byte JpegBase::com_    = 0xfe;
    const char JpegBase::exifId_[] = "Exif\0\0";
    const char JpegBase::jfifId_[] = "JFIF\0";

    JpegBase::JpegBase(BasicIo::AutoPtr io, bool create,
                       const byte initData[], long dataSize)
        : Image(mdExif | mdIptc | mdComment), io_(io)
    {
        if (create) {
            initImage(initData, dataSize);
        }
    }

    int JpegBase::initImage(const byte initData[], long dataSize)
    {
        if (io_->open() != 0) {
            return 4;
        }
        IoCloser closer(*io_);
        if (io_->write(initData, dataSize) != dataSize) {
            return 4;
        }
        return 0;
    }

    bool JpegBase::good() const
    {
        if (io_->open() != 0) return false;
        IoCloser closer(*io_);
        return isThisType(*io_, false);
    }

    void JpegBase::clearMetadata()
    {
        clearIptcData();
        clearExifData();
        clearComment();
    }

    void JpegBase::clearIptcData()
    {
        iptcData_.clear();
    }

    void JpegBase::clearExifData()
    {
        exifData_.clear();
    }

    void JpegBase::clearComment()
    {
        comment_.erase();
    }

    void JpegBase::setExifData(const ExifData& exifData)
    {
        exifData_ = exifData;
    }

    void JpegBase::setIptcData(const IptcData& iptcData)
    {
        iptcData_ = iptcData;
    }

    void JpegBase::setComment(const std::string& comment)
    {
        comment_ = comment;
    }

    void JpegBase::setMetadata(const Image& image)
    {
        setIptcData(image.iptcData());
        setExifData(image.exifData());
        setComment(image.comment());
    }

    int JpegBase::advanceToMarker() const
    {
        int c = -1;
        // Skips potential padding between markers
        while ((c=io_->getb()) != 0xff) {
            if (c == EOF) return -1;
        }

        // Markers can start with any number of 0xff
        while ((c=io_->getb()) == 0xff) {
            if (c == EOF) return -1;
        }
        return c;
    }

    void JpegBase::readMetadata()
    {
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isThisType(*io_, true)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(15);
        }
        clearMetadata();
        int search = 3;
        const long bufMinSize = 16;
        long bufRead = 0;
        DataBuf buf(bufMinSize);

        // Read section marker
        int marker = advanceToMarker();
        if (marker < 0) throw Error(15);

        while (marker != sos_ && marker != eoi_ && search > 0) {
            // Read size and signature (ok if this hits EOF)
            bufRead = io_->read(buf.pData_, bufMinSize);
            if (io_->error()) throw Error(14);
            uint16_t size = getUShort(buf.pData_, bigEndian);

            if (marker == app1_ && memcmp(buf.pData_ + 2, exifId_, 6) == 0) {
                if (size < 8) throw Error(15);
                // Seek to begining and read the Exif data
                io_->seek(8-bufRead, BasicIo::cur);
                long sizeExifData = size - 8;
                DataBuf rawExif(sizeExifData);
                io_->read(rawExif.pData_, sizeExifData);
                if (io_->error() || io_->eof()) throw Error(14);
                if (exifData_.load(rawExif.pData_, sizeExifData)) throw Error(36, "Exif");
                --search;
            }
            else if (marker == app13_ && memcmp(buf.pData_ + 2, Photoshop::ps3Id, 14) == 0) {
                if (size < 16) throw Error(15);
                // Read the rest of the APP13 segment
                // needed if bufMinSize!=16: io_->seek(16-bufRead, BasicIo::cur);
                DataBuf psData(size - 16);
                io_->read(psData.pData_, psData.size_);
                if (io_->error() || io_->eof()) throw Error(14);
                const byte *record = 0;
                uint16_t sizeIptc = 0;
                uint16_t sizeHdr = 0;
                // Find actual Iptc data within the APP13 segment
                if (!locateIptcData(psData.pData_, psData.size_, &record,
                            &sizeHdr, &sizeIptc)) {
                    assert(sizeIptc);
                    if (iptcData_.load(record + sizeHdr, sizeIptc)) throw Error(36, "IPTC");
                }
                --search;
            }
            else if (marker == com_ && comment_.empty())
            {
                if (size < 2) throw Error(15);
                // Jpegs can have multiple comments, but for now only read
                // the first one (most jpegs only have one anyway). Comments
                // are simple single byte ISO-8859-1 strings.
                io_->seek(2-bufRead, BasicIo::cur);
                buf.alloc(size-2);
                io_->read(buf.pData_, size-2);
                if (io_->error() || io_->eof()) throw Error(14);
                comment_.assign(reinterpret_cast<char*>(buf.pData_), size-2);
                while (   comment_.length()
                       && comment_.at(comment_.length()-1) == '\0') {
                    comment_.erase(comment_.length()-1);
                }
                --search;
            }
            else {
                if (size < 2) throw Error(15);
                // Skip the remainder of the unknown segment
                if (io_->seek(size-bufRead, BasicIo::cur)) throw Error(15);
            }
            // Read the beginning of the next segment
            marker = advanceToMarker();
            if (marker < 0) throw Error(15);
        }
    } // JpegBase::readMetadata

    // Operates on raw data (rather than file streams) to simplify reuse
    int JpegBase::locateIptcData(const byte *pPsData,
                                 long sizePsData,
                                 const byte **record,
                                 uint16_t *const sizeHdr,
                                 uint16_t *const sizeIptc) const
    {
        return locate8BimData(pPsData, sizePsData, Photoshop::iptc, record, sizeHdr, sizeIptc);
    }

    void JpegBase::writeMetadata()
    {
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        BasicIo::AutoPtr tempIo(io_->temporary()); // may throw
        assert (tempIo.get() != 0);

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw
    } // JpegBase::writeMetadata

    void JpegBase::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(20);
        if (!outIo.isopen()) throw Error(21);

        // Ensure that this is the correct image type
        if (!isThisType(*io_, true)) {
            if (io_->error() || io_->eof()) throw Error(20);
            throw Error(22);
        }

        const long bufMinSize = 16;
        long bufRead = 0;
        DataBuf buf(bufMinSize);
        const long seek = io_->tell();
        int count = 0;
        int search = 0;
        int insertPos = 0;
        int skipApp1Exif = -1;
        int skipApp13Ps3 = -1;
        int skipCom = -1;
        DataBuf psData;

        // Write image header
        if (writeHeader(outIo)) throw Error(21);

        // Read section marker
        int marker = advanceToMarker();
        if (marker < 0) throw Error(22);

        // First find segments of interest. Normally app0 is first and we want
        // to insert after it. But if app0 comes after com, app1 and app13 then
        // don't bother.
        while (marker != sos_ && marker != eoi_ && search < 3) {
            // Read size and signature (ok if this hits EOF)
            bufRead = io_->read(buf.pData_, bufMinSize);
            if (io_->error()) throw Error(20);
            uint16_t size = getUShort(buf.pData_, bigEndian);

            if (marker == app0_) {
                if (size < 2) throw Error(22);
                insertPos = count + 1;
                if (io_->seek(size-bufRead, BasicIo::cur)) throw Error(22);
            }
            else if (marker == app1_ && memcmp(buf.pData_ + 2, exifId_, 6) == 0) {
                if (size < 8) throw Error(22);
                skipApp1Exif = count;
                ++search;
                if (io_->seek(size-bufRead, BasicIo::cur)) throw Error(22);
            }
            else if (marker == app13_ && memcmp(buf.pData_ + 2, Photoshop::ps3Id, 14) == 0) {
                if (size < 16) throw Error(22);
                skipApp13Ps3 = count;
                ++search;
                // needed if bufMinSize!=16: io_->seek(16-bufRead, BasicIo::cur);
                psData.alloc(size - 16);
                // Load PS data now to allow reinsertion at any point
                io_->read(psData.pData_, psData.size_);
                if (io_->error() || io_->eof()) throw Error(20);
            }
            else if (marker == com_ && skipCom == -1) {
                if (size < 2) throw Error(22);
                // Jpegs can have multiple comments, but for now only handle
                // the first one (most jpegs only have one anyway).
                skipCom = count;
                ++search;
                if (io_->seek(size-bufRead, BasicIo::cur)) throw Error(22);
            }
            else {
                if (size < 2) throw Error(22);
                if (io_->seek(size-bufRead, BasicIo::cur)) throw Error(22);
            }
            marker = advanceToMarker();
            if (marker < 0) throw Error(22);
            ++count;
        }

        if (exifData_.count() > 0) ++search;
        if (iptcData_.count() > 0) ++search;
        if (!comment_.empty()) ++search;

        io_->seek(seek, BasicIo::beg);
        count = 0;
        marker = advanceToMarker();
        if (marker < 0) throw Error(22);

        // To simplify this a bit, new segments are inserts at either the start
        // or right after app0. This is standard in most jpegs, but has the
        // potential to change segment ordering (which is allowed).
        // Segments are erased if there is no assigned metadata.
        while (marker != sos_ && search > 0) {
            // Read size and signature (ok if this hits EOF)
            bufRead = io_->read(buf.pData_, bufMinSize);
            if (io_->error()) throw Error(20);
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
                    if (outIo.write(tmpBuf, 4) != 4) throw Error(21);
                    if (outIo.write((byte*)comment_.data(), (long)comment_.length())
                        != (long)comment_.length()) throw Error(21);
                    if (outIo.putb(0)==EOF) throw Error(21);
                    if (outIo.error()) throw Error(21);
                    --search;
                }
                if (exifData_.count() > 0) {
                    // Write APP1 marker, size of APP1 field, Exif id and Exif data
                    DataBuf rawExif(exifData_.copy());
                    tmpBuf[0] = 0xff;
                    tmpBuf[1] = app1_;
                    us2Data(tmpBuf + 2,
                            static_cast<uint16_t>(rawExif.size_+8),
                            bigEndian);
                    memcpy(tmpBuf + 4, exifId_, 6);
                    if (outIo.write(tmpBuf, 10) != 10) throw Error(21);
                    if (outIo.write(rawExif.pData_, rawExif.size_)
                        != rawExif.size_) throw Error(21);
                    if (outIo.error()) throw Error(21);
                    --search;
                }

                const byte *record = psData.pData_;
                uint16_t sizeIptc = 0;
                uint16_t sizeHdr = 0;
                // Safe to call with zero psData.size_
                locateIptcData(psData.pData_, psData.size_, &record, &sizeHdr, &sizeIptc);

                // Data is rounded to be even
                const int sizeOldData = sizeHdr + sizeIptc + (sizeIptc & 1);
                if (psData.size_ > sizeOldData || iptcData_.count() > 0) {
                    // rawIptc may have size of zero.
                    DataBuf rawIptc(iptcData_.copy());
                    // write app13 marker, new size, and ps3Id
                    tmpBuf[0] = 0xff;
                    tmpBuf[1] = app13_;
                    const int sizeNewData = rawIptc.size_ ?
                            rawIptc.size_ + (rawIptc.size_ & 1) + 12 : 0;
                    us2Data(tmpBuf + 2,
                            static_cast<uint16_t>(psData.size_-sizeOldData+sizeNewData+16),
                            bigEndian);
                    memcpy(tmpBuf + 4, Photoshop::ps3Id, 14);
                    if (outIo.write(tmpBuf, 18) != 18) throw Error(21);
                    if (outIo.error()) throw Error(21);

                    const long sizeFront = (long)(record - psData.pData_);
                    const long sizeEnd = psData.size_ - sizeFront - sizeOldData;
                    // write data before old record.
                    if (outIo.write(psData.pData_, sizeFront) != sizeFront) throw Error(21);

                    // write new iptc record if we have it
                    if (iptcData_.count() > 0) {
                        memcpy(tmpBuf, Photoshop::bimId, 4);
                        us2Data(tmpBuf+4, Photoshop::iptc, bigEndian);
                        tmpBuf[6] = 0;
                        tmpBuf[7] = 0;
                        ul2Data(tmpBuf + 8, rawIptc.size_, bigEndian);
                        if (outIo.write(tmpBuf, 12) != 12) throw Error(21);
                        if (outIo.write(rawIptc.pData_, rawIptc.size_)
                            != rawIptc.size_) throw Error(21);
                        // data is padded to be even (but not included in size)
                        if (rawIptc.size_ & 1) {
                            if (outIo.putb(0)==EOF) throw Error(21);
                        }
                        if (outIo.error()) throw Error(21);
                        --search;
                    }

                    // write existing stuff after record
                    if (outIo.write(record+sizeOldData, sizeEnd)
                        != sizeEnd) throw Error(21);
                    if (outIo.error()) throw Error(21);
                }
            }
            if (marker == eoi_) {
                break;
            }
            else if (skipApp1Exif==count || skipApp13Ps3==count || skipCom==count) {
                --search;
                io_->seek(size-bufRead, BasicIo::cur);
            }
            else {
                if (size < 2) throw Error(22);
                buf.alloc(size+2);
                io_->seek(-bufRead-2, BasicIo::cur);
                io_->read(buf.pData_, size+2);
                if (io_->error() || io_->eof()) throw Error(20);
                if (outIo.write(buf.pData_, size+2) != size+2) throw Error(21);
                if (outIo.error()) throw Error(21);
            }

            // Next marker
            marker = advanceToMarker();
            if (marker < 0) throw Error(22);
            ++count;
        }

        // Copy rest of the Io
        io_->seek(-2, BasicIo::cur);
        buf.alloc(4096);
        long readSize = 0;
        while ((readSize=io_->read(buf.pData_, buf.size_))) {
            if (outIo.write(buf.pData_, readSize) != readSize) throw Error(21);
        }
        if (outIo.error()) throw Error(21);

    } // JpegBase::doWriteMetadata


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

    JpegImage::JpegImage(BasicIo::AutoPtr io, bool create)
        : JpegBase(io, create, blank_, sizeof(blank_))
    {
    }

    int JpegImage::writeHeader(BasicIo& outIo) const
    {
        // Jpeg header
        byte tmpBuf[2];
        tmpBuf[0] = 0xff;
        tmpBuf[1] = soi_;
        if (outIo.write(tmpBuf, 2) != 2) return 4;
        if (outIo.error()) return 4;
        return 0;
    }

    bool JpegImage::isThisType(BasicIo& iIo, bool advance) const
    {
        return isJpegType(iIo, advance);
    }

    Image::AutoPtr newJpegInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image = Image::AutoPtr(new JpegImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isJpegType(BasicIo& iIo, bool advance)
    {
        bool result = true;
        byte tmpBuf[2];
        iIo.read(tmpBuf, 2);
        if (iIo.error() || iIo.eof()) return false;

        if (0xff!=tmpBuf[0] || JpegImage::soi_!=tmpBuf[1]) {
            result = false;
        }
        if (!advance || !result ) iIo.seek(-2, BasicIo::cur);
        return result;
    }

    const char ExvImage::exiv2Id_[] = "Exiv2";
    const byte ExvImage::blank_[] = { 0xff,0x01,'E','x','i','v','2',0xff,0xd9 };

    ExvImage::ExvImage(BasicIo::AutoPtr io, bool create)
        : JpegBase(io, create, blank_, sizeof(blank_))
    {
    }

    int ExvImage::writeHeader(BasicIo& outIo) const
    {
        // Exv header
        byte tmpBuf[7];
        tmpBuf[0] = 0xff;
        tmpBuf[1] = 0x01;
        memcpy(tmpBuf + 2, exiv2Id_, 5);
        if (outIo.write(tmpBuf, 7) != 7) return 4;
        if (outIo.error()) return 4;
        return 0;
    }

    bool ExvImage::isThisType(BasicIo& iIo, bool advance) const
    {
        return isExvType(iIo, advance);
    }

    Image::AutoPtr newExvInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image;
        if (create) {
            image = Image::AutoPtr(new ExvImage(io, true));
        }
        else {
            image = Image::AutoPtr(new ExvImage(io, false));
        }
        if (!image->good()) image.reset();
        return image;
    }

    bool isExvType(BasicIo& iIo, bool advance)
    {
        bool result = true;
        byte tmpBuf[7];
        iIo.read(tmpBuf, 7);
        if (iIo.error() || iIo.eof()) return false;

        if (   0xff != tmpBuf[0] || 0x01 != tmpBuf[1]
            || memcmp(tmpBuf + 2, ExvImage::exiv2Id_, 5) != 0) {
            result = false;
        }
        if (!advance || !result ) iIo.seek(-7, BasicIo::cur);
        return result;
    }

    // Todo: Generalised from JpegBase::locateIptcData without really understanding
    //       the format (in particular the header). So it remains to be confirmed 
    //       if this also makes sense for psTag != Photoshop::iptc
    int locate8BimData(const byte *pPsData,
                       long sizePsData,
                       uint16_t psTag,
                       const byte **record,
                       uint16_t *const sizeHdr,
                       uint16_t *const sizeData)
    {
        assert(record);
        assert(sizeHdr);
        assert(sizeData);
        // Used for error checking
        long position = 0;

        // Data should follow Photoshop format, if not exit
        while (   position <= sizePsData - 14
               && memcmp(pPsData + position, Photoshop::bimId, 4) == 0) {
            const byte *hrd = pPsData + position;
            position += 4;
            uint16_t type = getUShort(pPsData + position, bigEndian);
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

            if (type == psTag) {
                *sizeData = static_cast<uint16_t>(dataSize);
                *sizeHdr = psSize + 10;
                *record = hrd;
                return 0;
            }
            position += dataSize + (dataSize & 1);
        }
        return 3;
    } // locate8BimData

}                                       // namespace Exiv2
