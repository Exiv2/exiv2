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
  File:      exif.cpp
  Version:   $Name:  $ $Revision: 1.64 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.64 $ $RCSfile: exif.cpp,v $");

// Define DEBUG_MAKERNOTE to output debug information to std::cerr
#undef DEBUG_MAKERNOTE

// *****************************************************************************
// included header files
#include "exif.hpp"
#include "types.hpp"
#include "error.hpp"
#include "value.hpp"
#include "ifd.hpp"
#include "tags.hpp"
#include "image.hpp"
#include "makernote.hpp"

// + standard includes
#include <iostream>
#include <sstream>
#include <utility>
#include <algorithm>
#include <map>
#include <cstring>
#include <cassert>

// *****************************************************************************
// local declarations
namespace {

    /*
      Set the data of the entry identified by tag in ifd to an unsigned long
      with the value of offset. If no entry with this tag exists in ifd, an
      entry of type unsigned long with one component is created.
     */
    void setOffsetTag(Exiv2::Ifd& ifd,
                      int idx,
                      uint16_t tag,
                      uint32_t offset, 
                      Exiv2::ByteOrder byteOrder);

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Exifdatum::Exifdatum(const Entry& e, ByteOrder byteOrder)
        : pKey_(new ExifKey(e)), pValue_(0)
    {
        pValue_ = Value::create(TypeId(e.type()));
        pValue_->read(e.data(), e.count() * e.typeSize(), byteOrder);
    }

    Exifdatum::Exifdatum(const ExifKey& key, const Value* pValue) 
        : pKey_(key.clone()), pValue_(0)
    {
        if (pValue) pValue_ = pValue->clone();
    }

    Exifdatum::~Exifdatum()
    {
        delete pKey_;
        delete pValue_;
    }

    Exifdatum::Exifdatum(const Exifdatum& rhs)
        : Metadatum(rhs), pKey_(0), pValue_(0)
    {
        if (rhs.pKey_ != 0) pKey_ = rhs.pKey_->clone(); // deep copy
        if (rhs.pValue_ != 0) pValue_ = rhs.pValue_->clone(); // deep copy
    }

    Exifdatum& Exifdatum::operator=(const Exifdatum& rhs)
    {
        if (this == &rhs) return *this;
        Metadatum::operator=(rhs);

        delete pKey_;
        pKey_ = 0;
        if (rhs.pKey_ != 0) pKey_ = rhs.pKey_->clone(); // deep copy

        delete pValue_;
        pValue_ = 0;
        if (rhs.pValue_ != 0) pValue_ = rhs.pValue_->clone(); // deep copy

        return *this;
    } // Exifdatum::operator=
    
    void Exifdatum::setValue(const Value* pValue)
    {
        delete pValue_;
        pValue_ = pValue->clone();
    }

    void Exifdatum::setValue(const Entry& e, ByteOrder byteOrder)
    {
        delete pValue_;
        pValue_ = Value::create(TypeId(e.type()));
        pValue_->read(e.data(), e.count() * e.typeSize(), byteOrder);
    }

    void Exifdatum::setValue(const std::string& buf)
    {
        if (pValue_ == 0) pValue_ = Value::create(asciiString);
        pValue_->read(buf);
    }

    TiffThumbnail::TiffThumbnail()
        : offset_(0), size_(0), pImage_(0), ifd_(ifd1Id, 0, false)
    {
    }

    TiffThumbnail::~TiffThumbnail()
    {
        delete[] pImage_;
    }

    TiffThumbnail::TiffThumbnail(const TiffThumbnail& rhs)
        : offset_(rhs.offset_), size_(rhs.size_), pImage_(0), 
          ifd_(ifd1Id, 0, false)
    {
        if (rhs.pImage_ && rhs.size_ > 0) {
            pImage_ = new byte[rhs.size_];
            memcpy(pImage_, rhs.pImage_, rhs.size_);
            tiffHeader_.read(pImage_);
            ifd_.read(pImage_ + tiffHeader_.offset(),
                      size_ - tiffHeader_.offset(),
                      tiffHeader_.byteOrder(), tiffHeader_.offset());
        }
    }

    TiffThumbnail& TiffThumbnail::operator=(const TiffThumbnail& rhs)
    {
        DataBuf newImage;
        if (rhs.pImage_ && rhs.size_ > 0) {
            newImage.alloc(rhs.size_);
            memcpy(newImage.pData_, rhs.pImage_, rhs.size_);
            tiffHeader_.read(rhs.pImage_);
            ifd_.read(newImage.pData_ + tiffHeader_.offset(), 
                      rhs.size_ - tiffHeader_.offset(), 
                      tiffHeader_.byteOrder(), tiffHeader_.offset());
        }
        offset_ = rhs.offset_;
        size_ = rhs.size_;
        delete[] pImage_;
        pImage_ = newImage.release();
        return *this;
    }

