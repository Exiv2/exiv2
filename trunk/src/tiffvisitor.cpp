// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
  File:      tiffvisitor.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
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

#include "tiffcomposite_int.hpp" // Do not change the order of these 2 includes,
#include "tiffvisitor_int.hpp"   // see bug #487
#include "tiffimage_int.hpp"
#include "makernote2_int.hpp"
#include "exif.hpp"
#include "iptc.hpp"
#include "value.hpp"
#include "image.hpp"
#include "jpgimage.hpp"
#include "i18n.h"             // NLS support.

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    namespace Internal {

    TiffVisitor::TiffVisitor()
    {
        for (int i = 0; i < events_; ++i) {
            go_[i] = true;
        }
    }

    void TiffVisitor::setGo(GoEvent event, bool go)
    {
        assert(event >= 0 && event < events_);
        go_[event] = go;
    }

    bool TiffVisitor::go(GoEvent event) const
    {
        assert(event >= 0 && event < events_);
        return go_[event];
    }

    void TiffFinder::init(uint16_t tag, uint16_t group)
    {
        tag_ = tag;
        group_ = group;
        tiffComponent_ = 0;
    }

    void TiffFinder::findObject(TiffComponent* object)
    {
        if (object->tag() == tag_ && object->group() == group_) {
            tiffComponent_ = object;
            setGo(geTraverse, false);
        }
    }

    void TiffFinder::visitEntry(TiffEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitDataEntry(TiffDataEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitImageEntry(TiffImageEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitSizeEntry(TiffSizeEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitDirectory(TiffDirectory* object)
    {
        findObject(object);
    }

    void TiffFinder::visitSubIfd(TiffSubIfd* object)
    {
        findObject(object);
    }

    void TiffFinder::visitMnEntry(TiffMnEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitIfdMakernote(TiffIfdMakernote* object)
    {
        findObject(object);
    }

    void TiffFinder::visitArrayEntry(TiffArrayEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitArrayElement(TiffArrayElement* object)
    {
        findObject(object);
    }

    TiffDecoder::TiffDecoder(
        ExifData&            exifData,
        IptcData&            iptcData,
        XmpData&             xmpData,
        TiffComponent* const pRoot,
        FindDecoderFct       findDecoderFct
    )
        : exifData_(exifData),
          iptcData_(iptcData),
          xmpData_(xmpData),
          pRoot_(pRoot),
          findDecoderFct_(findDecoderFct),
          decodedIptc_(false)
    {
        assert(pRoot != 0);

        exifData_.clear();
        iptcData_.clear();
        xmpData_.clear();

        // Find camera make
        TiffFinder finder(0x010f, Group::ifd0);
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (te && te->pValue()) {
            make_ = te->pValue()->toString();
        }
    }

    void TiffDecoder::visitEntry(TiffEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitDataEntry(TiffDataEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitImageEntry(TiffImageEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitSizeEntry(TiffSizeEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitDirectory(TiffDirectory* /*object*/)
    {
        // Nothing to do
    }

    void TiffDecoder::visitSubIfd(TiffSubIfd* object)
    {
        decodeTiffEntry(object);
    }

    void TiffDecoder::visitMnEntry(TiffMnEntry* object)
    {
        if (!object->mn_) decodeTiffEntry(object);
    }

    void TiffDecoder::visitIfdMakernote(TiffIfdMakernote* /*object*/)
    {
        // Nothing to do
    }

    void TiffDecoder::decodeOlympThumb(const TiffEntryBase* object)
    {
        const DataValue* v = dynamic_cast<const DataValue*>(object->pValue());
        if (v != 0) {
            exifData_["Exif.Thumbnail.Compression"] = uint16_t(6);
            DataBuf buf(v->size());
            v->copy(buf.pData_);
            Exifdatum& ed = exifData_["Exif.Thumbnail.JPEGInterchangeFormat"];
            ed = uint32_t(0);
            ed.setDataArea(buf.pData_, buf.size_);
            exifData_["Exif.Thumbnail.JPEGInterchangeFormatLength"] = uint32_t(buf.size_);
        }
    }

    void TiffDecoder::getObjData(byte const*& pData,
                                 long& size,
                                 uint16_t tag,
                                 uint16_t group,
                                 const TiffEntryBase* object)
    {
        if (object && object->tag() == tag && object->group() == group) {
            pData = object->pData();
            size = object->size();
            return;
        }
        TiffFinder finder(tag, group);
        pRoot_->accept(finder);
        TiffEntryBase const* te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (te) {
            pData = te->pData();
            size = te->size();
            return;
        }
    }

    void TiffDecoder::decodeXmp(const TiffEntryBase* object)
    {
        // add Exif tag anyway
        decodeStdTiffEntry(object);

        byte const* pData = 0;
        long size = 0;
        getObjData(pData, size, 0x02bc, Group::ifd0, object);
        if (pData) {
            std::string xmpPacket;
            xmpPacket.assign(reinterpret_cast<const char*>(pData), size);
            std::string::size_type idx = xmpPacket.find_first_of('<');
            if (idx != std::string::npos && idx > 0) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: Removing " << static_cast<unsigned long>(idx)
						  << " characters from the beginning of the XMP packet\n";
#endif
                xmpPacket = xmpPacket.substr(idx);
            }
            if (XmpParser::decode(xmpData_, xmpPacket)) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Warning: Failed to decode XMP metadata.\n";
#endif
            }
        }
    } // TiffDecoder::decodeXmp

    void TiffDecoder::decodeIptc(const TiffEntryBase* object)
    {
        // add Exif tag anyway
        decodeStdTiffEntry(object);

        // All tags are read at this point, so the first time we come here,
        // find the relevant IPTC tag and decode IPTC if found
        if (decodedIptc_) {
            return;
        }
        decodedIptc_ = true;
        // 1st choice: IPTCNAA
        byte const* pData = 0;
        long size = 0;
        getObjData(pData, size, 0x83bb, Group::ifd0, object);
        if (pData) {
            if (0 == IptcParser::decode(iptcData_, pData, size)) {
                return;
            }
#ifndef SUPPRESS_WARNINGS
            else {
                std::cerr << "Warning: Failed to decode IPTC block found in "
                          << "Directory Image, entry 0x83bb\n";
            }
#endif
        }

        // 2nd choice if no IPTCNAA record found or failed to decode it:
        // ImageResources
        pData = 0;
        size = 0;
        getObjData(pData, size, 0x8649, Group::ifd0, object);
        if (pData) {
            byte const* record = 0;
            uint32_t sizeHdr = 0;
            uint32_t sizeData = 0;
            if (0 != Photoshop::locateIptcIrb(pData, size,
                                              &record, &sizeHdr, &sizeData)) {
                return;
            }
            if (0 == IptcParser::decode(iptcData_, record + sizeHdr, sizeData)) {
                return;
            }
#ifndef SUPPRESS_WARNINGS
            else {
                std::cerr << "Warning: Failed to decode IPTC block found in "
                          << "Directory Image, entry 0x8649\n";
            }
#endif
        }
    } // TiffMetadataDecoder::decodeIptc

    void TiffDecoder::decodeTiffEntry(const TiffEntryBase* object)
    {
        assert(object != 0);

        // Don't decode the entry if value is not set
        if (!object->pValue()) return;

        // Remember NewSubfileType
        if (object->tag() == 0x00fe) {
            groupType_[object->group()] = object->pValue()->toLong();
        }

        const DecoderFct decoderFct = findDecoderFct_(make_,
                                                      object->tag(),
                                                      object->group());
        // skip decoding if decoderFct == 0
        if (decoderFct) {
            EXV_CALL_MEMBER_FN(*this, decoderFct)(object);
        }
    } // TiffDecoder::decodeTiffEntry

    void TiffDecoder::decodeStdTiffEntry(const TiffEntryBase* object)
    {
        assert(object !=0);
        // Todo: ExifKey should have an appropriate c'tor, it should not be
        //       necessary to use groupName here
        ExifKey key(object->tag(), tiffGroupName(object->group()));
        exifData_.add(key, object->pValue());

    } // TiffDecoder::decodeTiffEntry

    void TiffDecoder::visitArrayEntry(TiffArrayEntry* /*object*/)
    {
        // Nothing to do
    }

    void TiffDecoder::visitArrayElement(TiffArrayElement* object)
    {
        decodeTiffEntry(object);
    }

    TiffEncoder::TiffEncoder(
            const ExifData&      exifData,
            const IptcData&      iptcData,
            const XmpData&       xmpData,
                  TiffComponent* pRoot,
                  ByteOrder      byteOrder,
                  FindEncoderFct findEncoderFct
    )
        : exifData_(exifData),
          iptcData_(iptcData),
          xmpData_(xmpData),
          del_(true),
          pRoot_(pRoot),
          pSourceTree_(0),
          byteOrder_(byteOrder),
          origByteOrder_(byteOrder),
          findEncoderFct_(findEncoderFct),
          dirty_(false),
          writeMethod_(wmNonIntrusive)
    {
        assert(pRoot != 0);

        encodeIptc();
        encodeXmp();

        // Find camera make
        ExifKey key("Exif.Image.Make");
        ExifData::const_iterator pos = exifData_.findKey(key);
        if (pos != exifData_.end()) {
            make_ = pos->toString();
        }
        if (make_.empty() && pRoot_) {
            TiffFinder finder(0x010f, Group::ifd0);
            pRoot_->accept(finder);
            TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
            if (te && te->pValue()) {
                make_ = te->pValue()->toString();
            }
        }
    }

    void TiffEncoder::encodeIptc()
    {
        // Update IPTCNAA Exif tag, if it exists. Delete the tag if there
        // is no IPTC data anymore.
        // If there is new IPTC data and Exif.Image.ImageResources does
        // not exist, create a new IPTCNAA Exif tag.
        bool del = false;
        const ExifKey iptcNaaKey("Exif.Image.IPTCNAA");
        ExifData::iterator pos = exifData_.findKey(iptcNaaKey);
        if (pos != exifData_.end()) {
            exifData_.erase(pos);
            del = true;
        }
        DataBuf rawIptc = IptcParser::encode(iptcData_);
        const ExifKey irbKey("Exif.Image.ImageResources");
        pos = exifData_.findKey(irbKey);
        if (rawIptc.size_ != 0 && (del || pos == exifData_.end())) {
            Value::AutoPtr value = Value::create(unsignedLong);
            value->read(rawIptc.pData_, rawIptc.size_, byteOrder_);
            Exifdatum iptcDatum(iptcNaaKey, value.get());
            exifData_.add(iptcDatum);
            pos = exifData_.findKey(irbKey); // needed after add()
        }
        // Also update IPTC IRB in Exif.Image.ImageResources if it exists,
        // but don't create it if not.
        if (pos != exifData_.end()) {
            DataBuf irbBuf(pos->value().size());
            pos->value().copy(irbBuf.pData_, invalidByteOrder);
            irbBuf = Photoshop::setIptcIrb(irbBuf.pData_, irbBuf.size_, iptcData_);
            exifData_.erase(pos);
            if (irbBuf.size_ != 0) {
                Value::AutoPtr value = Value::create(undefined);
                value->read(irbBuf.pData_, irbBuf.size_, invalidByteOrder);
                Exifdatum iptcDatum(irbKey, value.get());
                exifData_.add(iptcDatum);
            }
        }
    } // TiffEncoder::encodeIptc

    void TiffEncoder::encodeXmp()
    {
        const ExifKey xmpKey("Exif.Image.XMLPacket");
        // Remove any existing XMP Exif tag
        ExifData::iterator pos = exifData_.findKey(xmpKey);
        if (pos != exifData_.end()) {
            exifData_.erase(pos);
        }
        std::string xmpPacket;
        if (XmpParser::encode(xmpPacket, xmpData_)) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Failed to encode XMP metadata.\n";
#endif
        }
        if (!xmpPacket.empty()) {
            // Set the XMP Exif tag to the new value
            Value::AutoPtr value = Value::create(unsignedByte);
            value->read(reinterpret_cast<const byte*>(&xmpPacket[0]), xmpPacket.size(), invalidByteOrder);
            Exifdatum xmpDatum(xmpKey, value.get());
            exifData_.add(xmpDatum);
        }
    } // TiffEncoder::encodeXmp

    void TiffEncoder::setDirty(bool flag)
    {
        dirty_ = flag;
        setGo(geTraverse, !flag);
    }

    bool TiffEncoder::dirty() const
    {
        if (dirty_ || exifData_.count() > 0) return true;
        return false;
    }

    void TiffEncoder::visitEntry(TiffEntry* object)
    {
        encodeTiffComponent(object);
    }

    void TiffEncoder::visitDataEntry(TiffDataEntry* object)
    {
        encodeTiffComponent(object);
    }

    void TiffEncoder::visitImageEntry(TiffImageEntry* object)
    {
        encodeTiffComponent(object);
    }

    void TiffEncoder::visitSizeEntry(TiffSizeEntry* object)
    {
        encodeTiffComponent(object);
    }

    void TiffEncoder::visitDirectory(TiffDirectory* /*object*/)
    {
        // Nothing to do
    }

    void TiffEncoder::visitDirectoryNext(TiffDirectory* object)
    {
        // Update type and count in IFD entries, in case they changed
        assert(object != 0);

        byte* p = object->start() + 2;
        for (TiffDirectory::Components::iterator i = object->components_.begin();
             i != object->components_.end(); ++i) {
            p += updateDirEntry(p, byteOrder(), *i);
        }
    }

    uint32_t TiffEncoder::updateDirEntry(byte* buf,
                                         ByteOrder byteOrder,
                                         TiffComponent* pTiffComponent) const
    {
        assert(buf);
        assert(pTiffComponent);
        TiffEntryBase* pTiffEntry = dynamic_cast<TiffEntryBase*>(pTiffComponent);
        assert(pTiffEntry);
        us2Data(buf + 2, pTiffEntry->tiffType(), byteOrder);
        ul2Data(buf + 4, pTiffEntry->count(),    byteOrder);
        // Move data to offset field, if it fits and is not yet there.
        if (pTiffEntry->size() <= 4 && buf + 8 != pTiffEntry->pData()) {
#ifdef DEBUG
            std::cerr << "Copying data for tag " << pTiffEntry->tag()
                      << " to offset area.\n";
#endif
            memset(buf + 8, 0x0, 4);
            memcpy(buf + 8, pTiffEntry->pData(), pTiffEntry->size());
            memset(const_cast<byte*>(pTiffEntry->pData()), 0x0, pTiffEntry->size());
        }
        return 12;
    }

    void TiffEncoder::visitSubIfd(TiffSubIfd* object)
    {
        encodeTiffComponent(object);
    }

    void TiffEncoder::visitMnEntry(TiffMnEntry* object)
    {
        // Test is required here as well as in the callback encoder function
        if (!object->mn_) encodeTiffComponent(object);
    }

    void TiffEncoder::visitIfdMakernote(TiffIfdMakernote* object)
    {
        assert(object != 0);

        // Modify encoder for Makernote peculiarities, byte order
        if (object->byteOrder() != invalidByteOrder) {
            byteOrder_ = object->byteOrder();
        }
    } // TiffEncoder::visitIfdMakernote

    void TiffEncoder::visitIfdMakernoteEnd(TiffIfdMakernote* /*object*/)
    {
        // Reset byte order back to that from the c'tor
        byteOrder_ = origByteOrder_;

    } // TiffEncoder::visitIfdMakernoteEnd

    void TiffEncoder::visitArrayEntry(TiffArrayEntry* /*object*/)
    {
        // Nothing to do
    }

    void TiffEncoder::visitArrayElement(TiffArrayElement* object)
    {
        encodeTiffComponent(object);
    }

    void TiffEncoder::encodeTiffComponent(
              TiffEntryBase* object,
        const Exifdatum*     datum
    )
    {
        assert(object != 0);

        ExifData::iterator pos = exifData_.end();
        const Exifdatum* ed = datum;
        if (ed == 0) {
            ExifKey key(object->tag(), tiffGroupName(object->group()));
            pos = exifData_.findKey(key);
            if (pos == exifData_.end()) { // metadatum not found (deleted)
#ifdef DEBUG
                std::cerr << "DELETING          " << key << "\n";
#endif
                setDirty();
            }
            else {
                ed = &(*pos);
            }
        }
        if (ed) {
            const EncoderFct fct = findEncoderFct_(make_, object->tag(), object->group());
            if (fct) {
                // If an encoding function is registered for the tag, use it
                EXV_CALL_MEMBER_FN(*this, fct)(object, ed);
            }
            else {
                // Else use the encode function at the object (results in a double-dispatch
                // to the appropriate encoding function of the encoder.
                object->encode(*this, ed);
            }
        }
        if (del_ && pos != exifData_.end()) {
            exifData_.erase(pos);
        }
#ifdef DEBUG
        std::cerr << "\n";
#endif

    } // TiffEncoder::encodeTiffComponent

    void TiffEncoder::encodeArrayElement(TiffArrayElement* object, const Exifdatum* datum)
    {
        encodeTiffEntryBase(object, datum);
    } // TiffEncoder::encodeArrayElement

    void TiffEncoder::encodeArrayEntry(TiffArrayEntry* object, const Exifdatum* datum)
    {
        encodeOffsetEntry(object, datum);
    } // TiffEncoder::encodeArrayEntry

    void TiffEncoder::encodeDataEntry(TiffDataEntry* object, const Exifdatum* datum)
    {
        encodeOffsetEntry(object, datum);

        if (!dirty_ && writeMethod() == wmNonIntrusive) {
            assert(object);
            assert(object->pValue());
            if (  object->sizeDataArea_
                < static_cast<uint32_t>(object->pValue()->sizeDataArea())) {
#ifdef DEBUG
                ExifKey key(object->tag(), tiffGroupName(object->group()));
                std::cerr << "DATAAREA GREW     " << key << "\n";
#endif
                setDirty();
            }
            else {
                // Write the new dataarea, fill with 0x0
#ifdef DEBUG
                ExifKey key(object->tag(), tiffGroupName(object->group()));
                std::cerr << "Writing data area for " << key << "\n";
#endif
                DataBuf buf = object->pValue()->dataArea();
                memcpy(object->pDataArea_, buf.pData_, buf.size_);
                if (object->sizeDataArea_ - buf.size_ > 0) {
                    memset(object->pDataArea_ + buf.size_,
                           0x0, object->sizeDataArea_ - buf.size_);
                }
            }
        }

    } // TiffEncoder::encodeDataEntry

    void TiffEncoder::encodeTiffEntry(TiffEntry* object, const Exifdatum* datum)
    {
        encodeTiffEntryBase(object, datum);
    } // TiffEncoder::encodeTiffEntry

    void TiffEncoder::encodeImageEntry(TiffImageEntry* object, const Exifdatum* datum)
    {
        encodeOffsetEntry(object, datum);

        uint32_t sizeDataArea = object->pValue()->sizeDataArea();

        if (sizeDataArea > 0 && writeMethod() == wmNonIntrusive) {
#ifdef DEBUG
            std::cerr << "\t DATAAREA IS SET (NON-INTRUSIVE WRITING)";
#endif
            setDirty();
        }

        if (sizeDataArea > 0 && writeMethod() == wmIntrusive) {
#ifdef DEBUG
            std::cerr << "\t DATAAREA IS SET (INTRUSIVE WRITING)";
#endif
            // Set pseudo strips (without a data pointer) from the size tag
            ExifKey key(object->szTag(), tiffGroupName(object->szGroup()));
            ExifData::const_iterator pos = exifData_.findKey(key);
            const byte* zero = 0;
            if (pos == exifData_.end()) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: Size tag " << key
                          << " not found. Writing only one strip.\n";
#endif
                object->strips_.clear();
                object->strips_.push_back(std::make_pair(zero, sizeDataArea));
            }
            else {
                uint32_t sizeTotal = 0;
                object->strips_.clear();
                for (long i = 0; i < pos->count(); ++i) {
                    uint32_t len = pos->toLong(i);
                    object->strips_.push_back(std::make_pair(zero, len));
                    sizeTotal += len;
                }
                if (sizeTotal != sizeDataArea) {
#ifndef SUPPRESS_WARNINGS
                    ExifKey key2(object->tag(), tiffGroupName(object->group()));
                    std::cerr << "Error: Sum of all sizes of " << key
                              << " != data size of " << key2 << ". "
                              << "This results in an invalid image.\n";
#endif
                    // Todo: How to fix? Write only one strip?
                }
            }
        }

        if (sizeDataArea == 0 && writeMethod() == wmIntrusive) {
#ifdef DEBUG
            std::cerr << "\t USE STRIPS FROM SOURCE TREE IMAGE ENTRY";
#endif
            // Set strips from source tree
            if (pSourceTree_) {
                TiffFinder finder(object->tag(), object->group());
                pSourceTree_->accept(finder);
                TiffImageEntry* ti = dynamic_cast<TiffImageEntry*>(finder.result());
                if (ti) {
                    object->strips_ = ti->strips_;
                }
            }
#ifndef SUPPRESS_WARNINGS
            else {
                ExifKey key2(object->tag(), tiffGroupName(object->group()));
                std::cerr << "Warning: No image data to encode " << key2 << ".\n";
            }
#endif
        }

    } // TiffEncoder::encodeImageEntry

    void TiffEncoder::encodeMnEntry(TiffMnEntry* object, const Exifdatum* datum)
    {
        // Test is required here as well as in the visit function
        if (!object->mn_) encodeTiffEntryBase(object, datum);
    } // TiffEncoder::encodeMnEntry

    void TiffEncoder::encodeSizeEntry(TiffSizeEntry* object, const Exifdatum* datum)
    {
        encodeTiffEntryBase(object, datum);
    } // TiffEncoder::encodeSizeEntry

    void TiffEncoder::encodeSubIfd(TiffSubIfd* object, const Exifdatum* datum)
    {
        encodeOffsetEntry(object, datum);
    } // TiffEncoder::encodeSubIfd

    void TiffEncoder::encodeTiffEntryBase(TiffEntryBase* object, const Exifdatum* datum)
    {
        assert(object != 0);
        assert(datum != 0);

#ifdef DEBUG
        bool tooLarge = false;
#endif
        uint32_t newSize = datum->size();
        if (newSize > object->size_) { // value doesn't fit, encode for intrusive writing
            setDirty();
#ifdef DEBUG
            tooLarge = true;
#endif
        }
        object->updateValue(datum->getValue(), byteOrder()); // clones the value
#ifdef DEBUG
        ExifKey key(object->tag(), tiffGroupName(object->group()));
        std::cerr << "UPDATING DATA     " << key;
        if (tooLarge) {
            std::cerr << "\t\t\t ALLOCATED " << object->size_ << " BYTES";
        }
#endif
    } // TiffEncoder::encodeTiffEntryBase

    void TiffEncoder::encodeOffsetEntry(TiffEntryBase* object, const Exifdatum* datum)
    {
        assert(object != 0);
        assert(datum != 0);

        uint32_t newSize = datum->size();
        if (newSize > object->size_) { // value doesn't fit, encode for intrusive writing
            setDirty();
            object->updateValue(datum->getValue(), byteOrder()); // clones the value
#ifdef DEBUG
            ExifKey key(object->tag(), tiffGroupName(object->group()));
            std::cerr << "UPDATING DATA     " << key;
            std::cerr << "\t\t\t ALLOCATED " << object->size() << " BYTES";
#endif
        }
        else {
            object->setValue(datum->getValue()); // clones the value
#ifdef DEBUG
            ExifKey key(object->tag(), tiffGroupName(object->group()));
            std::cerr << "NOT UPDATING      " << key;
            std::cerr << "\t\t\t PRESERVE VALUE DATA";
#endif
        }

    } // TiffEncoder::encodeOffsetEntry

    void TiffEncoder::encodeOlympThumb(TiffEntryBase* object, const Exifdatum* datum)
    {
        // Todo
    }

    void TiffEncoder::encodeBigEndianEntry(TiffEntryBase* object, const Exifdatum* datum)
    {
        byteOrder_ = bigEndian;
        encodeTiffEntryBase(object, datum);
        byteOrder_ = origByteOrder_;
    }

    void TiffEncoder::add(
        TiffComponent*     pRootDir,
        TiffComponent*     pSourceDir,
        TiffCompFactoryFct createFct
    )
    {
        assert(pRootDir != 0);
        writeMethod_ = wmIntrusive;
        pSourceTree_ = pSourceDir;

        // Ensure that the exifData_ entries are not deleted, to be able to
        // iterate over all remaining entries.
        del_ = false;

        for (ExifData::const_iterator i = exifData_.begin();
             i != exifData_.end(); ++i) {

            // Assumption is that the corresponding TIFF entry doesn't exist

            // Todo: This takes tag and group straight from the Exif datum.
            // There is a need for a simple mapping and a provision for quite
            // sophisticated logic to determine the mapped tag and group to
            // handle complex cases (eg, NEF sub-IFDs)

            // Todo: getPath depends on the Creator class, not the createFct
            //       how to get it through to here???

            TiffPath tiffPath;
            TiffCreator::getPath(tiffPath, i->tag(), tiffGroupId(i->groupName()));
            TiffComponent* tc = pRootDir->addPath(i->tag(), tiffPath);
            TiffEntryBase* object = dynamic_cast<TiffEntryBase*>(tc);
#ifdef DEBUG
            if (object == 0) {
                std::cerr << "Warning: addPath() didn't add an entry for "
                          << i->groupName()
                          << " tag 0x" << std::setw(4) << std::setfill('0')
                          << std::hex << i->tag() << "\n";
            }
#endif
            if (object != 0) {
                encodeTiffComponent(object, &(*i));
            }
        }
    } // TiffEncoder::add

    const std::string TiffPrinter::indent_("   ");

    void TiffPrinter::incIndent()
    {
        prefix_ += indent_;
    } // TiffPrinter::incIndent

    void TiffPrinter::decIndent()
    {
        if (prefix_.length() >= indent_.length()) {
            prefix_.erase(prefix_.length() - indent_.length(), indent_.length());
        }
    } // TiffPrinter::decIndent

    void TiffPrinter::visitEntry(TiffEntry* object)
    {
        printTiffEntry(object, prefix());
    } // TiffPrinter::visitEntry

    void TiffPrinter::visitDataEntry(TiffDataEntry* object)
    {
        printTiffEntry(object, prefix());
        if (object->pValue()) {
            os_ << prefix() << _("Data area") << " "
                << object->pValue()->sizeDataArea()
                << " " << _("bytes.\n");
        }
    } // TiffPrinter::visitDataEntry

    void TiffPrinter::visitImageEntry(TiffImageEntry* object)
    {
        printTiffEntry(object, prefix());
    } // TiffPrinter::visitImageEntry

    void TiffPrinter::visitSizeEntry(TiffSizeEntry* object)
    {
        printTiffEntry(object, prefix());
    }

    void TiffPrinter::visitDirectory(TiffDirectory* object)
    {
        assert(object != 0);
        os_ << prefix() << tiffGroupName(object->group())
            << " " << _("directory with") << " "
            // cast to make MSVC happy
            << std::dec << static_cast<unsigned int>(object->components_.size());
        if (object->components_.size() == 1) os_ << " " << _("entry:\n");
        else os_ << " " << _("entries:\n");
        incIndent();

    } // TiffPrinter::visitDirectory

    void TiffPrinter::visitDirectoryNext(TiffDirectory* object)
    {
        decIndent();
        if (object->hasNext()) {
            if (object->pNext_) os_ << prefix() << _("Next directory:\n");
            else os_ << prefix() << _("No next directory\n");
        }
    } // TiffPrinter::visitDirectoryNext

    void TiffPrinter::visitDirectoryEnd(TiffDirectory* /*object*/)
    {
        // Nothing to do
    } // TiffPrinter::visitDirectoryEnd

    void TiffPrinter::visitSubIfd(TiffSubIfd* object)
    {
        os_ << prefix() << _("Sub-IFD") << " ";
        printTiffEntry(object);
    } // TiffPrinter::visitSubIfd

    void TiffPrinter::visitMnEntry(TiffMnEntry* object)
    {
        if (!object->mn_) printTiffEntry(object, prefix());
        else os_ << prefix() << _("Makernote") << " ";
    } // TiffPrinter::visitMnEntry

    void TiffPrinter::visitIfdMakernote(TiffIfdMakernote* /*object*/)
    {
        // Nothing to do
    } // TiffPrinter::visitIfdMakernote

    void TiffPrinter::printTiffEntry(TiffEntryBase* object,
                                     const std::string& px) const
    {
        assert(object != 0);

        os_ << px << tiffGroupName(object->group())
            << " " << _("tag") << " 0x" << std::setw(4) << std::setfill('0')
            << std::hex << std::right << object->tag()
            << ", " << _("type") << " 0x" << std::hex << object->tiffType()
            << ", " << std::dec << object->count() << " "<< _("component");
        if (object->count() > 1) os_ << "s";
        os_ << " in " << object->size() << " " << _("bytes");
        if (object->size() > 4) os_ << ", " << _("offset") << " " << object->offset();
        os_ << "\n";
        const Value* vp = object->pValue();
        if (vp && vp->count() < 100) os_ << prefix() << *vp;
        else os_ << prefix() << "...";
        os_ << "\n";

    } // TiffPrinter::printTiffEntry

    void TiffPrinter::visitArrayEntry(TiffArrayEntry* object)
    {
        os_ << prefix() << _("Array Entry") << " " << tiffGroupName(object->group())
            << " " << _("tag") << " 0x" << std::setw(4) << std::setfill('0')
            << std::hex << std::right << object->tag() << " " << _("with")
            << " " << std::dec << object->count() << " ";
        if (object->count() > 1) os_ << _("elements");
        else os_ << _("element");
        os_ << "\n";
    } // TiffPrinter::visitArrayEntry

    void TiffPrinter::visitArrayElement(TiffArrayElement* object)
    {
        printTiffEntry(object, prefix());
    } // TiffPrinter::visitArrayElement

    TiffReader::TiffReader(const byte*    pData,
                           uint32_t       size,
                           TiffComponent* pRoot,
                           TiffRwState::AutoPtr state)
        : pData_(pData),
          size_(size),
          pLast_(pData + size),
          pRoot_(pRoot),
          pState_(state.release()),
          pOrigState_(pState_)
    {
        assert(pData_);
        assert(size_ > 0);

    } // TiffReader::TiffReader

    TiffReader::~TiffReader()
    {
        if (pOrigState_ != pState_) delete pOrigState_;
        delete pState_;
    }

    void TiffReader::resetState() {
        if (pOrigState_ != pState_) delete pState_;
        pState_ = pOrigState_;
    }

    void TiffReader::changeState(TiffRwState::AutoPtr state)
    {
        if (state.get() != 0) {
            if (pOrigState_ != pState_) delete pState_;
            // 0 for create function indicates 'no change'
            if (state->createFct_ == 0) state->createFct_ = pState_->createFct_;
            // invalidByteOrder indicates 'no change'
            if (state->byteOrder_ == invalidByteOrder) state->byteOrder_ = pState_->byteOrder_;
            pState_ = state.release();
        }
    }

    ByteOrder TiffReader::byteOrder() const
    {
        assert(pState_);
        return pState_->byteOrder_;
    }

    uint32_t TiffReader::baseOffset() const
    {
        assert(pState_);
        return pState_->baseOffset_;
    }

    TiffComponent::AutoPtr TiffReader::create(uint32_t extendedTag,
                                              uint16_t group) const
    {
        assert(pState_);
        assert(pState_->createFct_);
        return pState_->createFct_(extendedTag, group);
    }

    void TiffReader::readDataEntryBase(TiffDataEntryBase* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        TiffFinder finder(object->szTag(), object->szGroup());
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        if (te && te->pValue()) {
            object->setStrips(te->pValue(), pData_, size_, baseOffset());
        }
    }

    void TiffReader::visitEntry(TiffEntry* object)
    {
        readTiffEntry(object);
    }

    void TiffReader::visitDataEntry(TiffDataEntry* object)
    {
        readDataEntryBase(object);
    }

    void TiffReader::visitImageEntry(TiffImageEntry* object)
    {
        readDataEntryBase(object);
    }

    void TiffReader::visitSizeEntry(TiffSizeEntry* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        TiffFinder finder(object->dtTag(), object->dtGroup());
        pRoot_->accept(finder);
        TiffDataEntryBase* te = dynamic_cast<TiffDataEntryBase*>(finder.result());
        if (te && te->pValue()) {
            te->setStrips(object->pValue(), pData_, size_, baseOffset());
        }
    }

    void TiffReader::visitDirectory(TiffDirectory* object)
    {
        assert(object != 0);

        const byte* p = object->start();
        assert(p >= pData_);

        if (p + 2 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: "
                      << "Directory " << tiffGroupName(object->group())
                      << ": IFD exceeds data buffer, cannot read entry count.\n";
#endif
            return;
        }
        const uint16_t n = getUShort(p, byteOrder());
        p += 2;
        // Sanity check with an "unreasonably" large number
        if (n > 256) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: "
                      << "Directory " << tiffGroupName(object->group()) << " with "
                      << n << " entries considered invalid; not read.\n";
#endif
            return;
        }
        for (uint16_t i = 0; i < n; ++i) {
            if (p + 12 > pLast_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << tiffGroupName(object->group())
                          << ": IFD entry " << i
                          << " lies outside of the data buffer.\n";
#endif
                return;
            }
            uint16_t tag = getUShort(p, byteOrder());
            TiffComponent::AutoPtr tc = create(tag, object->group());
            assert(tc.get());
            tc->setStart(p);
            object->addChild(tc);
            p += 12;
        }

        if (object->hasNext()) {
            if (p + 4 > pLast_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: "
                          << "Directory " << tiffGroupName(object->group())
                          << ": IFD exceeds data buffer, cannot read next pointer.\n";
#endif
                return;
            }
            TiffComponent::AutoPtr tc(0);
            uint32_t next = getLong(p, byteOrder());
            if (next) {
                tc = create(Tag::next, object->group());
#ifndef SUPPRESS_WARNINGS
                if (tc.get() == 0) {
                    std::cerr << "Warning: "
                              << "Directory " << tiffGroupName(object->group())
                              << " has an unhandled next pointer.\n";
                }
#endif
            }
            if (tc.get()) {
                if (baseOffset() + next > size_) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Error: "
                              << "Directory " << tiffGroupName(object->group())
                              << ": Next pointer is out of bounds; ignored.\n";
#endif
                    return;
                }
                tc->setStart(pData_ + baseOffset() + next);
                object->addNext(tc);
            }
        } // object->hasNext()

    } // TiffReader::visitDirectory

    void TiffReader::visitSubIfd(TiffSubIfd* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        if (   (object->tiffType() == ttUnsignedLong || object->tiffType() == ttSignedLong)
            && object->count() >= 1) {
            for (uint32_t i = 0; i < object->count(); ++i) {
                int32_t offset = getLong(object->pData() + 4*i, byteOrder());
                if (   baseOffset() + offset > size_
                    || static_cast<int32_t>(baseOffset()) + offset < 0) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Error: "
                              << "Directory " << tiffGroupName(object->group())
                              << ", entry 0x" << std::setw(4)
                              << std::setfill('0') << std::hex << object->tag()
                              << " Sub-IFD pointer " << i
                              << " is out of bounds; ignoring it.\n";
#endif
                    return;
                }
                // If there are multiple dirs, group is incremented for each
                TiffComponent::AutoPtr td(new TiffDirectory(object->tag(),
                                                            object->newGroup_ + i));
                td->setStart(pData_ + baseOffset() + offset);
                object->addChild(td);
            }
        }
#ifndef SUPPRESS_WARNINGS
        else {
            std::cerr << "Warning: "
                      << "Directory " << tiffGroupName(object->group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << object->tag()
                      << " doesn't look like a sub-IFD.\n";
        }
#endif

    } // TiffReader::visitSubIfd

    void TiffReader::visitMnEntry(TiffMnEntry* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        // Find camera make
        TiffFinder finder(0x010f, Group::ifd0);
        pRoot_->accept(finder);
        TiffEntryBase* te = dynamic_cast<TiffEntryBase*>(finder.result());
        std::string make;
        if (te && te->pValue()) {
            make = te->pValue()->toString();
            // create concrete makernote, based on make and makernote contents
            object->mn_ = TiffMnCreator::create(object->tag(),
                                                object->mnGroup_,
                                                make,
                                                object->pData_,
                                                object->size_,
                                                byteOrder());
        }
        if (object->mn_) object->mn_->setStart(object->pData());

    } // TiffReader::visitMnEntry

    void TiffReader::visitIfdMakernote(TiffIfdMakernote* object)
    {
        assert(object != 0);

        if (!object->readHeader(object->start(),
                                static_cast<uint32_t>(pLast_ - object->start()),
                                byteOrder())) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Failed to read "
                      << tiffGroupName(object->ifd_.group())
                      << " IFD Makernote header.\n";
#ifdef DEBUG
            if (static_cast<uint32_t>(pLast_ - object->start()) >= 16) {
                hexdump(std::cerr, object->start(), 16);
            }
#endif // DEBUG
#endif // SUPPRESS_WARNINGS
            setGo(geKnownMakernote, false);
            return;
        }
        // Modify reader for Makernote peculiarities, byte order and offset
        TiffRwState::AutoPtr state(
            new TiffRwState(object->byteOrder(),
                            object->baseOffset(static_cast<uint32_t>(object->start() - pData_))));
        changeState(state);
        object->ifd_.setStart(object->start() + object->ifdOffset());

    } // TiffReader::visitIfdMakernote

    void TiffReader::visitIfdMakernoteEnd(TiffIfdMakernote* /*object*/)
    {
        // Reset state (byte order, create function, offset) back to that for the image
        resetState();
    } // TiffReader::visitIfdMakernoteEnd

    void TiffReader::readTiffEntry(TiffEntryBase* object)
    {
        assert(object != 0);

        byte* p = object->start();
        assert(p >= pData_);

        if (p + 12 > pLast_) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Entry in directory " << tiffGroupName(object->group())
                      << "requests access to memory beyond the data buffer. "
                      << "Skipping entry.\n";
#endif
            return;
        }
        // Component already has tag
        p += 2;
        TiffType tiffType = getUShort(p, byteOrder());
        TypeId typeId = toTypeId(tiffType, object->tag(), object->group());
        long typeSize = TypeInfo::typeSize(typeId);
        if (0 == typeSize) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Directory " << tiffGroupName(object->group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << object->tag()
                      << " has unknown Exif (TIFF) type " << std::dec << tiffType
                      << "; setting type size 1.\n";
#endif
            typeSize = 1;
        }
        p += 2;
        uint32_t count = getULong(p, byteOrder());
        if (count >= 0x10000000) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Directory " << tiffGroupName(object->group())
                      << ", entry 0x" << std::setw(4)
                      << std::setfill('0') << std::hex << object->tag()
                      << " has invalid size "
                      << std::dec << count << "*" << typeSize
                      << "; skipping entry.\n";
#endif
            return;
        }
        p += 4;
        uint32_t size = typeSize * count;
        uint32_t offset = getLong(p, byteOrder());
        byte* pData = p;
        if (size > 4 && baseOffset() + offset >= size_) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: Offset of "
                          << "directory " << tiffGroupName(object->group())
                          << ", entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << object->tag()
                          << " is out of bounds: "
                          << "Offset = 0x" << std::setw(8)
                          << std::setfill('0') << std::hex << offset
                          << "; truncating the entry\n";
#endif
                size = 0;
        }
        if (size > 4) {
            pData = const_cast<byte*>(pData_) + baseOffset() + offset;
            if (size > static_cast<uint32_t>(pLast_ - pData)) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: Upper boundary of data for "
                          << "directory " << tiffGroupName(object->group())
                          << ", entry 0x" << std::setw(4)
                          << std::setfill('0') << std::hex << object->tag()
                          << " is out of bounds: "
                          << "Offset = 0x" << std::setw(8)
                          << std::setfill('0') << std::hex << offset
                          << ", size = " << std::dec << size
                          << ", exceeds buffer size by "
                          // cast to make MSVC happy
                          << static_cast<uint32_t>(pData + size - pLast_)
                          << " Bytes; adjusting the size\n";
#endif
                size = static_cast<uint32_t>(pLast_ - pData + 1);
                // Todo: adjust count, make size a multiple of typeSize
            }
        }
        Value::AutoPtr v = Value::create(typeId);
        assert(v.get());
        v->read(pData, size, byteOrder());

        object->setValue(v);
        object->setData(pData, size);
        object->setOffset(offset);

    } // TiffReader::readTiffEntry

    void TiffReader::visitArrayEntry(TiffArrayEntry* object)
    {
        assert(object != 0);

        readTiffEntry(object);
        // Todo: size here is that of the data area
        const uint16_t sz = static_cast<uint16_t>(object->size_ / object->elSize());
        for (uint16_t i = 0; i < sz; ++i) {
            uint16_t tag = i;
            TiffComponent::AutoPtr tc = create(tag, object->elGroup());
            assert(tc.get());
            tc->setStart(object->pData() + i * object->elSize());
            object->addChild(tc);
            // Hack: Exif.CanonCs.Lens has 3 components
            if (object->elGroup() == Group::canoncs && tag == 0x0017) {
                i += 2;
            }
        }

    } // TiffReader::visitArrayEntry

    void TiffReader::visitArrayElement(TiffArrayElement* object)
    {
        assert(object != 0);

        TypeId typeId = toTypeId(object->elTiffType(), object->tag(), object->group());
        uint32_t size = TypeInfo::typeSize(typeId);
        // Hack: Exif.CanonCs.Lens has 3 components
        if (object->group() == Group::canoncs && object->tag() == 0x0017) {
            size *= 3;
        }
        byte* pData   = object->start();
        assert(pData >= pData_);

        if (pData + size > pLast_) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Error: Array element in group "
                      << tiffGroupName(object->group())
                      << "requests access to memory beyond the data buffer. "
                      << "Skipping element.\n";
#endif
            return;
        }

        ByteOrder bo = object->elByteOrder();
        if (bo == invalidByteOrder) bo = byteOrder();
        Value::AutoPtr v = Value::create(typeId);
        assert(v.get());
        v->read(pData, size, bo);

        object->setValue(v);
        object->setData(pData, size);
        object->setOffset(0);
        object->setCount(1);

    } // TiffReader::visitArrayElement

}}                                      // namespace Internal, Exiv2
