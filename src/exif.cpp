// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// Define DEBUG_MAKERNOTE to output debug information to std::cerr, e.g, by
// calling make like this: make DEFS=-DDEBUG_MAKERNOTE exif.o
//#define DEBUG_MAKERNOTE

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "exif.hpp"
#include "types.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "value.hpp"
#include "ifd.hpp"
#include "tags.hpp"
#include "image.hpp"
#include "makernote.hpp"
#include "futils.hpp"

// + standard includes
#include <iostream>
#include <sstream>
#include <utility>
#include <algorithm>
#include <map>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <sys/types.h>                  // for stat()
#include <sys/stat.h>                   // for stat()
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                    // for stat()
#endif

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

    // Read file path into a DataBuf, which is returned.
    Exiv2::DataBuf readFile(const std::string& path);

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Exifdatum::Exifdatum(const Entry& e, ByteOrder byteOrder)
        : key_(ExifKey::AutoPtr(new ExifKey(e)))
    {
        setValue(e, byteOrder);
    }

    Exifdatum::Exifdatum(const ExifKey& key, const Value* pValue)
        : key_(key.clone())
    {
        if (pValue) value_ = pValue->clone();
    }

    Exifdatum::~Exifdatum()
    {
    }

    Exifdatum::Exifdatum(const Exifdatum& rhs)
        : Metadatum(rhs)
    {
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy
    }

    const Value& Exifdatum::value() const
    {
        if (value_.get() == 0) throw Error(8);
        return *value_;
    }

    Exifdatum& Exifdatum::operator=(const Exifdatum& rhs)
    {
        if (this == &rhs) return *this;
        Metadatum::operator=(rhs);

        key_.reset();
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy

        value_.reset();
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy

        return *this;
    } // Exifdatum::operator=

    Exifdatum& Exifdatum::operator=(const std::string& value)
    {
        setValue(value);
        return *this;
    }

    Exifdatum& Exifdatum::operator=(const uint16_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const uint32_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const URational& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const int16_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const int32_t& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const Rational& value)
    {
        return Exiv2::setValue(*this, value);
    }

    Exifdatum& Exifdatum::operator=(const Value& value)
    {
        setValue(&value);
        return *this;
    }

    void Exifdatum::setValue(const Value* pValue)
    {
        value_.reset();
        if (pValue) value_ = pValue->clone();
    }

    void Exifdatum::setValue(const Entry& e, ByteOrder byteOrder)
    {
        value_ = Value::create(TypeId(e.type()));
        value_->read(e.data(), e.count() * e.typeSize(), byteOrder);
        value_->setDataArea(e.dataArea(), e.sizeDataArea());
    }

    void Exifdatum::setValue(const std::string& value)
    {
        if (value_.get() == 0) {
            TypeId type = ExifTags::tagType(tag(), ifdId());
            value_ = Value::create(type);
        }
        value_->read(value);
    }

    TiffThumbnail& TiffThumbnail::operator=(const TiffThumbnail& /*rhs*/)
    {
        return *this;
    }

    int TiffThumbnail::setDataArea(ExifData& exifData, Ifd* pIfd1,
                                   const byte* buf, long len) const
    {
        // Create a DataBuf that can hold all strips
        ExifData::const_iterator sizes;
        ExifKey key("Exif.Thumbnail.StripByteCounts");
        sizes = exifData.findKey(key);
        if (sizes == exifData.end()) return 2;

        long totalSize = 0;
        for (long i = 0; i < sizes->count(); ++i) {
            totalSize += sizes->toLong(i);
        }
        DataBuf stripsBuf(totalSize);

        // Copy all strips into the data buffer. For each strip remember its
        // offset from the start of the data buffer
        ExifData::iterator stripOffsets;
        key = ExifKey("Exif.Thumbnail.StripOffsets");
        stripOffsets = exifData.findKey(key);
        if (stripOffsets == exifData.end()) return 2;
        if (stripOffsets->count() != sizes->count()) return 2;

        std::ostringstream os; // for the strip offsets
        long currentOffset = 0;
        long firstOffset = stripOffsets->toLong(0);
        long lastOffset = 0;
        long lastSize = 0;
        for (long i = 0; i < stripOffsets->count(); ++i) {
            long offset = stripOffsets->toLong(i);
            lastOffset = offset;
            long size = sizes->toLong(i);
            lastSize = size;
            if (len < offset + size) return 1;

            memcpy(stripsBuf.pData_ + currentOffset, buf + offset, size);
            os << currentOffset << " ";
            currentOffset += size;
        }

        // Set StripOffsets data area and relative offsets
        stripOffsets->setDataArea(stripsBuf.pData_, stripsBuf.size_);
        stripOffsets->setValue(os.str());

        // Set corresponding data area at IFD1, if it is a contiguous area
        if (pIfd1 && firstOffset + totalSize == lastOffset + lastSize) {
            Ifd::iterator pos = pIfd1->findTag(0x0111);
            assert(pos != pIfd1->end());
            pos->setDataArea(buf + firstOffset, totalSize);
        }

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

    DataBuf TiffThumbnail::copy(const ExifData& exifData) const
    {
        // Create a TIFF header and IFD1
        TiffHeader tiffHeader(exifData.byteOrder());
        Ifd ifd1(ifd1Id);

        // Populate IFD (without Exif and GPS tags) from metadata
        addToIfd(ifd1, exifData.begin(), exifData.end(), exifData.byteOrder());
        ifd1.erase(0x8769);
        ifd1.erase(0x8825);
        ifd1.sortByTag();

        long size = tiffHeader.size() + ifd1.size() + ifd1.dataSize();
        DataBuf buf(size);
        long len = tiffHeader.copy(buf.pData_);
        len += ifd1.copy(buf.pData_ + len, exifData.byteOrder(), len);
        assert(len == size);
        return buf;
    }

    JpegThumbnail& JpegThumbnail::operator=(const JpegThumbnail& /*rhs*/)
    {
        return *this;
    }

    int JpegThumbnail::setDataArea(ExifData& exifData, Ifd* pIfd1,
                                   const byte* buf, long len) const
    {
        ExifKey key("Exif.Thumbnail.JPEGInterchangeFormat");
        ExifData::iterator format = exifData.findKey(key);
        if (format == exifData.end()) return 1;
        long offset = format->toLong();
        key = ExifKey("Exif.Thumbnail.JPEGInterchangeFormatLength");
        ExifData::const_iterator length = exifData.findKey(key);
        if (length == exifData.end()) return 1;
        long size = length->toLong();
        if (len < offset + size) return 2;
        format->setDataArea(buf + offset, size);
        format->setValue("0");
        if (pIfd1) {
            Ifd::iterator pos = pIfd1->findTag(0x0201);
            assert(pos != pIfd1->end());
            pos->setDataArea(buf + offset, size);
        }
        return 0;
    } // JpegThumbnail::setDataArea

    const char* JpegThumbnail::format() const
    {
        return "JPEG";
    }

    const char* JpegThumbnail::extension() const
    {
        return ".jpg";
    }

    DataBuf JpegThumbnail::copy(const ExifData& exifData) const
    {
        ExifKey key("Exif.Thumbnail.JPEGInterchangeFormat");
        ExifData::const_iterator format = exifData.findKey(key);
        if (format == exifData.end()) return DataBuf();
        return format->dataArea();
    }

    ExifData::ExifData()
        : pTiffHeader_(0),
          pIfd0_(0), pExifIfd_(0), pIopIfd_(0), pGpsIfd_(0), pIfd1_(0),
          pMakerNote_(0), size_(0), pData_(0), compatible_(true)
    {
    }

    ExifData::ExifData(const ExifData& rhs)
        : exifMetadata_(rhs.exifMetadata_), pTiffHeader_(0),
          pIfd0_(0), pExifIfd_(0), pIopIfd_(0), pGpsIfd_(0), pIfd1_(0),
          pMakerNote_(0), size_(0), pData_(0), compatible_(rhs.compatible_)
    {
        pData_ = new byte[rhs.size_];
        size_ = rhs.size_;
        memcpy(pData_, rhs.pData_, rhs.size_);

        if (rhs.pTiffHeader_) {
            pTiffHeader_ = new TiffHeader(*rhs.pTiffHeader_);
        }
        if (rhs.pIfd0_) {
            pIfd0_ = new Ifd(*rhs.pIfd0_);
            pIfd0_->updateBase(pData_);
        }
        if (rhs.pExifIfd_) {
            pExifIfd_ = new Ifd(*rhs.pExifIfd_);
            pExifIfd_->updateBase(pData_);
        }
        if (rhs.pIopIfd_) {
            pIopIfd_ = new Ifd(*rhs.pIopIfd_);
            pIopIfd_->updateBase(pData_);
        }
        if (rhs.pGpsIfd_) {
            pGpsIfd_ = new Ifd(*rhs.pGpsIfd_);
            pGpsIfd_->updateBase(pData_);
        }
        if (rhs.pIfd1_) {
            pIfd1_ = new Ifd(*rhs.pIfd1_);
            pIfd1_->updateBase(pData_);
        }
        if (rhs.pMakerNote_) {
            pMakerNote_ = rhs.pMakerNote_->clone().release();
            pMakerNote_->updateBase(pData_);
        }
    }

    ExifData::~ExifData()
    {
        delete pTiffHeader_;
        delete pIfd0_;
        delete pExifIfd_;
        delete pIopIfd_;
        delete pGpsIfd_;
        delete pIfd1_;
        delete pMakerNote_;
        delete[] pData_;
    }

    ExifData& ExifData::operator=(const ExifData& rhs)
    {
        if (this == &rhs) return *this;

        exifMetadata_ = rhs.exifMetadata_;

        size_ = 0;
        delete[] pData_;
        pData_ = new byte[rhs.size_];
        size_ = rhs.size_;
        memcpy(pData_, rhs.pData_, rhs.size_);

        delete pTiffHeader_;
        pTiffHeader_ = 0;
        if (rhs.pTiffHeader_) {
            pTiffHeader_ = new TiffHeader(*rhs.pTiffHeader_);
        }
        delete pIfd0_;
        pIfd0_ = 0;
        if (rhs.pIfd0_) {
            pIfd0_ = new Ifd(*rhs.pIfd0_);
            pIfd0_->updateBase(pData_);
        }
        delete pExifIfd_;
        pExifIfd_ = 0;
        if (rhs.pExifIfd_) {
            pExifIfd_ = new Ifd(*rhs.pExifIfd_);
            pExifIfd_->updateBase(pData_);
        }
        delete pIopIfd_;
        pIopIfd_ = 0;
        if (rhs.pIopIfd_) {
            pIopIfd_ = new Ifd(*rhs.pIopIfd_);
            pIopIfd_->updateBase(pData_);
        }
        delete pGpsIfd_;
        pGpsIfd_ = 0;
        if (rhs.pGpsIfd_) {
            pGpsIfd_ = new Ifd(*rhs.pGpsIfd_);
            pGpsIfd_->updateBase(pData_);
        }
        delete pIfd1_;
        pIfd1_ = 0;
        if (rhs.pIfd1_) {
            pIfd1_ = new Ifd(*rhs.pIfd1_);
            pIfd1_->updateBase(pData_);
        }
        delete pMakerNote_;
        pMakerNote_ = 0;
        if (rhs.pMakerNote_) {
            pMakerNote_ = rhs.pMakerNote_->clone().release();
            pMakerNote_->updateBase(pData_);
        }

        compatible_ = rhs.compatible_;
        return *this;
    }

    Exifdatum& ExifData::operator[](const std::string& key)
    {
        ExifKey exifKey(key);
        iterator pos = findKey(exifKey);
        if (pos == end()) {
            add(Exifdatum(exifKey));
            pos = findKey(exifKey);
        }
        return *pos;
    }

    int ExifData::load(const byte* buf, long len)
    {
        // Copy the data buffer
        delete[] pData_;
        pData_ = new byte[len];
        memcpy(pData_, buf, len);
        size_ = len;

        // Read the TIFF header
        delete pTiffHeader_;
        pTiffHeader_ = new TiffHeader;
        assert(pTiffHeader_ != 0);
        int rc = pTiffHeader_->read(pData_);
        if (rc) return rc;

        // Read IFD0
        delete pIfd0_;
        pIfd0_ = new Ifd(ifd0Id, 0, false);
        assert(pIfd0_ != 0);
        rc = pIfd0_->read(pData_, size_, pTiffHeader_->offset(), byteOrder());
        if (rc) return rc;

        delete pExifIfd_;
        pExifIfd_ = new Ifd(exifIfdId, 0, false);
        assert(pExifIfd_ != 0);
        // Find and read ExifIFD sub-IFD of IFD0
        rc = pIfd0_->readSubIfd(*pExifIfd_, pData_, size_, byteOrder(), 0x8769);
        if (rc) return rc;
        // Find MakerNote in ExifIFD, create a MakerNote class
        Ifd::iterator pos = pExifIfd_->findTag(0x927c);
        Ifd::iterator make = pIfd0_->findTag(0x010f);
        Ifd::iterator model = pIfd0_->findTag(0x0110);
        if (   pos != pExifIfd_->end()
            && make != pIfd0_->end() && model != pIfd0_->end()) {
            // Todo: The conversion to string assumes that there is a \0 at the end
            // Todo: How to avoid the cast (is that a MSVC thing?)
            pMakerNote_ = MakerNoteFactory::create(
                              reinterpret_cast<const char*>(make->data()),
                              reinterpret_cast<const char*>(model->data()),
                              false,
                              pos->data(),
                              pos->size(),
                              byteOrder(),
                              pExifIfd_->offset() + pos->offset()).release();
        }
        // Read the MakerNote
        if (pMakerNote_) {
            rc = pMakerNote_->read(pData_, size_,
                                   pExifIfd_->offset() + pos->offset(),
                                   byteOrder());
            if (rc) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: Failed to read Makernote, rc = "
                          << rc << "\n";
#endif
                delete pMakerNote_;
                pMakerNote_ = 0;
            }
        }
        // If we successfully parsed the MakerNote, delete the raw MakerNote,
        // the parsed MakerNote is the primary MakerNote from now on
        if (pMakerNote_) {
            pExifIfd_->erase(pos);
        }

        delete pIopIfd_;
        pIopIfd_ = new Ifd(iopIfdId, 0, false);
        assert(pIopIfd_ != 0);
        // Find and read Interoperability IFD in ExifIFD
        rc = pExifIfd_->readSubIfd(*pIopIfd_, pData_, size_, byteOrder(), 0xa005);
        if (rc) return rc;

        delete pGpsIfd_;
        pGpsIfd_ = new Ifd(gpsIfdId, 0, false);
        assert(pGpsIfd_ != 0);
        // Find and read GPSInfo sub-IFD in IFD0
        rc = pIfd0_->readSubIfd(*pGpsIfd_, pData_, size_, byteOrder(), 0x8825);
        if (rc) return rc;

        delete pIfd1_;
        pIfd1_ = new Ifd(ifd1Id, 0, false);
        assert(pIfd1_ != 0);
        // Read IFD1
        if (pIfd0_->next()) {
            rc = pIfd1_->read(pData_, size_, pIfd0_->next(), byteOrder());
            if (rc) return rc;
        }
        // Find and delete ExifIFD sub-IFD of IFD1
        pos = pIfd1_->findTag(0x8769);
        if (pos != pIfd1_->end()) {
            pIfd1_->erase(pos);
            rc = 7;
        }
        // Find and delete GPSInfo sub-IFD in IFD1
        pos = pIfd1_->findTag(0x8825);
        if (pos != pIfd1_->end()) {
            pIfd1_->erase(pos);
            rc = 7;
        }
        // Copy all entries from the IFDs and the MakerNote to the metadata
        exifMetadata_.clear();
        add(pIfd0_->begin(), pIfd0_->end(), byteOrder());
        add(pExifIfd_->begin(), pExifIfd_->end(), byteOrder());
        if (pMakerNote_) {
            add(pMakerNote_->begin(), pMakerNote_->end(),
                (pMakerNote_->byteOrder() == invalidByteOrder ?
                    byteOrder() : pMakerNote_->byteOrder()));
        }
        add(pIopIfd_->begin(), pIopIfd_->end(), byteOrder());
        add(pGpsIfd_->begin(), pGpsIfd_->end(), byteOrder());
        add(pIfd1_->begin(), pIfd1_->end(), byteOrder());
        // Read the thumbnail (but don't worry whether it was successful or not)
        readThumbnail();

        return rc;
    } // ExifData::load


    DataBuf ExifData::copy()
    {
        DataBuf buf;
        // If we can update the internal IFDs and the underlying data buffer
        // from the metadata without changing the data size, then it is enough
        // to copy the data buffer.
        if (compatible_ && updateEntries()) {
#ifdef DEBUG_MAKERNOTE
            std::cerr << "->>>>>> using non-intrusive writing <<<<<<-\n";
#endif
            buf.alloc(size_);
            memcpy(buf.pData_, pData_, size_);
        }
        // Else we have to do it the hard way...
        else {
#ifdef DEBUG_MAKERNOTE
            std::cerr << "->>>>>> writing from metadata <<<<<<-\n";
#endif
            buf = copyFromMetadata();
        }
        return buf;
    }

    DataBuf ExifData::copyFromMetadata()
    {
        // Build IFD0
        Ifd ifd0(ifd0Id);
        addToIfd(ifd0, begin(), end(), byteOrder());

        // Build Exif IFD from metadata
        Ifd exifIfd(exifIfdId);
        addToIfd(exifIfd, begin(), end(), byteOrder());
        MakerNote::AutoPtr makerNote;
        if (pMakerNote_) {
            // Build MakerNote from metadata
            makerNote = pMakerNote_->create();
            addToMakerNote(makerNote.get(),
                           begin(), end(),
                           (pMakerNote_->byteOrder() == invalidByteOrder ?
                               byteOrder() : pMakerNote_->byteOrder()));
            // Create a placeholder MakerNote entry of the correct size and
            // add it to the Exif IFD (because we don't know the offset yet)
            Entry e;
            e.setIfdId(exifIfd.ifdId());
            e.setTag(0x927c);
            DataBuf tmpBuf(makerNote->size());
            memset(tmpBuf.pData_, 0x0, tmpBuf.size_);
            e.setValue(undefined, tmpBuf.size_, tmpBuf.pData_, tmpBuf.size_);
            exifIfd.erase(0x927c);
            exifIfd.add(e);
        }

        // Build Interoperability IFD from metadata
        Ifd iopIfd(iopIfdId);
        addToIfd(iopIfd, begin(), end(), byteOrder());

        // Build GPSInfo IFD from metadata
        Ifd gpsIfd(gpsIfdId);
        addToIfd(gpsIfd, begin(), end(), byteOrder());

        // build IFD1 from metadata
        Ifd ifd1(ifd1Id);
        addToIfd(ifd1, begin(), end(), byteOrder());
        // Set a temporary dummy offset in IFD0
        if (ifd1.size() > 0) {
            ifd0.setNext(1, byteOrder());
        }

        // Compute the new IFD offsets
        int exifIdx = ifd0.erase(0x8769);
        int gpsIdx  = ifd0.erase(0x8825);
        int iopIdx  = exifIfd.erase(0xa005);

        TiffHeader tiffHeader(byteOrder());
        long ifd0Offset = tiffHeader.size();
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

        // Set the offset to IFD1 in IFD0
        if (ifd1.size() > 0) {
            ifd0.setNext(ifd1Offset, byteOrder());
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

        // Allocate a data buffer big enough for all metadata
        long size = tiffHeader.size();
        size += ifd0.size() + ifd0.dataSize();
        size += exifIfd.size() + exifIfd.dataSize();
        size += iopIfd.size() + iopIfd.dataSize();
        size += gpsIfd.size() + gpsIfd.dataSize();
        size += ifd1.size() + ifd1.dataSize();
        DataBuf buf(size);

        // Copy the TIFF header, all IFDs, MakerNote and thumbnail to the buffer
        size = tiffHeader.copy(buf.pData_);
        ifd0.sortByTag();
        size += ifd0.copy(buf.pData_ + ifd0Offset, byteOrder(), ifd0Offset);
        exifIfd.sortByTag();
        size += exifIfd.copy(buf.pData_ + exifIfdOffset, byteOrder(), exifIfdOffset);
        if (makerNote.get() != 0) {
            // Copy the MakerNote over the placeholder data
            Entries::iterator mn = exifIfd.findTag(0x927c);
            // Do _not_ sort the makernote; vendors (at least Canon), don't seem
            // to bother about this TIFF standard requirement, so writing the
            // makernote as is might result in fewer deviations from the original
            makerNote->copy(buf.pData_ + exifIfdOffset + mn->offset(),
                            byteOrder(),
                            exifIfdOffset + mn->offset());
        }
        iopIfd.sortByTag();
        size += iopIfd.copy(buf.pData_ + iopIfdOffset, byteOrder(), iopIfdOffset);
        gpsIfd.sortByTag();
        size += gpsIfd.copy(buf.pData_ + gpsIfdOffset, byteOrder(), gpsIfdOffset);
        ifd1.sortByTag();
        size += ifd1.copy(buf.pData_ + ifd1Offset, byteOrder(), ifd1Offset);
        assert(size == buf.size_);
        return buf;
    } // ExifData::copyFromMetadata

    void ExifData::add(Entries::const_iterator begin,
                       Entries::const_iterator end,
                       ByteOrder byteOrder)
    {
        Entries::const_iterator i = begin;
        for (; i != end; ++i) {
            add(Exifdatum(*i, byteOrder));
        }
    }

    void ExifData::add(const ExifKey& key, const Value* pValue)
    {
        add(Exifdatum(key, pValue));
    }

    void ExifData::add(const Exifdatum& exifdatum)
    {
        if (ExifTags::isMakerIfd(exifdatum.ifdId())) {
            if (pMakerNote_ == 0) {
                pMakerNote_ = MakerNoteFactory::create(exifdatum.ifdId()).release();
            }
            if (pMakerNote_ == 0) throw Error(23, exifdatum.ifdId());
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

    void ExifData::setJpegThumbnail(const byte* buf, long size)
    {
        (*this)["Exif.Thumbnail.Compression"] = uint16_t(6);
        Exifdatum& format = (*this)["Exif.Thumbnail.JPEGInterchangeFormat"];
        format = uint32_t(0);
        format.setDataArea(buf, size);
        (*this)["Exif.Thumbnail.JPEGInterchangeFormatLength"] = uint32_t(size);
    }

    void ExifData::setJpegThumbnail(const byte* buf, long size,
                                    URational xres, URational yres, uint16_t unit)
    {
        setJpegThumbnail(buf, size);
        (*this)["Exif.Thumbnail.XResolution"] = xres;
        (*this)["Exif.Thumbnail.YResolution"] = yres;
        (*this)["Exif.Thumbnail.ResolutionUnit"] = unit;
    }

    void ExifData::setJpegThumbnail(const std::string& path)
    {
        DataBuf thumb = readFile(path); // may throw
        setJpegThumbnail(thumb.pData_, thumb.size_);
    }

    void ExifData::setJpegThumbnail(const std::string& path,
                                   URational xres, URational yres, uint16_t unit)
    {
        DataBuf thumb = readFile(path); // may throw
        setJpegThumbnail(thumb.pData_, thumb.size_, xres, yres, unit);
    }

    long ExifData::eraseThumbnail()
    {
        // First, determine if the thumbnail is at the end of the Exif data
        bool stp = stdThumbPosition();
        // Delete all Exif.Thumbnail.* (IFD1) metadata
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
        if (stp) {
            delta = size_;
            if (size_ > 0 && pIfd0_ && pIfd0_->next() > 0) {
                // Truncate IFD1 and thumbnail data from the data buffer
                size_ = pIfd0_->next();
                pIfd0_->setNext(0, byteOrder());
                if (pIfd1_) pIfd1_->clear();
            }
            delta -= size_;
        }
        else {
            // We will have to write the hard way and re-arrange the data
            compatible_ = false;
            if (pIfd1_) delta = pIfd1_->size() + pIfd1_->dataSize();
        }
        return delta;
    } // ExifData::eraseThumbnail

    bool ExifData::stdThumbPosition() const
    {
        if (   pIfd0_ == 0 || pExifIfd_ == 0 || pIopIfd_ == 0
            || pGpsIfd_ == 0 || pIfd1_ == 0) return true;

        // Todo: There is still an invalid assumption here: The data of an IFD
        //       can be stored in multiple non-contiguous blocks. In this case,
        //       dataOffset + dataSize does not point to the end of the IFD data.
        //       in particular, this is potentially the case for the remaining Exif
        //       data in the presence of a known Makernote.
        bool rc = true;
        Thumbnail::AutoPtr thumbnail = getThumbnail();
        if (thumbnail.get()) {
            long maxOffset;
            maxOffset = std::max(pIfd0_->offset(), pIfd0_->dataOffset());
            maxOffset = std::max(maxOffset, pExifIfd_->offset());
            maxOffset = std::max(maxOffset,   pExifIfd_->dataOffset()
                                            + pExifIfd_->dataSize());
            if (pMakerNote_) {
                maxOffset = std::max(maxOffset,   pMakerNote_->offset()
                                                + pMakerNote_->size());
            }
            maxOffset = std::max(maxOffset, pIopIfd_->offset());
            maxOffset = std::max(maxOffset,   pIopIfd_->dataOffset()
                                            + pIopIfd_->dataSize());
            maxOffset = std::max(maxOffset, pGpsIfd_->offset());
            maxOffset = std::max(maxOffset,   pGpsIfd_->dataOffset()
                                            + pGpsIfd_->dataSize());

            if (   maxOffset > pIfd1_->offset()
                || maxOffset > pIfd1_->dataOffset() && pIfd1_->dataOffset() > 0)
                rc = false;
            /*
               Todo: Removed condition from the above if(). Should be re-added...
                || maxOffset > pThumbnail_->offset()
            */
        }
        return rc;
    } // ExifData::stdThumbPosition

    ByteOrder ExifData::byteOrder() const
    {
        if (pTiffHeader_) return pTiffHeader_->byteOrder();
        return littleEndian;
    }

    int ExifData::writeThumbnail(const std::string& path) const
    {
        Thumbnail::AutoPtr thumbnail = getThumbnail();
        if (thumbnail.get() == 0) return 8;

        std::string name = path + thumbnail->extension();
        FileIo file(name);
        if (file.open("wb") != 0) {
            throw Error(10, name, "wb", strError());
        }

        DataBuf buf(thumbnail->copy(*this));
        if (file.write(buf.pData_, buf.size_) != buf.size_) {
            throw Error(2, name, strError(), "FileIo::write");
        }

        return 0;
    } // ExifData::writeThumbnail

    DataBuf ExifData::copyThumbnail() const
    {
        Thumbnail::AutoPtr thumbnail = getThumbnail();
        if (thumbnail.get() == 0) return DataBuf();
        return thumbnail->copy(*this);
    }

    const char* ExifData::thumbnailFormat() const
    {
        Thumbnail::AutoPtr thumbnail = getThumbnail();
        if (thumbnail.get() == 0) return "";
        return thumbnail->format();
    }

    const char* ExifData::thumbnailExtension() const
    {
        Thumbnail::AutoPtr thumbnail = getThumbnail();
        if (thumbnail.get() == 0) return "";
        return thumbnail->extension();
    }

    Thumbnail::AutoPtr ExifData::getThumbnail() const
    {
        Thumbnail::AutoPtr thumbnail;
        const_iterator pos = findKey(ExifKey("Exif.Thumbnail.Compression"));
        if (pos != end()) {
            long compression = pos->toLong();
            if (compression == 6) {
                thumbnail = Thumbnail::AutoPtr(new JpegThumbnail);
            }
            else {
                thumbnail = Thumbnail::AutoPtr(new TiffThumbnail);
            }
        }
        return thumbnail;

    } // ExifData::getThumbnail

    int ExifData::readThumbnail()
    {
        int rc = -1;
        Thumbnail::AutoPtr thumbnail = getThumbnail();
        if (thumbnail.get() != 0) {
            rc = thumbnail->setDataArea(*this, pIfd1_, pData_, size_);
        }
        return rc;

    } // ExifData::readThumbnail

    bool ExifData::updateEntries()
    {
        if (   pIfd0_ == 0 || pExifIfd_ == 0 || pIopIfd_ == 0
            || pGpsIfd_ == 0 || pIfd1_ == 0) return false;
        if (!this->compatible()) return false;

        bool compatible = true;
        compatible &= updateRange(pIfd0_->begin(), pIfd0_->end(), byteOrder());
        compatible &= updateRange(pExifIfd_->begin(), pExifIfd_->end(), byteOrder());
        if (pMakerNote_) {
            compatible &= updateRange(pMakerNote_->begin(),
                                      pMakerNote_->end(),
                                      (pMakerNote_->byteOrder() == invalidByteOrder ?
                                          byteOrder() : pMakerNote_->byteOrder()));
        }
        compatible &= updateRange(pIopIfd_->begin(), pIopIfd_->end(), byteOrder());
        compatible &= updateRange(pGpsIfd_->begin(), pGpsIfd_->end(), byteOrder());
        compatible &= updateRange(pIfd1_->begin(), pIfd1_->end(), byteOrder());

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
            else if (   entry->size() < md->size()
                     || entry->sizeDataArea() < md->sizeDataArea()) {
                compatible = false;
                continue;
            }
            else {
                // Hack: Set the entry's value only if there is no data area.
                // This ensures that the original offsets are not overwritten
                // with relative offsets from the Exifdatum (which require
                // conversion to offsets relative to the start of the TIFF
                // header and that is currently only done in intrusive write
                // mode). On the other hand, it is thus now not possible to
                // change the offsets of an entry with a data area in
                // non-intrusive mode. This can be considered a bug.
                // Todo: Fix me!
                if (md->sizeDataArea() == 0) {
                    DataBuf buf(md->size());
                    md->copy(buf.pData_, byteOrder);
                    entry->setValue(static_cast<uint16_t>(md->typeId()),
                                    md->count(),
                                    buf.pData_, md->size());
                }
                // Always set the data area
                DataBuf dataArea(md->dataArea());
                entry->setDataArea(dataArea.pData_, dataArea.size_);
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
            std::pair<bool, Entries::const_iterator> rc;
            rc = findEntry(md->ifdId(), md->idx());
            // Make sure that we have an entry
            if (!rc.first) {
                compatible = false;
                break;
            }
            // Make sure that the size of the Exifdatum fits the available size
            // of the entry
            if (   md->size() > rc.second->size()
                || md->sizeDataArea() > rc.second->sizeDataArea()) {
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

        if (ExifTags::isMakerIfd(ifdId) && pMakerNote_) {
            entry = pMakerNote_->findIdx(idx);
            if (entry != pMakerNote_->end()) {
                rc.first = true;
                rc.second = entry;
            }
            return rc;
        }
        const Ifd* ifd = getIfd(ifdId);
        if (ifd && isExifIfd(ifdId)) {
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
            ifd = pIfd0_;
            break;
        case exifIfdId:
            ifd = pExifIfd_;
            break;
        case iopIfdId:
            ifd = pIopIfd_;
            break;
        case gpsIfdId:
            ifd = pGpsIfd_;
            break;
        case ifd1Id:
            ifd = pIfd1_;
            break;
        default:
            ifd = 0;
            break;
        }
        return ifd;
    } // ExifData::getIfd

    // *************************************************************************
    // free functions

    bool hasMakerNote(const ExifData& exifData)
    {
        ExifData::const_iterator e = exifData.end();
        for (ExifData::const_iterator md = exifData.begin(); md != e; ++md) {
            if (ExifTags::isMakerIfd(md->ifdId())) return true;
        }
        return false;
    } // hasMakerNote

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
                   buf.pData_, buf.size_);

        DataBuf dataArea(md.dataArea());
        e.setDataArea(dataArea.pData_, dataArea.size_);

        ifd.add(e);
    } // addToIfd

    void addToMakerNote(MakerNote* makerNote,
                        ExifMetadata::const_iterator begin,
                        ExifMetadata::const_iterator end,
                        ByteOrder byteOrder)
    {
        for (ExifMetadata::const_iterator i = begin; i != end; ++i) {
            if (ExifTags::isMakerIfd(i->ifdId())) {
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

        DataBuf dataArea(md.dataArea());
        e.setDataArea(dataArea.pData_, dataArea.size_);

        makerNote->add(e);
    } // addToMakerNote

    std::ostream& operator<<(std::ostream& os, const Exifdatum& md)
    {
        return ExifTags::printTag(os, md.tag(), md.ifdId(), md.value());
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

    Exiv2::DataBuf readFile(const std::string& path)
    {
        Exiv2::FileIo file(path);
        if (file.open("rb") != 0) {
            throw Exiv2::Error(10, path, "rb", Exiv2::strError());
        }
        struct stat st;
        if (0 != stat(path.c_str(), &st)) {
            throw Exiv2::Error(2, path, Exiv2::strError(), "::stat");
        }
        Exiv2::DataBuf buf(st.st_size);
        long len = file.read(buf.pData_, buf.size_);
        if (len != buf.size_) {
            throw Exiv2::Error(2, path, Exiv2::strError(), "FileIo::read");
        }
        return buf;
    }

}