    int TiffThumbnail::read(const byte* buf,
                            long len,
                            const ExifData& exifData,
                            ByteOrder byteOrder)
    {
        DataBuf img;                        // temporary buffer
        img.alloc(len);
        memset(img.pData_, 0x0, img.size_);
        long buflen = 0;                    // actual number of bytes needed

        int rc = 0;
        // Copy the TIFF header
        TiffHeader tiffHeader(byteOrder);
        if (len < tiffHeader.size()) rc = 1;
        Ifd ifd1(ifd1Id);
        long ifdOffset = 0;
        if (rc == 0) {
            buflen += tiffHeader.copy(img.pData_);

            // Create IFD (without Exif and GPS tags) from metadata
            addToIfd(ifd1, exifData.begin(), exifData.end(), 
                     tiffHeader.byteOrder());
            ifd1.erase(0x8769);
            ifd1.erase(0x8825);

            // Do not copy the IFD yet, remember the location and leave a gap
            ifdOffset = buflen;
            buflen += ifd1.size() + ifd1.dataSize();
            if (len < buflen) rc = 1;
        }
        ExifData::const_iterator offsets;
        ExifData::const_iterator sizes;
        if (rc == 0) {
            // Copy thumbnail image data, remember the offsets used
            ExifKey key("Exif.Thumbnail.StripOffsets");
            offsets = exifData.findKey(key);
            if (offsets == exifData.end()) rc = 2;
        }
        if (rc == 0) {
            ExifKey key("Exif.Thumbnail.StripByteCounts");
            sizes = exifData.findKey(key);
            if (sizes == exifData.end()) rc = 2;
        }
        std::ostringstream os;                  // for the new strip offsets
        long minOffset = 0;
        if (rc == 0) {
            for (long k = 0; k < offsets->count(); ++k) {
                long offset = offsets->toLong(k);
                long size = sizes->toLong(k);
                if (len < offset + size || len < buflen + size) {
                    rc = 1;
                    break;
                }
                memcpy(img.pData_ + buflen, buf + offset, size);
                os << buflen << " ";
                buflen += size;

                minOffset = offset;             // just to initialize minOffset
            }
        }
        if (rc == 0) {
            for (long k = 0; k < offsets->count(); ++k) {
                minOffset = std::min(minOffset, offsets->toLong(k));
            }
            // Update the IFD with the actual strip offsets (replace existing entry)
            Exifdatum newOffsets(*offsets);
            newOffsets.setValue(os.str());
            ifd1.erase(0x0111);
            addToIfd(ifd1, newOffsets, tiffHeader.byteOrder());

            // Finally, sort and copy the IFD
            ifd1.sortByTag();
            ifd1.copy(img.pData_ + ifdOffset, tiffHeader.byteOrder(), ifdOffset);

            delete[] pImage_;
            pImage_ = new byte[buflen];
            memcpy(pImage_, img.pData_, buflen);
            size_ = buflen;
            offset_ = minOffset;
            tiffHeader_.read(pImage_);
            rc = ifd_.read(pImage_ + tiffHeader_.offset(), 
                           size_ - tiffHeader_.offset(), 
                           tiffHeader_.byteOrder(), tiffHeader_.offset());
        }

        return rc;
    } // TiffThumbnail::read

    const char* TiffThumbnail::format() const
    {
        return "TIFF";
    }

    const char* TiffThumbnail::extension() const
    {
        return ".tif";
    }

    int TiffThumbnail::write(const std::string& path) const
    {
        std::string name = path + extension();
        FILE *ofp = fopen(name.c_str(), "wb");
        if (!ofp) return -1;
        int rc = 0;
        if (fwrite(pImage_, 1, size_, ofp) != (size_t)size_) rc = 4;
        fclose(ofp);
        return rc;
    } // TiffThumbnail::write

    void TiffThumbnail::update(ExifData& exifData) const
    {
        // Todo: properly synchronize the Exif data with the actual thumbnail,
        //       i.e., synch all relevant metadata

        // Create metadata from the StripOffsets and StripByteCounts entries
        // and update the Exif data accordingly
        Entries::const_iterator entry = ifd_.findTag(0x0111);
        if (entry == ifd_.end()) throw Error("Bad thumbnail (0x0111)");
        ExifData::iterator md = exifData.findIfdIdIdx(entry->ifdId(), entry->idx());
        if (md == exifData.end()) {
            exifData.add(Exifdatum(*entry, tiffHeader_.byteOrder()));
        }
        else {
            md->setValue(*entry, tiffHeader_.byteOrder());
        }

        entry = ifd_.findTag(0x0117);
        if (entry == ifd_.end()) throw Error("Bad thumbnail (0x0117)");
        md = exifData.findIfdIdIdx(entry->ifdId(), entry->idx());
        if (md == exifData.end()) {
            exifData.add(Exifdatum(*entry, tiffHeader_.byteOrder()));
        }
        else {
            md->setValue(*entry, tiffHeader_.byteOrder());
        }

    } // TiffThumbnail::update

    long TiffThumbnail::copy(byte* buf) const
    {
        long offset = ifd_.offset() + ifd_.size() + ifd_.dataSize();
        long size = size_ - offset;
        memcpy(buf, pImage_ + offset, size);
        return size;
    }

    long TiffThumbnail::dataSize() const
    {
        return size_ - ifd_.offset() - ifd_.size() - ifd_.dataSize();
    }

    long TiffThumbnail::size() const
    {
        return size_;
    }

    long TiffThumbnail::offset() const
    {
        return offset_;
    }

