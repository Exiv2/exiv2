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
  Version:   $Name:  $ $Revision: 1.1 $
  Author(s): Brad Schick (brad) <schick@robotbattle.com>
  History:   24-Jul-04, brad: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.1 $ $RCSfile: datasets.cpp,v $")

// *****************************************************************************
// included header files
#include "datasets.hpp"
#include "error.hpp"
#include "types.hpp"
#include "value.hpp"

#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cstdlib>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    DataSet::DataSet(
        uint16 number, 
        const char* name,
        bool mandatory,
        bool repeatable,
        uint32 minbytes, 
        uint32 maxbytes,
        TypeId type,
        RecordId recordId,
        const char* photoshop
    )
        : number_(number), name_(name), mandatory_(mandatory), 
           repeatable_(repeatable), minbytes_(minbytes), maxbytes_(maxbytes),
           type_(type), recordId_(recordId), photoshop_(photoshop)
    {
    }

    RecordInfo::RecordInfo(
        RecordId recordId,
        const char* name,
        const char* desc
    )
        : recordId_(recordId), name_(name), desc_(desc)
    {
    }

    const RecordInfo IptcDataSets::recordInfo_[] = {
        RecordInfo(invalidRecord, "(invalid)", "(invalid)"),
        RecordInfo(envelope, "Envelope", "Envelope Record"),
        RecordInfo(application2, "Application2", "Application Record 2"),
        RecordInfo(lastRecord, "(invalid)", "(invalid)")
    };
    

    static const DataSet envelopeRecord[] = {
        DataSet(0, "Model Version", true, false, 2, 2, Exiv2::unsignedShort, envelope, ""),
        DataSet(5, "Destination", false, true, 0, 1024, Exiv2::string, envelope, ""),
        DataSet(20, "File Format", true, false, 2, 2, Exiv2::unsignedShort, envelope, ""),
        DataSet(22, "File Format Version", true, false, 2, 2, Exiv2::unsignedShort, envelope, ""),
        DataSet(30, "Service Identifier", true, false, 0, 10, Exiv2::string, envelope, ""),
        DataSet(40, "Envelope Number", true, false, 8, 8, Exiv2::string, envelope, ""),
        DataSet(50, "Product I.D.", false, true, 0, 32, Exiv2::string, envelope, ""),
        DataSet(60, "Envelope Priority", false, false, 1, 1, Exiv2::string, envelope, ""),
        DataSet(70, "Date Sent", true, false, 8, 8, Exiv2::date, envelope, ""),
        DataSet(80, "Time Sent", false, false, 11, 11, Exiv2::time, envelope, ""),
        DataSet(90, "Coded Character Set", false, false, 0, 32, Exiv2::undefined, envelope, ""),
        DataSet(100, "UNO", false, false, 14, 80, Exiv2::string, envelope, ""),
        DataSet(120, "ARM Identifier", false, false, 2, 2, Exiv2::unsignedShort, envelope, ""),
        DataSet(122, "ARM Version", false, false, 2, 2, Exiv2::unsignedShort, envelope, ""),
        DataSet(0xffff, "Invalid", false, false, 0, 0, Exiv2::unsignedShort, envelope, "")
    };

    static const DataSet application2Record[] = {
        DataSet(0, "Record Version", true, false, 2, 2, Exiv2::unsignedShort, application2, ""),
        DataSet(3, "Object Type Reference", false, false, 3, 67, Exiv2::string, application2, ""),
        DataSet(4, "Object Attribute Reference", false, true, 4, 68, Exiv2::string, application2, ""),
        DataSet(5, "Object Name", false, false, 0, 64, Exiv2::string, application2, "Document title"),
        DataSet(7, "Edit Status", false, false, 0, 64, Exiv2::string, application2, ""),
        DataSet(8, "Editorial Update", false, false, 2, 2, Exiv2::string, application2, ""),
        DataSet(10, "Urgency", false, false, 1, 1, Exiv2::string, application2, "Urgency"),
        DataSet(12, "Subject Reference", false, true, 13, 236, Exiv2::string, application2, ""),
        DataSet(15, "Category", false, false, 0, 3, Exiv2::string, application2, "Category"),
        DataSet(20, "Supplemental Category", false, true, 0, 32, Exiv2::string, application2, "Supplemental Categories"),
        DataSet(22, "Fixture Identifier", false, false, 0, 32, Exiv2::string, application2, ""),
        DataSet(25, "Keywords", false, true, 0, 64, Exiv2::string, application2, "Keywords"),
        DataSet(26, "Content Location Code", false, true, 3, 3, Exiv2::string, application2, ""),
        DataSet(27, "Content Location Name", false, true, 0, 64, Exiv2::string, application2, ""),
        DataSet(30, "Release Date", false, false, 8, 8, Exiv2::date, application2, ""),
        DataSet(35, "Release Time", false, false, 11, 11, Exiv2::time, application2, ""),
        DataSet(37, "Expiration Date", false, false, 8, 8, Exiv2::date, application2, ""),
        DataSet(38, "Expiration Time", false, false, 11, 11, Exiv2::time, application2, ""),
        DataSet(40, "Special Instructions", false, false, 0, 256, Exiv2::string, application2, "Instructions"),
        DataSet(42, "Action Advised", false, false, 2, 2, Exiv2::string, application2, ""),
        DataSet(45, "Reference Service", false, true, 0, 10, Exiv2::string, application2, ""),
        DataSet(47, "Reference Date", false, true, 8, 8, Exiv2::date, application2, ""),
        DataSet(50, "Reference Number", false, true, 8, 8, Exiv2::string, application2, ""),
        DataSet(55, "Date Created", false, false, 8, 8, Exiv2::date, application2, "Date created"),
        DataSet(60, "Time Created", false, false, 11, 11, Exiv2::time, application2, ""),
        DataSet(62, "Digital Creation Date", false, false, 8, 8, Exiv2::date, application2, ""),
        DataSet(63, "Digital Creation Time", false, false, 11, 11, Exiv2::time, application2, ""),
        DataSet(65, "Originating Program", false, false, 0, 32, Exiv2::string, application2, ""),
        DataSet(70, "Program Version", false, false, 0, 10, Exiv2::string, application2, ""),
        DataSet(75, "Object Cycle", false, false, 1, 1, Exiv2::string, application2, ""),
        DataSet(80, "By-line", false, true, 0, 32, Exiv2::string, application2, "Author"),
        DataSet(85, "By-line Title", false, true, 0, 32, Exiv2::string, application2, "Authors Position"),
        DataSet(90, "City", false, false, 0, 32, Exiv2::string, application2, "City"),
        DataSet(92, "Sub-location", false, false, 0, 32, Exiv2::string, application2, ""),
        DataSet(95, "Province/State", false, false, 0, 32, Exiv2::string, application2, "State/Province"),
        DataSet(100, "Country/Primary Location Code", false, false, 3, 3, Exiv2::string, application2, ""),
        DataSet(101, "Country/Primary Location Name", false, false, 0, 64, Exiv2::string, application2, "Country"),
        DataSet(103, "Original Transmission Reference", false, false, 0, 32, Exiv2::string, application2, "Transmission Reference"),
        DataSet(105, "Headline", false, false, 0, 256, Exiv2::string, application2, "Headline"),
        DataSet(110, "Credit", false, false, 0, 32, Exiv2::string, application2, "Credit"),
        DataSet(115, "Source", false, false, 0, 32, Exiv2::string, application2, "Source"),
        DataSet(116, "Copyright Notice", false, false, 0, 128, Exiv2::string, application2, "Copyright notice"),
        DataSet(118, "Contact", false, true, 0, 128, Exiv2::string, application2, ""),
        DataSet(120, "Caption/Abstract", false, false, 0, 2000, Exiv2::string, application2, "Description"),
        DataSet(122, "Writer/Editor", false, true, 0, 32, Exiv2::string, application2, "Description writer"),
        DataSet(125, "Rasterized Caption", false, false, 7360, 7360, Exiv2::undefined, application2, ""),
        DataSet(130, "Image Type", false, false, 2, 2, Exiv2::string, application2, ""),
        DataSet(131, "Image Orientation", false, false, 1, 1, Exiv2::string, application2, ""),
        DataSet(135, "Language Identifier", false, false, 2, 3, Exiv2::string, application2, ""),
        DataSet(150, "Audio Type", false, false, 2, 2, Exiv2::string, application2, ""),
        DataSet(151, "Audio Sampling Rate", false, false, 6, 6, Exiv2::string, application2, ""),
        DataSet(152, "Audio Sampling Resolution", false, false, 2, 2, Exiv2::string, application2, ""),
        DataSet(153, "Audio Duration", false, false, 6, 6, Exiv2::string, application2, ""),
        DataSet(154, "Audio Outcue", false, false, 0, 64, Exiv2::string, application2, ""),
        DataSet(200, "ObjectData Preview File Format", false, false, 2, 2, Exiv2::unsignedShort, application2, ""),
        DataSet(201, "ObjectData Preview File Format Version", false, false, 2, 2, Exiv2::unsignedShort, application2, ""),
        DataSet(202, "ObjectData Preview Data", false, false, 0, 256000, Exiv2::undefined, application2, ""),
        DataSet(0xffff, "Invalid", false, false, 0, 0, Exiv2::unsignedShort, application2, "")
    };

    // Dataset lookup lists.This is an array with pointers to one list per IIM4 Record. 
    // The RecordId is used as the index into the array.
    const DataSet* IptcDataSets::records_[] = {
        0, 
        envelopeRecord, application2Record, 
        0
    };

    int IptcDataSets::dataSetIdx(uint16 number, RecordId recordId)
    {
        assert( recordId > invalidRecord && recordId < lastRecord );
        const DataSet* dataSet = records_[recordId];
        if (dataSet == 0) return -1;
        int idx;
        for (idx = 0; dataSet[idx].number_ != number; ++idx) {
            if (dataSet[idx].number_ == 0xffff) return -1;
        }
        return idx;
    }

    int IptcDataSets::dataSetIdx(const std::string& dataSetName, RecordId recordId)
    {
        assert( recordId > invalidRecord && recordId < lastRecord );
        const DataSet* dataSet = records_[recordId];
        if (dataSet == 0) return -1;
        int idx;
        for (idx = 0; dataSet[idx].name_ != dataSetName; ++idx) {
            if (dataSet[idx].number_ == 0xffff) return -1;
        }
        return idx;
    }

    const char* IptcDataSets::dataSetName(uint16 number, RecordId recordId)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) throw Error("No dataSet for record Id");
        return records_[recordId][idx].name_;
    }

    const char* IptcDataSets::recordName(RecordId recordId)
    {
        assert( recordId > invalidRecord && recordId < lastRecord );
        return recordInfo_[recordId].name_;
    }

    const char* IptcDataSets::recordDesc(RecordId recordId)
    {
        assert( recordId > invalidRecord && recordId < lastRecord );
        return recordInfo_[recordId].desc_;
    }

    RecordId IptcDataSets::recordId(const std::string& recordName)
    {
        int i;
        for (i = int(lastRecord) - 1; i > 0; --i) {
            if (recordInfo_[i].name_ == recordName) break;
        }
        return RecordId(i);
    }

    std::string IptcDataSets::makeKey(const DataSet& dataSet)
    {
        return std::string(recordName(dataSet.recordId_)) 
            + "." + dataSet.name_;
    }

    std::string IptcDataSets::makeKey(uint16 number, RecordId recordId)
    {
        return std::string(recordName(recordId)) 
            + "." + std::string(dataSetName(number, recordId));
    }

    // This 'database lookup' function returns a match if it exists
    std::pair<uint16, RecordId> IptcDataSets::decomposeKey(const std::string& key)
    {
        // Get the record, record name and dataSet name parts of the key
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string recordName = key.substr(0, pos1);
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string dataSetName = key.substr(pos0, pos1 - pos0);

        // Use the parts of the key to find dataSet and recordInfo
        RecordId recId = recordId(recordName);
        if (recId == invalidRecord) return std::make_pair(0xffff, invalidRecord);

        int dataSet = dataSetIdx(dataSetName, recId);
        if (dataSet == -1 ) return std::make_pair(0xffff, invalidRecord);

        return std::make_pair(records_[recId][dataSet].number_, recId);
    } // IptcDataSets::decomposeKey

  /* Not sure if this will be needed since there really isn't
     any special formating for IPTC data.
    std::ostream& IptcDataSets::printDataSet(std::ostream& os,
                                     uint16 number, 
                                     RecordId recordId,
                                     const Value& value)
    {
        int idx = dataSetIdx(number, recordId);
        if (idx == -1) throw Error("No dataSet for record Id");
        return os << value;
    }*/

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
                  << dataSet.minbytes_ << ", "
                  << dataSet.maxbytes_ << ", "
                  << IptcDataSets::makeKey(dataSet) ;
    }

}                                       // namespace Exiv2
