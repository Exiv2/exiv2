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
  @file    tags.hpp
  @brief   Exif tag and type information
  @version $Name:  $ $Revision: 1.22 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Jan-04, ahu: created
           11-Feb-04, ahu: isolated as a component
 */
#ifndef TAGS_HPP_
#define TAGS_HPP_

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

    //! Type for a function pointer for functions interpreting the tag value
    typedef std::ostream& (*PrintFct)(std::ostream&, const Value&);

    /*!
      @brief Section identifiers to logically group tags. A section consists
             of nothing more than a name, based on the Exif standard.
     */
    enum SectionId { sectionIdNotSet, 
                     imgStruct, recOffset, imgCharacter, otherTags, exifFormat, 
                     exifVersion, imgConfig, userInfo, relatedFile, dateTime,
                     captureCond, gpsTags, iopTags, 
                     lastSectionId };

// *****************************************************************************
// class definitions

    //! Contains information pertaining to one IFD
    struct IfdInfo {
        //! Constructor
        IfdInfo(IfdId ifdId, const char* name, const char* item);
        IfdId ifdId_;                           //!< IFD id
        const char* name_;                      //!< IFD name
        const char* item_;                      //!< Related image item
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
            uint16 tag, 
            const char* name,
            const char* desc, 
            IfdId ifdId,
            SectionId sectionId,
            PrintFct printFct
        );
        uint16 tag_;                            //!< Tag
        const char* name_;                      //!< One word tag label
        const char* desc_;                      //!< Short tag description
        IfdId ifdId_;                           //!< Link to the (prefered) IFD
        SectionId sectionId_;                   //!< Section id
        PrintFct printFct_;                     //!< Pointer to tag print function
    }; // struct TagInfo

    //! Container for Exif tag information. Implemented as a static class.
    class ExifTags {
        //! Prevent construction: not implemented.
        ExifTags() {}
        //! Prevent copy-construction: not implemented.
        ExifTags(const ExifTags& rhs);
        //! Prevent assignment: not implemented.
        ExifTags& operator=(const ExifTags& rhs);

    public:
        /*!
          @brief Return the name of the tag.
          @param tag The tag
          @param ifdId IFD id
          @return The name of the tag or a string indicating that the 
                  tag is unknown. 
          @throw Error ("No taginfo for IFD") if there is no tag info
                 data for the given IFD id in the lookup tables.
         */
        static const char* tagName(uint16 tag, IfdId ifdId);
        /*!
          @brief Return the description of the tag.
          @param tag The tag
          @param ifdId IFD id
          @return The description of the tag or a string indicating that
                 the tag is unknown. 
          @throw Error ("No taginfo for IFD") if there is no tag info
                 data for the given IFD id in the lookup tables.
         */
        static const char* tagDesc(uint16 tag, IfdId ifdId);
        //! Return the tag for one combination of IFD id and tagName
        static uint16 tag(const std::string& tagName, IfdId ifdId);
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
          @throw Error ("No taginfo for IFD") if there is no tag info
                 data for the given IFD id in the lookup tables.
         */
        static const char* sectionName(uint16 tag, IfdId ifdId);
        /*!
          @brief Return the description of the section for a combination of 
                 tag and IFD id.
          @param tag The tag
          @param ifdId IFD id
          @return The description of the section or a string indicating that
                 the section or the tag is unknown. 
          @throw Error ("No taginfo for IFD") if there is no tag info
                 data for the given IFD id in the lookup tables.
         */
        static const char* sectionDesc(uint16 tag, IfdId ifdId);
        //! Return the section id for a section name
        static SectionId sectionId(const std::string& sectionName);
        /*!
          @brief Return the key for the tag and IFD id.  The key is of the form
                 'ifdItem.sectionName.tagName'.
         */
        static std::string makeKey(uint16 tag, IfdId ifdId);
        /*!
          @brief Return tag and IFD id pair for the key.
          @return A pair consisting of the tag and IFD id.
          @throw Error ("Invalid key") if the key cannot be parsed into
                 item item, section name and tag name parts.
         */
        static std::pair<uint16, IfdId> decomposeKey(const std::string& key);
        //! Interpret and print the value of an Exif tag
        static std::ostream& printTag(std::ostream& os,
                                      uint16 tag, 
                                      IfdId ifdId,
                                      const Value& value);
        //! Print a list of all tags to standart output
        static void taglist(std::ostream& os);

    private:
        static int tagInfoIdx(uint16 tag, IfdId ifdId);
        static int tagInfoIdx(const std::string& tagName, IfdId ifdId);

        static const IfdInfo     ifdInfo_[];
        static const SectionInfo sectionInfo_[];

        static const TagInfo*    tagInfos_[];

    }; // class ExifTags

