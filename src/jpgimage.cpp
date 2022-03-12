// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "config.h"

#include "jpgimage.hpp"
#include "image_int.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "helper_functions.hpp"
#include "enforce.hpp"
#include "safe_op.hpp"

#ifdef WIN32
#include <windows.h>
#else
#define BYTE   char
#define USHORT uint16_t
#define ULONG  uint32_t
#endif

#include "fff.h"

#include <iostream>

// *****************************************************************************
// class member definitions

namespace Exiv2 {
    static inline bool inRange(int lo,int value, int hi)
    {
        return lo<=value && value <= hi;
    }

    static inline bool inRange2(int value,int lo1,int hi1, int lo2,int hi2)
    {
        return inRange(lo1,value,hi1) || inRange(lo2,value,hi2);
    }

    bool Photoshop::isIrb(const byte* pPsData, size_t sizePsData)
    {
        if (sizePsData < 4)
            return false;
        for (auto&& i : irbId_) {
            assert(strlen(i) == 4);
            if (memcmp(pPsData, i, 4) == 0)
                return true;
        }
        return false;
    }

    bool Photoshop::valid(const byte* pPsData, size_t sizePsData)
    {
        const byte* record = nullptr;
        uint32_t sizeIptc = 0;
        uint32_t sizeHdr = 0;
        const byte* pCur = pPsData;
        const byte* pEnd = pPsData + sizePsData;
        int ret = 0;
        while (pCur < pEnd
               && 0 == (ret = Photoshop::locateIptcIrb(pCur, (pEnd - pCur), &record, &sizeHdr, &sizeIptc))) {
            pCur = record + sizeHdr + sizeIptc + (sizeIptc & 1);
        }
        return ret >= 0;
    }

