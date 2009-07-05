// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2009 Andreas Huggel <ahuggel@gmx.net>
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
    class ExifData;
    class Value;
    struct TagInfo;

// *****************************************************************************
// type definitions

    //! Type for a function pointer for functions interpreting the tag value
    typedef std::ostream& (*PrintFct)(std::ostream&, const Value&, const ExifData* pExifData);
    //! A function returning a tag list.
    typedef const TagInfo* (*TagListFct)();
    /*!
      @brief Section identifiers to logically group tags. A section consists
             of nothing more than a name, based on the Exif standard.
     */
    enum SectionId { sectionIdNotSet,
                     imgStruct, recOffset, imgCharacter, otherTags, exifFormat,
                     exifVersion, imgConfig, userInfo, relatedFile, dateTime,
                     captureCond, gpsTags, iopTags, makerTags, dngTags, panaRaw,
                     lastSectionId };

// *****************************************************************************
// class definitions

    //! The details of an IFD.
    struct EXIV2API IfdInfo {
        struct Item;
        bool operator==(IfdId ifdId) const;     //!< Comparison operator for IFD id
        bool operator==(const Item& item) const;       //!< Comparison operator for IFD item
        IfdId ifdId_;                           //!< IFD id
        const char* name_;                      //!< IFD name
        const char* item_; //!< Related IFD item. This is also an IFD name, unique for each IFD.
        TagListFct tagList_;                    //!< Tag list
    };

    //! Search key to find an IfdInfo by its IFD item.
    struct EXIV2API IfdInfo::Item {
        Item(const std::string& item);          //!< Constructor
        std::string i_;                         //!< IFD item
    };

    //! The details of a section.
    struct EXIV2API SectionInfo {
        SectionId sectionId_;                   //!< Section id
        const char* name_;                      //!< Section name (one word)
        const char* desc_;                      //!< Section description
    };

    //! Tag information
    struct EXIV2API TagInfo {
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
    struct EXIV2API TagDetails {
        long val_;                              //!< Tag value
        const char* label_;                     //!< Translation of the tag value

        //! Comparison operator for use with the find template
        bool operator==(long key) const { return val_ == key; }
    }; // struct TagDetails

    /*!
      @brief Helper structure for lookup tables for translations of bitmask
             values to human readable labels.
     */
    struct EXIV2API TagDetailsBitmask {
        uint32_t mask_;                         //!< Bitmask value
        const char* label_;                     //!< Description of the tag value
    }; // struct TagDetailsBitmask

    /*!
      @brief Helper structure for lookup tables for translations of controlled
             vocabulary strings to their descriptions.
     */
    struct EXIV2API TagVocabulary {
        const char* voc_;                       //!< Vocabulary string
        const char* label_;                     //!< Description of the vocabulary string

        /*!
          @brief Comparison operator for use with the find template

          Compare vocabulary strings like "PR-NON" with keys like
          "http://ns.useplus.org/ldf/vocab/PR-NON" and return true if the vocabulary
          string matches the end of the key.
         */
        bool operator==(const std::string& key) const;
    }; // struct TagDetails

    /*!
      @brief Generic pretty-print function to translate a long value to a description
             by looking up a reference table.
     */
    template <int N, const TagDetails (&array)[N]>
    std::ostream& printTag(std::ostream& os, const Value& value, const ExifData*)
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
    std::ostream& printTagBitmask(std::ostream& os, const Value& value, const ExifData*)
    {
        const uint32_t val = static_cast<uint32_t>(value.toLong());
        if (val == 0 && N > 0) {
            const TagDetailsBitmask* td = *(&array);
            if (td->mask_ == 0) return os << exvGettext(td->label_);
        }
        bool sep = false;
        for (int i = 0; i < N; ++i) {
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

    /*!
      @brief Generic pretty-print function to translate a controlled vocabulary value (string)
             to a description by looking up a reference table.
     */
    template <int N, const TagVocabulary (&array)[N]>
    std::ostream& printTagVocabulary(std::ostream& os, const Value& value, const ExifData*)
    {
        const TagVocabulary* td = find(array, value.toString());
        if (td) {
            os << exvGettext(td->label_);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

//! Shortcut for the printTagVocabulary template which requires typing the array name only once.
#define EXV_PRINT_VOCABULARY(array) printTagVocabulary<EXV_COUNTOF(array), array>

    //! Exif tag reference, implemented as a static class.
    class EXIV2API ExifTags {
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
                                      const Value& value,
                                      const ExifData* pExifData =0);
        //! Return read-only list of built-in IFD0/1 tags
        static const TagInfo* ifdTagList();
        //! Return read-only list of built-in Exif IFD tags
        static const TagInfo* exifTagList();
        //! Return read-only list of built-in IOP tags
        static const TagInfo* iopTagList();
        //! Return read-only list of built-in GPS tags
        static const TagInfo* gpsTagList();
        //! Return read-only list of built-in Exiv2 Makernote info tags
        static const TagInfo* mnTagList();
        //! Print a list of all standard Exif tags to output stream
        static void taglist(std::ostream& os);
        //! Print the list of tags for \em %IfdId
        static void taglist(std::ostream& os, IfdId ifdId);
        /*!
          @brief Return true if \em ifdId is an %Ifd id which is
                 a makernote %Ifd id. Note: Calling this function with
                 makerIfd returns false.
        */
        static bool isMakerIfd(IfdId ifdId);
        /*!
          @brief Return true if \em ifdId is an Exif %Ifd Id, i.e., one of
                 ifd0Id, exifIfdId, gpsIfdId, iopIfdId or ifd1Id, else false.
                 This is used to differentiate between standard Exif %Ifds
                 and %Ifds associated with the makernote.
        */
        static bool isExifIfd(IfdId ifdId);

    private:
        static const TagInfo* tagList(IfdId ifdId);
        static const TagInfo* tagInfo(uint16_t tag, IfdId ifdId);
        static const TagInfo* tagInfo(const std::string& tagName, IfdId ifdId);

        static const IfdInfo     ifdInfo_[];     //!< All Exif and Makernote tag lists
        static const SectionInfo sectionInfo_[]; //!< Exif (and one common Makernote) sections

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
        //! Set the index.
        void setIdx(int idx);
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
        //! Return the index (unique id of this key within the original Exif data, 0 if not set)
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
        EXV_DLLLOCAL virtual ExifKey* clone_() const;

        // DATA
        static const char* familyName_;

        uint16_t tag_;                  //!< Tag value
        IfdId ifdId_;                   //!< The IFD associated with this tag
        std::string ifdItem_;           //!< The IFD item
        int idx_;                       //!< Unique id of the Exif key in the image
        std::string key_;               //!< Key
    }; // class ExifKey

// *****************************************************************************
// free functions

    //! Output operator for TagInfo
    EXIV2API std::ostream& operator<<(std::ostream& os, const TagInfo& ti);

    //! @name Functions printing interpreted tag values
    //@{
    //! Default print function, using the Value output operator
    EXIV2API std::ostream& printValue(std::ostream& os, const Value& value, const ExifData*);
    //! Print the value converted to a long
    EXIV2API std::ostream& printLong(std::ostream& os, const Value& value, const ExifData*);
    //! Print a Rational or URational value in floating point format
    EXIV2API std::ostream& printFloat(std::ostream& os, const Value& value, const ExifData*);
    //! Print a longitude or latitude value
    EXIV2API std::ostream& printDegrees(std::ostream& os, const Value& value, const ExifData*);
    //! Print function converting from UCS-2LE to UTF-8
    EXIV2API std::ostream& printUcs2(std::ostream& os, const Value& value, const ExifData*);
    //! Print function for Exif units
    EXIV2API std::ostream& printExifUnit(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS version
    EXIV2API std::ostream& print0x0000(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS altitude ref
    EXIV2API std::ostream& print0x0005(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS altitude
    EXIV2API std::ostream& print0x0006(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS timestamp
    EXIV2API std::ostream& print0x0007(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS status
    EXIV2API std::ostream& print0x0009(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS measurement mode
    EXIV2API std::ostream& print0x000a(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS speed ref
    EXIV2API std::ostream& print0x000c(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS destination distance ref
    EXIV2API std::ostream& print0x0019(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS differential correction
    EXIV2API std::ostream& print0x001e(std::ostream& os, const Value& value, const ExifData*);
    //! Print orientation
    EXIV2API std::ostream& print0x0112(std::ostream& os, const Value& value, const ExifData*);
    //! Print YCbCrPositioning
    EXIV2API std::ostream& print0x0213(std::ostream& os, const Value& value, const ExifData*);
    //! Print the copyright
    EXIV2API std::ostream& print0x8298(std::ostream& os, const Value& value, const ExifData*);
    //! Print the exposure time
    EXIV2API std::ostream& print0x829a(std::ostream& os, const Value& value, const ExifData*);
    //! Print the f-number
    EXIV2API std::ostream& print0x829d(std::ostream& os, const Value& value, const ExifData*);
    //! Print exposure program
    EXIV2API std::ostream& print0x8822(std::ostream& os, const Value& value, const ExifData*);
    //! Print ISO speed ratings
    EXIV2API std::ostream& print0x8827(std::ostream& os, const Value& value, const ExifData*);
    //! Print components configuration specific to compressed data
    EXIV2API std::ostream& print0x9101(std::ostream& os, const Value& value, const ExifData*);
    //! Print exposure time converted from APEX shutter speed value
    EXIV2API std::ostream& print0x9201(std::ostream& os, const Value& value, const ExifData*);
    //! Print f-number converted from APEX aperture value
    EXIV2API std::ostream& print0x9202(std::ostream& os, const Value& value, const ExifData*);
    //! Print the exposure bias value
    EXIV2API std::ostream& print0x9204(std::ostream& os, const Value& value, const ExifData*);
    //! Print the subject distance
    EXIV2API std::ostream& print0x9206(std::ostream& os, const Value& value, const ExifData*);
    //! Print metering mode
    EXIV2API std::ostream& print0x9207(std::ostream& os, const Value& value, const ExifData*);
    //! Print light source
    EXIV2API std::ostream& print0x9208(std::ostream& os, const Value& value, const ExifData*);
    //! Print the actual focal length of the lens
    EXIV2API std::ostream& print0x920a(std::ostream& os, const Value& value, const ExifData*);
    //! Print the user comment
    EXIV2API std::ostream& print0x9286(std::ostream& os, const Value& value, const ExifData*);
    //! Print color space
    EXIV2API std::ostream& print0xa001(std::ostream& os, const Value& value, const ExifData*);
    //! Print sensing method
    EXIV2API std::ostream& print0xa217(std::ostream& os, const Value& value, const ExifData*);
    //! Print file source
    EXIV2API std::ostream& print0xa300(std::ostream& os, const Value& value, const ExifData*);
    //! Print scene type
    EXIV2API std::ostream& print0xa301(std::ostream& os, const Value& value, const ExifData*);
    //! Print custom rendered
    EXIV2API std::ostream& print0xa401(std::ostream& os, const Value& value, const ExifData*);
    //! Print exposure mode
    EXIV2API std::ostream& print0xa402(std::ostream& os, const Value& value, const ExifData*);
    //! Print white balance
    EXIV2API std::ostream& print0xa403(std::ostream& os, const Value& value, const ExifData*);
    //! Print digital zoom ratio
    EXIV2API std::ostream& print0xa404(std::ostream& os, const Value& value, const ExifData*);
    //! Print 35mm equivalent focal length
    EXIV2API std::ostream& print0xa405(std::ostream& os, const Value& value, const ExifData*);
    //! Print scene capture type
    EXIV2API std::ostream& print0xa406(std::ostream& os, const Value& value, const ExifData*);
    //! Print gain control
    EXIV2API std::ostream& print0xa407(std::ostream& os, const Value& value, const ExifData*);
    //! Print saturation
    EXIV2API std::ostream& print0xa409(std::ostream& os, const Value& value, const ExifData*);
    //! Print subject distance range
    EXIV2API std::ostream& print0xa40c(std::ostream& os, const Value& value, const ExifData*);
    //! Print GPS direction ref
    EXIV2API std::ostream& printGPSDirRef(std::ostream& os, const Value& value, const ExifData*);
    //! Print contrast, sharpness (normal, soft, hard)
    EXIV2API std::ostream& printNormalSoftHard(std::ostream& os, const Value& value, const ExifData*);
    //! Print any version packed in 4 Bytes format : major major minor minor
    EXIV2API std::ostream& printExifVersion(std::ostream& os, const Value& value, const ExifData*);
    //! Print any version encoded in the ASCII string majormajorminorminor
    EXIV2API std::ostream& printXmpVersion(std::ostream& os, const Value& value, const ExifData*);
    //! Print a date following the format YYYY-MM-DDTHH:MM:SSZ
    EXIV2API std::ostream& printXmpDate(std::ostream& os, const Value& value, const ExifData*);
    //@}

    //! Calculate F number from an APEX aperture value
    EXIV2API float fnumber(float apertureValue);

    //! Calculate the exposure time from an APEX shutter speed value
    EXIV2API URational exposureTime(float shutterSpeedValue);

}                                       // namespace Exiv2

#endif                                  // #ifndef TAGS_HPP_