// *****************************************************************************
// free functions

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
    //! Print the unit for measuring X and Y resolution
    std::ostream& printUnit(std::ostream& os, const Value& value);

    //! Print the compression scheme used for the image data
    std::ostream& print0x0103(std::ostream& os, const Value& value);
    //! Print the pixel composition
    std::ostream& print0x0106(std::ostream& os, const Value& value);
    //! Print the orientation
    std::ostream& print0x0112(std::ostream& os, const Value& value);
    //! Print the YCbCrPositioning
    std::ostream& print0x0213(std::ostream& os, const Value& value);
    //! Print the Copyright 
    std::ostream& print0x8298(std::ostream& os, const Value& value);
    //! Print the Exposure time
    std::ostream& print0x829a(std::ostream& os, const Value& value);
    //! Print the F number
    std::ostream& print0x829d(std::ostream& os, const Value& value);
    //! Print the Exposure mode
    std::ostream& print0x8822(std::ostream& os, const Value& value);
    //! Print ISO speed ratings
    std::ostream& print0x8827(std::ostream& os, const Value& value);
    //! Print components configuration specific to compressed data
    std::ostream& print0x9101(std::ostream& os, const Value& value);
    //! Print the exposure bias value
    std::ostream& print0x9204(std::ostream& os, const Value& value);
    //! Print the subject distance
    std::ostream& print0x9206(std::ostream& os, const Value& value);
    //! Print the metering mode
    std::ostream& print0x9207(std::ostream& os, const Value& value);
    //! Print the light source
    std::ostream& print0x9208(std::ostream& os, const Value& value);
    //! Print the flash status
    std::ostream& print0x9209(std::ostream& os, const Value& value);
    //! Print the actual focal length of the lens
    std::ostream& print0x920a(std::ostream& os, const Value& value);
    //! Print the user comment
    std::ostream& print0x9286(std::ostream& os, const Value& value);
    //! Print color space information
    std::ostream& print0xa001(std::ostream& os, const Value& value);
    //! Print info on image sensor type on the camera or input device
    std::ostream& print0xa217(std::ostream& os, const Value& value);
    //! Print file source
    std::ostream& print0xa300(std::ostream& os, const Value& value);
    //! Print scene type
    std::ostream& print0xa301(std::ostream& os, const Value& value);
    //! Print the exposure mode
    std::ostream& print0xa402(std::ostream& os, const Value& value);
    //! Print white balance information
    std::ostream& print0xa403(std::ostream& os, const Value& value);
    //! Print digital zoom ratio
    std::ostream& print0xa404(std::ostream& os, const Value& value);
    //! Print 35mm equivalent focal length 
    std::ostream& print0xa405(std::ostream& os, const Value& value);
    //! Print scene capture type
    std::ostream& print0xa406(std::ostream& os, const Value& value);
    //! Print overall image gain adjustment
    std::ostream& print0xa407(std::ostream& os, const Value& value);
    //! Print contract adjustment
    std::ostream& print0xa408(std::ostream& os, const Value& value);
    //! Print saturation adjustment
    std::ostream& print0xa409(std::ostream& os, const Value& value);
    //! Print sharpness adjustment
    std::ostream& print0xa40a(std::ostream& os, const Value& value);
    //! Print subject distance range
    std::ostream& print0xa40c(std::ostream& os, const Value& value);
    //@}
}                                       // namespace Exiv2

#endif                                  // #ifndef TAGS_HPP_