    void TiffThumbnail::setOffsets(Ifd& ifd1, ByteOrder byteOrder)
    {
        // Adjust the StripOffsets, assuming that the existing TIFF strips
        // start immediately after the thumbnail IFD
        long shift = ifd1.offset() + ifd1.size() + ifd1.dataSize() 
            - ifd_.offset() - ifd_.size() - ifd_.dataSize();
        Ifd::const_iterator pos = ifd_.findTag(0x0111);
        if (pos == ifd_.end()) throw Error("Bad thumbnail (0x0111)");
        Exifdatum offsets(*pos, tiffHeader_.byteOrder());
        std::ostringstream os;
        long minOffset = 0;
        for (long k = 0; k < offsets.count(); ++k) {
            os << offsets.toLong(k) + shift << " ";
            minOffset = offsets.toLong(k) + shift; // initialize minOffset
        }
        offsets.setValue(os.str());
        for (long k = 0; k < offsets.count(); ++k) {
            minOffset = std::min(minOffset, offsets.toLong(k));
        }
        offset_ = minOffset;
        // Update the IFD with the re-calculated strip offsets 
        // (replace existing entry)
        ifd1.erase(0x0111);
        addToIfd(ifd1, offsets, byteOrder);

    } // TiffThumbnail::setOffsets

    JpegThumbnail::JpegThumbnail()
        : offset_(0), size_(0), pImage_(0)
    {
    }

    JpegThumbnail::~JpegThumbnail()
    {
        delete[] pImage_;
    }

    JpegThumbnail::JpegThumbnail(const JpegThumbnail& rhs)
        : offset_(rhs.offset_), size_(rhs.size_), pImage_(0)
    {
        if (rhs.pImage_ && rhs.size_ > 0) {
            pImage_ = new byte[rhs.size_];
            memcpy(pImage_, rhs.pImage_, rhs.size_);
        }
    }

    JpegThumbnail& JpegThumbnail::operator=(const JpegThumbnail& rhs)
    {
        DataBuf newImage;
        if (rhs.pImage_ && rhs.size_ > 0) {
            newImage.alloc(rhs.size_);
            memcpy(newImage.pData_, rhs.pImage_, rhs.size_);
        }
        offset_ = rhs.offset_;
        size_ = rhs.size_;
        delete[] pImage_;
        pImage_ = newImage.release();
        return *this;
    }

    int JpegThumbnail::read(const byte* buf, 
                            long len,
                            const ExifData& exifData,
                            ByteOrder byteOrder) 
    {
        ExifKey key("Exif.Thumbnail.JPEGInterchangeFormat");
        ExifData::const_iterator pos = exifData.findKey(key);
        if (pos == exifData.end()) return 2;
        long offset = pos->toLong();
        ExifKey key2("Exif.Thumbnail.JPEGInterchangeFormatLength");
        pos = exifData.findKey(key2);
        if (pos == exifData.end()) return 2;
        long size = pos->toLong();
        if (len < offset + size) return 1;
        delete[] pImage_;
        pImage_ = new byte[size];
        memcpy(pImage_, buf + offset, size);
        size_ = size;
        offset_ = offset;
        return 0;
    } // JpegThumbnail::read

    const char* JpegThumbnail::format() const
    {
        return "JPEG";
    }

    const char* JpegThumbnail::extension() const
    {
        return ".jpg";
    }

    int JpegThumbnail::write(const std::string& path) const
    {
        std::string name = path + extension();
        FILE *ofp = fopen(name.c_str(), "wb");
        if (!ofp) return -1;
        int rc = 0;
        if (fwrite(pImage_, 1, size_, ofp) != (size_t)size_) rc = 4;
        fclose(ofp);
        return rc;
    } // JpegThumbnail::write

    void JpegThumbnail::update(ExifData& exifData) const
    {
        ExifKey key("Exif.Thumbnail.JPEGInterchangeFormat");
        ExifData::iterator pos = exifData.findKey(key);
        if (pos == exifData.end()) {
            Value* value = Value::create(unsignedLong);
            exifData.add(key, value);
            delete value;
            pos = exifData.findKey(key);
        }
        pos->setValue(toString(offset_));

        ExifKey key2("Exif.Thumbnail.JPEGInterchangeFormatLength");
        pos = exifData.findKey(key2);
        if (pos == exifData.end()) {
            Value *value = Value::create(unsignedLong);
            exifData.add(key2, value);
            delete value;            
            pos = exifData.findKey(key2);
        }
        pos->setValue(toString(size_));

    } // JpegThumbnail::update

    long JpegThumbnail::copy(byte* buf) const
    {
        memcpy(buf, pImage_, size_);
        return size_;
    }

    long JpegThumbnail::dataSize() const
    {
        return size_;
    }

    long JpegThumbnail::size() const
    {
        return size_;
    }

    long JpegThumbnail::offset() const
    {
        return offset_;
    }

    void JpegThumbnail::setOffsets(Ifd& ifd1, ByteOrder byteOrder)
    {
        Ifd::iterator pos = ifd1.findTag(0x0201);
        if (pos == ifd1.end()) throw Error("Bad thumbnail (0x0201)");
        offset_ = ifd1.offset() + ifd1.size() + ifd1.dataSize();
        pos->setValue(offset_, byteOrder);
    }

    ExifData::ExifData() 
        : pThumbnail_(0), pMakerNote_(0), ifd0_(ifd0Id, 0, false), 
          exifIfd_(exifIfdId, 0, false), iopIfd_(iopIfdId, 0, false), 
          gpsIfd_(gpsIfdId, 0, false), ifd1_(ifd1Id, 0, false), 
          size_(0), pData_(0), compatible_(true)
    {
    }

    ExifData::~ExifData()
    {
        delete pMakerNote_;
        delete pThumbnail_;
        delete[] pData_;
    }