    // Todo: Generalised from JpegBase::locateIptcData without really understanding
    //       the format (in particular the header). So it remains to be confirmed
    //       if this also makes sense for psTag != Photoshop::iptc
    int Photoshop::locateIrb(const byte*     pPsData,
                             size_t          sizePsData,
                             uint16_t        psTag,
                             const byte**    record,
                             uint32_t *const sizeHdr,
                             uint32_t *const sizeData)
    {
        assert(record);
        assert(sizeHdr);
        assert(sizeData);
        if (sizePsData < 12) {
            return 3;
        }

        // Used for error checking
        size_t position = 0;
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Photoshop::locateIrb: ";
#endif
        // Data should follow Photoshop format, if not exit
        while (position <= sizePsData - 12 && isIrb(pPsData + position, 4)) {
            const byte *hrd = pPsData + position;
            position += 4;
            uint16_t type = getUShort(pPsData + position, bigEndian);
            position += 2;
#ifdef EXIV2_DEBUG_MESSAGES
            std::cerr << "0x" << std::hex << type << std::dec << " ";
#endif
            // Pascal string is padded to have an even size (including size byte)
            byte psSize = pPsData[position] + 1;
            psSize += (psSize & 1);
            position += psSize;
            if (position + 4 > sizePsData) {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Warning: "
                          << "Invalid or extended Photoshop IRB\n";
#endif
                return -2;
            }
            uint32_t dataSize = getULong(pPsData + position, bigEndian);
            position += 4;
            if (dataSize > (sizePsData - position)) {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Warning: "
                          << "Invalid Photoshop IRB data size "
                          << dataSize << " or extended Photoshop IRB\n";
#endif
                return -2;
            }
#ifdef EXIV2_DEBUG_MESSAGES
            if (   (dataSize & 1) && position + dataSize == sizePsData) {
                std::cerr << "Warning: "
                          << "Photoshop IRB data is not padded to even size\n";
            }
#endif
            if (type == psTag) {
#ifdef EXIV2_DEBUG_MESSAGES
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
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "pPsData doesn't start with '8BIM'\n";
#endif
        if (position < sizePsData) {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cerr << "Warning: "
                      << "Invalid or extended Photoshop IRB\n";
#endif
            return -2;
        }
        return 3;
    }

    int Photoshop::locateIptcIrb(const byte*     pPsData,
                                 size_t sizePsData,
                                 const byte**    record,
                                 uint32_t *const sizeHdr,
                                 uint32_t *const sizeData)
    {
        return locateIrb(pPsData, sizePsData, iptc_,
                         record, sizeHdr, sizeData);
    }

    int Photoshop::locatePreviewIrb(const byte*     pPsData,
                                    size_t sizePsData,
                                    const byte**    record,
                                    uint32_t *const sizeHdr,
                                    uint32_t *const sizeData)
    {
        return locateIrb(pPsData, sizePsData, preview_,
                         record, sizeHdr, sizeData);
    }

    DataBuf Photoshop::setIptcIrb(const byte* pPsData, size_t sizePsData, const IptcData& iptcData)
    {
        if (sizePsData > 0)
            assert(pPsData);
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "IRB block at the beginning of Photoshop::setIptcIrb\n";
        if (sizePsData == 0)
            std::cerr << "  None.\n";
        else hexdump(std::cerr, pPsData, sizePsData);
#endif
        const byte* record    = pPsData;
        uint32_t    sizeIptc  = 0;
        uint32_t    sizeHdr   = 0;
        DataBuf rc;
        // Safe to call with zero psData.size_
        if (0 > Photoshop::locateIptcIrb(pPsData, sizePsData, &record, &sizeHdr, &sizeIptc)) {
            return rc;
        }
        Blob psBlob;
        const auto sizeFront = static_cast<size_t>(record - pPsData);
        // Write data before old record.
        if (sizePsData > 0 && sizeFront > 0) {
            append(psBlob, pPsData, sizeFront);
        }
        // Write new iptc record if we have it
        DataBuf rawIptc = IptcParser::encode(iptcData);
        if (!rawIptc.empty()) {
            std::array<byte, 12> tmpBuf;
            std::copy_n(Photoshop::irbId_[0], 4, tmpBuf.data());
            us2Data(tmpBuf.data() + 4, iptc_, bigEndian);
            tmpBuf[6] = 0;
            tmpBuf[7] = 0;
            ul2Data(tmpBuf.data() + 8, static_cast<uint32_t>(rawIptc.size()), bigEndian);
            append(psBlob, tmpBuf.data(), 12);
            append(psBlob, rawIptc.c_data(), rawIptc.size());
            // Data is padded to be even (but not included in size)
            if (rawIptc.size() & 1)
                psBlob.push_back(0x00);
        }
        // Write existing stuff after record,
        // skip the current and all remaining IPTC blocks
        size_t pos = sizeFront;
        while (0 == Photoshop::locateIptcIrb(pPsData + pos, sizePsData - pos, &record, &sizeHdr, &sizeIptc)) {
            const auto newPos = static_cast<size_t>(record - pPsData);
            // Copy data up to the IPTC IRB
            if (newPos > pos) {
                append(psBlob, pPsData + pos, newPos - pos);
            }
            // Skip the IPTC IRB
            pos = newPos + sizeHdr + sizeIptc + (sizeIptc & 1);
        }
        if (pos < sizePsData) {
            append(psBlob, pPsData + pos, sizePsData - pos);
        }
        // Data is rounded to be even
        if (!psBlob.empty())
            rc = DataBuf(&psBlob[0], psBlob.size());
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "IRB block at the end of Photoshop::setIptcIrb\n";
        if (rc.empty())
            std::cerr << "  None.\n";
        else hexdump(std::cerr, rc.c_data(), rc.size());
#endif
        return rc;

    }

    bool JpegBase::markerHasLength(byte marker) {
        return (marker >= sof0_ && marker <= sof15_) ||
               (marker >= app0_ && marker <= (app0_ | 0x0F)) ||
               marker == dht_ ||
               marker == dqt_ ||
               marker == dri_ ||
               marker == com_ ||
               marker == sos_;
    }

    JpegBase::JpegBase(ImageType type, BasicIo::UniquePtr io, bool create,
                       const byte initData[], size_t dataSize)
        : Image(type, mdExif | mdIptc | mdXmp | mdComment, std::move(io))
    {
        if (create) {
            initImage(initData, dataSize);
        }
    }

    int JpegBase::initImage(const byte initData[], size_t dataSize)
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

    byte JpegBase::advanceToMarker(ErrorCode err) const
    {
        int c = -1;
        // Skips potential padding between markers
        while ((c=io_->getb()) != 0xff) {
            if (c == EOF)
                throw Error(err);
        }

        // Markers can start with any number of 0xff
        while ((c=io_->getb()) == 0xff) {
        }
        if (c == EOF)
            throw Error(err);

        return static_cast<byte>(c);
    }

    void JpegBase::readMetadata()
    {
        int rc = 0; // Todo: this should be the return value

        if (io_->open() != 0) throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isThisType(*io_, true)) {
            if (io_->error() || io_->eof()) throw Error(ErrorCode::kerFailedToReadImageData);
            throw Error(ErrorCode::kerNotAJpeg);
        }
        clearMetadata();
        int search = 6 ; // Exif, ICC, XMP, Comment, IPTC, SOF
        Blob psBlob;
        bool foundCompletePsData = false;
        bool foundExifData = false;
        bool foundXmpData = false;
        bool foundIccData = false;

        // Read section marker
        byte marker = advanceToMarker(ErrorCode::kerNotAJpeg);

        while (marker != sos_ && marker != eoi_ && search > 0) {
            // 2-byte buffer for reading the size.
            byte sizebuf[2];
            uint16_t size = 0; // Size of the segment, including the 2-byte size field
            if (markerHasLength(marker)) {
                io_->readOrThrow(sizebuf, 2, ErrorCode::kerFailedToReadImageData);
                size = getUShort(sizebuf, bigEndian);
                enforce(size >= 2, ErrorCode::kerFailedToReadImageData);
            }

            // Read the rest of the segment.
            DataBuf buf(size);
            /// \todo check if it makes sense to check for size
            if (size > 0) {
                io_->readOrThrow(buf.data(2), size - 2, ErrorCode::kerFailedToReadImageData);
                buf.copyBytes(0, sizebuf, 2);
            }

            if (   !foundExifData
                && marker == app1_
                && size >= 8  // prevent out-of-bounds read in memcmp on next line
                && buf.cmpBytes(2, exifId_, 6) == 0) {
                ByteOrder bo = ExifParser::decode(exifData_, buf.c_data(8), size - 8);
                setByteOrder(bo);
                if (size > 8 && byteOrder() == invalidByteOrder) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode Exif metadata.\n";
#endif
                    exifData_.clear();
                }
                --search;
                foundExifData = true;
            }
            else if (   !foundXmpData
                     && marker == app1_
                     && size >= 31  // prevent out-of-bounds read in memcmp on next line
                     && buf.cmpBytes(2, xmpId_, 29) == 0) {
                xmpPacket_.assign(buf.c_str(31), size - 31);
                if (!xmpPacket_.empty() && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
                }
                --search;
                foundXmpData = true;
            }
            else if (   !foundCompletePsData
                     && marker == app13_
                     && size >= 16  // prevent out-of-bounds read in memcmp on next line
                     && buf.cmpBytes(2, Photoshop::ps3Id_, 14) == 0) {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Found app13 segment, size = " << size << "\n";
#endif
                // Append to psBlob
                append(psBlob, buf.c_data(16), size - 16);
                // Check whether psBlob is complete
                if (!psBlob.empty() && Photoshop::valid(&psBlob[0], psBlob.size())) {
                    --search;
                    foundCompletePsData = true;
                }
            }
            else if (marker == com_ && comment_.empty())
            {
                // JPEGs can have multiple comments, but for now only read
                // the first one (most jpegs only have one anyway). Comments
                // are simple single byte ISO-8859-1 strings.
                comment_.assign(buf.c_str(2), size - 2);
                while (   comment_.length()
                       && comment_.at(comment_.length()-1) == '\0') {
                    comment_.erase(comment_.length()-1);
                }
                --search;
            }
            else if (   marker == app2_
                     && size >= 13  // prevent out-of-bounds read in memcmp on next line
                     && buf.cmpBytes(2, iccId_,11)==0) {
                if (size < 2+14+4) {
                    rc = 8;
                    break;
                }
                // ICC profile
                if ( ! foundIccData  ) {
                    foundIccData = true ;
                    --search ;
                }
                auto chunk = static_cast<int>(buf.read_uint8(2 + 12));
                auto chunks = static_cast<int>(buf.read_uint8(2 + 13));
                // ICC1v43_2010-12.pdf header is 14 bytes
                // header = "ICC_PROFILE\0" (12 bytes)
                // chunk/chunks are a single byte
                // Spec 7.2 Profile bytes 0-3 size
                uint32_t s = buf.read_uint32(2+14, bigEndian);
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Found ICC Profile chunk " << chunk
                          << " of "    << chunks
                          << (chunk==1 ? " size: " : "" ) << (chunk==1 ? s : 0)
                          << std::endl  ;
#endif
                // #1286 profile can be padded
                size_t icc_size = size-2-14;
                if (chunk==1 && chunks==1) {
                  enforce(s <= static_cast<uint32_t>(icc_size), ErrorCode::kerInvalidIccProfile);
                  icc_size = s;
                }

                DataBuf profile(Safe::add(iccProfile_.size(), icc_size));
                if (!iccProfile_.empty()) {
                    profile.copyBytes(0, iccProfile_.c_data(), iccProfile_.size());
                }
                profile.copyBytes(iccProfile_.size(), buf.c_data(2+14), icc_size);
                setIccProfile(std::move(profile),chunk==chunks);
            }
            else if (  pixelHeight_ == 0 && inRange2(marker,sof0_,sof3_,sof5_,sof15_) ) {
                // We hit a SOFn (start-of-frame) marker
                if (size < 8) {
                    rc = 7;
                    break;
                }
                pixelHeight_ = buf.read_uint16(3, bigEndian);
                pixelWidth_ = buf.read_uint16(5, bigEndian);
                if (pixelHeight_ != 0) --search;
            }

            // Read the beginning of the next segment
            try {
                marker = advanceToMarker(ErrorCode::kerFailedToReadImageData);
            } catch (Error&) {
                rc = 5;
                break;
            }
        } // while there are segments to process

        if (!psBlob.empty()) {
            // Find actual IPTC data within the psBlob
            Blob iptcBlob;
            const byte* record = nullptr;
            uint32_t sizeIptc = 0;
            uint32_t sizeHdr = 0;
            const byte* pCur = &psBlob[0];
            const byte* pEnd = pCur + psBlob.size();
            while (   pCur < pEnd
                   && 0 == Photoshop::locateIptcIrb(pCur, pEnd - pCur, &record, &sizeHdr, &sizeIptc)) {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Found IPTC IRB, size = " << sizeIptc << "\n";
#endif
                if (sizeIptc) {
                    append(iptcBlob, record + sizeHdr, sizeIptc);
                }
                pCur = record + sizeHdr + sizeIptc + (sizeIptc & 1);
            }
            if (!iptcBlob.empty() &&
                IptcParser::decode(iptcData_, &iptcBlob[0], iptcBlob.size())) {
#ifndef SUPPRESS_WARNINGS
                EXV_WARNING << "Failed to decode IPTC metadata.\n";
#endif
                iptcData_.clear();
            }
        }

        if (rc != 0) {
#ifndef SUPPRESS_WARNINGS
            EXV_WARNING << "JPEG format error, rc = " << rc << "\n";
#endif
        }
    } // JpegBase::readMetadata

#define REPORT_MARKER if ( (option == kpsBasic||option == kpsRecursive) ) \
     out << Internal::stringFormat("%8ld | 0xff%02x %-5s", \
                             io_->tell()-2,marker,nm[marker].c_str())

