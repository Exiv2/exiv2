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
  File:      datasets.cpp
  Version:   $Rev$
  Author(s): Brad Schick (brad) <brad@robotbattle.com>
  History:   24-Jul-04, brad: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "datasets.hpp"
#include "error.hpp"
#include "types.hpp"
#include "value.hpp"
#include "metadatum.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    DataSet::DataSet(
        uint16_t number, 
        const char* name,
        const char* desc,
        bool mandatory,
        bool repeatable,
        uint32_t minbytes, 
        uint32_t maxbytes,
        TypeId type,
        uint16_t recordId,
        const char* photoshop
    )
        : number_(number), name_(name), desc_(desc), mandatory_(mandatory), 
           repeatable_(repeatable), minbytes_(minbytes), maxbytes_(maxbytes),
           type_(type), recordId_(recordId), photoshop_(photoshop)
    {
    }

    RecordInfo::RecordInfo(
        uint16_t recordId,
        const char* name,
        const char* desc
    )
        : recordId_(recordId), name_(name), desc_(desc)
    {
    }

    const RecordInfo IptcDataSets::recordInfo_[] = {
        RecordInfo(IptcDataSets::invalidRecord, "(invalid)", "(invalid)"),
        RecordInfo(IptcDataSets::envelope, "Envelope", "IIM envelope record"),
        RecordInfo(IptcDataSets::application2, "Application2", "IIM application record 2"),
    };
    
    static const DataSet envelopeRecord[] = {
        DataSet(IptcDataSets::ModelVersion, "ModelVersion", "Version of IIM part 1", true, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::Destination, "Destination", "Routing information", false, true, 0, 1024, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::FileFormat, "FileFormat", "IIM appendix A file format", true, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::FileVersion, "FileVersion", "File format version", true, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::ServiceId, "ServiceId", "Identifies the provider and product", true, false, 0, 10, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::EnvelopeNumber, "EnvelopeNumber", "Combined unique identification", true, false, 8, 8, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::ProductId, "ProductId", "Identifies service subset", false, true, 0, 32, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::EnvelopePriority, "EnvelopePriority", "Envelope handling priority", false, false, 1, 1, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::DateSent, "DateSent", "Date material was sent", true, false, 8, 8, Exiv2::date, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::TimeSent, "TimeSent", "Time material was sent", false, false, 11, 11, Exiv2::time, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::CharacterSet, "CharacterSet", "Specifies character sets", false, false, 0, 32, Exiv2::undefined, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::UNO, "UNO", "Unique Name of Object", false, false, 14, 80, Exiv2::string, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::ARMId, "ARMId", "Abstract Relationship Method identifier", false, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::envelope, ""),
        DataSet(IptcDataSets::ARMVersion, "ARMVersion", "Abstract Relationship Method version", false, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::envelope, ""),
        DataSet(0xffff, "(Invalid)", "(Invalid)", false, false, 0, 0, Exiv2::unsignedShort, IptcDataSets::envelope, "")
    };

    static const DataSet application2Record[] = {
        DataSet(IptcDataSets::RecordVersion, "RecordVersion", "Version of IIM part 2", true, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ObjectType, "ObjectType", "IIM appendix G object type", false, false, 3, 67, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ObjectAttribute, "ObjectAttribute", "IIM appendix G object attribute", false, true, 4, 68, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ObjectName, "ObjectName", "Shorthand reference of content", false, false, 0, 64, Exiv2::string, IptcDataSets::application2, "Document title"),
        DataSet(IptcDataSets::EditStatus, "EditStatus", "Content status", false, false, 0, 64, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::EditorialUpdate, "EditorialUpdate", "Indicates the type of update", false, false, 2, 2, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Urgency, "Urgency", "Editorial urgency of content", false, false, 1, 1, Exiv2::string, IptcDataSets::application2, "Urgency"),
        DataSet(IptcDataSets::Subject, "Subject", "Structured definition of the subject", false, true, 13, 236, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Category, "Category", "Identifies the subject", false, false, 0, 3, Exiv2::string, IptcDataSets::application2, "Category"),
        DataSet(IptcDataSets::SuppCategory, "SuppCategory", "Refines the subject", false, true, 0, 32, Exiv2::string, IptcDataSets::application2, "Supplemental Categories"),
        DataSet(IptcDataSets::FixtureId, "FixtureId", "Identifies content that recurs", false, false, 0, 32, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Keywords, "Keywords", "Information retrieval words", false, true, 0, 64, Exiv2::string, IptcDataSets::application2, "Keywords"),
        DataSet(IptcDataSets::LocationCode, "LocationCode", "ISO country code for content", false, true, 3, 3, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::LocationName, "LocationName", "Full country name for content", false, true, 0, 64, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ReleaseDate, "ReleaseDate", "Earliest intended usable date", false, false, 8, 8, Exiv2::date, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ReleaseTime, "ReleaseTime", "Earliest intended usable time", false, false, 11, 11, Exiv2::time, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ExpirationDate, "ExpirationDate", "Latest intended usable date", false, false, 8, 8, Exiv2::date, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ExpirationTime, "ExpirationTime", "Latest intended usable time", false, false, 11, 11, Exiv2::time, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::SpecialInstructions, "SpecialInstructions", "Editorial usage instructions", false, false, 0, 256, Exiv2::string, IptcDataSets::application2, "Instructions"),
        DataSet(IptcDataSets::ActionAdvised, "ActionAdvised", "Action provided to previous data", false, false, 2, 2, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ReferenceService, "ReferenceService", "Service Identifier of a prior envelope", false, true, 0, 10, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ReferenceDate, "ReferenceDate", "Date of a prior envelope", false, true, 8, 8, Exiv2::date, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ReferenceNumber, "ReferenceNumber", "Envelope Number of a prior envelope", false, true, 8, 8, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::DateCreated, "DateCreated", "Creation date of intellectual content", false, false, 8, 8, Exiv2::date, IptcDataSets::application2, "Date created"),
        DataSet(IptcDataSets::TimeCreated, "TimeCreated", "Creation time of intellectual content", false, false, 11, 11, Exiv2::time, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::DigitizationDate, "DigitizationDate", "Creation date of digital representation", false, false, 8, 8, Exiv2::date, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::DigitizationTime, "DigitizationTime", "Creation time of digital representation", false, false, 11, 11, Exiv2::time, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Program, "Program", "Content creation program", false, false, 0, 32, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ProgramVersion, "ProgramVersion", "Content creation program version", false, false, 0, 10, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ObjectCycle, "ObjectCycle", "Morning, evening, or both", false, false, 1, 1, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Byline, "Byline", "Name of content creator", false, true, 0, 32, Exiv2::string, IptcDataSets::application2, "Author"),
        DataSet(IptcDataSets::BylineTitle, "BylineTitle", "Title of content creator", false, true, 0, 32, Exiv2::string, IptcDataSets::application2, "Authors Position"),
        DataSet(IptcDataSets::City, "City", "City of content origin", false, false, 0, 32, Exiv2::string, IptcDataSets::application2, "City"),
        DataSet(IptcDataSets::SubLocation, "SubLocation", "Location within city", false, false, 0, 32, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ProvinceState, "ProvinceState", "Province/State of content origin", false, false, 0, 32, Exiv2::string, IptcDataSets::application2, "State/Province"),
        DataSet(IptcDataSets::CountryCode, "CountryCode", "ISO country code of content origin", false, false, 3, 3, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::CountryName, "CountryName", "Full country name of content origin", false, false, 0, 64, Exiv2::string, IptcDataSets::application2, "Country"),
        DataSet(IptcDataSets::TransmissionReference, "TransmissionReference", "Location of original transmission", false, false, 0, 32, Exiv2::string, IptcDataSets::application2, "Transmission Reference"),
        DataSet(IptcDataSets::Headline, "Headline", "Content synopsis", false, false, 0, 256, Exiv2::string, IptcDataSets::application2, "Headline"),
        DataSet(IptcDataSets::Credit, "Credit", "Content provider", false, false, 0, 32, Exiv2::string, IptcDataSets::application2, "Credit"),
        DataSet(IptcDataSets::Source, "Source", "Original owner of content", false, false, 0, 32, Exiv2::string, IptcDataSets::application2, "Source"),
        DataSet(IptcDataSets::Copyright, "Copyright", "Necessary copyright notice", false, false, 0, 128, Exiv2::string, IptcDataSets::application2, "Copyright notice"),
        DataSet(IptcDataSets::Contact, "Contact", "Person or organisation to contact", false, true, 0, 128, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Caption, "Caption", "Content description", false, false, 0, 2000, Exiv2::string, IptcDataSets::application2, "Description"),
        DataSet(IptcDataSets::Writer, "Writer", "Person responsible for caption", false, true, 0, 32, Exiv2::string, IptcDataSets::application2, "Description writer"),
        DataSet(IptcDataSets::RasterizedCaption, "RasterizedCaption", "Black and white caption image", false, false, 7360, 7360, Exiv2::undefined, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ImageType, "ImageType", "Color components in an image", false, false, 2, 2, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::ImageOrientation, "ImageOrientation", "Indicates the layout of an image", false, false, 1, 1, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Language, "Language", "ISO 639:1988 language code", false, false, 2, 3, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::AudioType, "AudioType", "Information about audio content", false, false, 2, 2, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::AudioRate, "AudioRate", "Sampling rate of audio content", false, false, 6, 6, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::AudioResolution, "AudioResolution", "Sampling resolution of audio content", false, false, 2, 2, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::AudioDuration, "AudioDuration", "Duration of audio content", false, false, 6, 6, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::AudioOutcue, "AudioOutcue", "Final words or sounds of audio content", false, false, 0, 64, Exiv2::string, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::PreviewFormat, "PreviewFormat", "IIM appendix A file format of preview", false, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::PreviewVersion, "PreviewVersion", "File format version of preview", false, false, 2, 2, Exiv2::unsignedShort, IptcDataSets::application2, ""),
        DataSet(IptcDataSets::Preview, "Preview", "Binary preview data", false, false, 0, 256000, Exiv2::undefined, IptcDataSets::application2, ""),
        DataSet(0xffff, "(Invalid)", "(Invalid)", false, false, 0, 0, Exiv2::unsignedShort, IptcDataSets::application2, "")
    };

    // Dataset lookup lists.This is an array with pointers to one list per IIM4 Record. 
    // The record id is used as the index into the array.
    const DataSet* IptcDataSets::records_[] = {
        0, 
        envelopeRecord, application2Record, 
        0
    };

    int IptcDataSets::dataSetIdx(uint16_t number, uint16_t recordId)
    {
        if( recordId != envelope && recordId != application2 ) return -1;
        const DataSet* dataSet = records_[recordId];
        if (dataSet == 0) return -1;
        int idx;
        for (idx = 0; dataSet[idx].number_ != number; ++idx) {
            if (dataSet[idx].number_ == 0xffff) return -1;
        }
        return idx;
    }

    int IptcDataSets::dataSetIdx(const std::string& dataSetName, uint16_t recordId)
    {
        if( recordId != envelope && recordId != application2 ) return -1;
        const DataSet* dataSet = records_[recordId];
        if (dataSet == 0) return -1;
        int idx;
        for (idx = 0; dataSet[idx].name_ != dataSetName; ++idx) {
            if (dataSet[idx].number_ == 0xffff) return -1;
        }
        return idx;
    }

    TypeId IptcDataSets::dataSetType(uint16_t number, uint16_t recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) throw Error("No dataSet for record Id");
        return records_[recordId][idx].type_;
    }

    std::string IptcDataSets::dataSetName(uint16_t number, uint16_t recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx != -1) return records_[recordId][idx].name_;

        std::ostringstream os;
        os << "0x" << std::setw(4) << std::setfill('0') << std::right
           << std::hex << number;
        return os.str();
    }

    const char* IptcDataSets::dataSetDesc(uint16_t number, uint16_t recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) throw Error("No dataSet for record Id");
        return records_[recordId][idx].desc_;
    }

    const char* IptcDataSets::dataSetPsName(uint16_t number, uint16_t recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) throw Error("No dataSet for record Id");
        return records_[recordId][idx].photoshop_;
    }

    bool IptcDataSets::dataSetRepeatable(uint16_t number, uint16_t recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) throw Error("No dataSet for record Id");
        return records_[recordId][idx].repeatable_;
    }

    uint16_t IptcDataSets::dataSet(const std::string& dataSetName, 
                                   uint16_t recordId)
    {
        uint16_t dataSet;
        int idx = dataSetIdx(dataSetName, recordId);
        if (idx != -1) {
            // dataSetIdx checks the range of recordId
            dataSet = records_[recordId][idx].number_;
        }
        else {
            // Todo: Check format of tagName
            std::istringstream is(dataSetName);
            is >> std::hex >> dataSet;
        }
        return dataSet;
    }

    std::string IptcDataSets::recordName(uint16_t recordId)
    {
        if (recordId == envelope || recordId == application2) {
            return recordInfo_[recordId].name_;            
        }

        std::ostringstream os;
        os << "0x" << std::setw(4) << std::setfill('0') << std::right
           << std::hex << recordId;
        return os.str();
    }

    const char* IptcDataSets::recordDesc(uint16_t recordId)
    {
        if( recordId != envelope && recordId != application2 ) {
            throw Error("Unknown record");
        }
        return recordInfo_[recordId].desc_;
    }

    uint16_t IptcDataSets::recordId(const std::string& recordName)
    {
        uint16_t i;
        for (i = application2; i > 0; --i) {
            if (recordInfo_[i].name_ == recordName) break;
        }
        if (i == 0) {
            // Todo: Check format of recordName
            std::istringstream is(recordName);
            is >> std::hex >> i;
        }
        return i;
    }

    void IptcDataSets::dataSetList(std::ostream& os)
    {
        const int count = sizeof(records_)/sizeof(records_[0]);
        for (int i=0; i < count; ++i) {
            const DataSet *record = records_[i];
            for (int j=0; record != 0 && record[j].number_ != 0xffff; ++j) {
                os << record[j] << "\n";
            }
        }
    } // IptcDataSets::dataSetList
    
    const char* IptcKey::familyName_ = "Iptc";

    IptcKey::IptcKey(const std::string& key)
        : key_(key)
    {
        decomposeKey();
    }

    IptcKey::IptcKey(uint16_t tag, uint16_t record)
        : tag_(tag), record_(record)
    {
        makeKey();
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

    IptcKey::AutoPtr IptcKey::clone() const
    {
        return AutoPtr(clone_());
    }

    IptcKey* IptcKey::clone_() const
    {
        return new IptcKey(*this);
    }

    void IptcKey::decomposeKey()
    {
        // Get the family name, record name and dataSet name parts of the key
        std::string::size_type pos1 = key_.find('.');
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string familyName = key_.substr(0, pos1);
        if (familyName != std::string(familyName_)) {
            throw Error("Invalid key");
        }
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key_.find('.', pos0);
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string recordName = key_.substr(pos0, pos1 - pos0);
        if (recordName == "") throw Error("Invalid key");
        std::string dataSetName = key_.substr(pos1 + 1);
        if (dataSetName == "") throw Error("Invalid key");

        // Use the parts of the key to find dataSet and recordId
        uint16_t recId = IptcDataSets::recordId(recordName);
        uint16_t dataSet = IptcDataSets::dataSet(dataSetName, recId);

        // Possibly translate hex name parts (0xabcd) to real names 
        recordName = IptcDataSets::recordName(recId);
        dataSetName = IptcDataSets::dataSetName(dataSet, recId);

        tag_ = dataSet;
        record_ = recId;
        key_ = familyName + "." + recordName + "." + dataSetName;
    } // IptcKey::decomposeKey

    void IptcKey::makeKey()
    {
        key_ = std::string(familyName_)
            + "." + IptcDataSets::recordName(record_)
            + "." + IptcDataSets::dataSetName(tag_, record_);
    }

    // *************************************************************************
    // free functions

    std::ostream& operator<<(std::ostream& os, const DataSet& dataSet) 
    {
        IptcKey iptcKey(dataSet.number_, dataSet.recordId_);
        return os << dataSet.name_ << ", "
                  << std::dec << dataSet.number_ << ", "
                  << "0x" << std::setw(4) << std::setfill('0') 
                  << std::right << std::hex << dataSet.number_ << ", "
                  << IptcDataSets::recordName(dataSet.recordId_) << ", "
                  << std::boolalpha << dataSet.mandatory_ << ", "
                  << dataSet.repeatable_ << ", "
                  << std::dec << dataSet.minbytes_ << ", "
                  << dataSet.maxbytes_ << ", "
                  << iptcKey.key() << ", "
                  << dataSet.desc_;
    }

}                                       // namespace Exiv2