    int ExifData::read(const std::string& path)
    {
        if (!fileExists(path, true)) return -1;
        Image::AutoPtr image = ImageFactory::instance().open(path);
        if (image.get() == 0) {
            // We don't know this type of file
            return -2;
        }

        int rc = image->readMetadata();
        if (rc == 0) {
            if (image->sizeExifData() > 0) {
                rc = read(image->exifData(), image->sizeExifData());
            }
            else {
                rc = 3;
            }
        }
        return rc;
    }

    int ExifData::read(const byte* buf, long len)
    {
        // Copy the data buffer
        delete[] pData_;
        pData_ = new byte[len];
        memcpy(pData_, buf, len);
        size_ = len;

        // Read the TIFF header
        int ret = 0;
        int rc = tiffHeader_.read(pData_);
        if (rc) return rc;

        // Read IFD0
        rc = ifd0_.read(pData_ + tiffHeader_.offset(), 
                        size_ - tiffHeader_.offset(), 
                        byteOrder(), 
                        tiffHeader_.offset());
        if (rc) return rc;
        // Find and read ExifIFD sub-IFD of IFD0
        rc = ifd0_.readSubIfd(exifIfd_, pData_, size_, byteOrder(), 0x8769);
        if (rc) return rc;
        // Find MakerNote in ExifIFD, create a MakerNote class 
        Ifd::iterator pos = exifIfd_.findTag(0x927c);
        Ifd::iterator make = ifd0_.findTag(0x010f);
        Ifd::iterator model = ifd0_.findTag(0x0110);
        if (pos != exifIfd_.end() && make != ifd0_.end() && model != ifd0_.end()) {
            MakerNoteFactory& mnf = MakerNoteFactory::instance();
            // Todo: The conversion to string assumes that there is a \0 at the end
            // Todo: How to avoid the cast (is that a MSVC thing?)
            pMakerNote_ = mnf.create(reinterpret_cast<const char*>(make->data()), 
                                     reinterpret_cast<const char*>(model->data()), 
                                     false,
                                     pos->data(), 
                                     pos->size(),
                                     byteOrder(),
                                     exifIfd_.offset() + pos->offset());
        }
        // Read the MakerNote
        if (pMakerNote_) {
            rc = pMakerNote_->read(pos->data(), 
                                   pos->size(),
                                   byteOrder(),
                                   exifIfd_.offset() + pos->offset());
            if (rc) {
                // Todo: How to handle debug output like this
                std::cerr << "Warning: Failed to read " 
                          << pMakerNote_->ifdItem()
                          << " Makernote, rc = " << rc << "\n";

                delete pMakerNote_;
                pMakerNote_ = 0;
            }
        }
        // If we successfully parsed the MakerNote, delete the raw MakerNote,
        // the parsed MakerNote is the primary MakerNote from now on
        if (pMakerNote_) {
            exifIfd_.erase(pos);
        }
        // Find and read Interoperability IFD in ExifIFD
        rc = exifIfd_.readSubIfd(iopIfd_, pData_, size_, byteOrder(), 0xa005);
        if (rc) return rc;
        // Find and read GPSInfo sub-IFD in IFD0
        rc = ifd0_.readSubIfd(gpsIfd_, pData_, size_, byteOrder(), 0x8825);
        if (rc) return rc;
        // Read IFD1
        if (ifd0_.next()) {
            rc = ifd1_.read(pData_ + ifd0_.next(), 
                            size_ - ifd0_.next(), 
                            byteOrder(), 
                            ifd0_.next());
            if (rc) return rc;
        }
        // Find and delete ExifIFD sub-IFD of IFD1
        pos = ifd1_.findTag(0x8769);
        if (pos != ifd1_.end()) {
            ifd1_.erase(pos);
            ret = 7;
        }
        // Find and delete GPSInfo sub-IFD in IFD1
        pos = ifd1_.findTag(0x8825);
        if (pos != ifd1_.end()) {
            ifd1_.erase(pos);
            ret = 7;
        }
        // Copy all entries from the IFDs and the MakerNote to the metadata
        exifMetadata_.clear();
        add(ifd0_.begin(), ifd0_.end(), byteOrder());
        add(exifIfd_.begin(), exifIfd_.end(), byteOrder());
        if (pMakerNote_) {
            add(pMakerNote_->begin(), pMakerNote_->end(), pMakerNote_->byteOrder());
        }
        add(iopIfd_.begin(), iopIfd_.end(), byteOrder()); 
        add(gpsIfd_.begin(), gpsIfd_.end(), byteOrder());
        add(ifd1_.begin(), ifd1_.end(), byteOrder());
        // Read the thumbnail (but don't worry whether it was successful or not)
        readThumbnail();

        return ret;
    } // ExifData::read

    int ExifData::erase(const std::string& path) const
    {
        if (!fileExists(path, true)) return -1;
        Image::AutoPtr image = ImageFactory::instance().open(path);
        if (image.get() == 0) return -2;

        // Read all metadata then erase only Exif data
        int rc = image->readMetadata();
        if (rc == 0) {
            image->clearExifData();
            rc = image->writeMetadata();
        }
        return rc;
    } // ExifData::erase

    int ExifData::write(const std::string& path) 
    {
        // Remove the Exif section from the file if there is no metadata 
        if (count() == 0 && !pThumbnail_) return erase(path);

        if (!fileExists(path, true)) return -1;
        Image::AutoPtr image = ImageFactory::instance().open(path);
        if (image.get() == 0) return -2;

        DataBuf buf(size());
        long actualSize = copy(buf.pData_);
        assert(actualSize <= buf.size_);

        // Read all metadata to preserve non-Exif data
        int rc = image->readMetadata();
        if (rc == 0) {
            image->setExifData(buf.pData_, actualSize);
            rc = image->writeMetadata();
        }
        return rc;
    } // ExifData::write

