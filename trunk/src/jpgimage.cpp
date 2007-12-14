// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2007 Andreas Huggel <ahuggel@gmx.net>
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
EXIV2_RCSID("@(#) $Id$")

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

    const byte     JpegBase::sos_      = 0xda;
    const byte     JpegBase::eoi_      = 0xd9;
    const byte     JpegBase::app0_     = 0xe0;
    const byte     JpegBase::app1_     = 0xe1;
    const byte     JpegBase::app13_    = 0xed;
    const byte     JpegBase::com_      = 0xfe;
    const char     JpegBase::exifId_[] = "Exif\0\0";
    const char     JpegBase::jfifId_[] = "JFIF\0";
    const char     JpegBase::xmpId_[]  = "http://ns.adobe.com/xap/1.0/\0";

    const char     Photoshop::ps3Id_[] = "Photoshop 3.0\0";
    const char     Photoshop::bimId_[] = "8BIM";
    const uint16_t Photoshop::iptc_    = 0x0404;

    // Todo: Generalised from JpegBase::locateIptcData without really understanding
    //       the format (in particular the header). So it remains to be confirmed
    //       if this also makes sense for psTag != Photoshop::iptc
    int Photoshop::locateIrb(const byte*     pPsData,
                             long            sizePsData,
                             uint16_t        psTag,
                             const byte**    record,
                             uint32_t *const sizeHdr,
                             uint32_t *const sizeData)
    {
        assert(record);
        assert(sizeHdr);
        assert(sizeData);
        // Used for error checking
        long position = 0;
#ifdef DEBUG
        std::cerr << "Photoshop::locateIrb: ";
#endif
        // Data should follow Photoshop format, if not exit
        while (   position <= sizePsData - 14
               && memcmp(pPsData + position, Photoshop::bimId_, 4) == 0) {
            const byte *hrd = pPsData + position;
            position += 4;
            uint16_t type = getUShort(pPsData + position, bigEndian);
            position += 2;
#ifdef DEBUG
            std::cerr << "0x" << std::hex << type << std::dec << " ";
#endif
            // Pascal string is padded to have an even size (including size byte)
            byte psSize = pPsData[position] + 1;
            psSize += (psSize & 1);
            position += psSize;
            if (position + 4 > sizePsData) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Invalid Photoshop IRB\n";
#endif
                return -2;
            }
            uint32_t dataSize = getULong(pPsData + position, bigEndian);
            position += 4;
            if (dataSize > static_cast<uint32_t>(sizePsData - position)) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Invalid Photoshop IRB data size "
                          << dataSize << "\n";
#endif
                return -2;
            }
#ifndef DEBUG
            if (   (dataSize & 1) 
                && position + dataSize == static_cast<uint32_t>(sizePsData)) {
                std::cerr << "Warning: "
                          << "Photoshop IRB data is not padded to even size\n";
            }
#endif
            if (type == psTag) {
#ifdef DEBUG
                std::cerr << "ok\n";
#endif
                *sizeData = dataSize;
                *sizeHdr = psSize + 10;
                *record = hrd;
                return 0;
            }
            // Data size is also padded to be even
            position += dataSize + (dataSize & 1);
        }
#ifdef DEBUG
        std::cerr << "pPsData doesn't start with '8BIM'\n";
