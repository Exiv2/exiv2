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
  Version:   $Name:  $ $Revision: 1.34 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.34 $ $RCSfile: exif.cpp,v $")

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
#include <iomanip>
#include <sstream>
#include <fstream>
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
    void setOffsetTag(Exif::Ifd& ifd,
                      int idx,
                      Exif::uint16 tag,
                      Exif::uint32 offset, 
                      Exif::ByteOrder byteOrder);

}

// *****************************************************************************
// class member definitions
namespace Exif {

    Metadatum::Metadatum(const Entry& e, ByteOrder byteOrder)
        : tag_(e.tag()), ifdId_(e.ifdId()), idx_(e.idx()), 
          pMakerNote_(e.makerNote()), pValue_(0), key_(makeKey(e))
    {
        pValue_ = Value::create(TypeId(e.type()));
        pValue_->read(e.data(), e.count() * e.typeSize(), byteOrder);
    }

    Metadatum::Metadatum(const std::string& key, 
                         const Value* value, 
                         MakerNote* makerNote)
        : idx_(0), pMakerNote_(makerNote), pValue_(0), key_(key)
    {
        if (value) pValue_ = value->clone();
        std::pair<uint16, IfdId> p = decomposeKey(key, makerNote);
        if (p.first == 0xffff) throw Error("Invalid key");
        tag_ = p.first;
        if (p.second == ifdIdNotSet) throw Error("Invalid key");
        ifdId_ = p.second;
    }

    Metadatum::~Metadatum()
    {
        delete pValue_;
        // do *not* delete the MakerNote
    }

    Metadatum::Metadatum(const Metadatum& rhs)
        : tag_(rhs.tag_), ifdId_(rhs.ifdId_), idx_(rhs.idx_),
          pMakerNote_(rhs.pMakerNote_), pValue_(0), key_(rhs.key_)
    {
        if (rhs.pValue_ != 0) pValue_ = rhs.pValue_->clone(); // deep copy
    }

    Metadatum& Metadatum::operator=(const Metadatum& rhs)
    {
        if (this == &rhs) return *this;
        tag_ = rhs.tag_;
        ifdId_ = rhs.ifdId_;
        idx_ = rhs.idx_;
        pMakerNote_ = rhs.pMakerNote_;
        delete pValue_;
        pValue_ = 0;
        if (rhs.pValue_ != 0) pValue_ = rhs.pValue_->clone(); // deep copy
        key_ = rhs.key_;
        return *this;
    } // Metadatum::operator=
    
    void Metadatum::setValue(const Value* pValue)
    {
        delete pValue_;
        pValue_ = pValue->clone();
    }

    void Metadatum::setValue(const Entry& e, ByteOrder byteOrder)
    {
        delete pValue_;
        pValue_ = Value::create(TypeId(e.type()));
        pValue_->read(e.data(), e.count() * e.typeSize(), byteOrder);
    }

    void Metadatum::setValue(const std::string& buf)
    {
        if (pValue_ == 0) pValue_ = Value::create(asciiString);
        pValue_->read(buf);
    }

    std::string Metadatum::tagName() const
    {
        if (ifdId_ == makerIfd && pMakerNote_ != 0) {
            return pMakerNote_->tagName(tag_);
        }
        return ExifTags::tagName(tag_, ifdId_); 
    }

    std::string Metadatum::sectionName() const 
    {
        if (ifdId_ == makerIfd && pMakerNote_ != 0) {
            return pMakerNote_->sectionName(tag_);
        }        
        return ExifTags::sectionName(tag_, ifdId_); 
    }

    TiffThumbnail::TiffThumbnail()
        : size_(0), pImage_(0), ifd_(ifd1, 0, false)
    {
    }

    TiffThumbnail::~TiffThumbnail()
    {
        delete[] pImage_;
    }