    long ExifData::copy(byte* buf)
    {
        long size = 0;
        // If we can update the internal IFDs and the underlying data buffer
        // from the metadata without changing the data size, then it is enough
        // to copy the data buffer.
        if (compatible_ && updateEntries()) {
#ifdef DEBUG_MAKERNOTE
            std::cerr << "->>>>>> using non-intrusive writing <<<<<<-\n";
#endif
            memcpy(buf, pData_, size_);
            size = size_;
        }
        // Else we have to do it the hard way...
        else {
#ifdef DEBUG_MAKERNOTE
            std::cerr << "->>>>>> writing from metadata <<<<<<-\n";
#endif
            size = copyFromMetadata(buf);
        }
        return size;
    }

    long ExifData::copyFromMetadata(byte* buf)
    {
        // Build IFD0
        Ifd ifd0(ifd0Id);
        addToIfd(ifd0, begin(), end(), byteOrder());

        // Build Exif IFD from metadata
        Ifd exifIfd(exifIfdId);
        addToIfd(exifIfd, begin(), end(), byteOrder());
        MakerNote* pMakerNote = 0;
        if (pMakerNote_) {
            // Build MakerNote from metadata
            pMakerNote = pMakerNote_->clone();
            addToMakerNote(pMakerNote, begin(), end(), pMakerNote_->byteOrder());
            // Create a placeholder MakerNote entry of the correct size and
            // add it to the Exif IFD (because we don't know the offset yet)
            Entry e;
            e.setIfdId(exifIfd.ifdId());
            e.setTag(0x927c);
            DataBuf buf(pMakerNote->size());
            memset(buf.pData_, 0x0, buf.size_);
            e.setValue(undefined, buf.size_, buf.pData_, buf.size_);
            exifIfd.erase(0x927c);
            exifIfd.add(e);
        }

        // Build Interoperability IFD from metadata
        Ifd iopIfd(iopIfdId);
        addToIfd(iopIfd, begin(), end(), byteOrder());

        // Build GPSInfo IFD from metadata
        Ifd gpsIfd(gpsIfdId);
        addToIfd(gpsIfd, begin(), end(), byteOrder());

        // Compute the new IFD offsets
        int exifIdx = ifd0.erase(0x8769);
        int gpsIdx  = ifd0.erase(0x8825);
        int iopIdx  = exifIfd.erase(0xa005);

        long ifd0Offset = tiffHeader_.size();
        bool addOffsetTag = false;
        long exifIfdOffset = ifd0Offset + ifd0.size() + ifd0.dataSize();
        if (exifIfd.size() > 0 || iopIfd.size() > 0) {
            exifIfdOffset += 12; 
            addOffsetTag = true; 
        }
        if (gpsIfd.size() > 0) {
            exifIfdOffset += 12; 
            addOffsetTag = true; 
        }
        if (ifd0.size() == 0 && addOffsetTag) {
            exifIfdOffset += 6; 
        }
        addOffsetTag = false;
        long iopIfdOffset = exifIfdOffset + exifIfd.size() + exifIfd.dataSize(); 
        if (iopIfd.size() > 0) {
            iopIfdOffset += 12;
            addOffsetTag = true; 
        }
        if (exifIfd.size() == 0 && addOffsetTag) {
            iopIfdOffset += 6;
        }
        long gpsIfdOffset = iopIfdOffset + iopIfd.size() + iopIfd.dataSize();
        long ifd1Offset   = gpsIfdOffset + gpsIfd.size() + gpsIfd.dataSize();

        // build IFD1 from updated metadata if there is a thumbnail
        Ifd ifd1(ifd1Id, ifd1Offset);
        if (pThumbnail_) {
            // Update Exif data from thumbnail
            pThumbnail_->update(*this);
            addToIfd(ifd1, begin(), end(), byteOrder());
            pThumbnail_->setOffsets(ifd1, byteOrder());
            // Set the offset to IFD1 in IFD0
            if (ifd1.size() > 0) {
                ifd0.setNext(ifd1Offset, byteOrder());
            }
        }

        // Set the offset to the Exif IFD in IFD0
        if (exifIfd.size() > 0 || iopIfd.size() > 0) {
            setOffsetTag(ifd0, exifIdx, 0x8769, exifIfdOffset, byteOrder());
        }
        // Set the offset to the GPSInfo IFD in IFD0
        if (gpsIfd.size() > 0) {
            setOffsetTag(ifd0, gpsIdx, 0x8825, gpsIfdOffset, byteOrder());
        }
        // Set the offset to the Interoperability IFD in Exif IFD
        if (iopIfd.size() > 0) {
            setOffsetTag(exifIfd, iopIdx, 0xa005, iopIfdOffset, byteOrder());
        }

        // Copy the TIFF header, all IFDs, MakerNote and thumbnail to the buffer
        tiffHeader_.copy(buf);
        ifd0.sortByTag();
        ifd0.copy(buf + ifd0Offset, byteOrder(), ifd0Offset);
        exifIfd.sortByTag();
        exifIfd.copy(buf + exifIfdOffset, byteOrder(), exifIfdOffset);
        if (pMakerNote) {
            // Copy the MakerNote over the placeholder data
            Entries::iterator mn = exifIfd.findTag(0x927c);
            // Do _not_ sort the makernote; vendors (at least Canon), don't seem
            // to bother about this TIFF standard requirement, so writing the
            // makernote as is might result in fewer deviations from the original
            pMakerNote->copy(buf + exifIfdOffset + mn->offset(),
                             byteOrder(),
                             exifIfdOffset + mn->offset());
            delete pMakerNote;
            pMakerNote = 0;
        }
        iopIfd.sortByTag();
        iopIfd.copy(buf + iopIfdOffset, byteOrder(), iopIfdOffset);
        gpsIfd.sortByTag();
        gpsIfd.copy(buf + gpsIfdOffset, byteOrder(), gpsIfdOffset);
        long len = ifd1Offset;
        if (pThumbnail_) {
            ifd1.sortByTag();
            ifd1.copy(buf + ifd1Offset, byteOrder(), ifd1Offset);
            long thumbOffset = ifd1Offset + ifd1.size() + ifd1.dataSize();
            len = thumbOffset;
            len += pThumbnail_->copy(buf + thumbOffset);
        }

        return len;

    } // ExifData::copyFromMetadata