#endif
        return 3;
    } // Photoshop::locateIrb

    int Photoshop::locateIptcIrb(const byte*     pPsData,
                                 long            sizePsData,
                                 const byte**    record,
                                 uint32_t *const sizeHdr,
                                 uint32_t *const sizeData)
    {
        return locateIrb(pPsData, sizePsData, iptc_,
                         record, sizeHdr, sizeData);
    }

    DataBuf Photoshop::setIptcIrb(const byte*     pPsData,
                                  long            sizePsData,
                                  const IptcData& iptcData)
    {
        if (sizePsData > 0) assert(pPsData);
#ifdef DEBUG
        std::cerr << "IRB block at the beginning of Photoshop::setIptcIrb\n";
        if (sizePsData == 0) std::cerr << "  None.\n";
        else hexdump(std::cerr, pPsData, sizePsData);
#endif
        const byte* record    = pPsData;
        uint32_t    sizeIptc  = 0;
        uint32_t    sizeHdr   = 0;
        DataBuf rc;
        // Safe to call with zero psData.size_
        if (0 > Photoshop::locateIptcIrb(pPsData, sizePsData,
                                         &record, &sizeHdr, &sizeIptc)) {
            return rc;
        }
        Blob psBlob;
        const uint32_t sizeFront = static_cast<uint32_t>(record - pPsData);
        // Write data before old record.
        if (sizePsData > 0 && sizeFront > 0) {
            append(psBlob, pPsData, sizeFront);
        }
        // Write new iptc record if we have it
        DataBuf rawIptc(iptcData.copy());
        if (rawIptc.size_ > 0) {
            byte tmpBuf[12];
            std::memcpy(tmpBuf, Photoshop::bimId_, 4);
            us2Data(tmpBuf + 4, iptc_, bigEndian);
            tmpBuf[6] = 0;
            tmpBuf[7] = 0;
            ul2Data(tmpBuf + 8, rawIptc.size_, bigEndian);
            append(psBlob, tmpBuf, 12);
            append(psBlob, rawIptc.pData_, rawIptc.size_);
            // Data is padded to be even (but not included in size)
            if (rawIptc.size_ & 1) psBlob.push_back(0x00);
        }
        // Write existing stuff after record, data is rounded to be even.
        const uint32_t sizeOldData = sizeHdr + sizeIptc + (sizeIptc & 1);
        // Note: Because of the rounding, sizeFront + sizeOldData can be 
        // _greater_ than sizePsData by 1 (not just equal), if the original
        // data was not padded.
        if (static_cast<uint32_t>(sizePsData) > sizeFront + sizeOldData) {
            append(psBlob, record + sizeOldData,
                   sizePsData - sizeFront - sizeOldData);
        }
        if (psBlob.size() > 0) rc = DataBuf(&psBlob[0], static_cast<long>(psBlob.size()));
#ifdef DEBUG
        std::cerr << "IRB block at the end of Photoshop::setIptcIrb\n";
        if (rc.size_ == 0) std::cerr << "  None.\n";
        else hexdump(std::cerr, rc.pData_, rc.size_);
#endif
        return rc;

    } // Photoshop::setIptcIrb

    JpegBase::JpegBase(int type, BasicIo::AutoPtr io, bool create,
                       const byte initData[], long dataSize)
        : Image(type, mdExif | mdIptc | mdComment, io)
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

    int JpegBase::advanceToMarker() const
    {
        int c = -1;
        // Skips potential padding between markers
        while ((c=io_->getb()) != 0xff) {
            if (c == EOF) return -1;
        }

        // Markers can start with any number of 0xff
        while ((c=io_->getb()) == 0xff) {
            if (c == EOF) return -2;
        }
        return c;
    }

    void JpegBase::readMetadata()
    {
        int rc = 0; // Todo: this should be the return value

        if (io_->open() != 0) throw Error(9, io_->path(), strError());
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isThisType(*io_, true)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(15);
        }
        clearMetadata();
        int search = 4;
        const long bufMinSize = 36;
        long bufRead = 0;
        DataBuf buf(bufMinSize);
        Blob iptcBlob;
        bool foundPsData = false;

        // Read section marker
        int marker = advanceToMarker();
        if (marker < 0) throw Error(15);

        while (marker != sos_ && marker != eoi_ && search > 0) {
            // Read size and signature (ok if this hits EOF)
            std::memset(buf.pData_, 0x0, buf.size_);
            bufRead = io_->read(buf.pData_, bufMinSize);
            if (io_->error()) throw Error(14);
            if (bufRead < 2) throw Error(15);
            uint16_t size = getUShort(buf.pData_, bigEndian);

            if (foundPsData && marker != app13_) {
                // For IPTC, decrement search only after all app13 segments are
                // loaded, assuming they all appear in sequence. But decode IPTC
                // data after the loop, in case an app13 is the last segment
                // before sos or eoi.
                foundPsData = false;
                if (--search == 0) break;
            }

            if (marker == app1_ && memcmp(buf.pData_ + 2, exifId_, 6) == 0) {
                if (size < 8) {
                    rc = 1;
                    break;
                }
                // Seek to beginning and read the Exif data
                io_->seek(8 - bufRead, BasicIo::cur);
                DataBuf rawExif(size - 8);
                io_->read(rawExif.pData_, rawExif.size_);
                if (io_->error() || io_->eof()) throw Error(14);
                if (exifData_.load(rawExif.pData_, rawExif.size_)) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: Failed to decode Exif metadata.\n";
#endif
                    exifData_.clear();
                }
                --search;
            }
            else if (marker == app1_ && memcmp(buf.pData_ + 2, xmpId_, 29) == 0) {
                if (size < 31) {
                    rc = 6;
                    break;
                }
                // Seek to beginning and read the XMP packet
                io_->seek(31 - bufRead, BasicIo::cur);
                DataBuf xmpPacket(size - 31);
                io_->read(xmpPacket.pData_, xmpPacket.size_);
                if (io_->error() || io_->eof()) throw Error(14);
                xmpPacket_.assign(reinterpret_cast<char*>(xmpPacket.pData_), xmpPacket.size_);
                if (xmpPacket_.size() > 0 && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: Failed to decode XMP metadata.\n";
#endif
                }
                --search;
            }
            else if (   marker == app13_
                     && memcmp(buf.pData_ + 2, Photoshop::ps3Id_, 14) == 0) {
                if (size < 16) {
                    rc = 2;
                    break;
                }
                // Read the rest of the APP13 segment
                io_->seek(16 - bufRead, BasicIo::cur);
                DataBuf psData(size - 16);
                io_->read(psData.pData_, psData.size_);
                if (io_->error() || io_->eof()) throw Error(14);
                const byte *record = 0;
                uint32_t sizeIptc = 0;
                uint32_t sizeHdr = 0;
#ifdef DEBUG
                std::cerr << "Found app13 segment, size = " << size << "\n";
                //hexdump(std::cerr, psData.pData_, psData.size_);
#endif
                // Find actual IPTC data within the APP13 segment
                const byte* pEnd = psData.pData_ + psData.size_;
                const byte* pCur = psData.pData_;
                while (   pCur < pEnd
                       && 0 == Photoshop::locateIptcIrb(pCur,
                                                        pEnd - pCur,
                                                        &record,
                                                        &sizeHdr,
                                                        &sizeIptc)) {
                    if (sizeIptc) {
#ifdef DEBUG
                        std::cerr << "Found IPTC IRB, size = " << sizeIptc << "\n";
#endif
                        append(iptcBlob, record + sizeHdr, sizeIptc);
                    }
                    pCur = record + sizeHdr + sizeIptc;
                    pCur += (sizeIptc & 1);
                }
                foundPsData = true;
            }
            else if (marker == com_ && comment_.empty())
            {
                if (size < 2) {
                    rc = 3;
                    break;
                }
                // JPEGs can have multiple comments, but for now only read
                // the first one (most jpegs only have one anyway). Comments
                // are simple single byte ISO-8859-1 strings.
                io_->seek(2 - bufRead, BasicIo::cur);
                DataBuf comment(size - 2);
                io_->read(comment.pData_, comment.size_);
                if (io_->error() || io_->eof()) throw Error(14);
                comment_.assign(reinterpret_cast<char*>(comment.pData_), comment.size_);
                while (   comment_.length()
                       && comment_.at(comment_.length()-1) == '\0') {
                    comment_.erase(comment_.length()-1);
                }
                --search;
            }
            else {
                if (size < 2) {
                    rc = 4;
                    break;
                }
                // Skip the remainder of the unknown segment
                if (io_->seek(size-bufRead, BasicIo::cur)) throw Error(14);
            }
            // Read the beginning of the next segment
            marker = advanceToMarker();
            if (marker < 0) {
                rc = 5;
                break;
            }
        } // while there are segments to process

        if (   iptcBlob.size() > 0
            && iptcData_.load(&iptcBlob[0], static_cast<long>(iptcBlob.size()))) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to decode IPTC metadata.\n";