    TiffThumbnail::TiffThumbnail(const TiffThumbnail& rhs)
        : size_(rhs.size_), pImage_(0), ifd_(ifd1, 0, false)
    {
        if (rhs.pImage_ && rhs.size_ > 0) {
            pImage_ = new char[rhs.size_];
            memcpy(pImage_, rhs.pImage_, rhs.size_);
            tiffHeader_.read(pImage_);
            ifd_.read(pImage_ + tiffHeader_.offset(),
                      tiffHeader_.byteOrder(), tiffHeader_.offset());
        }
    }

    TiffThumbnail& TiffThumbnail::operator=(const TiffThumbnail& rhs)
    {
        char* pNewImage = 0;
        if (rhs.pImage_ && rhs.size_ > 0) {
            pNewImage = new char[rhs.size_];
            memcpy(pNewImage, rhs.pImage_, rhs.size_);
            tiffHeader_.read(rhs.pImage_);
            ifd_.read(pNewImage + tiffHeader_.offset(), 
                      tiffHeader_.byteOrder(), tiffHeader_.offset());
        }
        size_ = rhs.size_;
        delete[] pImage_;
        pImage_ = pNewImage;
        return *this;
    }

    int TiffThumbnail::read(const char* buf,
                            const ExifData& exifData,
                            ByteOrder byteOrder)
    {
        char* data = new char[64*1024];     // temporary buffer Todo: handle larger
        memset(data, 0x0, 64*1024);         // images (which violate the Exif Std)
        long len = 0;                       // number of bytes in the buffer

        // Copy the TIFF header
        TiffHeader tiffHeader(byteOrder);
        len += tiffHeader.copy(data);

        // Create IFD (without Exif and GPS tags) from metadata
        Ifd ifd1(ifd1);
        addToIfd(ifd1, exifData.begin(), exifData.end(), tiffHeader.byteOrder());
        Ifd::iterator i = ifd1.findTag(0x8769);
        if (i != ifd1.end()) ifd1.erase(i);
        i = ifd1.findTag(0x8825);
        if (i != ifd1.end()) ifd1.erase(i);

        // Do not copy the IFD yet, remember the location and leave a gap
        long ifdOffset = len;
        len += ifd1.size() + ifd1.dataSize();

        // Copy thumbnail image data, remember the offsets used
        std::string key = "Thumbnail.RecordingOffset.StripOffsets";
        ExifData::const_iterator offsets = exifData.findKey(key);
        if (offsets == exifData.end()) return 2;
        key = "Thumbnail.RecordingOffset.StripByteCounts";
        ExifData::const_iterator sizes = exifData.findKey(key);
        if (sizes == exifData.end()) return 2;
        std::ostringstream os;                  // for the new strip offsets
        for (long k = 0; k < offsets->count(); ++k) {
            long offset = offsets->toLong(k);
            long size = sizes->toLong(k);
            memcpy(data + len, buf + offset, size);
            os << len << " ";
            len += size;
        }

        // Update the IFD with the actual strip offsets (replace existing entry)
        Metadatum newOffsets(*offsets);
        newOffsets.setValue(os.str());
        i = ifd1.findTag(0x0111);
        if (i != ifd1.end()) ifd1.erase(i);
        addToIfd(ifd1, newOffsets, tiffHeader.byteOrder());

        // Finally, sort and copy the IFD
        ifd1.sortByTag();
        ifd1.copy(data + ifdOffset, tiffHeader.byteOrder(), ifdOffset);

        delete[] pImage_;
        pImage_ = new char[len];
        memcpy(pImage_, data, len);
        size_ = len;
        tiffHeader_.read(pImage_);
        ifd_.read(pImage_ + tiffHeader_.offset(), 
                  tiffHeader_.byteOrder(), tiffHeader_.offset());
        delete[] data;

        return 0;
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
        std::ofstream file(name.c_str(), std::ios::binary);
        if (!file) return 1;
        file.write(pImage_, size_);
        if (!file.good()) return 2;
        return 0;
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
            exifData.add(Metadatum(*entry, tiffHeader_.byteOrder()));
        }
        else {
            md->setValue(*entry, tiffHeader_.byteOrder());
        }