    long ExifData::size() const
    {
        long size;
        if (compatible()) {
            size = size_;
        }
        else {
            size = tiffHeader_.size();
            std::map<IfdId, int> ifdEntries;
            const_iterator mdEnd = this->end();
            for (const_iterator md = begin(); md != mdEnd; ++md) {
                size += md->size();
                ifdEntries[md->ifdId()] += 1;
            }
            std::map<IfdId, int>::const_iterator eEnd = ifdEntries.end();
            std::map<IfdId, int>::const_iterator e;
            for (e = ifdEntries.begin(); e != eEnd; ++e) {
                // Skip IFD1 entries if there is no thumbnail (maybe it was deleted)
                if (e->first == ifd1Id && !pThumbnail_) continue;
                size += 2 + 12 * e->second + 4;
            }
            // Add the size of the thumbnail image data (w/o IFD for TIFF thumbs)
            if (pThumbnail_) {
                size += pThumbnail_->dataSize();
            }
            // Add 1k to account for the possibility that Thumbnail::update
            // may add entries to IFD1
            size += 1024;
        }
        return size;
    } // ExifData::size

    int ExifData::writeExifData(const std::string& path)
    {
        DataBuf buf(this->size());
        long actualSize = copy(buf.pData_);
        assert(actualSize <= buf.size_);

        ExvImage exvImage(path, true);
        if (!exvImage.good()) return -1;
        exvImage.setExifData(buf.pData_, actualSize);
        return exvImage.writeMetadata();
    } // ExifData::writeExifData

    void ExifData::add(Entries::const_iterator begin, 
                       Entries::const_iterator end,
                       ByteOrder byteOrder)
    {
        Entries::const_iterator i = begin;
        for (; i != end; ++i) {
            add(Exifdatum(*i, byteOrder));
        }
    }

    void ExifData::add(const ExifKey& key, Value* pValue)
    {
        add(Exifdatum(key, pValue));
    }

    void ExifData::add(const Exifdatum& exifdatum)
    {
        if (exifdatum.ifdId() == makerIfdId) {
            if (   pMakerNote_ 
                && pMakerNote_->ifdItem() != exifdatum.groupName()) {
                throw Error("Inconsistent MakerNote");
            }
            if (!pMakerNote_) {
                MakerNoteFactory& mnf = MakerNoteFactory::instance();
                pMakerNote_ = mnf.create(exifdatum.groupName());
            }
        }
        // allow duplicates
        exifMetadata_.push_back(exifdatum);
    }

    ExifData::const_iterator ExifData::findKey(const ExifKey& key) const
    {
        return std::find_if(exifMetadata_.begin(), exifMetadata_.end(),
                            FindMetadatumByKey(key.key()));
    }

    ExifData::iterator ExifData::findKey(const ExifKey& key)
    {
        return std::find_if(exifMetadata_.begin(), exifMetadata_.end(),
                            FindMetadatumByKey(key.key()));
    }

    ExifData::const_iterator ExifData::findIfdIdIdx(IfdId ifdId, int idx) const
    {
        return std::find_if(exifMetadata_.begin(), exifMetadata_.end(),
                            FindMetadatumByIfdIdIdx(ifdId, idx));
    }

    ExifData::iterator ExifData::findIfdIdIdx(IfdId ifdId, int idx)
    {
        return std::find_if(exifMetadata_.begin(), exifMetadata_.end(),
                            FindMetadatumByIfdIdIdx(ifdId, idx));
    }

    void ExifData::sortByKey()
    {
        std::sort(exifMetadata_.begin(), exifMetadata_.end(), cmpMetadataByKey);
    }

    void ExifData::sortByTag()
    {
        std::sort(exifMetadata_.begin(), exifMetadata_.end(), cmpMetadataByTag);
    }

    ExifData::iterator ExifData::erase(ExifData::iterator pos)
    {
        return exifMetadata_.erase(pos);
    }

