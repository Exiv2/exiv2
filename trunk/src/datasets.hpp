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
/*!
  @file    datasets.hpp
  @brief   Iptc dataSet and type information
  @version $Name:  $ $Revision: 1.2 $
  @author  Brad Schick (brad) <schick@robotbattle.com>
  @date    24-Jul-04, brad: created
 */
#ifndef DATASETS_HPP_
#define DATASETS_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <string>
#include <utility>                              // for std::pair
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    //! Contains information about one record
    struct RecordInfo {
        //! Constructor
        RecordInfo(uint16 recordId, const char* name, const char* desc);
        uint16 recordId_;                     //!< Record id
        const char* name_;                      //!< Record name (one word)
        const char* desc_;                      //!< Record description
    };

    //! Dataset information
    struct DataSet {
        //! Constructor
        DataSet(
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
        );
        //@{
        uint16 number_;
        const char* name_;
        const char* desc_;
        bool mandatory_;
        bool repeatable_;
        uint32 minbytes_;
        uint32 maxbytes_;
        TypeId type_;
        uint16 recordId_;
        const char* photoshop_;
        //@}
    }; // struct DataSet

    //! Container for Iptc dataset information. Implemented as a static class.
    class IptcDataSets {
    public:
        /*!
          @name Record identifiers
          @brief Record identifiers to logically group dataSets. There are other
                 possible record types, but they are not standardized by the Iptc
                 IIM4 standard (and not commonly used in images).
         */
        //@{
        static const uint16 invalidRecord = 0;
        static const uint16 envelope = 1;
        static const uint16 application2 = 2;
        //@}

        //! @name Dataset identifiers
        //@{
        static const uint16 ModelVersion           = 0;
        static const uint16 Destination            = 5;
        static const uint16 FileFormat             = 20;
        static const uint16 FileVersion            = 22;
        static const uint16 ServiceId              = 30;
        static const uint16 EnvelopeNumber         = 40;
        static const uint16 ProductId              = 50;
        static const uint16 EnvelopePriority       = 60;
        static const uint16 DateSent               = 70;
        static const uint16 TimeSent               = 80;
        static const uint16 CharacterSet           = 90;
        static const uint16 UNO                    = 100;
        static const uint16 ARMId                  = 120;
        static const uint16 ARMVersion             = 122;
        static const uint16 RecordVersion          = 0;
        static const uint16 ObjectType             = 3;
        static const uint16 ObjectAttribute        = 4;
        static const uint16 ObjectName             = 5;
        static const uint16 EditStatus             = 7;
        static const uint16 EditorialUpdate        = 8;
        static const uint16 Urgency                = 10;
        static const uint16 Subject                = 12;
        static const uint16 Category               = 15;
        static const uint16 SuppCategory           = 20;
        static const uint16 FixtureId              = 22;
        static const uint16 Keywords               = 25;
        static const uint16 LocationCode           = 26;
        static const uint16 LocationName           = 27;
        static const uint16 ReleaseDate            = 30;
        static const uint16 ReleaseTime            = 35;
        static const uint16 ExpirationDate         = 37;
        static const uint16 ExpirationTime         = 38;
        static const uint16 SpecialInstructions    = 40;
        static const uint16 ActionAdvised          = 42;
        static const uint16 ReferenceService       = 45;
        static const uint16 ReferenceDate          = 47;
        static const uint16 ReferenceNumber        = 50;
        static const uint16 DateCreated            = 55;
        static const uint16 TimeCreated            = 60;
        static const uint16 DigitizationDate       = 62;
        static const uint16 DigitizationTime       = 63;
        static const uint16 Program                = 65;
        static const uint16 ProgramVersion         = 70;
        static const uint16 ObjectCycle            = 75;
        static const uint16 Byline                 = 80;
        static const uint16 BylineTitle            = 85;
        static const uint16 City                   = 90;
        static const uint16 SubLocation            = 92;
        static const uint16 ProvinceState          = 95;
        static const uint16 CountryCode            = 100;
        static const uint16 CountryName            = 101;
        static const uint16 TransmissionReference  = 103;
        static const uint16 Headline               = 105;
        static const uint16 Credit                 = 110;
        static const uint16 Source                 = 115;
        static const uint16 Copyright              = 116;
        static const uint16 Contact                = 118;
        static const uint16 Caption                = 120;
        static const uint16 Writer                 = 122;
        static const uint16 RasterizedCaption      = 125;
        static const uint16 ImageType              = 130;
        static const uint16 ImageOrientation       = 131;
        static const uint16 Language               = 135;
        static const uint16 AudioType              = 150;
        static const uint16 AudioRate              = 151;
        static const uint16 AudioResolution        = 152;
        static const uint16 AudioDuration          = 153;
        static const uint16 AudioOutcue            = 154;
        static const uint16 PreviewFormat          = 200;
        static const uint16 PreviewVersion         = 201;
        static const uint16 Preview                = 202;
        //@}
        
    private:
        //! Prevent construction: not implemented.
        IptcDataSets() {}
        //! Prevent copy-construction: not implemented.
        IptcDataSets(const IptcDataSets& rhs);
        //! Prevent assignment: not implemented.
        IptcDataSets& operator=(const IptcDataSets& rhs);

    public:
        /*!
          @brief Return the name of the dataset.
          @param number The dataset number
          @param recordId The Iptc record Id 
          @return The name of the dataset
          @throw Error ("No dataset for recordId") if there is no dataset info
                 for the given record id in the lookup tables.
         */
        static const char* dataSetName(uint16 number, uint16 recordId);
        /*!
          @brief Return the description of the dataset.
          @param number The dataset number
          @param recordId The Iptc record Id 
          @return The description of the dataset
          @throw Error ("No dataset for recordId") if there is no dataset info
                 for the given record id in the lookup tables.
         */
        static const char* dataSetDesc(uint16 number, uint16 recordId);
        /*!
          @brief Return the photohsop name of a given dataset.
          @param number The dataset number
          @param recordId The Iptc record Id 
          @return The name used by photoshop for a dataset or an empty
                string if photoshop does not use the dataset.
          @throw Error ("No dataset for recordId") if there is no dataset info
                 for the given record id in the lookup tables.
         */
        static const char* dataSetPsName(uint16 number, uint16 recordId);
        /*!
          @brief Check if a given dataset is repeatable
          @param number The dataset number
          @param recordId The Iptc record Id 
          @return true if the given dataset is repeatable otherwise false
          @throw Error ("No dataset for recordId") if there is no dataset info
                 for the given record id in the lookup tables.
         */
        static bool dataSetRepeatable(uint16 number, uint16 recordId);
        //! Return the dataSet number for dataset name and record id
        static uint16 dataSet(const std::string& dataSetName, uint16 recordId);
        //! Return the type for dataSet number and Record id
        static TypeId dataSetType(uint16 number, uint16 recordId);
        //! Return the name of the Record
        static const char* recordName(uint16 recordId);
        /*!
           @brief Return the description of a record
           @param recordId Record Id number
           @return the description of the Record
           @throw Error("Unknown record");
         */
        static const char* recordDesc(uint16 recordId);
        /*!
           @brief Return the Id number of a record
           @param recordName Name of a record type
           @return the Id number of a Record
           @throw Error("Unknown record");
         */
        static uint16 recordId(const std::string& recordName);
        /*!
          @brief Return the key for the dataSet number and record id. The key is
                 of the form 'recordName.dataSetName'.
         */
        static std::string makeKey(uint16 number, uint16 recordId);
        /*!
          @brief Return the key for the dataSet. The key is of the form
                 'recordName.dataSetName'.
         */
        static std::string makeKey(const DataSet& dataSet);
        /*!
          @brief Return dataSet and record id pair for the key.
          @return A pair consisting of the dataSet number and record id.
          @throw Error ("Invalid key") if the key cannot be parsed into
                 record name and dataSet name parts.
         */
        static std::pair<uint16, uint16> decomposeKey(const std::string& key);
        //! Print a list of all dataSets to output stream
        static void dataSetList(std::ostream& os);

    private:
        static int dataSetIdx(uint16 number, uint16 recordId);
        static int dataSetIdx(const std::string& dataSetName, uint16 recordId);

        static const DataSet* records_[];
        static const RecordInfo recordInfo_[];

    }; // class IptcDataSets

// *****************************************************************************
// free functions

    //! Output operator for dataSet
    std::ostream& operator<<(std::ostream& os, const DataSet& dataSet);

}                                       // namespace Exiv2

#endif                                  // #ifndef DATASETS_HPP_