#endif
            iptcData_.clear();
        }

        if (rc != 0) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: JPEG format error, rc = " << rc << "\n";
#endif
        }
    } // JpegBase::readMetadata

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

        const long bufMinSize = 36;
        long bufRead = 0;
        DataBuf buf(bufMinSize);
        const long seek = io_->tell();
        int count = 0;
        int search = 0;
        int insertPos = 0;
        int skipApp1Exif = -1;
        int skipApp1Xmp = -1;
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
        while (marker != sos_ && marker != eoi_ && search < 4) {
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
            else if (marker == app1_ && memcmp(buf.pData_ + 2, xmpId_, 29) == 0) {
                if (size < 31) throw Error(22);
                skipApp1Xmp = count;
                ++search;
                if (io_->seek(size-bufRead, BasicIo::cur)) throw Error(22);
            }
            else if (marker == app13_ && memcmp(buf.pData_ + 2, Photoshop::ps3Id_, 14) == 0) {
#ifdef DEBUG
                std::cerr << "Found APP13 Photoshop PS3 segment\n";
#endif
                if (size < 16) throw Error(22);
                skipApp13Ps3 = count;
                ++search;
                io_->seek(16 - bufRead, BasicIo::cur);
                psData.alloc(size - 16);
                // Load PS data now to allow reinsertion at any point
                io_->read(psData.pData_, size - 16);
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
        if (writeXmpFromPacket() == false && xmpData_.count() > 0) ++search;
        if (writeXmpFromPacket() == true && xmpPacket_.size() > 0) ++search;
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
                byte tmpBuf[64];
                if (!comment_.empty()) {
                    // Write COM marker, size of comment, and string
                    tmpBuf[0] = 0xff;
                    tmpBuf[1] = com_;

                    if (comment_.length() + 3 > 0xffff) throw Error(37, "JPEG comment");
                    us2Data(tmpBuf + 2, static_cast<uint16_t>(comment_.length() + 3), bigEndian);

                    if (outIo.write(tmpBuf, 4) != 4) throw Error(21);
                    if (outIo.write((byte*)comment_.data(), (long)comment_.length())
                        != (long)comment_.length()) throw Error(21);
                    if (outIo.putb(0)==EOF) throw Error(21);
                    if (outIo.error()) throw Error(21);
                    --search;
                }
                if (exifData_.count() > 0) {
                    DataBuf rawExif = exifData_.copy();
                    if (rawExif.size_ > 0) {
                        // Write APP1 marker, size of APP1 field, Exif id and Exif data
                        tmpBuf[0] = 0xff;
                        tmpBuf[1] = app1_;

                        if (rawExif.size_ + 8 > 0xffff) throw Error(37, "Exif");
                        us2Data(tmpBuf + 2, static_cast<uint16_t>(rawExif.size_ + 8), bigEndian);
                        std::memcpy(tmpBuf + 4, exifId_, 6);
                        if (outIo.write(tmpBuf, 10) != 10) throw Error(21);

                        // Write new Exif data buffer
                        if (   outIo.write(rawExif.pData_, rawExif.size_)
                            != rawExif.size_) throw Error(21);
                        if (outIo.error()) throw Error(21);
                        --search;
                    }
                }
                if (writeXmpFromPacket() == false) {
                    if (XmpParser::encode(xmpPacket_, xmpData_)) {
#ifndef SUPPRESS_WARNINGS
                        std::cerr << "Error: Failed to encode XMP metadata.\n";
#endif
                    }
                }
                if (xmpPacket_.size() > 0) {
                    // Write APP1 marker, size of APP1 field, XMP id and XMP packet
                    tmpBuf[0] = 0xff;
                    tmpBuf[1] = app1_;

                    if (xmpPacket_.size() + 31 > 0xffff) throw Error(37, "XMP");
                    us2Data(tmpBuf + 2, static_cast<uint16_t>(xmpPacket_.size() + 31), bigEndian);
                    std::memcpy(tmpBuf + 4, xmpId_, 29);
                    if (outIo.write(tmpBuf, 33) != 33) throw Error(21);

                    // Write new XMP packet
                    if (   outIo.write(reinterpret_cast<const byte*>(xmpPacket_.data()), static_cast<long>(xmpPacket_.size()))
                        != static_cast<long>(xmpPacket_.size())) throw Error(21);
                    if (outIo.error()) throw Error(21);
                    --search;
                }
                if (psData.size_ > 0 || iptcData_.count() > 0) {
                    // Set the new IPTC IRB, keeps existing IRBs but removes the
                    // IPTC block if there is no new IPTC data to write
                    DataBuf newPsData = Photoshop::setIptcIrb(psData.pData_,
                                                              psData.size_,
                                                              iptcData_);
                    if (newPsData.size_ > 0) {
                        // Write APP13 marker, new size, and ps3Id
                        tmpBuf[0] = 0xff;
                        tmpBuf[1] = app13_;

                        if (newPsData.size_ + 16 > 0xffff) throw Error(37, "IPTC");
                        us2Data(tmpBuf + 2, static_cast<uint16_t>(newPsData.size_ + 16), bigEndian);
                        std::memcpy(tmpBuf + 4, Photoshop::ps3Id_, 14);
                        if (outIo.write(tmpBuf, 18) != 18) throw Error(21);
                        if (outIo.error()) throw Error(21);

                        // Write new Photoshop IRB data buffer
                        if (   outIo.write(newPsData.pData_, newPsData.size_)
                            != newPsData.size_) throw Error(21);
                        if (outIo.error()) throw Error(21);
                    }
                    if (iptcData_.count() > 0) {
                        --search;
                    }
                }
            }
            if (marker == eoi_) {
                break;
            }
            else if (   skipApp1Exif == count
                     || skipApp1Xmp  == count
                     || skipApp13Ps3 == count
                     || skipCom      == count) {
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
        : JpegBase(ImageType::jpeg, io, create, blank_, sizeof(blank_))
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
        Image::AutoPtr image(new JpegImage(io, create));
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

        if (0xff != tmpBuf[0] || JpegImage::soi_ != tmpBuf[1]) {
            result = false;
        }
        if (!advance || !result ) iIo.seek(-2, BasicIo::cur);
        return result;
    }

    const char ExvImage::exiv2Id_[] = "Exiv2";
    const byte ExvImage::blank_[] = { 0xff,0x01,'E','x','i','v','2',0xff,0xd9 };

    ExvImage::ExvImage(BasicIo::AutoPtr io, bool create)
        : JpegBase(ImageType::exv, io, create, blank_, sizeof(blank_))
    {
    }

    int ExvImage::writeHeader(BasicIo& outIo) const
    {
        // Exv header
        byte tmpBuf[7];
        tmpBuf[0] = 0xff;
        tmpBuf[1] = 0x01;
        std::memcpy(tmpBuf + 2, exiv2Id_, 5);
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

}                                       // namespace Exiv2
