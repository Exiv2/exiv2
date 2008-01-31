// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
#include "value.hpp"

// + standard includes
#include <string>
#include <utility>                              // for std::pair
#include <iostream>
#include <memory>

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class Value;
    class Entry;

// *****************************************************************************
// type definitions

    //! Type for a function pointer for functions interpreting the tag value
    typedef std::ostream& (*PrintFct)(std::ostream&, const Value&);

    /*!
      @brief Section identifiers to logically group tags. A section consists
             of nothing more than a name, based on the Exif standard.
     */
    enum SectionId { sectionIdNotSet,
                     imgStruct, recOffset, imgCharacter, otherTags, exifFormat,
                     exifVersion, imgConfig, userInfo, relatedFile, dateTime,
                     captureCond, gpsTags, iopTags, makerTags,
                     lastSectionId };

// *****************************************************************************
// class definitions

    //! Contains information pertaining to one IFD
    struct IfdInfo {
        //! Constructor
        IfdInfo(IfdId ifdId, const char* name, const char* item);
        IfdId ifdId_;                           //!< IFD id
        const char* name_;                      //!< IFD name
        //! Related IFD item. This is also an IFD name, unique for each IFD.
        const char* item_;
    };

    //! Contains information pertaining to one section
    struct SectionInfo {
        //! Constructor
        SectionInfo(SectionId sectionId, const char* name, const char* desc);
        SectionId sectionId_;                   //!< Section id
        const char* name_;                      //!< Section name (one word)
        const char* desc_;                      //!< Section description
    };

    //! Tag information
    struct TagInfo {
        //! Constructor
        TagInfo(
            uint16_t tag,
            const char* name,
            const char* title,
            const char* desc,
            IfdId ifdId,
            SectionId sectionId,
            TypeId typeId,
            PrintFct printFct
        );
        uint16_t tag_;                          //!< Tag
        const char* name_;                      //!< One word tag label
        const char* title_;                     //!< Tag title
        const char* desc_;                      //!< Short tag description
        IfdId ifdId_;                           //!< Link to the (prefered) IFD
        SectionId sectionId_;                   //!< Section id
        TypeId typeId_;                         //!< Type id
        PrintFct printFct_;                     //!< Pointer to tag print function
    }; // struct TagInfo

    /*!
      @brief Helper structure for lookup tables for translations of numeric
             tag values to human readable labels.
     */
    struct TagDetails {
        long val_;                              //!< Tag value
        const char* label_;                     //!< Translation of the tag value

        //! Comparison operator for use with the find template
        bool operator==(long key) const { return val_ == key; }
    }; // struct TagDetails

    /*!
      @brief Helper structure for lookup tables for translations of bitmask
             values to human readable labels.
     */
    struct TagDetailsBitmask {
        uint32_t mask_;                         //!< Bitmask value
        const char* label_;                     //!< Description of the tag value
    }; // struct TagDetailsBitmask

    /*!
      @brief Generic print function to translate a long value to a description
             by looking up a reference table.
     */
    template <int N, const TagDetails (&array)[N]>
    std::ostream& printTag(std::ostream& os, const Value& value)
    {
        const TagDetails* td = find(array, value.toLong());
        if (td) {
            os << exvGettext(td->label_);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

//! Shortcut for the printTag template which requires typing the array name only once.
#define EXV_PRINT_TAG(array) printTag<EXV_COUNTOF(array), array>

    /*!
      @brief Generic print function to translate a long value to a description
             by looking up bitmasks in a reference table.
     */
    template <int N, const TagDetailsBitmask (&array)[N]>
    std::ostream& printTagBitmask(std::ostream& os, const Value& value)
    {
        const uint32_t val = static_cast<uint32_t>(value.toLong());
        bool sep = false;
        for (int i = 0; i < N; i++) {
            // *& acrobatics is a workaround for a MSVC 7.1 bug
            const TagDetailsBitmask* td = *(&array) + i;

            if (val & td->mask_) {
                if (sep) {
                    os << ", " << exvGettext(td->label_);
                }
                else {
                    os << exvGettext(td->label_);
                    sep = true;
                }
            }
        }
        return os;
    }

//! Shortcut for the printTagBitmask template which requires typing the array name only once.
#define EXV_PRINT_TAG_BITMASK(array) printTagBitmask<EXV_COUNTOF(array), array>

    //! Container for Exif tag information. Implemented as a static class.
    class ExifTags {
        //! Prevent construction: not implemented.
        ExifTags();
        //! Prevent copy-construction: not implemented.
        ExifTags(const ExifTags& rhs);
        //! Prevent assignment: not implemented.
        ExifTags& operator=(const ExifTags& rhs);

    public:
        /*!
          @brief Return the name of the tag or a string with the hexadecimal
                 value of the tag in the form "0x01ff", if the tag is not
                 a known Exif tag.

          @param tag The tag
          @param ifdId IFD id
          @return The name of the tag or a string containing the hexadecimal
                  value of the tag in the form "0x01ff", if this is an unknown
                  tag.
         */
        static std::string tagName(uint16_t tag, IfdId ifdId);
        /*!
          @brief Return the title (label) of the tag.
                 (Deprecated, use tagLabel() instead.)

          @param tag The tag
          @param ifdId IFD id
          @return The title (label) of the tag.
         */
        static const char* tagTitle(uint16_t tag, IfdId ifdId);
        /*!
          @brief Return the title (label) of the tag.

          @param tag The tag
          @param ifdId IFD id
          @return The title (label) of the tag.
         */
        static const char* tagLabel(uint16_t tag, IfdId ifdId);
        /*!
          @brief Return the description of the tag.
          @param tag The tag
          @param ifdId IFD id
          @return The description of the tag or a string indicating that
                 the tag is unknown.
         */
        static const char* tagDesc(uint16_t tag, IfdId ifdId);
        /*!
          @brief Return the tag for one combination of IFD id and tagName.
                 If the tagName is not known, it expects tag names in the
                 form "0x01ff" and converts them to unsigned integer.

          @throw Error if the tagname or ifdId is invalid
         */
        static uint16_t tag(const std::string& tagName, IfdId ifdId);
        //! Return the IFD id for an IFD item
        static IfdId ifdIdByIfdItem(const std::string& ifdItem);
        //! Return the name of the IFD
        static const char* ifdName(IfdId ifdId);
        //! Return the related image item (image or thumbnail)
        static const char* ifdItem(IfdId ifdId);
        //! Return the name of the section
        static const char* sectionName(SectionId sectionId);
        /*!
          @brief Return the name of the section for a combination of
                 tag and IFD id.
          @param tag The tag
          @param ifdId IFD id
          @return The name of the section or a string indicating that the
                  section or the tag is unknown.
         */
        static const char* sectionName(uint16_t tag, IfdId ifdId);
        /*!
          @brief Return the description of the section for a combination of
                 tag and IFD id.
          @param tag The tag
          @param ifdId IFD id
          @return The description of the section or a string indicating that
                 the section or the tag is unknown.
         */
        static const char* sectionDesc(uint16_t tag, IfdId ifdId);
        //! Return the section id for a section name
        static SectionId sectionId(const std::string& sectionName);
        //! Return the type for tag and IFD id
        static TypeId tagType(uint16_t tag, IfdId ifdId);
        //! Interpret and print the value of an Exif tag
        static std::ostream& printTag(std::ostream& os,
                                      uint16_t tag,
                                      IfdId ifdId,
                                      const Value& value);
        //! Return read-only list of built-in IFD0/1 tags
        static const TagInfo* ifdTagList();
        //! Return read-only list of built-in Exif IFD tags
        static const TagInfo* exifTagList();
        //! Return read-only list of built-in IOP tags
        static const TagInfo* iopTagList();
        //! Return read-only list of built-in GPS tags
        static const TagInfo* gpsTagList();
        //! Print a list of all standard Exif tags to output stream
        static void taglist(std::ostream& os);
        //! Print a list of all tags related to one makernote %IfdId
        static void makerTaglist(std::ostream& os, IfdId ifdId);
        //! Register an %IfdId with the base IFD %TagInfo list for a makernote
        static void registerBaseTagInfo(IfdId ifdId);
        /*!
          @brief Register an %IfdId and %TagInfo list for a makernote

          @throw Error if the MakerTagInfo registry is full
         */
        static void registerMakerTagInfo(IfdId ifdId, const TagInfo* tagInfo);
        /*!
          @brief Return true if \em ifdId is an %Ifd Id which is registered
                 as a makernote %Ifd id. Note: Calling this function with
                 makerIfd returns false.
        */
        static bool isMakerIfd(IfdId ifdId);

    private:
        static int tagInfoIdx(uint16_t tag, IfdId ifdId);
        static const TagInfo* makerTagInfo(uint16_t tag, IfdId ifdId);
        static const TagInfo* makerTagInfo(const std::string& tagName,
                                           IfdId ifdId);

        static const IfdInfo     ifdInfo_[];
        static const SectionInfo sectionInfo_[];

        static const TagInfo*    tagInfos_[];

        static const int         MAX_MAKER_TAG_INFOS = 64;
        static const TagInfo*    makerTagInfos_[MAX_MAKER_TAG_INFOS];
        static IfdId             makerIfdIds_[MAX_MAKER_TAG_INFOS];

    }; // class ExifTags

    /*!
      @brief Concrete keys for Exif metadata.
     */
    class ExifKey : public Key {
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
        //! Constructor to build an ExifKey from an IFD entry.
        explicit ExifKey(const Entry& e);
        //! Copy constructor
        ExifKey(const ExifKey& rhs);
        virtual ~ExifKey();
        //@}

        //! @name Manipulators
        //@{
        /*!
          @brief Assignment operator.
         */
        ExifKey& operator=(const ExifKey& rhs);
        //@}

        //! @name Accessors
        //@{
        virtual std::string key() const { return key_; }
        virtual const char* familyName() const { return familyName_; }
        /*!
          @brief Return the name of the group (the second part of the key).
                 For Exif keys, the group name is the IFD item.
        */
        virtual std::string groupName() const { return ifdItem(); }
        virtual std::string tagName() const;
        virtual std::string tagLabel() const;
        virtual uint16_t tag() const { return tag_; }

        AutoPtr clone() const;
        //! Return the IFD id
        IfdId ifdId() const { return ifdId_; }
        //! Return the name of the IFD
        const char* ifdName() const { return ExifTags::ifdName(ifdId()); }
        //! Return the related image item
        std::string ifdItem() const { return ifdItem_; }
        //! Return the name of the Exif section (deprecated)
        std::string sectionName() const;
        //! Return the index (unique id of this key within the original IFD)
        int idx() const { return idx_; }
        //@}

    protected:
        //! @name Manipulators
        //@{
        /*!
          @brief Set the key corresponding to the tag and IFD id.
                 The key is of the form '<b>Exif</b>.ifdItem.tagName'.
         */
        void makeKey();
        /*!
          @brief Parse and convert the key string into tag and IFD Id.
                 Updates data members if the string can be decomposed,
                 or throws \em Error .

          @throw Error if the key cannot be decomposed.
         */
        void decomposeKey();
        //@}

    private:
        //! Internal virtual copy constructor.
        virtual ExifKey* clone_() const;

        // DATA
        static const char* familyName_;

        uint16_t tag_;                  //!< Tag value
        IfdId ifdId_;                   //!< The IFD associated with this tag
        std::string ifdItem_;           //!< The IFD item
        int idx_;                       //!< Unique id of an entry within one IFD
        std::string key_;               //!< Key
    }; // class ExifKey

// *****************************************************************************
// free functions

    /*!
      @brief Return true if \em ifdId is an Exif %Ifd Id, i.e., one of
             ifd0Id, exifIfdId, gpsIfdId, iopIfdId or ifd1Id, else false.
             This is used to differentiate between standard Exif %Ifds
             and %Ifds associated with the makernote.
     */
    bool isExifIfd(IfdId ifdId);

    //! Output operator for TagInfo
    std::ostream& operator<<(std::ostream& os, const TagInfo& ti);

    //! @name Functions printing interpreted tag values
    //@{
    //! Default print function, using the Value output operator
    std::ostream& printValue(std::ostream& os, const Value& value);
    //! Print the value converted to a long
    std::ostream& printLong(std::ostream& os, const Value& value);
    //! Print a Rational or URational value in floating point format
    std::ostream& printFloat(std::ostream& os, const Value& value);
    //! Print a longitude or latitude value
    std::ostream& printDegrees(std::ostream& os, const Value& value);
    //! Print function converting from UCS-2LE to UTF-8
    std::ostream& printUcs2(std::ostream& os, const Value& value);
    //! Print function for Exif units
    std::ostream& printExifUnit(std::ostream& os, const Value& value);
    //! Print GPS version
    std::ostream& print0x0000(std::ostream& os, const Value& value);
    //! Print GPS altitude
    std::ostream& print0x0006(std::ostream& os, const Value& value);
    //! Print GPS timestamp
    std::ostream& print0x0007(std::ostream& os, const Value& value);
    //! Print orientation
    std::ostream& print0x0112(std::ostream& os, const Value& value);
    //! Print YCbCrPositioning
    std::ostream& print0x0213(std::ostream& os, const Value& value);
    //! Print the copyright
    std::ostream& print0x8298(std::ostream& os, const Value& value);
    //! Print the exposure time
    std::ostream& print0x829a(std::ostream& os, const Value& value);
    //! Print the f-number
    std::ostream& print0x829d(std::ostream& os, const Value& value);
    //! Print exposure program
    std::ostream& print0x8822(std::ostream& os, const Value& value);
    //! Print ISO speed ratings
    std::ostream& print0x8827(std::ostream& os, const Value& value);
    //! Print components configuration specific to compressed data
    std::ostream& print0x9101(std::ostream& os, const Value& value);
    //! Print exposure time converted from APEX shutter speed value
    std::ostream& print0x9201(std::ostream& os, const Value& value);
    //! Print f-number converted from APEX aperture value
    std::ostream& print0x9202(std::ostream& os, const Value& value);
    //! Print the exposure bias value
    std::ostream& print0x9204(std::ostream& os, const Value& value);
    //! Print the subject distance
    std::ostream& print0x9206(std::ostream& os, const Value& value);
    //! Print metering mode
    std::ostream& print0x9207(std::ostream& os, const Value& value);
    //! Print light source
    std::ostream& print0x9208(std::ostream& os, const Value& value);
    //! Print the actual focal length of the lens
    std::ostream& print0x920a(std::ostream& os, const Value& value);
    //! Print the user comment
    std::ostream& print0x9286(std::ostream& os, const Value& value);
    //! Print color space
    std::ostream& print0xa001(std::ostream& os, const Value& value);
    //! Print sensing method
    std::ostream& print0xa217(std::ostream& os, const Value& value);
    //! Print file source
    std::ostream& print0xa300(std::ostream& os, const Value& value);
    //! Print scene type
    std::ostream& print0xa301(std::ostream& os, const Value& value);
    //! Print custom rendered
    std::ostream& print0xa401(std::ostream& os, const Value& value);
    //! Print exposure mode
    std::ostream& print0xa402(std::ostream& os, const Value& value);
    //! Print white balance
    std::ostream& print0xa403(std::ostream& os, const Value& value);
    //! Print digital zoom ratio
    std::ostream& print0xa404(std::ostream& os, const Value& value);
    //! Print 35mm equivalent focal length
    std::ostream& print0xa405(std::ostream& os, const Value& value);
    //! Print scene capture type
    std::ostream& print0xa406(std::ostream& os, const Value& value);
    //! Print gain control
    std::ostream& print0xa407(std::ostream& os, const Value& value);
    //! Print saturation
    std::ostream& print0xa409(std::ostream& os, const Value& value);
    //! Print subject distance range
    std::ostream& print0xa40c(std::ostream& os, const Value& value);
    //! Print contrast, sharpness (normal, soft, hard)
    std::ostream& printNormalSoftHard(std::ostream& os, const Value& value);
    //! Print any version packed in 4 Bytes format : major major minor minor
    std::ostream& printExifVersion(std::ostream& os, const Value& value);
    //! Print any version encoded in the ASCII string majormajorminorminor
    std::ostream& printXmpVersion(std::ostream& os, const Value& value);
    //! Print a date following the format YYYY-MM-DDTHH:MM:SSZ
    std::ostream& printXmpDate(std::ostream& os, const Value& value);
    //@}

    //! Calculate F number from an APEX aperture value
    float fnumber(float apertureValue);

    //! Calculate the exposure time from an APEX shutter speed value
    URational exposureTime(float shutterSpeedValue);

}                                       // namespace Exiv2

#endif                                  // #ifndef TAGS_HPP_
