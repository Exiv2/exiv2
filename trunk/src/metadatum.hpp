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
  @file    metadatum.hpp
  @brief   Provides class Metadatum
  @version $Name:  $ $Revision: 1.1 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @author  Brad Schick (brad) 
           <a href="mailto:schick@robotbattle.com">schick@robotbattle.com</a>
  @date    09-Jan-04, ahu: created<BR>
           31-Jul-04, brad: isolated as a component
 */
#ifndef METADATUM_HPP_
#define METADATUM_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "value.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    /*!
      @brief Information related to one Exif tag.
     */
    class Metadatum {
    public:
        //! @name Manipulators
        //@{
        /*!
          @brief Set the value. This method copies (clones) the value pointed
                 to by pValue.
         */
        virtual void setValue(const Value* pValue) =0;
        /*!
          @brief Set the value to the string buf. 
                 Uses Value::read(const std::string& buf). If the metadatum does
                 not have a value yet, then an AsciiValue is created.
         */
        virtual void setValue(const std::string& buf) =0;
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
        virtual long copy(byte* buf, ByteOrder byteOrder) const =0;
        /*!
          @brief Return the key of the metadatum. The key is of the form
                 'ifdItem.sectionName.tagName'. Note however that the key
                 is not necessarily unique, i.e., an ExifData may contain
                 multiple metadata with the same key.
         */
        virtual std::string key() const =0;
        //! Return the name of the tag (which is also the third part of the key)
        virtual std::string tagName() const =0;
        //! Return the tag
        virtual uint16 tag() const =0;
        //! Return the type id of the value
        virtual TypeId typeId() const =0;
        //! Return the name of the type
        virtual const char* typeName() const =0;
        //! Return the size in bytes of one component of this type
        virtual long typeSize() const =0;
        //! Return the number of components in the value
        virtual long count() const =0;
        //! Return the size of the value in bytes
        virtual long size() const =0;
        //! Return the value as a string.
        virtual std::string toString() const =0;
        /*!
          @brief Return the n-th component of the value converted to long. The
                 return value is -1 if the value of the Metadatum is not set and
                 the behaviour of the method is undefined if there is no n-th
                 component.
         */
        virtual long toLong(long n =0) const =0;
        /*!
          @brief Return the n-th component of the value converted to float.  The
                 return value is -1 if the value of the Metadatum is not set and
                 the behaviour of the method is undefined if there is no n-th
                 component.
         */
        virtual float toFloat(long n =0) const =0;
        /*!
          @brief Return the n-th component of the value converted to
                 Rational. The return value is -1/1 if the value of the
                 Metadatum is not set and the behaviour of the method is
                 undefined if there is no n-th component.
         */
        virtual Rational toRational(long n =0) const =0;
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
        virtual Value* getValue() const =0;
        /*!
          @brief Return a constant reference to the value. 

          This method is provided mostly for convenient and versatile output of
          the value which can (to some extent) be formatted through standard
          stream manipulators.  Do not attempt to write to the value through
          this reference. 

          <b>Example:</b> <br>
          @code
          ExifData::const_iterator i = exifData.findKey(key);
          if (i != exifData.end()) {
              std::cout << i->key() << " " << std::hex << i->value() << "\n";
          }
          @endcode

          @return A constant reference to the value.
          @throw Error ("Value not set") if the value is not set.
         */
        virtual const Value& value() const =0;
        //@}

    protected:
        //! @name Creators
        //@{
        //! Default Constructor
        Metadatum() {}
        //! Copy constructor
        Metadatum(const Metadatum& rhs) {}
        //! Destructor
        virtual ~Metadatum() {}
        //@}

        //! @name Manipulators
        //@{
        //! Assignment operator
        Metadatum& operator=(const Metadatum& rhs) { return *this; }
        //@}        

    }; // class Metadatum

    //! Unary predicate that matches a Exifdatum with a given key
    class FindMetadatumByKey {
    public:
        //! Constructor, initializes the object with the tag to look for
        FindMetadatumByKey(const std::string& key) : key_(key) {}
        /*!
          @brief Returns true if the key of the argument metadatum is equal
          to that of the object.
        */
        bool operator()(const Metadatum& metadatum) const
            { return key_ == metadatum.key(); }

    private:
        std::string key_;
        
    }; // class FindMetadatumByTag


    /*!
      @brief Output operator for Metadatum types, printing the interpreted
             tag value.
     */
    std::ostream& operator<<(std::ostream& os, const Metadatum& md);
    /*!
      @brief Compare two metadata by tag. Return true if the tag of metadatum
             lhs is less than that of rhs.
     */
    bool cmpMetadataByTag(const Metadatum& lhs, const Metadatum& rhs);
    /*!
      @brief Compare two metadata by key. Return true if the key of metadatum
             lhs is less than that of rhs.
     */
    bool cmpMetadataByKey(const Metadatum& lhs, const Metadatum& rhs);

}                                       // namespace Exiv2

#endif                                  // #ifndef METADATUM_HPP_
