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
  @version $Name:  $ $Revision: 1.1 $
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
// class declarations
    class Value;

// *****************************************************************************
// type definitions

    /*!
      @brief record identifiers to logically group dataSets. A record consists
             of nothing more than a name, based on the Iptc standard. There are
             other record types, but they are not standardized by the Iptc IIM4
             standard (and not commonly used).
     */
    enum RecordId { invalidRecord, envelope, application2, lastRecord };

// *****************************************************************************
// class definitions

    //! Contains information about one record
    struct RecordInfo {
        //! Constructor
        RecordInfo(RecordId recordId, const char* name, const char* desc);
        RecordId recordId_;                     //!< Record id
        const char* name_;                      //!< Record name (one word)
        const char* desc_;                      //!< Record description
    };

    //! Dataset information
    struct DataSet {
        //! Constructor
        DataSet(
            uint16 number, 
            const char* name,
            bool mandatory,
            bool repeatable,
            uint32 minbytes, 
            uint32 maxbytes,
            TypeId type,
            RecordId recordId,
            const char* photoshop
        );
        uint16 number_;                         
        const char* name_;
        bool mandatory_;                         
        bool repeatable_;                       
        uint32 minbytes_;                       
        uint32 maxbytes_;                         
        TypeId type_;
        RecordId recordId_;
        const char* photoshop_;
    }; // struct DataSet

    //! Container for Iptc dataset information. Implemented as a static class.
    class IptcDataSets {
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
        static const char* dataSetName(uint16 number, RecordId recordId);
        //! Return the dataSet for one combination of Record id and dataSet
        static uint16 dataSet(const std::string& dataSetName, RecordId recordId);
        //! Return the name of the Record
        static const char* recordName(RecordId recordId);
        //! Return the description of the Record
        static const char* recordDesc(RecordId recordId);
        //! Return the record id for a record name
        static RecordId recordId(const std::string& recordName);
        /*!
          @brief Return the key for the dataSet number and record id. The key is
                 of the form 'recordName.dataSetName'.
         */
        static std::string makeKey(uint16 number, RecordId recordId);
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
        static std::pair<uint16, RecordId> decomposeKey(const std::string& key);
        //! Interpret and print the value of an Iptc dataSet
/*        static std::ostream& printDataSet(std::ostream& os,
                                      uint16 number, 
                                      RecordId recordId,
                                      const Value& value);*/
        //! Print a list of all dataSets to output stream
        static void dataSetList(std::ostream& os);

    private:
        static int dataSetIdx(uint16 number, RecordId recordId);
        static int dataSetIdx(const std::string& dataSetName, RecordId recordId);

        static const DataSet* records_[];
        static const RecordInfo recordInfo_[];

    }; // class IptcDataSets

// *****************************************************************************
// free functions

    //! Output operator for dataSet
    std::ostream& operator<<(std::ostream& os, const DataSet& dataSet);

}                                       // namespace Exiv2

#endif                                  // #ifndef DATASETS_HPP_
