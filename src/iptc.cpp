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
  File:      iptc.cpp
  Version:   $Name:  $ $Revision: 1.4 $
  Author(s): Brad Schick (brad) <schick@robotbattle.com>
  History:   31-July-04, brad: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.4 $ $RCSfile: iptc.cpp,v $");

// Define DEBUG_MAKERNOTE to output debug information to std::cerr
#undef DEBUG_MAKERNOTE

// *****************************************************************************
// included header files
#include "iptc.hpp"
#include "types.hpp"
#include "error.hpp"
#include "value.hpp"
#include "datasets.hpp"
#include "image.hpp"

// + standard includes
#include <iostream>
#include <algorithm>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    IptcKey::IptcKey(const std::string& key)
        : key_(key)
    {
        decomposeKey();
    }

    IptcKey::IptcKey(uint16_t tag, uint16_t record)
        : tag_(tag), record_(record), key_(IptcDataSets::makeKey(tag, record))
    {
    }

    IptcKey::IptcKey(const IptcKey& rhs)
        : tag_(rhs.tag_), record_(rhs.record_), key_(rhs.key_)
    {
    }

    IptcKey& IptcKey::operator=(const IptcKey& rhs)
    {
        if (this == &rhs) return *this;
        Key::operator=(rhs);
        tag_ = rhs.tag_;
        record_ = rhs.record_;
        key_ = rhs.key_;
        return *this;
    }

    IptcKey* IptcKey::clone() const
    {
        return new IptcKey(*this);
    }

    void IptcKey::decomposeKey()
    {
        std::pair<uint16_t, uint16_t> p = IptcDataSets::decomposeKey(key_);
        if (p.first == 0xffff) throw Error("Invalid key");
        if (p.second == IptcDataSets::invalidRecord) throw Error("Invalid key");
        tag_ = p.first;
        record_ = p.second;
    }

    Iptcdatum::Iptcdatum(const IptcKey& key, 
                         const Value* value)
        : pKey_(key.clone()), pValue_(0), modified_(false)
    {
        if (value) pValue_ = value->clone();
    }

    Iptcdatum::Iptcdatum(const Iptcdatum& rhs)
        : Metadatum(rhs), pKey_(0), pValue_(0), modified_(false)
    {
        if (rhs.pKey_ != 0) pKey_ = rhs.pKey_->clone(); // deep copy
        if (rhs.pValue_ != 0) pValue_ = rhs.pValue_->clone(); // deep copy
    }

    Iptcdatum::~Iptcdatum()
    {
        delete pKey_;
        delete pValue_;
    }

    Iptcdatum& Iptcdatum::operator=(const Iptcdatum& rhs)
    {
        if (this == &rhs) return *this;
        Metadatum::operator=(rhs);
        modified_ = true;

        delete pKey_;
        pKey_ = 0;
        if (rhs.pKey_ != 0) pKey_ = rhs.pKey_->clone(); // deep copy

        delete pValue_;
        pValue_ = 0;
        if (rhs.pValue_ != 0) pValue_ = rhs.pValue_->clone(); // deep copy

        return *this;
    } // Iptcdatum::operator=
    
    void Iptcdatum::setValue(const Value* pValue)
    {
        modified_ = true;
        delete pValue_;
        pValue_ = pValue->clone();
    }

    void Iptcdatum::setValue(const std::string& buf)
    {
        modified_ = true;
        if (pValue_ == 0) pValue_ = Value::create(string);
        pValue_->read(buf);
    }

    const byte IptcData::marker_ = 0x1C;          // Dataset marker

    IptcData::IptcData() 
        : size_(0), pData_(0), modified_(false)
    {
    }

    IptcData::~IptcData()
    {
        delete[] pData_;
    }

    int IptcData::read(const std::string& path)
    {
        if (!fileExists(path, true)) return -1;
        Image* pImage = ImageFactory::instance().open(path);
        if (pImage) {
            int rc = pImage->readMetadata();
            if (rc == 0) {
                if (pImage->sizeIptcData() > 0) {
                    rc = read(pImage->iptcData(), pImage->sizeIptcData());
                }
                else {
                    rc = 3;
                }
            }
            delete pImage;
            return rc;
        }
        // We don't know this type of file
        return -2;
    }

    int IptcData::read(const byte* buf, long len)
    {
        // Copy the data buffer
        delete[] pData_;
        pData_ = new byte[len];
        memcpy(pData_, buf, len);
        size_ = len;
        const byte* pRead = pData_;
        iptcMetadata_.clear();

        int rc = 0;
        uint16_t record = 0;
        uint16_t dataSet = 0;
        uint32_t sizeData = 0;
        byte extTest = 0;

        while (pRead < pData_ + size_) {
            if (*pRead++ != marker_) return 5;
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
            rc = readData(dataSet, record, pRead, sizeData);
            if( rc ) return rc;
            pRead += sizeData;
        }

        modified_ = false;
        return rc;
    } // IptcData::read

    int IptcData::readData(uint16_t dataSet, uint16_t record, 
                           const byte* data, uint32_t sizeData)
    {
        Value* val = 0;
        try {
            // If the type is unknown or the parse failes then
            // default to undefined type below.
            TypeId type = IptcDataSets::dataSetType(dataSet, record);
            val = Value::create(type);
            val->read(data, sizeData, bigEndian);
        } 
        catch (const Error&) {
            // Default to loading as raw bytes
            delete val;
            val = Value::create(undefined);
            val->read(data, sizeData, bigEndian);
        }
        IptcKey key(dataSet, record);
        add(key, val);
        delete val;
        return 0;
    }

    int IptcData::erase(const std::string& path) const
    {
        if (!fileExists(path, true)) return -1;
        Image* pImage = ImageFactory::instance().open(path);
        if (pImage == 0) return -2;

        // Read all metadata then erase only Iptc data
        int rc = pImage->readMetadata();
        if( rc == 0 ) {
            pImage->clearIptcData();
            rc = pImage->writeMetadata();
        }
        delete pImage;
        return rc;
    } // IptcData::erase

    int IptcData::write(const std::string& path) 
    {
        // Remove the Iptc section from the file if there is no metadata 
        if (count() == 0) return erase(path);

        if (!fileExists(path, true)) return -1;
        Image* pImage = ImageFactory::instance().open(path);
        if (pImage == 0) return -2;

        updateBuffer();

        // Read all metadata to preserve non-Iptc data
        int rc = pImage->readMetadata();
        if( rc == 0 ) {
            pImage->setIptcData(pData_, size_);
            rc = pImage->writeMetadata();
        }
        delete pImage;
        return rc;
    } // IptcData::write

    bool IptcData::modified() const
    {
        if (!modified_) {
            // check each metadata entry
            const_iterator iter = iptcMetadata_.begin();
            const_iterator end = iptcMetadata_.end();
            for( ; iter != end && !modified_; ++iter ) {
                modified_ = iter->modified();
            }
        }
        return modified_;
    }

    void IptcData::clearModified()
    {
        // check each metadata entry
        iterator iter = iptcMetadata_.begin();
        iterator end = iptcMetadata_.end();
        for( ; iter != end; ++iter ) {
            iter->clearModified();
        }
        modified_ = false;
    }
    
    long IptcData::copy(byte* buf)
    {
        updateBuffer();
        memcpy(buf, pData_, size_);
        return size_;
    }

    void IptcData::updateBuffer() 
    {
        if (!modified()) return;

        delete [] pData_;
        size_ = size();
        pData_ = new byte[size_];
        byte *pWrite = pData_;

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

        clearModified();
    }

    long IptcData::size() const
    {
        if (!modified()) return size_;
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

    int IptcData::writeIptcData(const std::string& path)
    {
        updateBuffer();
        ExvImage exvImage(path, true);
        if (!exvImage.good()) return -1;
        exvImage.setIptcData(pData_, size_);
        return exvImage.writeMetadata();
    } // IptcData::writeIptcData

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
        modified_ = true;
        // allow duplicates
        iptcMetadata_.push_back(iptcDatum);
        return 0;
    }

    IptcData::const_iterator IptcData::findKey(const IptcKey& key) const
    {
        return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(),
                            FindMetadatumByKey(key.key()));
    }

    IptcData::iterator IptcData::findKey(const IptcKey& key)
    {
        return std::find_if(iptcMetadata_.begin(), iptcMetadata_.end(),
                            FindMetadatumByKey(key.key()));
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
        modified_ = true;
        return iptcMetadata_.erase(pos);
    }

    std::string IptcData::strError(int rc, const std::string& path)
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
            error += "Couldn't read from the input file";
            break;
        case 2:
            error += "This does not look like a JPEG image";
            break;
        case 3:
            error += "No Iptc data found in the file";
            break;
        case 4:
            error += "Writing to the output file failed";
            break;
        case 5:
            error += "Invalid or corrupt Iptc data";
            break;
        case 6:
            error += "Iptc dataset already exists and is not repeatable";
            break;

        default:
            error += "Accessing Iptc data failed, rc = " + toString(rc);
            break;
        }
        return error;
    } // IptcData::strError

    // *************************************************************************
    // free functions
    std::ostream& operator<<(std::ostream& os, const Iptcdatum& md)
    {
        return os << md.value();
    }

}                                       // namespace Exiv2