        entry = ifd_.findTag(0x0117);
        if (entry == ifd_.end()) throw Error("Bad thumbnail (0x0117)");
        md = exifData.findIfdIdIdx(entry->ifdId(), entry->idx());
        if (md == exifData.end()) {
            exifData.add(Metadatum(*entry, tiffHeader_.byteOrder()));
        }
        else {
            md->setValue(*entry, tiffHeader_.byteOrder());
        }

    } // TiffThumbnail::update

    long TiffThumbnail::copy(char* buf) const
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

    void TiffThumbnail::setOffsets(Ifd& ifd1, ByteOrder byteOrder) const
    {
        // Adjust the StripOffsets, assuming that the existing TIFF strips
        // start immediately after the thumbnail IFD
        long shift = ifd1.offset() + ifd1.size() + ifd1.dataSize() 
            - ifd_.offset() - ifd_.size() - ifd_.dataSize();
        Ifd::const_iterator pos = ifd_.findTag(0x0111);
        if (pos == ifd_.end()) throw Error("Bad thumbnail (0x0111)");
        Metadatum offsets(*pos, tiffHeader_.byteOrder());
        std::ostringstream os;
        for (long k = 0; k < offsets.count(); ++k) {
            os << offsets.toLong(k) + shift << " ";
        }
        offsets.setValue(os.str());
        // Update the IFD with the re-calculated strip offsets 
        // (replace existing entry)
        Ifd::iterator i = ifd1.findTag(0x0111);
        if (i != ifd1.end()) ifd1.erase(i);
        addToIfd(ifd1, offsets, byteOrder);

    } // TiffThumbnail::setOffsets

    JpegThumbnail::JpegThumbnail()
        : size_(0), pImage_(0)
    {
    }

    JpegThumbnail::~JpegThumbnail()
    {
        delete[] pImage_;
    }

    JpegThumbnail::JpegThumbnail(const JpegThumbnail& rhs)
        : size_(rhs.size_), pImage_(0)
    {
        if (rhs.pImage_ && rhs.size_ > 0) {
            pImage_ = new char[rhs.size_];
            memcpy(pImage_, rhs.pImage_, rhs.size_);
        }
    }

    JpegThumbnail& JpegThumbnail::operator=(const JpegThumbnail& rhs)
    {
        char* pNewImage = 0;
        if (rhs.pImage_ && rhs.size_ > 0) {
            pNewImage = new char[rhs.size_];
            memcpy(pNewImage, rhs.pImage_, rhs.size_);
        }
        size_ = rhs.size_;
        delete[] pImage_;
        pImage_ = pNewImage;
        return *this;
    }

    int JpegThumbnail::read(const char* buf, 
                            const ExifData& exifData,
                            ByteOrder byteOrder) 
    {
        std::string key = "Thumbnail.RecordingOffset.JPEGInterchangeFormat";
        ExifData::const_iterator pos = exifData.findKey(key);
        if (pos == exifData.end()) return 1;
        long offset = pos->toLong();
        key = "Thumbnail.RecordingOffset.JPEGInterchangeFormatLength";
        pos = exifData.findKey(key);
        if (pos == exifData.end()) return 1;
        long size = pos->toLong();
        delete[] pImage_;
        pImage_ = new char[size];
        memcpy(pImage_, buf + offset, size);
        size_ = size;
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
        std::ofstream file(name.c_str(), std::ios::binary);
        if (!file) return 1;
        file.write(pImage_, size_);
        if (!file.good()) return 2;
        return 0;
    } // JpegThumbnail::write

    void JpegThumbnail::update(ExifData& exifData) const
    {
        std::string key = "Thumbnail.RecordingOffset.JPEGInterchangeFormat";
        ExifData::iterator pos = exifData.findKey(key);
        if (pos == exifData.end()) {
            Value* value = Value::create(unsignedLong);
            exifData.add(key, value);
            delete value;
            pos = exifData.findKey(key);
        }
        pos->setValue("0");

        key = "Thumbnail.RecordingOffset.JPEGInterchangeFormatLength";
        pos = exifData.findKey(key);
        if (pos == exifData.end()) {
            Value *value = Value::create(unsignedLong);
            exifData.add(key, value);
            delete value;            
            pos = exifData.findKey(key);
        }
        pos->setValue(toString(size_));

    } // JpegThumbnail::update

    long JpegThumbnail::copy(char* buf) const
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

    void JpegThumbnail::setOffsets(Ifd& ifd1, ByteOrder byteOrder) const
    {
        Ifd::iterator pos = ifd1.findTag(0x0201);
        if (pos == ifd1.end()) throw Error("Bad thumbnail (0x0201)");
        pos->setValue(ifd1.offset() + ifd1.size() + ifd1.dataSize(), byteOrder);
    }

    ExifData::ExifData() 
        : pThumbnail_(0), pMakerNote_(0), ifd0_(ifd0, 0, false), 
          exifIfd_(exifIfd, 0, false), iopIfd_(iopIfd, 0, false), 
          gpsIfd_(gpsIfd, 0, false), ifd1_(ifd1, 0, false), 
          size_(0), pData_(0)
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
        std::ifstream file(path.c_str(), std::ios::binary);
        if (!file) return -1;
        Image* pImage = ImageFactory::instance().create(file);
        file.close();
        if (pImage == 0) return -2;
        // Todo: should we use file to read from? (and write isThisType so 
        //       that it doesn't advance the stream)
        int rc = pImage->readExifData(path);
        if (rc == 0) rc = read(pImage->exifData(), pImage->sizeExifData());
        delete pImage;
        return rc;
    }

    int ExifData::read(const char* buf, long len)
    {
        // Copy the data buffer
        delete[] pData_;
        pData_ = new char[len];
        memcpy(pData_, buf, len);
        size_ = len;

        // Read the TIFF header
        int ret = 0;
        int rc = tiffHeader_.read(pData_);
        if (rc) return rc;

        // Read IFD0
        rc = ifd0_.read(pData_ + tiffHeader_.offset(), 
                        byteOrder(), 
                        tiffHeader_.offset());
        if (rc) return rc;
        // Find and read ExifIFD sub-IFD of IFD0
        rc = ifd0_.readSubIfd(exifIfd_, pData_, byteOrder(), 0x8769);
        if (rc) return rc;
        // Find MakerNote in ExifIFD, create a MakerNote class 
        Ifd::iterator pos = exifIfd_.findTag(0x927c);
        Ifd::iterator make = ifd0_.findTag(0x010f);
        Ifd::iterator model = ifd0_.findTag(0x0110);
        if (pos != exifIfd_.end() && make != ifd0_.end() && model != ifd0_.end()) {
            MakerNoteFactory& mnf = MakerNoteFactory::instance();
            // Todo: The conversion to string assumes that there is a \0 at the end
            pMakerNote_ = mnf.create(make->data(), model->data(), false);
        }
        // Read the MakerNote
        if (pMakerNote_) {
            rc = pMakerNote_->read(pos->data(), 
                                   pos->size(),
                                   byteOrder(),
                                   exifIfd_.offset() + pos->offset());
            if (rc) {
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
        rc = exifIfd_.readSubIfd(iopIfd_, pData_, byteOrder(), 0xa005);
        if (rc) return rc;
        // Find and read GPSInfo sub-IFD in IFD0
        rc = ifd0_.readSubIfd(gpsIfd_, pData_, byteOrder(), 0x8825);
        if (rc) return rc;
        // Read IFD1
        if (ifd0_.next()) {
            rc = ifd1_.read(pData_ + ifd0_.next(), byteOrder(), ifd0_.next());
            if (rc) return rc;
        }
        // Find and delete ExifIFD sub-IFD of IFD1
        pos = ifd1_.findTag(0x8769);
        if (pos != ifd1_.end()) {
            ifd1_.erase(pos);
            ret = -99;
        }
        // Find and delete GPSInfo sub-IFD in IFD1
        pos = ifd1_.findTag(0x8825);
        if (pos != ifd1_.end()) {
            ifd1_.erase(pos);
            ret = -99;
        }

        // Copy all entries from the IFDs and the MakerNote to the metadata
        metadata_.clear();
        add(ifd0_.begin(), ifd0_.end(), byteOrder());
        add(exifIfd_.begin(), exifIfd_.end(), byteOrder());
        if (pMakerNote_) {
            add(pMakerNote_->begin(), pMakerNote_->end(), pMakerNote_->byteOrder());
        }
        add(iopIfd_.begin(), iopIfd_.end(), byteOrder()); 
        add(gpsIfd_.begin(), gpsIfd_.end(), byteOrder());
        add(ifd1_.begin(), ifd1_.end(), byteOrder());

        // Read the thumbnail
        readThumbnail();

        return ret;
    } // ExifData::read

    int ExifData::write(const std::string& path) 
    {
        long size = this->size();
        char* buf = new char[size];
        long actualSize = copy(buf);
        assert(actualSize <= size);

        std::ifstream file(path.c_str(), std::ios::binary);
        if (!file) return -1;
        Image* pImage = ImageFactory::instance().create(file);
        file.close();
        if (pImage == 0) return -2;
        pImage->setExifData(buf, actualSize);
        delete[] buf;
        // Todo: Should this method take a path and stream arg?
        //       Then we could reuse the file stream from above.
        int rc = pImage->writeExifData(path);
        delete pImage;
        return rc;
    } // ExifData::write

    long ExifData::copy(char* buf)
    {
        long size = 0;
        // If we can update the internal IFDs and the underlying data buffer
        // from the metadata without changing the data size, then it is enough
        // to copy the data buffer.
        if (updateEntries()) {

//ahu Todo: remove debugging output
std::cerr << "->>>>>> using non-intrusive writing <<<<<<-\n";

            memcpy(buf, pData_, size_);
            size = size_;
        }
        // Else we have to do it the hard way...
        else {

//ahu Todo: remove debugging output
std::cerr << "->>>>>> writing from metadata <<<<<<-\n";

            size = copyFromMetadata(buf);
        }
        return size;
    }

    long ExifData::copyFromMetadata(char* buf)
    {
        // Copy the TIFF header
        long ifd0Offset = tiffHeader_.copy(buf);

        // Build IFD0
        Ifd ifd0(ifd0, ifd0Offset);
        addToIfd(ifd0, begin(), end(), byteOrder());

        // Build Exif IFD from metadata
        long exifIfdOffset = ifd0Offset + ifd0.size() + ifd0.dataSize();
        Ifd exifIfd(exifIfd, exifIfdOffset);
        addToIfd(exifIfd, begin(), end(), byteOrder());
        MakerNote* makerNote = 0;
        if (pMakerNote_) {
            // Build MakerNote from metadata
            makerNote = pMakerNote_->clone();
            addToMakerNote(makerNote, begin(), end(), pMakerNote_->byteOrder());
            // Create a placeholder MakerNote entry of the correct size and
            // add it to the Exif IFD (because we don't know the offset yet)
            Entry e;
            e.setIfdId(exifIfd.ifdId());
            e.setTag(0x927c);
            long size = makerNote->size();
            char* buf = new char[size];
            memset(buf, 0x0, size);
            e.setValue(undefined, size, buf, size); 
            exifIfd.add(e);
            delete[] buf;
        }

        // Set the offset to the Exif IFD in IFD0
        int idx = ifd0.erase(0x8769);
        if (exifIfd.size() > 0) {
            setOffsetTag(ifd0, idx, 0x8769, exifIfdOffset, byteOrder());
        }

        // Build Interoperability IFD from metadata
        long iopIfdOffset = exifIfdOffset + exifIfd.size() + exifIfd.dataSize();
        Ifd iopIfd(iopIfd, iopIfdOffset);
        addToIfd(iopIfd, begin(), end(), byteOrder());

        // Set the offset to the Interoperability IFD in Exif IFD
        idx = exifIfd.erase(0xa005);
        if (iopIfd.size() > 0) {
            setOffsetTag(exifIfd, idx, 0xa005, iopIfdOffset, byteOrder());
        }

        // Build GPSInfo IFD from metadata
        long gpsIfdOffset = iopIfdOffset + iopIfd.size() + iopIfd.dataSize();
        Ifd gpsIfd(gpsIfd, gpsIfdOffset);
        addToIfd(gpsIfd, begin(), end(), byteOrder());

        // Set the offset to the GPSInfo IFD in IFD0
        idx = ifd0.erase(0x8825);
        if (gpsIfd.size() > 0) {
            setOffsetTag(ifd0, idx, 0x8825, gpsIfdOffset, byteOrder());
        }

        // build IFD1 from updated metadata if there is a thumbnail
        long ifd1Offset = gpsIfdOffset + gpsIfd.size() + gpsIfd.dataSize();
        Ifd ifd1(ifd1, ifd1Offset);
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

        // Copy all IFDs, the MakerNote data and the thumbnail to the data buffer
        ifd0.sortByTag();
        ifd0.copy(buf + ifd0Offset, byteOrder(), ifd0Offset);
        exifIfd.sortByTag();
        exifIfd.copy(buf + exifIfdOffset, byteOrder(), exifIfdOffset);
        if (makerNote) {
            // Copy the MakerNote over the placeholder data
            Entries::iterator mn = exifIfd.findTag(0x927c);
            // Do _not_ sort the makernote; vendors (at least Canon), don't seem
            // to bother about this TIFF standard requirement, so writing the
            // makernote as is might result in fewer deviations from the original
            makerNote->copy(buf + exifIfdOffset + mn->offset(),
                            byteOrder(),
                            exifIfdOffset + mn->offset());
            delete makerNote;
            makerNote = 0;
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
                if (e->first == ifd1 && !pThumbnail_) continue;
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

    void ExifData::add(Entries::const_iterator begin, 
                       Entries::const_iterator end,
                       ByteOrder byteOrder)
    {
        Entries::const_iterator i = begin;
        for (; i != end; ++i) {
            add(Metadatum(*i, byteOrder));
        }
    }

    void ExifData::add(const std::string& key, Value* value)
    {
        add(Metadatum(key, value));
    }

    void ExifData::add(const Metadatum& metadatum)
    {
        // allow duplicates
        metadata_.push_back(metadatum);
    }

    ExifData::const_iterator ExifData::findKey(const std::string& key) const
    {
        return std::find_if(metadata_.begin(), metadata_.end(),
                            FindMetadatumByKey(key));
    }

    ExifData::iterator ExifData::findKey(const std::string& key)
    {
        return std::find_if(metadata_.begin(), metadata_.end(),
                            FindMetadatumByKey(key));
    }

    ExifData::const_iterator ExifData::findIfdIdIdx(IfdId ifdId, int idx) const
    {
        return std::find_if(metadata_.begin(), metadata_.end(),
                            FindMetadatumByIfdIdIdx(ifdId, idx));
    }

    ExifData::iterator ExifData::findIfdIdIdx(IfdId ifdId, int idx)
    {
        return std::find_if(metadata_.begin(), metadata_.end(),
                            FindMetadatumByIfdIdIdx(ifdId, idx));
    }

    void ExifData::sortByKey()
    {
        std::sort(metadata_.begin(), metadata_.end(), cmpMetadataByKey);
    }

    void ExifData::sortByTag()
    {
        std::sort(metadata_.begin(), metadata_.end(), cmpMetadataByTag);
    }

    void ExifData::erase(ExifData::iterator pos)
    {
        metadata_.erase(pos);
    }

    long ExifData::eraseThumbnail()
    {
        // Delete all Thumbnail.*.* (IFD1) metadata 
        for (Metadata::iterator i = begin(); i != end(); ++i) {
            if (i->ifdId() == ifd1) erase(i);
        }
        // Truncate IFD1 and thumbnail data from the data buffer
        long delta = size_;
        if (size_ > 0) size_ = ifd0_.next();
        delta -= size_;
        ifd0_.setNext(0, byteOrder());
        // Delete the thumbnail itself
        delete pThumbnail_;
        pThumbnail_ = 0;
        return delta;
    }

    int ExifData::readThumbnail()
    {
        delete pThumbnail_;
        pThumbnail_ = 0;
        int rc = -1;
        const_iterator pos = findKey("Thumbnail.ImageStructure.Compression");
        if (pos != end()) {
            long compression = pos->toLong();
            if (compression == 6) {
                pThumbnail_ = new JpegThumbnail;
            }
            else {
                pThumbnail_ = new TiffThumbnail;
            }
            rc = pThumbnail_->read(pData_, *this, byteOrder());
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
        compatible |= updateRange(ifd0_.begin(), ifd0_.end(), byteOrder());
        compatible |= updateRange(exifIfd_.begin(), exifIfd_.end(), byteOrder());
        if (pMakerNote_) {
            compatible |= updateRange(pMakerNote_->begin(), 
                                      pMakerNote_->end(), 
                                      pMakerNote_->byteOrder());
        }
        compatible |= updateRange(iopIfd_.begin(), iopIfd_.end(), byteOrder());
        compatible |= updateRange(gpsIfd_.begin(), gpsIfd_.end(), byteOrder());
        if (pThumbnail_) {
            compatible |= updateRange(ifd1_.begin(), ifd1_.end(), byteOrder());
        }

        return compatible;
    } // ExifData::updateEntries

    bool ExifData::updateRange(const Entries::iterator& begin, 
                               const Entries::iterator& end,
                               ByteOrder byteOrder)
    {
        bool compatible = true;
        for (Entries::iterator entry = begin; entry != end; ++entry) {
            // find the corresponding metadatum
            const_iterator md = findIfdIdIdx(entry->ifdId(), entry->idx());
            if (md == this->end()) {
                // corresponding metadatum was deleted: this is not (yet) a
                // supported non-intrusive write operation.
                compatible = false;
                continue;
            }
            if (entry->count() == 0 && md->count() == 0) {
                // Special case: don't do anything if both the entry and 
                // metadatum have no data. This is to preserve the original
                // data in the offset field of an IFD entry with count 0,
                // if the metadatum was not changed.
            }
            else {
                char* buf = new char[md->size()];
                md->copy(buf, byteOrder);
                entry->setValue(md->typeId(), md->count(), buf, md->size());
                delete[] buf;
            }
        }
        return compatible;
    } // ExifData::updateRange

    bool ExifData::compatible() const
    {
        bool compatible = true;
        // For each metadatum, check if it is compatible with the corresponding
        // IFD or MakerNote entry
        for (const_iterator md = begin(); md != this->end(); ++md) {
            // Skip IFD1 entries if there is no thumbnail (maybe it was deleted)
            if (md->ifdId() == ifd1 && !pThumbnail_) continue;
            std::pair<bool, Entries::const_iterator> rc;
            rc = findEntry(md->ifdId(), md->idx());
            // Make sure that we have an entry
            if (!rc.first) {
                compatible = false;
                break;
            }
            // Make sure that the size of the metadatum fits the available size
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

        if (ifdId == makerIfd && pMakerNote_) {
            entry = pMakerNote_->findIdx(idx);
            if (entry != pMakerNote_->end()) {
                rc.first = true;
                rc.second = entry;
            }
            return rc;
        }
        const Ifd* ifd = getIfd(ifdId);
        if (ifdId != makerIfd && ifd) {
            entry = ifd->findIdx(idx);
            if (entry != ifd->end()) {
                rc.first = true;
                rc.second = entry;
            }
        }
        return rc;
    }

    const Ifd* ExifData::getIfd(IfdId ifdId) const
    {
        const Ifd* ifd = 0;
        switch (ifdId) {
        case ifd0: 
            ifd = &ifd0_;
            break;
        case exifIfd: 
            ifd = &exifIfd_;
            break;
        case iopIfd: 
            ifd = &iopIfd_;
            break;
        case gpsIfd: 
            ifd = &gpsIfd_;
            break;
        case ifd1: 
            ifd = &ifd1_;
            break;
        default:
            ifd = 0;
            break;
        }
        return ifd;
    }

    // *************************************************************************
    // free functions

    void addToIfd(Ifd& ifd, 
                  Metadata::const_iterator begin, 
                  Metadata::const_iterator end, 
                  ByteOrder byteOrder)
    {
        for (Metadata::const_iterator i = begin; i != end; ++i) {
            // add only metadata with matching IFD id
            if (i->ifdId() == ifd.ifdId()) {
                addToIfd(ifd, *i, byteOrder);
            }
        }
    } // addToIfd

    void addToIfd(Ifd& ifd, const Metadatum& metadatum, ByteOrder byteOrder)
    {
        assert(ifd.alloc());

        Entry e;
        e.setIfdId(metadatum.ifdId());
        e.setIdx(metadatum.idx());
        e.setTag(metadatum.tag());
        e.setOffset(0);  // will be calculated when the IFD is written
        char* buf = new char[metadatum.size()];
        metadatum.copy(buf, byteOrder);
        e.setValue(metadatum.typeId(), metadatum.count(), buf, metadatum.size()); 
        ifd.add(e);
        delete[] buf;
    } // addToIfd

    void addToMakerNote(MakerNote* makerNote,
                        Metadata::const_iterator begin,
                        Metadata::const_iterator end, 
                        ByteOrder byteOrder)
    {
        for (Metadata::const_iterator i = begin; i != end; ++i) {
            // add only metadata with IFD id 'makerIfd'
            if (i->ifdId() == makerIfd) {
                addToMakerNote(makerNote, *i, byteOrder);
            }
        }
    } // addToMakerNote

    void addToMakerNote(MakerNote* makerNote,
                        const Metadatum& metadatum,
                        ByteOrder byteOrder)
    {
        Entry e;
        e.setIfdId(metadatum.ifdId());
        e.setIdx(metadatum.idx());
        e.setTag(metadatum.tag());
        e.setOffset(0);  // will be calculated when the makernote is written
        char* buf = new char[metadatum.size()];
        metadatum.copy(buf, byteOrder);
        e.setValue(metadatum.typeId(), metadatum.count(), buf, metadatum.size()); 
        makerNote->add(e);
        delete[] buf;
    } // addToMakerNote

    bool cmpMetadataByTag(const Metadatum& lhs, const Metadatum& rhs)
    {
        return lhs.tag() < rhs.tag();
    }

    bool cmpMetadataByKey(const Metadatum& lhs, const Metadatum& rhs)
    {
        return lhs.key() < rhs.key();
    }

    std::ostream& operator<<(std::ostream& os, const Metadatum& md)
    {
        if (md.ifdId() == makerIfd && md.makerNote() != 0) {
            return md.makerNote()->printTag(os, md.tag(), md.value());
        }
        return ExifTags::printTag(os, md.tag(), md.ifdId(), md.value());
    }

    std::string makeKey(const Entry& entry)
    {
        if (entry.ifdId() == makerIfd && entry.makerNote() != 0) {
            return entry.makerNote()->makeKey(entry.tag());
        }
        return ExifTags::makeKey(entry.tag(), entry.ifdId());
    }

    std::pair<uint16, IfdId> decomposeKey(const std::string& key,
                                          const MakerNote* makerNote)
    {
        std::pair<uint16, IfdId> p = ExifTags::decomposeKey(key);
        if (p.second == makerIfd && makerNote != 0) {
            p.first = makerNote->decomposeKey(key);
        }
        return p;
    }

}                                       // namespace Exif

// *****************************************************************************
// local definitions
namespace {

    void setOffsetTag(Exif::Ifd& ifd,
                      int idx,
                      Exif::uint16 tag,
                      Exif::uint32 offset, 
                      Exif::ByteOrder byteOrder)
    {
        Exif::Ifd::iterator pos = ifd.findTag(tag);
        if (pos == ifd.end()) {
            Exif::Entry e(ifd.alloc());
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
