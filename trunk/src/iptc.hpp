// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
  @brief   Encoding and decoding of Iptc data
  @version $Rev$
  @author  Brad Schick (brad) 
           <a href="mailto:brad@robotbattle.com">brad@robotbattle.com</a>
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
      @brief Information related to one Iptc dataset. An Iptc metadatum consists 
             of an IptcKey and a Value and provides methods to manipulate these.
     */
    class Iptcdatum : public Metadatum {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for new tags created by an application. The
                 %Iptcdatum is created from a key / value pair. %Iptcdatum
                 copies (clones) the value if one is provided. Alternatively, a
                 program can create an 'empty' %Iptcdatum with only a key and
                 set the value using setValue().

          @param key The key of the %Iptcdatum.
          @param pValue Pointer to a %Iptcdatum value.
          @throw Error if the key cannot be parsed and converted
                 to a tag number and record id.
         */
        explicit Iptcdatum(const IptcKey& key, 
                           const Value* pValue =0);
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
          @brief Assign \em value to the %Iptcdatum. The type of the new Value
                 is set to UShortValue.
         */
        Iptcdatum& operator=(const uint16_t& value);
        /*!
          @brief Assign \em value to the %Iptcdatum. 
                 Calls setValue(const std::string&).
         */
        Iptcdatum& operator=(const std::string& value);
        /*!
          @brief Assign \em value to the %Iptcdatum.
                 Calls setValue(const Value*).
         */
        Iptcdatum& operator=(const Value& value);
        /*!
          @brief Set the Value. This method copies (clones) the %Value pointed
                 to by \em pValue.
         */
        void setValue(const Value* pValue);
        /*!
          @brief Set the value to the string \em value, using 
                 Value::read(const std::string&). 
                 If the %Iptcdatum does not have a Value yet, then a %Value of
                 the correct type for this %Iptcdatum is created. If that
                 fails (because of an unknown dataset), a StringValue is 
                 created.
         */
        void setValue(const std::string& value);
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
            { return value_.get() == 0 ? 0 : value_->copy(buf, byteOrder); }
        /*!
          @brief Return the key of the Iptcdatum. The key is of the form
                 '<b>Iptc</b>.recordName.datasetName'. Note however that the key
                 is not necessarily unique, i.e., an IptcData may contain
                 multiple metadata with the same key.
         */
        std::string key() const { return key_.get() == 0 ? "" : key_->key(); }
        /*!
           @brief Return the name of the record
           @return record name
         */
        std::string recordName() const
            { return key_.get() == 0 ? "" : key_->recordName(); }
        /*!
           @brief Return the record id 
           @return record id
         */
        uint16_t record() const 
            { return key_.get() == 0 ? 0 : key_->record(); }
        /*!
           @brief Return the name of the tag (aka dataset)
           @return tag name
         */
        std::string tagName() const
            { return key_.get() == 0 ? "" : key_->tagName(); }
        //! Return the tag (aka dataset) number
        uint16_t tag() const
            { return key_.get() == 0 ? 0 : key_->tag(); }
        //! Return the type id of the value
        TypeId typeId() const 
            { return value_.get() == 0 ? invalidTypeId : value_->typeId(); }
        //! Return the name of the type
        const char* typeName() const { return TypeInfo::typeName(typeId()); }
        //! Return the size in bytes of one component of this type
        long typeSize() const { return TypeInfo::typeSize(typeId()); }
        //! Return the number of components in the value
        long count() const { return value_.get() == 0 ? 0 : value_->count(); }
        //! Return the size of the value in bytes
        long size() const { return value_.get() == 0 ? 0 : value_->size(); }
        //! Return the value as a string.
        std::string toString() const 
            { return value_.get() == 0 ? "" : value_->toString(); }
        /*!
          @brief Return the n-th component of the value converted to long. The
                 return value is -1 if the value of the Iptcdatum is not set and
                 the behaviour of the method is undefined if there is no n-th
                 component.
         */
        long toLong(long n =0) const 
            { return value_.get() == 0 ? -1 : value_->toLong(n); }
        /*!
          @brief Return the n-th component of the value converted to float.  The
                 return value is -1 if the value of the Iptcdatum is not set and
                 the behaviour of the method is undefined if there is no n-th
                 component.
         */
        float toFloat(long n =0) const 
            { return value_.get() == 0 ? -1 : value_->toFloat(n); }
        /*!
          @brief Return the n-th component of the value converted to
                 Rational. The return value is -1/1 if the value of the
                 Iptcdatum is not set and the behaviour of the method is
                 undefined if there is no n-th component.
         */
        Rational toRational(long n =0) const 
            { return value_.get() == 0 ? Rational(-1, 1) : value_->toRational(n); }
        /*!
          @brief Return an auto-pointer to a copy (clone) of the value. The
                 caller owns this copy and the auto-pointer ensures that it will
                 be deleted.

          This method is provided for users who need full control over the 
          value. A caller may, e.g., downcast the pointer to the appropriate
          subclass of Value to make use of the interface of the subclass to set
          or modify its contents.
          
          @return An auto-pointer to a copy (clone) of the value, 0 if the value
                  is not set.
         */
        Value::AutoPtr getValue() const 
            { return value_.get() == 0 ? Value::AutoPtr(0) : value_->clone(); }
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
          @throw Error If the value is not set.
         */
        const Value& value() const;
        //@}

    private:
        // DATA
        IptcKey::AutoPtr key_;                  //!< Key
        Value::AutoPtr   value_;                //!< Value

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
        FindMetadatumById(uint16_t dataset, uint16_t record)
            : dataset_(dataset), record_(record) {}
        /*!
          @brief Returns true if the record and dataset id of the argument
                Iptcdatum is equal to that of the object.
        */
        bool operator()(const Iptcdatum& iptcdatum) const
            { return dataset_ == iptcdatum.tag() && record_ == iptcdatum.record(); }

    private:
        uint16_t dataset_;
        uint16_t record_;
    
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
    public:
        //! IptcMetadata iterator type
        typedef IptcMetadata::iterator iterator;
        //! IptcMetadata const iterator type
        typedef IptcMetadata::const_iterator const_iterator;

        // Use the compiler generated constructors and assignment operator

        //! @name Manipulators
        //@{
        /*!
          @brief Load the Iptc data from a byte buffer. The format must follow
                 the IPTC IIM4 standard.
          @param buf Pointer to the data buffer to read from
          @param len Number of bytes in the data buffer 
          @return 0 if successful;<BR>
                 5 if Iptc data is invalid or corrupt;<BR>
         */
        int load(const byte* buf, long len);
        /*!
          @brief Write the Iptc data to a data buffer and return the data buffer.
                 Caller owns this buffer. The copied data follows the IPTC IIM4
                 standard.
          @return Data buffer containing the Iptc data.
         */
        DataBuf copy();
        /*!
          @brief Returns a reference to the %Iptcdatum that is associated with a
                 particular \em key. If %IptcData does not already contain such
                 an %Iptcdatum, operator[] adds object \em Iptcdatum(key).

          @note  Since operator[] might insert a new element, it can't be a const
                 member function.
         */
        Iptcdatum& operator[](const std::string& key);
        /*!
          @brief Add an %Iptcdatum from the supplied key and value pair. This
                 method copies (clones) the value. A check for non-repeatable
                 datasets is performed.
          @return 0 if successful;<BR>
                  6 if the dataset already exists and is not repeatable
         */
        int add(const IptcKey& key, Value* value);
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
        /*!
          @brief Delete all Iptcdatum instances resulting in an empty container.
         */
        void clear() { iptcMetadata_.clear(); }
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
        iterator findKey(const IptcKey& key);
        /*!
          @brief Find a Iptcdatum with the given record and dataset it, 
                return a const iterator to it. If multiple entries with the
                same Ids exists, it is undefined which of the matching
                metadata is found.
         */
        iterator findId(uint16_t dataset, 
                        uint16_t record = IptcDataSets::application2);
        //@}

        //! @name Accessors
        //@{
        //! Begin of the metadata
        const_iterator begin() const { return iptcMetadata_.begin(); }
        //! End of the metadata
        const_iterator end() const { return iptcMetadata_.end(); }
        /*!
          @brief Find an Iptcdatum with the given key, return a const iterator
                 to it.  If multiple metadata with the same key exist it is
                 undefined which of the matching metadata is found.
         */
        const_iterator findKey(const IptcKey& key) const;
        /*!
          @brief Find a Iptcdatum with the given record and dataset number, 
                return a const iterator to it.  If multiple metadata with the
                same Ids exist it is undefined which of the matching
                metadata is found.
         */
        const_iterator findId(uint16_t dataset, 
                              uint16_t record = IptcDataSets::application2) const;
        //! Return true if there is no Iptc metadata
        bool empty() const { return count() == 0; }
        //! Get the number of metadata entries
        long count() const { return static_cast<long>(iptcMetadata_.size()); }
        /*!
          @brief Return the exact size of all contained Iptc metadata
         */
        long size() const;
        //@}

    private:
        /*!
          @brief Read a single dataset payload and create a new metadata entry
          @param dataSet DataSet number
          @param record Record Id
          @param data Pointer to the first byte of dataset payload
          @param sizeData Length in bytes of dataset payload
          @return 0 if successful.
         */
        int readData(uint16_t dataSet, uint16_t record, 
                     const byte* data, uint32_t sizeData);

        // Constant data
        static const byte marker_;          // Dataset marker
        
        // DATA
        IptcMetadata iptcMetadata_;
    }; // class IptcData

}                                       // namespace Exiv2

#endif                                  // #ifndef IPTC_HPP_