    void JpegBase::printStructure(std::ostream& out, PrintStructureOption option, int depth)
    {
        if (io_->open() != 0)
            throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
        // Ensure that this is the correct image type
        if (!isThisType(*io_, false)) {
            if (io_->error() || io_->eof())
                throw Error(ErrorCode::kerFailedToReadImageData);
            throw Error(ErrorCode::kerNotAJpeg);
        }

        bool bPrint = option == kpsBasic || option == kpsRecursive;
        std::vector<long> iptcDataSegs;

        if (bPrint || option == kpsXMP || option == kpsIccProfile || option == kpsIptcErase) {
            // mnemonic for markers
            std::string nm[256];
            nm[0xd8] = "SOI";
            nm[0xd9] = "EOI";
            nm[0xda] = "SOS";
            nm[0xdb] = "DQT";
            nm[0xdd] = "DRI";
            nm[0xfe] = "COM";

            // 0xe0 .. 0xef are APPn
            // 0xc0 .. 0xcf are SOFn (except 4)
            nm[0xc4] = "DHT";
            for (int i = 0; i <= 15; i++) {
                char MN[16];
                snprintf(MN, sizeof(MN), "APP%d", i);
                nm[0xe0 + i] = MN;
                if (i != 4) {
                    snprintf(MN, sizeof(MN), "SOF%d", i);
                    nm[0xc0 + i] = MN;
                }
            }

            // Container for the signature
            bool bExtXMP = false;

            // Read section marker
            byte marker = advanceToMarker(ErrorCode::kerNotAJpeg);

            bool done = false;
            bool first = true;
            while (!done) {
                // print marker bytes
                if (first && bPrint) {
                    out << "STRUCTURE OF JPEG FILE: " << io_->path() << std::endl;
                    out << " address | marker       |  length | data" << std::endl;
                    REPORT_MARKER;
                }
                first = false;
                bool bLF = bPrint;

                // 2-byte buffer for reading the size.
                byte sizebuf[2];
                uint16_t size = 0;
                if (markerHasLength(marker)) {
                    io_->readOrThrow(sizebuf, 2, ErrorCode::kerFailedToReadImageData);
                    size = getUShort(sizebuf, bigEndian);
                    // `size` is the size of the segment, including the 2-byte size field
                    // that we just read.
                    enforce(size >= 2, ErrorCode::kerFailedToReadImageData);
                }

                // Read the rest of the segment.
                DataBuf buf(size);
                if (size > 0) {
                    assert(size >= 2); // enforced above
                    io_->readOrThrow(buf.data(2), size - 2, ErrorCode::kerFailedToReadImageData);
                    buf.copyBytes(0, sizebuf, 2);
                }

                if (bPrint && markerHasLength(marker))
                    out << Internal::stringFormat(" | %7d ", size);

                // print signature for APPn
                if (marker >= app0_ && marker <= (app0_ | 0x0F)) {
                    assert(markerHasLength(marker));
                    assert(size >= 2); // Because this marker has a length field.
                    // http://www.adobe.com/content/dam/Adobe/en/devnet/xmp/pdfs/XMPSpecificationPart3.pdf p75
                    const std::string signature = string_from_unterminated(buf.c_str(2), size - 2);

                    // 728 rmills@rmillsmbp:~/gnu/exiv2/ttt $ exiv2 -pS test/data/exiv2-bug922.jpg
                    // STRUCTURE OF JPEG FILE: test/data/exiv2-bug922.jpg
                    // address | marker     | length  | data
                    //       0 | 0xd8 SOI   |       0
                    //       2 | 0xe1 APP1  |     911 | Exif..MM.*.......%.........#....
                    //     915 | 0xe1 APP1  |     870 | http://ns.adobe.com/xap/1.0/.<x:
                    //    1787 | 0xe1 APP1  |   65460 | http://ns.adobe.com/xmp/extensio
                    if (option == kpsXMP && signature.rfind("http://ns.adobe.com/x", 0) == 0) {
                        // extract XMP
                        const char* xmp = buf.c_str();
                        size_t start = 2;

                        // http://wwwimages.adobe.com/content/dam/Adobe/en/devnet/xmp/pdfs/XMPSpecificationPart3.pdf
                        // if we find HasExtendedXMP, set the flag and ignore this block
                        // the first extended block is a copy of the Standard block.
                        // a robust implementation allows extended blocks to be out of sequence
                        // we could implement out of sequence with a dictionary of sequence/offset
                        // and dumping the XMP in a post read operation similar to kpsIptcErase
                        // for the moment, dumping 'on the fly' is working fine
                        if (!bExtXMP) {
                            while (start < size && xmp[start]) {
                                start++;
                            }
                            start++;
                            if (start < size) {
                                const std::string xmp_from_start =
                                    string_from_unterminated(&xmp[start], size - start);
                                if (xmp_from_start.find("HasExtendedXMP", start) != std::string::npos) {
                                    start = size;  // ignore this packet, we'll get on the next time around
                                    bExtXMP = true;
                                }
                            }
                        } else {
                            start = 2 + 35 + 32 + 4 + 4;  // Adobe Spec, p19
                        }

                        enforce(start <= size, ErrorCode::kerInvalidXmpText);
                        out.write(reinterpret_cast<const char*>(&xmp[start]), size - start);
                        done = !bExtXMP;
                    } else if (option == kpsIccProfile && signature == iccId_) {
                        // extract ICCProfile
                        if (size >= 16) {
                            out.write(buf.c_str(16), size - 16);
#ifdef EXIV2_DEBUG_MESSAGES
                            std::cout << "iccProfile size = " << size - 16 << std::endl;
#endif
                        }
                    } else if (option == kpsIptcErase && signature == "Photoshop 3.0") {
                        // delete IPTC data segment from JPEG
                        iptcDataSegs.push_back(io_->tell() - size);
                        iptcDataSegs.push_back(size);
                    } else if (bPrint) {
                        const size_t start = 2;
                        const size_t end = size > 34 ? 34 : size;
                        out << "| " << Internal::binaryToString(makeSlice(buf, start, end));
                        if (signature == iccId_) {
                            // extract the chunk information from the buffer
                            //
                            // the buffer looks like this in this branch
                            // ICC_PROFILE\0AB
                            // where A & B are bytes (the variables chunk & chunks)
                            //
                            // We cannot extract the variables A and B from the signature string, as they are beyond the
                            // null termination (and signature ends there).
                            // => Read the chunk info from the DataBuf directly
                            enforce<std::out_of_range>(size >= 16, "Buffer too small to extract chunk information.");
                            const int chunk = buf.read_uint8(2 + 12);
                            const int chunks = buf.read_uint8(2 + 13);
                            out << Internal::stringFormat(" chunk %d/%d", chunk, chunks);
                        }
                    }

                    // for MPF: http://www.sno.phy.queensu.ca/~phil/exiftool/TagNames/MPF.html
                    // for FLIR: http://owl.phy.queensu.ca/~phil/exiftool/TagNames/FLIR.html
                    bool bFlir = option == kpsRecursive && marker == (app0_ + 1) && signature == "FLIR";
                    bool bExif = option == kpsRecursive && marker == (app0_ + 1) && signature == "Exif";
                    bool bMPF = option == kpsRecursive && marker == (app0_ + 2) && signature == "MPF";
                    bool bPS = option == kpsRecursive && signature == "Photoshop 3.0";
                    if (bFlir || bExif || bMPF || bPS) {
                        // extract Exif data block which is tiff formatted
                        out << std::endl;

//                        const byte* exif = buf.c_data();
                        uint32_t start = signature == "Exif" ? 8 : 6;
                        uint32_t max = static_cast<uint32_t>(size) - 1;

                        // is this an fff block?
                        if (bFlir) {
                            start = 2;
                            bFlir = false;
                            while (start+3 <= max) {
                                if (std::strcmp(buf.c_str(start), "FFF") == 0) {
                                    bFlir = true;
                                    break;
                                }
                                start++;
                            }
                        }

                        // there is a header in FLIR, followed by a tiff block
                        // Hunt down the tiff using brute force
                        if (bFlir) {
                            // FLIRFILEHEAD* pFFF = (FLIRFILEHEAD*) (exif+start) ;
                            while (start < max) {
                                if (buf.read_uint8(start) == 'I' && buf.read_uint8(start + 1) == 'I')
                                    break;
                                if (buf.read_uint8(start) == 'M' && buf.read_uint8(start + 1) == 'M')
                                    break;
                                start++;
                            }
#ifdef EXIV2_DEBUG_MESSAGES
                            if (start < max)
                                std::cout << "  FFF start = " << start << std::endl;
                            // << " index = " << pFFF->dwIndexOff << std::endl;
#endif
                        }

                        if (bPS) {
                            IptcData::printStructure(out, makeSlice(buf, 0, size), depth);
                        } else {
                            if (start < max) {
                                // create a copy on write memio object with the data, then print the structure
                                MemIo p (buf.c_data(start), size - start);
                                printTiffStructure(p, out, option, depth);
                            }
                        }

                        // restore and clean up
                        bLF = false;
                    }
                }

                // print COM marker
                if (bPrint && marker == com_) {
                    assert(markerHasLength(marker));
                    assert(size >= 2); // Because this marker has a length field.
                    // size includes 2 for the two bytes for size!
                    const size_t n = (size - 2) > 32 ? 32 : size - 2;
                    // start after the two bytes
                    out << "| "
                        << Internal::binaryToString(
                               makeSlice(buf, 2, n + 2 /* cannot overflow as n is at most size - 2 */));
                }

                if (bLF)
                    out << std::endl;

                if (marker != sos_) {
                    // Read the beginning of the next segment
                    marker = advanceToMarker(ErrorCode::kerNoImageInInputData);
                    REPORT_MARKER;
                }
                done |= marker == eoi_ || marker == sos_;
                if (done && bPrint)
                    out << std::endl;
            }
        }
        if (option == kpsIptcErase && !iptcDataSegs.empty()) {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "iptc data blocks: " << iptcDataSegs.size() << std::endl;
            uint32_t toggle = 0;
            for (auto&& iptc : iptcDataSegs) {
                std::cout << iptc;
                if (toggle++ % 2)
                    std::cout << std::endl;
                else
                    std::cout << ' ';
            }
#endif
            size_t count = iptcDataSegs.size();

            // figure out which blocks to copy
            std::vector<size_t> pos(count + 2);
            pos[0] = 0;
            // copy the data that is not iptc
            auto it = iptcDataSegs.begin();
            for (size_t i = 0; i < count; i++) {
                bool bOdd = (i % 2) != 0;
                bool bEven = !bOdd;
                pos[i + 1] = bEven ? *it : pos[i] + *it;
                ++it;
            }
            pos[count + 1] = io_->size();
#ifdef EXIV2_DEBUG_MESSAGES
            for (size_t i = 0; i < count + 2; i++)
                std::cout << pos[i] << " ";
            std::cout << std::endl;
#endif
            // $ dd bs=1 skip=$((0)) count=$((13164)) if=ETH0138028.jpg of=E1.jpg
            // $ dd bs=1 skip=$((49304)) count=2000000  if=ETH0138028.jpg of=E2.jpg
            // cat E1.jpg E2.jpg > E.jpg
            // exiv2 -pS E.jpg

            // binary copy io_ to a temporary file
            auto tempIo = std::make_unique<MemIo>();
            for (size_t i = 0; i < (count / 2) + 1; i++) {
                size_t start = pos[2 * i] + 2;  // step JPG 2 byte marker
                if (start == 2)
                    start = 0;  // read the file 2 byte SOI
                size_t length = pos[2 * i + 1] - start;
                if (length) {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cout << start << ":" << length << std::endl;
#endif
                    io_->seekOrThrow(start, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
                    DataBuf buf(length);
                    io_->readOrThrow(buf.data(), buf.size(), ErrorCode::kerFailedToReadImageData);
                    tempIo->write(buf.c_data(), buf.size());
                }
            }

            io_->seekOrThrow(0, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
            io_->transfer(*tempIo);  // may throw
            io_->seekOrThrow(0, BasicIo::beg, ErrorCode::kerFailedToReadImageData);
            readMetadata();
        }
    }

    void JpegBase::writeMetadata()
    {
        if (io_->open() != 0) {
            throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        auto tempIo = std::make_unique<MemIo>();

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw
    }

    DataBuf JpegBase::readNextSegment(byte marker)
    {
        // 2-byte buffer for reading the size.
        byte sizebuf[2];
        uint16_t size = 0;
        if (markerHasLength(marker)) {
            io_->readOrThrow(sizebuf, 2, ErrorCode::kerFailedToReadImageData);
            size = getUShort(sizebuf, bigEndian);
            // `size` is the size of the segment, including the 2-byte size field
            // that we just read.
            enforce(size >= 2, ErrorCode::kerFailedToReadImageData);
        }

        // Read the rest of the segment.
        DataBuf buf(size);
        if (size > 0) {
            assert(size >= 2); // enforced above
            io_->readOrThrow(buf.data(2), size - 2, ErrorCode::kerFailedToReadImageData);
            buf.copyBytes(0, sizebuf, 2);
        }
        return buf;
    }

    void JpegBase::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen())
            throw Error(ErrorCode::kerInputDataReadFailed);
        if (!outIo.isopen())
            throw Error(ErrorCode::kerImageWriteFailed);

        // Ensure that this is the correct image type
        if (!isThisType(*io_, true)) {
            if (io_->error() || io_->eof())
                throw Error(ErrorCode::kerInputDataReadFailed);
            throw Error(ErrorCode::kerNoImageInInputData);
        }

        // Used to initialize search variables such as skipCom.
        static const size_t notfound = std::numeric_limits<size_t>::max();

        const long seek = io_->tell();
        size_t count = 0;
        size_t search = 0;
        size_t insertPos = 0;
        size_t comPos = 0;
        size_t skipApp1Exif = notfound;
        size_t skipApp1Xmp = notfound;
        bool foundCompletePsData = false;
        bool foundIccData = false;
        std::vector<size_t> skipApp13Ps3;
        std::vector<size_t> skipApp2Icc;
        size_t skipCom = notfound;
        Blob psBlob;
        DataBuf rawExif;
        xmpData().usePacket(writeXmpFromPacket());

        // Write image header
        if (writeHeader(outIo))
            throw Error(ErrorCode::kerImageWriteFailed);

        // Read section marker
        byte marker = advanceToMarker(ErrorCode::kerNoImageInInputData);

        // First find segments of interest. Normally app0 is first and we want
        // to insert after it. But if app0 comes after com, app1 and app13 then
        // don't bother.
        while (marker != sos_ && marker != eoi_ && search < 6) {
            DataBuf buf = readNextSegment(marker);

            if (marker == app0_) {
                insertPos = count + 1;
            } else if (skipApp1Exif == notfound &&
                       marker == app1_ &&
                       buf.size() >= 8 && // prevent out-of-bounds read in memcmp on next line
                       buf.cmpBytes(2, exifId_, 6) == 0) {
                skipApp1Exif = count;
                ++search;
                if (buf.size() > 8) {
                    rawExif.alloc(buf.size() - 8);
                    rawExif.copyBytes(0, buf.c_data(8), buf.size() - 8);
                }
            } else if (skipApp1Xmp == notfound &&
                       marker == app1_ &&
                       buf.size() >= 31 && // prevent out-of-bounds read in memcmp on next line
                       buf.cmpBytes(2, xmpId_, 29) == 0) {
                skipApp1Xmp = count;
                ++search;
            } else if (marker == app2_ &&
                       buf.size() >= 13 && // prevent out-of-bounds read in memcmp on next line
                       buf.cmpBytes(2, iccId_, 11) == 0) {
                skipApp2Icc.push_back(count);
                if (!foundIccData) {
                    ++search;
                    foundIccData = true;
                }
            } else if (!foundCompletePsData &&
                       marker == app13_ &&
                       buf.size() >= 16 && // prevent out-of-bounds read in memcmp on next line
                       buf.cmpBytes(2, Photoshop::ps3Id_, 14) == 0) {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Found APP13 Photoshop PS3 segment\n";
#endif
                skipApp13Ps3.push_back(count);
                // Append to psBlob
                append(psBlob, buf.c_data(16), buf.size() - 16);
                // Check whether psBlob is complete
                if (!psBlob.empty() && Photoshop::valid(&psBlob[0], psBlob.size())) {
                    foundCompletePsData = true;
                }
            } else if (marker == com_ && skipCom == notfound) {
                // Jpegs can have multiple comments, but for now only handle
                // the first one (most jpegs only have one anyway).
                skipCom = count;
                ++search;
            }

            // As in jpeg-6b/wrjpgcom.c:
            // We will insert the new comment marker just before SOFn.
            // This (a) causes the new comment to appear after, rather than before,
            // existing comments; and (b) ensures that comments come after any JFIF
            // or JFXX markers, as required by the JFIF specification.
            if (comPos == 0 && inRange2(marker, sof0_, sof3_, sof5_, sof15_)) {
                comPos = count;
                ++search;
            }
            marker = advanceToMarker(ErrorCode::kerNoImageInInputData);
            ++count;
        }

        if (!foundCompletePsData && !psBlob.empty())
            throw Error(ErrorCode::kerNoImageInInputData);
        search += skipApp13Ps3.size() + skipApp2Icc.size();

        if (comPos == 0) {
            if (marker == eoi_)
                comPos = count;
            else
                comPos = insertPos;
            ++search;
        }
        if (exifData_.count() > 0)
            ++search;
        if (!writeXmpFromPacket() && xmpData_.count() > 0)
            ++search;
        if (writeXmpFromPacket() && !xmpPacket_.empty())
            ++search;
        if (foundCompletePsData || iptcData_.count() > 0)
            ++search;
        if (!comment_.empty())
            ++search;

        io_->seekOrThrow(seek, BasicIo::beg, ErrorCode::kerNoImageInInputData);
        count = 0;
        marker = advanceToMarker(ErrorCode::kerNoImageInInputData);

        // To simplify this a bit, new segments are inserts at either the start
        // or right after app0. This is standard in most jpegs, but has the
        // potential to change segment ordering (which is allowed).
        // Segments are erased if there is no assigned metadata.
        while (marker != sos_ && search > 0) {
            DataBuf buf = readNextSegment(marker);

            if (insertPos == count) {
                // Write Exif data first so that - if there is no app0 - we
                // create "Exif images" according to the Exif standard.
                if (exifData_.count() > 0) {
                    Blob blob;
                    ByteOrder bo = byteOrder();
                    if (bo == invalidByteOrder) {
                        bo = littleEndian;
                        setByteOrder(bo);
                    }
                    WriteMethod wm = ExifParser::encode(blob, rawExif.c_data(), rawExif.size(), bo, exifData_);
                    const byte* pExifData = rawExif.c_data();
                    size_t exifSize = rawExif.size();
                    if (wm == wmIntrusive) {
                        pExifData = !blob.empty() ? &blob[0] : nullptr;
                        exifSize = blob.size();
                    }
                    if (exifSize > 0) {
                        std::array<byte, 10> tmpBuf;
                        // Write APP1 marker, size of APP1 field, Exif id and Exif data
                        tmpBuf[0] = 0xff;
                        tmpBuf[1] = app1_;

                        if (exifSize > 0xffff - 8)
                            throw Error(ErrorCode::kerTooLargeJpegSegment, "Exif");
                        us2Data(tmpBuf.data() + 2, static_cast<uint16_t>(exifSize + 8), bigEndian);
                        std::memcpy(tmpBuf.data() + 4, exifId_, 6);
                        if (outIo.write(tmpBuf.data(), 10) != 10)
                            throw Error(ErrorCode::kerImageWriteFailed);

                        // Write new Exif data buffer
                        if (outIo.write(pExifData, exifSize) != exifSize)
                            throw Error(ErrorCode::kerImageWriteFailed);
                        if (outIo.error())
                            throw Error(ErrorCode::kerImageWriteFailed);
                        --search;
                    }
                }
                if (!writeXmpFromPacket()) {
                    if (XmpParser::encode(xmpPacket_, xmpData_,
                                          XmpParser::useCompactFormat | XmpParser::omitAllFormatting) > 1) {
#ifndef SUPPRESS_WARNINGS
                        EXV_ERROR << "Failed to encode XMP metadata.\n";
#endif
                    }
                }
                if (!xmpPacket_.empty()) {
                    std::array<byte, 33> tmpBuf;
                    // Write APP1 marker, size of APP1 field, XMP id and XMP packet
                    tmpBuf[0] = 0xff;
                    tmpBuf[1] = app1_;

                    if (xmpPacket_.size() > 0xffff - 31)
                        throw Error(ErrorCode::kerTooLargeJpegSegment, "XMP");
                    us2Data(tmpBuf.data() + 2, static_cast<uint16_t>(xmpPacket_.size() + 31), bigEndian);
                    std::memcpy(tmpBuf.data() + 4, xmpId_, 29);
                    if (outIo.write(tmpBuf.data(), 33) != 33)
                        throw Error(ErrorCode::kerImageWriteFailed);

                    // Write new XMP packet
                    if (outIo.write(reinterpret_cast<const byte*>(xmpPacket_.data()),
                                    xmpPacket_.size()) != xmpPacket_.size())
                        throw Error(ErrorCode::kerImageWriteFailed);
                    if (outIo.error())
                        throw Error(ErrorCode::kerImageWriteFailed);
                    --search;
                }

                if (iccProfileDefined()) {
                    std::array<byte, 4> tmpBuf;
                    // Write APP2 marker, size of APP2 field, and IccProfile
                    // See comments in readMetadata() about the ICC embedding specification
                    tmpBuf[0] = 0xff;
                    tmpBuf[1] = app2_;

                    const long chunk_size = 256 * 256 - 40;  // leave bytes for marker, header and padding
                    size_t size = iccProfile_.size();
                    assert(size > 0); // Because iccProfileDefined() == true
                    if (size >= 255 * chunk_size)
                        throw Error(ErrorCode::kerTooLargeJpegSegment, "IccProfile");
                    const size_t chunks = 1 + (size - 1) / chunk_size;
                    assert(chunks <= 255); // Because size < 255 * chunk_size
                    for (size_t chunk = 0; chunk < chunks; chunk++) {
                        size_t bytes = size > chunk_size ? chunk_size : size;  // bytes to write
                        size -= bytes;

                        // write JPEG marker (2 bytes)
                        if (outIo.write(tmpBuf.data(), 2) != 2)
                            throw Error(ErrorCode::kerImageWriteFailed);  // JPEG Marker
                        // write length (2 bytes).  length includes the 2 bytes for the length
                        us2Data(tmpBuf.data() + 2, static_cast<uint16_t>(2 + 14 + bytes), bigEndian);
                        if (outIo.write(tmpBuf.data() + 2, 2) != 2)
                            throw Error(ErrorCode::kerImageWriteFailed);  // JPEG Length

                        // write the ICC_PROFILE header (14 bytes)
                        uint8_t pad[2];
                        pad[0] = static_cast<uint8_t>(chunk + 1);
                        pad[1] = static_cast<uint8_t>(chunks);
                        outIo.write(reinterpret_cast<const byte*>(iccId_), 12);
                        outIo.write(reinterpret_cast<const byte*>(pad), 2);
                        if (outIo.write(iccProfile_.c_data(chunk * chunk_size), bytes) != bytes)
                            throw Error(ErrorCode::kerImageWriteFailed);
                        if (outIo.error())
                            throw Error(ErrorCode::kerImageWriteFailed);
                    }
                    --search;
                }

                if (foundCompletePsData || iptcData_.count() > 0) {
                    // Set the new IPTC IRB, keeps existing IRBs but removes the
                    // IPTC block if there is no new IPTC data to write
                    DataBuf newPsData = Photoshop::setIptcIrb(!psBlob.empty() ? psBlob.data() : nullptr,
                                                              psBlob.size(), iptcData_);
                    const long maxChunkSize = 0xffff - 16;
                    const byte* chunkStart = newPsData.c_data();
                    const byte* chunkEnd = newPsData.empty() ? nullptr : newPsData.c_data(newPsData.size()-1);
                    while (chunkStart < chunkEnd) {
                        // Determine size of next chunk
                        size_t chunkSize = (chunkEnd + 1 - chunkStart);
                        if (chunkSize > maxChunkSize) {
                            chunkSize = maxChunkSize;
                            // Don't break at a valid IRB boundary
                            const auto writtenSize = static_cast<long>(chunkStart - newPsData.c_data());
                            if (Photoshop::valid(newPsData.c_data(), writtenSize + chunkSize)) {
                                // Since an IRB has minimum size 12,
                                // (chunkSize - 8) can't be also a IRB boundary
                                chunkSize -= 8;
                            }
                        }

                        // Write APP13 marker, chunk size, and ps3Id
                        std::array<byte, 18> tmpBuf;
                        tmpBuf[0] = 0xff;
                        tmpBuf[1] = app13_;
                        us2Data(tmpBuf.data() + 2, static_cast<uint16_t>(chunkSize + 16), bigEndian);
                        std::memcpy(tmpBuf.data() + 4, Photoshop::ps3Id_, 14);
                        if (outIo.write(tmpBuf.data(), 18) != 18)
                            throw Error(ErrorCode::kerImageWriteFailed);
                        if (outIo.error())
                            throw Error(ErrorCode::kerImageWriteFailed);

                        // Write next chunk of the Photoshop IRB data buffer
                        if (outIo.write(chunkStart, chunkSize) != chunkSize)
                            throw Error(ErrorCode::kerImageWriteFailed);
                        if (outIo.error())
                            throw Error(ErrorCode::kerImageWriteFailed);

                        chunkStart += chunkSize;
                    }
                    --search;
                }
            }
            if (comPos == count) {
                if (!comment_.empty()) {
                    std::array<byte, 4> tmpBuf;
                    // Write COM marker, size of comment, and string
                    tmpBuf[0] = 0xff;
                    tmpBuf[1] = com_;

                    if (comment_.length() > 0xffff - 3)
                        throw Error(ErrorCode::kerTooLargeJpegSegment, "JPEG comment");
                    us2Data(tmpBuf.data() + 2, static_cast<uint16_t>(comment_.length() + 3), bigEndian);

                    if (outIo.write(tmpBuf.data(), 4) != 4)
                        throw Error(ErrorCode::kerImageWriteFailed);
                    if (outIo.write(reinterpret_cast<byte*>(const_cast<char*>(comment_.data())),
                                    comment_.length()) != comment_.length())
                        throw Error(ErrorCode::kerImageWriteFailed);
                    if (outIo.putb(0) == EOF)
                        throw Error(ErrorCode::kerImageWriteFailed);
                    if (outIo.error())
                        throw Error(ErrorCode::kerImageWriteFailed);
                    --search;
                }
                --search;
            }
            if (marker == eoi_) {
                break;
            }
            if (skipApp1Exif == count || skipApp1Xmp == count ||
                std::find(skipApp13Ps3.begin(), skipApp13Ps3.end(), count) != skipApp13Ps3.end() ||
                std::find(skipApp2Icc.begin(), skipApp2Icc.end(), count) != skipApp2Icc.end() || skipCom == count) {
                --search;
            } else {
                std::array<byte, 2> tmpBuf;
                // Write marker and a copy of the segment.
                tmpBuf[0] = 0xff;
                tmpBuf[1] = marker;
                if (outIo.write(tmpBuf.data(), 2) != 2)
                    throw Error(ErrorCode::kerImageWriteFailed);
                if (outIo.write(buf.c_data(), buf.size()) != buf.size())
                    throw Error(ErrorCode::kerImageWriteFailed);
                if (outIo.error())
                    throw Error(ErrorCode::kerImageWriteFailed);
            }

            // Next marker
            marker = advanceToMarker(ErrorCode::kerNoImageInInputData);
            ++count;
        }

        // Populate the fake data, only make sense for remoteio, httpio and sshio.
        // it avoids allocating memory for parts of the file that contain image-date.
        io_->populateFakeData();

        // Write the final marker, then copy rest of the Io.
        byte tmpBuf[2];
        tmpBuf[0] = 0xff;
        tmpBuf[1] = marker;
        if (outIo.write(tmpBuf, 2) != 2)
            throw Error(ErrorCode::kerImageWriteFailed);

        DataBuf buf(4096);
        size_t readSize = 0;
        while ((readSize = io_->read(buf.data(), buf.size()))) {
            if (outIo.write(buf.c_data(), readSize) != readSize)
                throw Error(ErrorCode::kerImageWriteFailed);
        }
        if (outIo.error())
            throw Error(ErrorCode::kerImageWriteFailed);

    }  // JpegBase::doWriteMetadata

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

    JpegImage::JpegImage(BasicIo::UniquePtr io, bool create)
        : JpegBase(ImageType::jpeg, std::move(io), create, blank_, sizeof(blank_))
    {
    }

    std::string JpegImage::mimeType() const
    {
        return "image/jpeg";
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

    Image::UniquePtr newJpegInstance(BasicIo::UniquePtr io, bool create)
    {
        auto image = std::make_unique<JpegImage>(std::move(io), create);
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
        if (iIo.error() || iIo.eof())
            return false;

        if (0xff != tmpBuf[0] || JpegImage::soi_ != tmpBuf[1]) {
            result = false;
        }
        if (!advance || !result )
            iIo.seek(-2, BasicIo::cur);
        return result;
    }

    ExvImage::ExvImage(BasicIo::UniquePtr io, bool create)
        : JpegBase(ImageType::exv, std::move(io), create, blank_, sizeof(blank_))
    {
    }

    std::string ExvImage::mimeType() const
    {
        return "image/x-exv";
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

    Image::UniquePtr newExvInstance(BasicIo::UniquePtr io, bool create)
    {
        auto image = std::make_unique<ExvImage>(std::move(io), create);
        if (!image->good())
            image.reset();
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
        if (!advance || !result) iIo.seek(-7, BasicIo::cur);
        return result;
    }

}                                       // namespace Exiv2