    long ExifData::eraseThumbnail()
    {
        // Delete all Thumbnail.*.* (IFD1) metadata 
        ExifMetadata::iterator i = begin(); 
        while (i != end()) {
            if (i->ifdId() == ifd1Id) {
                i = erase(i);
            }
            else {
                ++i;
            }
        }
        long delta = 0;
        if (stdThumbPosition()) {
            delta = size_;
            if (size_ > 0 && ifd0_.next() > 0) {
                // Truncate IFD1 and thumbnail data from the data buffer
                size_ = ifd0_.next();
                ifd0_.setNext(0, byteOrder());
            }
            delta -= size_;
        }
        else {
            // We will have to write the hard way and re-arrange the data
            compatible_ = false;
            delta = ifd1_.size() + ifd1_.dataSize() 
                + pThumbnail_ ? pThumbnail_->size() : 0; 
        }
        // Delete the thumbnail itself
        if (pThumbnail_) {
            delete pThumbnail_;
            pThumbnail_ = 0;
        }
        return delta;
    } // ExifData::eraseThumbnail

    bool ExifData::stdThumbPosition() const
    {
        // Todo: There is still an invalid assumption here: The data of an IFD
        //       can be stored in multiple non-contiguous blocks. In this case,
        //       dataOffset + dataSize does not point to the end of the IFD data.
        //       in particular, this is potentially the case for the remaining Exif
        //       data in the presence of a known Makernote.
        bool rc = true;
        if (pThumbnail_) {
            long maxOffset;
            maxOffset = std::max(ifd0_.offset(), ifd0_.dataOffset());
            maxOffset = std::max(maxOffset, exifIfd_.offset());
            maxOffset = std::max(maxOffset,   exifIfd_.dataOffset() 
                                            + exifIfd_.dataSize());
            if (pMakerNote_) {
                maxOffset = std::max(maxOffset,   pMakerNote_->offset()
                                                + pMakerNote_->size());
            }
            maxOffset = std::max(maxOffset, iopIfd_.offset());
            maxOffset = std::max(maxOffset,   iopIfd_.dataOffset()
                                            + iopIfd_.dataSize());
            maxOffset = std::max(maxOffset, gpsIfd_.offset());
            maxOffset = std::max(maxOffset,   gpsIfd_.dataOffset()
                                            + gpsIfd_.dataSize());

            if (   maxOffset > ifd1_.offset()
                || maxOffset > ifd1_.dataOffset() && ifd1_.dataOffset() > 0
                || maxOffset > pThumbnail_->offset()) rc = false;
        }
        return rc;
    } // ExifData::stdThumbPosition

    int ExifData::readThumbnail()
    {
        delete pThumbnail_;
        pThumbnail_ = 0;
        int rc = -1;
        const_iterator pos 
            = findKey(ExifKey("Exif.Thumbnail.Compression"));
        if (pos != end()) {
            long compression = pos->toLong();
            if (compression == 6) {
                pThumbnail_ = new JpegThumbnail;
            }
            else {
                pThumbnail_ = new TiffThumbnail;
            }
            rc = pThumbnail_->read(pData_, size_, *this, byteOrder());
            if (rc != 0) {
                delete pThumbnail_;
                pThumbnail_ = 0;
            }
        }

        return rc;
    } // ExifData::readThumbnail

    bool ExifData::updateEntries()
    {
        if (!this->compatible()) return false;

        bool compatible = true;
        compatible &= updateRange(ifd0_.begin(), ifd0_.end(), byteOrder());
        compatible &= updateRange(exifIfd_.begin(), exifIfd_.end(), byteOrder());
        if (pMakerNote_) {
            compatible &= updateRange(pMakerNote_->begin(), 
                                      pMakerNote_->end(), 
                                      pMakerNote_->byteOrder());
        }
        compatible &= updateRange(iopIfd_.begin(), iopIfd_.end(), byteOrder());
        compatible &= updateRange(gpsIfd_.begin(), gpsIfd_.end(), byteOrder());
        if (pThumbnail_) {
            compatible &= updateRange(ifd1_.begin(), ifd1_.end(), byteOrder());
        }

        return compatible;
    } // ExifData::updateEntries

    bool ExifData::updateRange(const Entries::iterator& begin, 
                               const Entries::iterator& end,
                               ByteOrder byteOrder)
    {
        bool compatible = true;
        for (Entries::iterator entry = begin; entry != end; ++entry) {
            // find the corresponding Exifdatum
            const_iterator md = findIfdIdIdx(entry->ifdId(), entry->idx());
            if (md == this->end()) {
                // corresponding Exifdatum was deleted: this is not (yet) a
                // supported non-intrusive write operation.
                compatible = false;
                continue;
            }
            if (entry->count() == 0 && md->count() == 0) {
                // Special case: don't do anything if both the entry and 
                // Exifdatum have no data. This is to preserve the original
                // data in the offset field of an IFD entry with count 0,
                // if the Exifdatum was not changed.
            }
            else {
                DataBuf buf(md->size());
                md->copy(buf.pData_, byteOrder);
                entry->setValue(static_cast<uint16_t>(md->typeId()), md->count(), 
                                buf.pData_, md->size());
            }
        }
        return compatible;
    } // ExifData::updateRange

    bool ExifData::compatible() const
    {
        bool compatible = true;
        // For each Exifdatum, check if it is compatible with the corresponding
        // IFD or MakerNote entry
        for (const_iterator md = begin(); md != this->end(); ++md) {
            // Skip IFD1 entries if there is no thumbnail (maybe it was deleted)
            if (md->ifdId() == ifd1Id && !pThumbnail_) continue;
            std::pair<bool, Entries::const_iterator> rc;
            rc = findEntry(md->ifdId(), md->idx());
            // Make sure that we have an entry
            if (!rc.first) {
                compatible = false;
                break;
            }
            // Make sure that the size of the Exifdatum fits the available size
            // of the entry
            if (md->size() > rc.second->size()) {
                compatible = false;
                break;
            }
        }
        return compatible;
    } // ExifData::compatible

