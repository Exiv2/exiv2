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
  @version $Name:  $ $Revision: 1.6 $
  @author  Brad Schick (brad) <schick@robotbattle.com>
  @date    24-Jul-04, brad: created
 */
#ifndef DATASETS_HPP_
#define DATASETS_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "metadatum.hpp"

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
        RecordInfo(uint16_t recordId, const char* name, const char* desc);
        uint16_t recordId_;                     //!< Record id
        const char* name_;                      //!< Record name (one word)
        const char* desc_;                      //!< Record description
    };

    //! Dataset information
    struct DataSet {
        //! Constructor
        DataSet(
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
        );
        //@{
        uint16_t number_;
        const char* name_;
        const char* desc_;
        bool mandatory_;
        bool repeatable_;
        uint32_t minbytes_;
        uint32_t maxbytes_;
        TypeId type_;
        uint16_t recordId_;
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
        static const uint16_t invalidRecord = 0;
        static const uint16_t envelope = 1;
        static const uint16_t application2 = 2;
        //@}

        //! @name Dataset identifiers
        //@{
        static const uint16_t ModelVersion           = 0;
        static const uint16_t Destination            = 5;
        static const uint16_t FileFormat             = 20;
        static const uint16_t FileVersion            = 22;
        static const uint16_t ServiceId              = 30;
        static const uint16_t EnvelopeNumber         = 40;
        static const uint16_t ProductId              = 50;
        static const uint16_t EnvelopePriority       = 60;
        static const uint16_t DateSent               = 70;
        static const uint16_t TimeSent               = 80;
        static const uint16_t CharacterSet           = 90;
        static const uint16_t UNO                    = 100;
        static const uint16_t ARMId                  = 120;
        static const uint16_t ARMVersion             = 122;
        static const uint16_t RecordVersion          = 0;
        static const uint16_t ObjectType             = 3;
        static const uint16_t ObjectAttribute        = 4;
        static const uint16_t ObjectName             = 5;
        static const uint16_t EditStatus             = 7;
        static const uint16_t EditorialUpdate        = 8;
        static const uint16_t Urgency                = 10;
        static const uint16_t Subject                = 12;
        static const uint16_t Category               = 15;
        static const uint16_t SuppCategory           = 20;
        static const uint16_t FixtureId              = 22;
        static const uint16_t Keywords               = 25;
        static const uint16_t LocationCode           = 26;
        static const uint16_t LocationName           = 27;
        static const uint16_t ReleaseDate            = 30;
        static const uint16_t ReleaseTime            = 35;
        static const uint16_t ExpirationDate         = 37;
        static const uint16_t ExpirationTime         = 38;
        static const uint16_t SpecialInstructions    = 40;
        static const uint16_t ActionAdvised          = 42;
        static const uint16_t ReferenceService       = 45;
        static const uint16_t ReferenceDate          = 47;
        static const uint16_t ReferenceNumber        = 50;
        static const uint16_t DateCreated            = 55;
        static const uint16_t TimeCreated            = 60;
        static const uint16_t DigitizationDate       = 62;
        static const uint16_t DigitizationTime       = 63;
        static const uint16_t Program                = 65;
        static const uint16_t ProgramVersion         = 70;
        static const uint16_t ObjectCycle            = 75;
        static const uint16_t Byline                 = 80;
        static const uint16_t BylineTitle            = 85;
        static const uint16_t City                   = 90;
        static const uint16_t SubLocation            = 92;
        static const uint16_t ProvinceState          = 95;
        static const uint16_t CountryCode            = 100;
        static const uint16_t CountryName            = 101;
        static const uint16_t TransmissionReference  = 103;
        static const uint16_t Headline               = 105;
        static const uint16_t Credit                 = 110;
        static const uint16_t Source                 = 115;
        static const uint16_t Copyright              = 116;
        static const uint16_t Contact                = 118;
        static const uint16_t Caption                = 120;
        static const uint16_t Writer                 = 122;
        static const uint16_t RasterizedCaption      = 125;
        static const uint16_t ImageType              = 130;
        static const uint16_t ImageOrientation       = 131;
        static const uint16_t Language               = 135;
        static const uint16_t AudioType              = 150;
        static const uint16_t AudioRate              = 151;
        static const uint16_t AudioResolution        = 152;
        static const uint16_t AudioDuration          = 153;
        static const uint16_t AudioOutcue            = 154;
        static const uint16_t PreviewFormat          = 200;
        static const uint16_t PreviewVersion         = 201;
        static const uint16_t Preview                = 202;
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
          @return The name of the dataset or a string containing the hexadecimal
                  value of the dataset in the form "0x01ff", if this is an unknown 
                  dataset.
         */
        static std::string dataSetName(uint16_t number, uint16_t recordId);
        /*!
          @brief Return the description of the dataset.
          @param number The dataset number
          @param recordId The Iptc record Id 
          @return The description of the dataset
          @throw Error ("No dataset for recordId") if there is no dataset info
                 for the given record id in the lookup tables.
         */
        static const char* dataSetDesc(uint16_t number, uint16_t recordId);
        /*!
          @brief Return the photohsop name of a given dataset.
          @param number The dataset number
          @param recordId The Iptc record Id 
          @return The name used by photoshop for a dataset or an empty
                string if photoshop does not use the dataset.
          @throw Error ("No dataset for recordId") if there is no dataset info
                 for the given record id in the lookup tables.
         */
        static const char* dataSetPsName(uint16_t number, uint16_t recordId);
        /*!
          @brief Check if a given dataset is repeatable
          @param number The dataset number
          @param recordId The Iptc record Id 
          @return true if the given dataset is repeatable otherwise false
          @throw Error ("No dataset for recordId") if there is no dataset info
                 for the given record id in the lookup tables.
         */
        static bool dataSetRepeatable(uint16_t number, uint16_t recordId);
        //! Return the dataSet number for dataset name and record id
        static uint16_t dataSet(const std::string& dataSetName, uint16_t recordId);
        //! Return the type for dataSet number and Record id
        static TypeId dataSetType(uint16_t number, uint16_t recordId);
        /*!
          @brief Return the name of the Record
          @param recordId The record id
          @return The name of the record or a string containing the hexadecimal
                  value of the record in the form "0x01ff", if this is an
                  unknown record.
         */
        static std::string recordName(uint16_t recordId);
        /*!
           @brief Return the description of a record
           @param recordId Record Id number
           @return the description of the Record
           @throw Error("Unknown record");
         */
        static const char* recordDesc(uint16_t recordId);
        /*!
           @brief Return the Id number of a record
           @param recordName Name of a record type
           @return the Id number of a Record
           @throw Error("Unknown record");
         */
        static uint16_t recordId(const std::string& recordName);
        //! Print a list of all dataSets to output stream
        static void dataSetList(std::ostream& os);

    private:
        static int dataSetIdx(uint16_t number, uint16_t recordId);
        static int dataSetIdx(const std::string& dataSetName, uint16_t recordId);

        static const DataSet* records_[];
        static const RecordInfo recordInfo_[];

    }; // class IptcDataSets

    /*!
      @brief Concrete keys for Iptc metadata.
     */
    class IptcKey : public Key {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor to create an Iptc key from a key string. 

          @param key The key string.
          @throw Error ("Invalid key") if the first part of the key is not 
                 'Iptc' or the remaining parts of the key cannot be parsed and
                 converted to a record name and a dataset name.
        */
        explicit IptcKey(const std::string& key);
        /*!
          @brief Constructor to create an Iptc key from dataset and record ids.
          @param tag Dataset id
          @param record Record id
         */
        IptcKey(uint16_t tag, uint16_t record);
        //! Copy constructor
        IptcKey(const IptcKey& rhs);
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Assignment operator.
         */
        IptcKey& operator=(const IptcKey& rhs);
        //@}

        //! @name Accessors
        //@{
        virtual std::string key() const { return key_; }
        virtual const char* familyName() const { return familyName_; }
        /*!
          @brief Return the name of the group (the second part of the key).
                 For Iptc keys, the group name is the record name.
        */
        virtual std::string groupName() const { return recordName(); }
        virtual std::string tagName() const
            { return IptcDataSets::dataSetName(tag_, record_); }
        virtual uint16_t tag() const { return tag_; }
        virtual IptcKey* clone() const;

        //! Return the name of the record
        std::string recordName() const
            { return IptcDataSets::recordName(record_); }
        //! Return the record id
        uint16_t record() const { return record_; }
        //@}

    protected:
        //! @name Manipulators
        //@{
        /*!
          @brief Set the key corresponding to the dataset and record id. 
                 The key is of the form '<b>Iptc</b>.recordName.dataSetName'.
         */
        void makeKey();
        /*!
          @brief Parse and convert the key string into dataset and record id.
                 Updates data members if the string can be decomposed, or throws
                 Error ("Invalid key").

          @throw Error ("Invalid key") if the key cannot be decomposed.
         */
        void decomposeKey();
        //@}

    private:
        // DATA
        static const char* familyName_;

        uint16_t tag_;                 //!< Tag value
        uint16_t record_;              //!< Record value 
        std::string key_;              //!< Key

    }; // class IptcKey

// *****************************************************************************
// free functions

    //! Output operator for dataSet
    std::ostream& operator<<(std::ostream& os, const DataSet& dataSet);

}                                       // namespace Exiv2

#endif                                  // #ifndef DATASETS_HPP_
