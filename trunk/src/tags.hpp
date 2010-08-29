// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2010 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @file    tags.hpp
  @brief   Exif tag and type information
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Jan-04, ahu: created<BR>
           11-Feb-04, ahu: isolated as a component
 */
#ifndef TAGS_HPP_
#define TAGS_HPP_

// *****************************************************************************
// included header files
#include "metadatum.hpp"
#include "types.hpp"

// + standard includes
#include <string>
#include <iosfwd>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;
    class Value;
    struct TagInfo;

// *****************************************************************************
// type definitions

    //! Type for a function pointer for functions interpreting the tag value
    typedef std::ostream& (*PrintFct)(std::ostream&, const Value&, const ExifData* pExifData);
    //! A function returning a tag list.
    typedef const TagInfo* (*TagListFct)();

// *****************************************************************************
// class definitions

    //! The details of an IFD.
    struct EXIV2API GroupInfo {
        struct Item;
        bool operator==(int ifdId) const;       //!< Comparison operator for IFD id
        bool operator==(const Item& item) const;       //!< Comparison operator for IFD item
        int ifdId_;                             //!< IFD id
        const char* name_;                      //!< IFD name
        const char* item_; //!< Related IFD item. This is also an IFD name, unique for each IFD.
        TagListFct tagList_;                    //!< Tag list
    };

    //! Search key to find a GroupInfo by its IFD item.
    struct EXIV2API GroupInfo::Item {
        Item(const std::string& item);          //!< Constructor
        std::string i_;                         //!< IFD item
    };

    //! Tag information
    struct EXIV2API TagInfo {
        //! Constructor
        TagInfo(
            uint16_t tag,
            const char* name,
            const char* title,
            const char* desc,
            int ifdId,
            int sectionId,
            TypeId typeId,
            int16_t count,
            PrintFct printFct
        );
        uint16_t tag_;                          //!< Tag
        const char* name_;                      //!< One word tag label
        const char* title_;                     //!< Tag title
        const char* desc_;                      //!< Short tag description
        int ifdId_;                             //!< Link to the (preferred) IFD
        int sectionId_;                         //!< Section id
        TypeId typeId_;                         //!< Type id
        int16_t count_;                         //!< The number of values (not bytes!), 0=any, -1=count not known.
        PrintFct printFct_;                     //!< Pointer to tag print function
    }; // struct TagInfo

    //! Access to Exif group and tag lists and misc. tag reference methods, implemented as a static class.
    class EXIV2API ExifTags {
        //! Prevent construction: not implemented.
        ExifTags();
        //! Prevent copy-construction: not implemented.
        ExifTags(const ExifTags& rhs);
        //! Prevent assignment: not implemented.
        ExifTags& operator=(const ExifTags& rhs);

    public:
        //! Return read-only list of build-in groups
        static const GroupInfo* groupList();
        //! Return read-only list of built-in \em group tags.
        static const TagInfo* tagList(const std::string& group);

        //! Print a list of all standard Exif tags to output stream
        static void taglist(std::ostream& os);
        //! Print the list of tags for \em group
        static void taglist(std::ostream& os, const std::string& group);

        /*!
          @brief Return true if \em group is a makernote group which is
                 a makernote group.
        */
        static bool isMakerGroup(const std::string& group);
        /*!
          @brief Return true if \em group is a TIFF or Exif IFD, else false.
                 This is used to differentiate between standard Exif IFDs
                 and IFDs associated with the makernote.
        */
        static bool isExifGroup(const std::string& group);

    }; // class ExifTags

    /*!
      @brief Concrete keys for Exif metadata.
     */
    class EXIV2API ExifKey : public Key {
    public:
        //! Shortcut for an %ExifKey auto pointer.
        typedef std::auto_ptr<ExifKey> AutoPtr;

        //! @name Creators
        //@{
        /*!
          @brief Constructor to create an Exif key from a key string.

          @param key The key string.
          @throw Error if the first part of the key is not '<b>Exif</b>' or
                 the remainin parts of the key cannot be parsed and
                 converted to an ifd-item and tag name.
        */
        explicit ExifKey(const std::string& key);
        /*!
          @brief Constructor to create an Exif key from a tag and IFD item
                 string.
          @param tag The tag value
          @param ifdItem The IFD string. For MakerNote tags, this must be the
                 IFD item of the specific MakerNote. "MakerNote" is not allowed.
          @throw Error if the key cannot be constructed from the tag and IFD
                 item parameters.
         */
        ExifKey(uint16_t tag, const std::string& ifdItem);
        /*!
          @brief Constructor to create an Exif key from a tag info structure
          @param tagInfo The tag info structure
          @throw Error if the key cannot be constructed from the tag and IFD
                 item parameters.
         */
        ExifKey(const TagInfo& tagInfo);
        //! Copy constructor
        ExifKey(const ExifKey& rhs);
        //! Destructor
        virtual ~ExifKey();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Assignment operator.
         */
        ExifKey& operator=(const ExifKey& rhs);
        //! Set the index.
        void setIdx(int idx);
        //@}

        //! @name Accessors
        //@{
        virtual std::string key() const;
        virtual const char* familyName() const;
        /*!
          @brief Return the name of the group (the second part of the key).
                 For Exif keys, the group name is the IFD item.
        */
        virtual std::string groupName() const;
        virtual std::string tagName() const;
        virtual std::string tagLabel() const;
        //! Return the tag description.
        std::string tagDesc() const;             // Todo: should be in the base class
        //! Return the default type id for this tag.
        TypeId defaultTypeId() const;              // Todo: should be in the base class
        virtual uint16_t tag() const;

        AutoPtr clone() const;
        //! Return the IFD id as an integer. (Do not use, this is meant for library internal use.)
        int ifdId() const;
        //! Return the name of the IFD
        const char* ifdName() const;
        //! Return the name of the Exif section (deprecated)
        std::string sectionName() const;
        //! Return the index (unique id of this key within the original Exif data, 0 if not set)
        int idx() const;
        //@}

    private:
        //! Internal virtual copy constructor.
        EXV_DLLLOCAL virtual ExifKey* clone_() const;

    private:
        // Pimpl idiom
        struct Impl;
        Impl* p_;

    }; // class ExifKey

// *****************************************************************************
// free functions

    //! Output operator for TagInfo
    EXIV2API std::ostream& operator<<(std::ostream& os, const TagInfo& ti);

}                                       // namespace Exiv2

#endif                                  // #ifndef TAGS_HPP_
