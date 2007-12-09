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
  File:      iptc.cpp
  Version:   $Rev$
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History:   31-July-04, brad: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "iptc.hpp"
#include "types.hpp"
#include "error.hpp"
#include "value.hpp"
#include "datasets.hpp"
#include "jpgimage.hpp"

// + standard includes
#include <iostream>
#include <algorithm>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Iptcdatum::Iptcdatum(const IptcKey& key,
                         const Value* pValue)
        : key_(key.clone())
    {
        if (pValue) value_ = pValue->clone();
    }

    Iptcdatum::Iptcdatum(const Iptcdatum& rhs)
        : Metadatum(rhs)
    {
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy
    }

    Iptcdatum::~Iptcdatum()
    {
    }

    const Value& Iptcdatum::value() const
    {
        if (value_.get() == 0) throw Error(8);
        return *value_;
    }

    Iptcdatum& Iptcdatum::operator=(const Iptcdatum& rhs)
    {
        if (this == &rhs) return *this;
        Metadatum::operator=(rhs);

        key_.reset();
        if (rhs.key_.get() != 0) key_ = rhs.key_->clone(); // deep copy

        value_.reset();
        if (rhs.value_.get() != 0) value_ = rhs.value_->clone(); // deep copy

        return *this;
    } // Iptcdatum::operator=

    Iptcdatum& Iptcdatum::operator=(const uint16_t& value)
    {
        UShortValue::AutoPtr v(new UShortValue);
        v->value_.push_back(value);
        value_ = v;
        return *this;
    }

    Iptcdatum& Iptcdatum::operator=(const std::string& value)
    {
        setValue(value);
        return *this;
    }

    Iptcdatum& Iptcdatum::operator=(const Value& value)
    {
        setValue(&value);
        return *this;
    }

    void Iptcdatum::setValue(const Value* pValue)
    {
        value_.reset();
        if (pValue) value_ = pValue->clone();
    }

    void Iptcdatum::setValue(const std::string& value)
    {
        if (value_.get() == 0) {
            TypeId type = IptcDataSets::dataSetType(tag(), record());
            value_ = Value::create(type);
        }
        value_->read(value);
    }

    const byte IptcData::marker_ = 0x1C;          // Dataset marker

    Iptcdatum& IptcData::operator[](const std::string& key)
    {
        IptcKey iptcKey(key);
        iterator pos = findKey(iptcKey);
        if (pos == end()) {
            add(Iptcdatum(iptcKey));
            pos = findKey(iptcKey);
        }
        return *pos;
    }

    int IptcData::load(const byte* buf, long len)
    {
#ifdef DEBUG
        std::cerr << "IptcData::load, len = " << len << "\n";
#endif
        const byte* pRead = buf;
        iptcMetadata_.clear();

        uint16_t record = 0;
        uint16_t dataSet = 0;
        uint32_t sizeData = 0;
        byte extTest = 0;

        while (pRead + 3 < buf + len) {
            // First byte should be a marker. If it isn't, scan forward and skip
            // the chunk bytes present in some images. This deviates from the
            // standard, which advises to treat such cases as errors.
            if (*pRead++ != marker_) continue;
            record = *pRead++;
            dataSet = *pRead++;

            extTest = *pRead;
            if (extTest & 0x80) {
                // extended dataset
                uint16_t sizeOfSize = (getUShort(pRead, bigEndian) & 0x7FFF);
                if (sizeOfSize > 4) return 5;
                pRead += 2;
                sizeData = 0;
                for (; sizeOfSize > 0; --sizeOfSize) {
                    sizeData |= *pRead++ << (8 *(sizeOfSize-1));
                }
            }
            else {
                // standard dataset
                sizeData = getUShort(pRead, bigEndian);
                pRead += 2;
            }
            if (pRead + sizeData <= buf + len) {
                int rc = 0;
                if ((rc = readData(dataSet, record, pRead, sizeData)) != 0) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: "
                              << "Failed to read IPTC dataset "
                              << IptcKey(dataSet, record)
                              << " (rc = " << rc << "); skipped.\n";
#endif
                }
            }
#ifndef SUPPRESS_WARNINGS
            else {
                std::cerr << "Warning: "
                          << "IPTC dataset " << IptcKey(dataSet, record)
                          << " has invalid size " << sizeData << "; skipped.\n";
            }
#endif
            pRead += sizeData;
        }

        return 0;
    } // IptcData::load

    int IptcData::readData(uint16_t dataSet, uint16_t record,
                           const byte* data, uint32_t sizeData)
    {
        Value::AutoPtr value;
        TypeId type = IptcDataSets::dataSetType(dataSet, record);
        value = Value::create(type);
        int rc = value->read(data, sizeData, bigEndian);
        if (0 == rc) {
            IptcKey key(dataSet, record);
            add(key, value.get());
        }
        else if (1 == rc) {
            // If the first attempt failed, try with a string value
            value = Value::create(string);
            rc = value->read(data, sizeData, bigEndian);
            if (0 == rc) {
                IptcKey key(dataSet, record);
                add(key, value.get());
            }
        }
        return rc;
    }

    DataBuf IptcData::copy() const
    {
        DataBuf buf(size());
        byte *pWrite = buf.pData_;

        const_iterator iter = iptcMetadata_.begin();
        const_iterator end = iptcMetadata_.end();
        for ( ; iter != end; ++iter) {
            // marker, record Id, dataset num
            *pWrite++ = marker_;
            *pWrite++ = static_cast<byte>(iter->record());
            *pWrite++ = static_cast<byte>(iter->tag());

            // extended or standard dataset?
            long dataSize = iter->size();
            if (dataSize > 32767) {
                // always use 4 bytes for extended length
                uint16_t sizeOfSize = 4 | 0x8000;
                us2Data(pWrite, sizeOfSize, bigEndian);
                pWrite += 2;
                ul2Data(pWrite, dataSize, bigEndian);
                pWrite += 4;
            }
            else {
                us2Data(pWrite, static_cast<uint16_t>(dataSize), bigEndian);
                pWrite += 2;
            }

            pWrite += iter->value().copy(pWrite, bigEndian);
        }

        return buf;
    } // IptcData::copy

    long IptcData::size() const
    {
        long newSize = 0;
        const_iterator iter = iptcMetadata_.begin();
        const_iterator end = iptcMetadata_.end();
        for ( ; iter != end; ++iter) {
            // marker, record Id, dataset num, first 2 bytes of size
            newSize += 5;
            long dataSize = iter->size();
            newSize += dataSize;
            if (dataSize > 32767) {
                // extended dataset (we always use 4 bytes)
                newSize += 4;
            }
        }
        return newSize;
    } // IptcData::size

    int IptcData::add(const IptcKey& key, Value* value)
    {
        return add(Iptcdatum(key, value));
    }

    int IptcData::add(const Iptcdatum& iptcDatum)
    {
        if (!IptcDataSets::dataSetRepeatable(
               iptcDatum.tag(), iptcDatum.record()) &&
               findId(iptcDatum.tag(), iptcDatum.record()) != end()) {
             return 6;
        }
        // allow duplicates
        iptcMetadata_.push_back(iptcDatum);
        return 0;
    }

    IptcData::const_iterator IptcData::findKey(const IptcKey& key) const
    {
        return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(),
                            FindMetadatumById(key.tag(), key.record()));
    }

    IptcData::iterator IptcData::findKey(const IptcKey& key)
    {
        return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(),
                            FindMetadatumById(key.tag(), key.record()));
    }

    IptcData::const_iterator IptcData::findId(uint16_t dataset, uint16_t record) const
    {
        return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(),
                            FindMetadatumById(dataset, record));
    }

    IptcData::iterator IptcData::findId(uint16_t dataset, uint16_t record)
    {
        return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(),
                            FindMetadatumById(dataset, record));
    }

    void IptcData::sortByKey()
    {
        std::sort(iptcMetadata_.begin(), iptcMetadata_.end(), cmpMetadataByKey);
    }

    void IptcData::sortByTag()
    {
        std::sort(iptcMetadata_.begin(), iptcMetadata_.end(), cmpMetadataByTag);
    }

    IptcData::iterator IptcData::erase(IptcData::iterator pos)
    {
        return iptcMetadata_.erase(pos);
    }

    // *************************************************************************
    // free functions
    std::ostream& operator<<(std::ostream& os, const Iptcdatum& md)
    {
        return os << md.value();
    }

}                                       // namespace Exiv2
