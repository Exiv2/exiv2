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
  Version:   $Name:  $ $Revision: 1.3 $
  Author(s): Brad Schick (brad) <schick@robotbattle.com>
  History:   24-Jul-04, brad: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.3 $ $RCSfile: datasets.cpp,v $");

// *****************************************************************************
// included header files
#include "datasets.hpp"
#include "error.hpp"
#include "types.hpp"
#include "value.hpp"

#include <iostream>
#include <iomanip>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    DataSet::DataSet(
        uint16 number, 
        const char* name,
        const char* desc,
        bool mandatory,
        bool repeatable,
        uint32 minbytes, 
        uint32 maxbytes,
        TypeId type,
        uint16 recordId,
        const char* photoshop
    )
        : number_(number), name_(name), desc_(desc), mandatory_(mandatory), 
           repeatable_(repeatable), minbytes_(minbytes), maxbytes_(maxbytes),
           type_(type), recordId_(recordId), photoshop_(photoshop)
    {
    }

    RecordInfo::RecordInfo(
        uint16 recordId,
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

    int IptcDataSets::dataSetIdx(uint16 number, uint16 recordId)
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

    int IptcDataSets::dataSetIdx(const std::string& dataSetName, uint16 recordId)
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

    TypeId IptcDataSets::dataSetType(uint16 number, uint16 recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) throw Error("No dataSet for record Id");
        return records_[recordId][idx].type_;
    }

    const char* IptcDataSets::dataSetName(uint16 number, uint16 recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) throw Error("No dataSet for record Id");
        return records_[recordId][idx].name_;
    }

    const char* IptcDataSets::dataSetDesc(uint16 number, uint16 recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) throw Error("No dataSet for record Id");
        return records_[recordId][idx].desc_;
    }

    const char* IptcDataSets::dataSetPsName(uint16 number, uint16 recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) throw Error("No dataSet for record Id");
        return records_[recordId][idx].photoshop_;
    }

    bool IptcDataSets::dataSetRepeatable(uint16 number, uint16 recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) throw Error("No dataSet for record Id");
        return records_[recordId][idx].repeatable_;
    }

    const char* IptcDataSets::recordName(uint16 recordId)
    {
        if( recordId != envelope && recordId != application2 ) {
            throw Error("Unknown record");
        }
        return recordInfo_[recordId].name_;
    }

    const char* IptcDataSets::recordDesc(uint16 recordId)
    {
        if( recordId != envelope && recordId != application2 ) {
            throw Error("Unknown record");
        }
        return recordInfo_[recordId].desc_;
    }

    uint16 IptcDataSets::recordId(const std::string& recordName)
    {
        uint16 i;
        for (i = application2; i > 0; --i) {
            if (recordInfo_[i].name_ == recordName) break;
        }
        return i;
    }

    std::string IptcDataSets::makeKey(const DataSet& dataSet)
    {
        return "Iptc." + std::string(recordName(dataSet.recordId_)) 
            + "." + dataSet.name_;
    }

    std::string IptcDataSets::makeKey(uint16 number, uint16 recordId)
    {
        return "Iptc." + std::string(recordName(recordId)) 
            + "." + std::string(dataSetName(number, recordId));
    }

    // This 'database lookup' function returns a match if it exists
    std::pair<uint16, uint16> IptcDataSets::decomposeKey(const std::string& key)
    {
        // Get the type, record name and dataSet name parts of the key
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string type = key.substr(0, pos1);
        if (type != "Iptc") throw Error("Invalid key");
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string recordName = key.substr(pos0, pos1 - pos0);
        if (recordName == "") throw Error("Invalid key");
        std::string dataSetName = key.substr(pos1 + 1);
        if (dataSetName == "") throw Error("Invalid key");

        // Use the parts of the key to find dataSet and recordInfo
        uint16 recId = recordId(recordName);
        if (recId == invalidRecord) return std::make_pair((uint16)0xffff, invalidRecord);

        int idx = dataSetIdx(dataSetName, recId);
        if (idx == -1 ) return std::make_pair((uint16)0xffff, invalidRecord);

        return std::make_pair(records_[recId][idx].number_, recId);
    } // IptcDataSets::decomposeKey

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
    
    // *************************************************************************
    // free functions

    std::ostream& operator<<(std::ostream& os, const DataSet& dataSet) 
    {
        return os << dataSet.name_ << ", "
                  << std::dec << dataSet.number_ << ", "
                  << "0x" << std::setw(4) << std::setfill('0') 
                  << std::right << std::hex << dataSet.number_ << ", "
                  << IptcDataSets::recordName(dataSet.recordId_) << ", "
                  << std::boolalpha << dataSet.mandatory_ << ", "
                  << dataSet.repeatable_ << ", "
                  << std::dec << dataSet.minbytes_ << ", "
                  << dataSet.maxbytes_ << ", "
                  << IptcDataSets::makeKey(dataSet) << ", "
                  << dataSet.desc_;
    }

}                                       // namespace Exiv2