    std::pair<bool, Entries::const_iterator> 
    ExifData::findEntry(IfdId ifdId, int idx) const
    {
        Entries::const_iterator entry;
        std::pair<bool, Entries::const_iterator> rc(false, entry);

        if (ifdId == makerIfdId && pMakerNote_) {
            entry = pMakerNote_->findIdx(idx);
            if (entry != pMakerNote_->end()) {
                rc.first = true;
                rc.second = entry;
            }
            return rc;
        }
        const Ifd* ifd = getIfd(ifdId);
        if (ifdId != makerIfdId && ifd) {
            entry = ifd->findIdx(idx);
            if (entry != ifd->end()) {
                rc.first = true;
                rc.second = entry;
            }
        }
        return rc;
    } // ExifData::findEntry

    const Ifd* ExifData::getIfd(IfdId ifdId) const
    {
        const Ifd* ifd = 0;
        switch (ifdId) {
        case ifd0Id: 
            ifd = &ifd0_;
            break;
        case exifIfdId: 
            ifd = &exifIfd_;
            break;
        case iopIfdId: 
            ifd = &iopIfd_;
            break;
        case gpsIfdId: 
            ifd = &gpsIfd_;
            break;
        case ifd1Id: 
            ifd = &ifd1_;
            break;
        default:
            ifd = 0;
            break;
        }
        return ifd;
    } // ExifData::getIfd

    std::string ExifData::strError(int rc, const std::string& path)
    {
        std::string error = path + ": ";
        switch (rc) {
        case -1:
            error += "Failed to open the file";
            break;
        case -2:
            error += "The file contains data of an unknown image type";
            break;
        case -3:
            error += "Couldn't open temporary file";
            break;
        case -4:
            error += "Renaming temporary file failed";
            break;
        case 1:
            error += "Couldn't read from the input stream";
            break;
        case 2:
            error += "This does not look like a JPEG image";
            break;
        case 3:
            error += "No Exif data found in the file";
            break;
        case 4:
            error += "Writing to the output stream failed";
            break;
        case 5:
            error += "No JFIF APP0 or Exif APP1 segment found in the file";
            break;
        case 6:
            error += "Exif data contains a broken IFD";
            break;
        case 7:
            error += "Unsupported Exif or GPS data found in IFD1";
            break;

        default:
            error += "Accessing Exif data failed, rc = " + toString(rc);
            break;
        }
        return error;
    } // ExifData::strError

    // *************************************************************************
    // free functions

    void addToIfd(Ifd& ifd, 
                  ExifMetadata::const_iterator begin, 
                  ExifMetadata::const_iterator end, 
                  ByteOrder byteOrder)
    {
        for (ExifMetadata::const_iterator i = begin; i != end; ++i) {
            // add only metadata with matching IFD id
            if (i->ifdId() == ifd.ifdId()) {
                addToIfd(ifd, *i, byteOrder);
            }
        }
    } // addToIfd

    void addToIfd(Ifd& ifd, const Exifdatum& md, ByteOrder byteOrder)
    {
        assert(ifd.alloc());

        Entry e;
        e.setIfdId(md.ifdId());
        e.setIdx(md.idx());
        e.setTag(md.tag());
        e.setOffset(0);  // will be calculated when the IFD is written

        DataBuf buf(md.size());
        md.copy(buf.pData_, byteOrder);
        e.setValue(static_cast<uint16_t>(md.typeId()), md.count(), 
                   buf.pData_, md.size()); 
        ifd.add(e);
    } // addToIfd

    void addToMakerNote(MakerNote* makerNote,
                        ExifMetadata::const_iterator begin,
                        ExifMetadata::const_iterator end, 
                        ByteOrder byteOrder)
    {
        for (ExifMetadata::const_iterator i = begin; i != end; ++i) {
            // add only metadata with IFD id 'makerIfd'
            if (i->ifdId() == makerIfdId) {
                addToMakerNote(makerNote, *i, byteOrder);
            }
        }
    } // addToMakerNote

    void addToMakerNote(MakerNote* makerNote, 
                        const Exifdatum& md, 
                        ByteOrder byteOrder)
    {
        Entry e;
        e.setIfdId(md.ifdId());
        e.setIdx(md.idx());
        e.setTag(md.tag());
        e.setOffset(0);  // will be calculated when the makernote is written

        DataBuf buf(md.size());
        md.copy(buf.pData_, byteOrder);
        e.setValue(static_cast<uint16_t>(md.typeId()), md.count(),
                   buf.pData_, md.size()); 
        makerNote->add(e);
    } // addToMakerNote

    std::ostream& operator<<(std::ostream& os, const Exifdatum& md)
    {
        return md.pKey_->printTag(os, md.value());
    }

}                                       // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {

    void setOffsetTag(Exiv2::Ifd& ifd,
                      int idx,
                      uint16_t tag,
                      uint32_t offset, 
                      Exiv2::ByteOrder byteOrder)
    {
        Exiv2::Ifd::iterator pos = ifd.findTag(tag);
        if (pos == ifd.end()) {
            Exiv2::Entry e(ifd.alloc());
            e.setIfdId(ifd.ifdId());
            e.setIdx(idx);
            e.setTag(tag);
            e.setOffset(0);  // will be calculated when the IFD is written
            ifd.add(e);
            pos = ifd.findTag(tag);
        }
        pos->setValue(offset, byteOrder);
    }

}
