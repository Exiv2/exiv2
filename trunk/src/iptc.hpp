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
  @file    iptc.hpp
  @brief   Encoding and decoding of IPTC data
  @version $Name:  $ $Revision: 1.2 $
  @author  Brad Schick (brad) 
           <a href="mailto:schick@robotbattle.com">schick@robotbattle.com</a>
  @date    31-Jul-04, brad: created
 */
#ifndef IPTC_HPP_
#define IPTC_HPP_

// *****************************************************************************
// included header files
#include "metadatum.hpp"
#include "types.hpp"
#include "error.hpp"
#include "value.hpp"
#include "datasets.hpp"

// + standard includes
#include <string>
#include <vector>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    /*!
      @brief Information related to one Iptc dataset.
     */
    class Iptcdatum : public Metadatum {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for new tags created by an application. The
                 Iptcdatum is created from a key / value pair. %Iptcdatum copies
                 (clones) the value if one is provided. Alternatively, a program
                 can create an 'empty' Iptcdatum with only a key and set the
                 value using setValue().

          @param key The key of the Iptcdatum.
          @param value Pointer to a Iptcdatum value.
          @throw Error ("Invalid key") if the key cannot be parsed and converted
                 to a tag number and record id.
         */
        explicit Iptcdatum(const std::string& key, 
                           const Value* value =0);
        //! Copy constructor
        Iptcdatum(const Iptcdatum& rhs);
        //! Destructor
        virtual ~Iptcdatum();
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator
        Iptcdatum& operator=(const Iptcdatum& rhs);
        /*!
          @brief Set the value. This method copies (clones) the value pointed
                 to by pValue.
         */
        void setValue(const Value* pValue);
        /*!
          @brief Set the value to the string buf. 
                 Uses Value::read(const std::string& buf). If the Iptcdatum does
                 not have a value yet, then an StringValue is created.
         */
        void setValue(const std::string& buf);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Write value to a data buffer and return the number
                 of bytes written.

          The user must ensure that the buffer has enough memory. Otherwise
          the call results in undefined behaviour.

          @param buf Data buffer to write to.
          @param byteOrder Applicable byte order (little or big endian).
          @return Number of characters written.
        */
        long copy(byte* buf, ByteOrder byteOrder) const 
            { return pValue_ == 0 ? 0 : pValue_->copy(buf, byteOrder); }
        /*!
          @brief Return the key of the Iptcdatum. The key is of the form
                 'Iptc.recordName.datasetName'. Note however that the key
                 is not necessarily unique, i.e., an IptcData may contain
                 multiple metadata with the same key.
         */
        std::string key() const { return key_; }
        /*!
           @brief Return the name of the record
           @return record name
           @throw Error("Unknown record");
         */
        std::string recordName() const;
        /*!
           @brief Return the record id 
           @return record id
         */
        uint16 record() const { return record_; }
        /*!
           @brief Return the name of the tag (aka dataset)
           @return tag name
           @throw Error("No dataSet for record Id") if tag is unknown
         */
        std::string tagName() const;
        //! Return the tag (aka dataset) number
        uint16 tag() const { return tag_; }
        //! Return the type id of the value
        TypeId typeId() const 
            { return pValue_ == 0 ? invalidTypeId : pValue_->typeId(); }
        //! Return the name of the type
        const char* typeName() const { return TypeInfo::typeName(typeId()); }
        //! Return the size in bytes of one component of this type
        long typeSize() const { return TypeInfo::typeSize(typeId()); }
        //! Return the number of components in the value
        long count() const { return pValue_ == 0 ? 0 : pValue_->count(); }
        //! Return the size of the value in bytes
        long size() const { return pValue_ == 0 ? 0 : pValue_->size(); }
        //! Return true if value was modified, otherwise false
        bool modified() const { return modified_; }
        //! Return the value as a string.
        std::string toString() const 
            { return pValue_ == 0 ? "" : pValue_->toString(); }
        /*!
          @brief Return the n-th component of the value converted to long. The
                 return value is -1 if the value of the Iptcdatum is not set and
                 the behaviour of the method is undefined if there is no n-th
                 component.
         */
        long toLong(long n =0) const 
            { return pValue_ == 0 ? -1 : pValue_->toLong(n); }
        /*!
          @brief Return the n-th component of the value converted to float.  The
                 return value is -1 if the value of the Iptcdatum is not set and
                 the behaviour of the method is undefined if there is no n-th
                 component.
         */
        float toFloat(long n =0) const 
            { return pValue_ == 0 ? -1 : pValue_->toFloat(n); }
        /*!
          @brief Return the n-th component of the value converted to
                 Rational. The return value is -1/1 if the value of the
                 Iptcdatum is not set and the behaviour of the method is
                 undefined if there is no n-th component.
         */
        Rational toRational(long n =0) const 
            { return pValue_ == 0 ? Rational(-1, 1) : pValue_->toRational(n); }
        /*!
          @brief Return a pointer to a copy (clone) of the value. The caller
                 is responsible to delete this copy when it's no longer needed.

          This method is provided for users who need full control over the 
          value. A caller may, e.g., downcast the pointer to the appropriate
          subclass of Value to make use of the interface of the subclass to set
          or modify its contents.
          
          @return A pointer to a copy (clone) of the value, 0 if the value is 
                  not set.
         */
        Value* getValue() const { return pValue_ == 0 ? 0 : pValue_->clone(); }
        /*!
          @brief Return a constant reference to the value. 

          This method is provided mostly for convenient and versatile output of
          the value which can (to some extent) be formatted through standard
          stream manipulators.  Do not attempt to write to the value through
          this reference. 

          <b>Example:</b> <br>
          @code
          IptcData::const_iterator i = iptcData.findKey(key);
          if (i != iptcData.end()) {
              std::cout << i->key() << " " << std::hex << i->value() << "\n";
          }
          @endcode

          @return A constant reference to the value.
          @throw Error ("Value not set") if the value is not set.
         */
        const Value& value() const 
            { if (pValue_) return *pValue_; throw Error("Value not set"); }
        //@}

        /*! 
          @brief Not really meant for public use, but public so IptcData doesn't
                 have to be made a friend
         */
        void clearModified() { modified_ = false; }

    private:
        // DATA
        uint16 tag_;                   //!< Tag value
        uint16 record_;                //!< Record value 
        Value* pValue_;                //!< Pointer to the value
        std::string key_;              //!< Key
        bool modified_;                //!< Change indicator

    }; // class Iptcdatum

    /*!
      @brief Output operator for Iptcdatum types, printing the interpreted
             tag value.
     */
    std::ostream& operator<<(std::ostream& os, const Iptcdatum& md);

    //! Container type to hold all metadata
    typedef std::vector<Iptcdatum> IptcMetadata;

    //! Unary predicate that matches an Iptcdatum with given record and dataset
    class FindMetadatumById {
    public:
        //! Constructor, initializes the object with the record and dataset id
        FindMetadatumById(uint16 dataset, uint16 record)
            : dataset_(dataset), record_(record) {}
        /*!
          @brief Returns true if the record and dataset id of the argument
                Iptcdatum is equal to that of the object.
        */
        bool operator()(const Iptcdatum& iptcdatum) const
            { return dataset_ == iptcdatum.tag() && record_ == iptcdatum.record(); }

    private:
        uint16 dataset_;
        uint16 record_;
    
    }; // class FindMetadatumById

    /*!
      @brief A container for Iptc data. This is a top-level class of 
             the %Exiv2 library.

      Provide high-level access to the Iptc data of an image:
      - read Iptc information from JPEG files
      - access metadata through keys and standard C++ iterators
      - add, modify and delete metadata 
      - write Iptc data to JPEG files
      - extract Iptc metadata to files, insert from these files
    */
    class IptcData {
        //! @name Not implemented
        //@{
        //! Copying not allowed
        IptcData(const IptcData& rhs);
        //! Assignment not allowed
        IptcData& operator=(const IptcData& rhs);
        //@}
    public:
        //! IptcMetadata iterator type
        typedef IptcMetadata::iterator iterator;
        //! IptcMetadata const iterator type
        typedef IptcMetadata::const_iterator const_iterator;

        //! @name Creators
        //@{
        //! Default constructor
        IptcData();
        //! Destructor
        ~IptcData();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Read the Iptc data from file path.
          @param path Path to the file
          @return  0 if successful;<BR>
                   3 if the file contains no Iptc data;<BR>
                  the return code of Image::readMetadata()
                    if the call to this function fails;<BR>
                  the return code of read(const char* buf, long len)
                    if the call to this function fails;<BR>
         */
        int read(const std::string& path);
        /*!
          @brief Read the Iptc data from a byte buffer. The format must follow
                 the IPTC IIM4 standard.
          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @return 0 if successful;<BR>
                 5 if Iptc data is invalid or corrupt;<BR>
         */
        int read(const byte* buf, long len);
        /*!
          @brief Write the Iptc data to file path. If an Iptc data section
                 already exists in the file, it is replaced.  If there is no
                 metadata to write, the Iptc data section is
                 deleted from the file.  Otherwise, an Iptc data section is
                 created.
          @return 0 if successful;<BR>
                -2 if the file contains an unknown image type;<BR>
                the return code of Image::writeMetadata()
                    if the call to this function fails;<BR>
         */
        int write(const std::string& path);
        /*!
          @brief Write the Iptc data to a binary file. By convention, the
                 filename extension should be ".exv". This file format contains
                 the Iptc data as it is found in a JPEG file. Exv files can
                 be read with int read(const std::string& path) just like
                 normal image files.
          @return 0 if successful;<BR>
                 the return code of Image::writeMetadata()
                    if the call to this function fails;<BR>
         */
        int writeIptcData(const std::string& path);
        /*!
          @brief Write the Iptc data to a data buffer, return number of bytes 
                 written. The copied data follows the IPTC IIM4 standard.
          @param buf The data buffer to write to.  The user must ensure that the
                 buffer has enough memory using the %size method. Otherwise
                 the call results in undefined behaviour.
          @return Number of characters written to the buffer.
         */
        long copy(byte* buf);
        /*!
          @brief Add a Iptcdatum from the supplied key and value pair. This
                 method copies (clones) the value. A check for non-repeatable
                 datasets is performed.
          @return 0 if successful;<BR>
                 6 if the dataset already exists and is not repeatable;<BR>
         */
        int add(const std::string& key, Value* value);
        /*! 
          @brief Add a copy of the Iptcdatum to the Iptc metadata. A check
                 for non-repeatable datasets is performed.
          @return 0 if successful;<BR>
                 6 if the dataset already exists and is not repeatable;<BR>
         */
        int add(const Iptcdatum& iptcdatum);
        /*!
          @brief Delete the Iptcdatum at iterator position pos, return the 
                 position of the next Iptcdatum. Note that iterators into
                 the metadata, including pos, are potentially invalidated 
                 by this call.
         */
        iterator erase(iterator pos);
        //! Sort metadata by key
        void sortByKey();
        //! Sort metadata by tag (aka dataset)
        void sortByTag();
        //! Begin of the metadata
        iterator begin() { return iptcMetadata_.begin(); }
        //! End of the metadata
        iterator end() { return iptcMetadata_.end(); }
        /*!
          @brief Find a Iptcdatum with the given key, return an iterator to it.
                 If multiple entries with the same key exist, it is undefined 
                 which of the matching metadata is found.
         */
        iterator findKey(const std::string& key);
        /*!
          @brief Find a Iptcdatum with the given record and dataset it, 
                return a const iterator to it. If multiple entries with the
                same Ids exists, it is undefined which of the matching
                metadata is found.
         */
        iterator findId(uint16 dataset, 
                        uint16 record = IptcDataSets::application2);
        //@}

        //! @name Accessors
        //@{
        /*!
          @brief Erase the Iptc data from file path. 
          @param path Path to the file.
          @return 0 if successful;<BR>
                -2 if the file contains an unknown image type;<BR>
                the return code of Image::writeMetadata()
                    if the call to this function fails;<BR>
         */
        int erase(const std::string& path) const;
        //! Return true if any metadata was modified, added, or erased.
        bool modified() const;
        //! Begin of the metadata
        const_iterator begin() const { return iptcMetadata_.begin(); }
        //! End of the metadata
        const_iterator end() const { return iptcMetadata_.end(); }
        /*!
          @brief Find a Iptcdatum with the given key, return a const iterator to
                 it.  If multiple metadata with the same key exist it is
                 undefined which of the matching metadata is found.
         */
        const_iterator findKey(const std::string& key) const;
        /*!
          @brief Find a Iptcdatum with the given record and dataset number, 
                return a const iterator to it.  If multiple metadata with the
                same Ids exist it is undefined which of the matching
                metadata is found.
         */
        const_iterator findId(uint16 dataset, 
                              uint16 record = IptcDataSets::application2) const;
        //! Get the number of metadata entries
        long count() const { return static_cast<long>(iptcMetadata_.size()); }
        /*!
          @brief Return the exact size of all contained Iptc metadata
         */
        long size() const;
        //@}

        /*!
          @brief Convert the return code from 
                 int read(const std::string& path),
                 int read(const byte* buf, long len),
                 int write(const std::string& path),
                 int writeIptcData(const std::string& path), 
                 int erase(const std::string& path) const 
                 into an error message.

                 Todo: Implement global handling of error messages
         */
        static std::string strError(int rc, const std::string& path);

    private:
        /*!
          @brief Read a single dataset payload and create a new metadata entry
          @param dataSet DataSet number
          @param record Record Id
          @param data Pointer to the first byte of dataset payload
          @param sizeData Length in bytes of dataset payload
          @return 0 if successful.
         */
        int readData(uint16 dataSet, uint16 record, 
                     const byte* data, uint32 sizeData);

        //! Resets modified flag
        void clearModified();
        
        //! @name Manipulators
        //@{
        /*!
          @brief Rebuilds the Iptc data buffer from scratch using the current
                metadata. After this method is called, pData_ points to the
                updated data.
         */
        void updateBuffer();
        //@}

        // Constant data
        static const byte marker_;          // Dataset marker
        
        // DATA
        IptcMetadata iptcMetadata_;
        long size_;              //!< Size of the Iptc raw data in bytes
        byte* pData_;            //!< Iptc raw data buffer
        mutable bool modified_;
    }; // class IptcData

// *****************************************************************************
// free functions

    /*!
      @brief Return a key for the entry.  The key is of the form
             'Iptc.recordName.datasetName'. 
      @throw Error ("No dataSet for record Id") if the dataset number or 
                record Id is unknown
    */
    std::string makeKey(uint16 number, uint16 record);
    /*!
      @brief Return the record and dataset id pair for the key.
      @return A pair consisting of the record and dataset id.
      @throw Error ("Invalid key") if the key cannot be parsed into
          valid record and dataset parts.
    */
    std::pair<uint16, uint16> decomposeKey(const std::string& key);

}                                       // namespace Exiv2

#endif                                  // #ifndef IPTC_HPP_

