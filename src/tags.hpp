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
  @brief   %Exif tag and type information
  @version $Name:  $ $Revision: 1.11 $
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    15-Jan-04, ahu: created
 */
#ifndef TAGS_HPP_
#define TAGS_HPP_

// *****************************************************************************
// included header files

// + standard includes
#include <string>
#include <utility>                              // for std::pair
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exif {

// *****************************************************************************
// class declarations
    class Value;

// *****************************************************************************
// type definitions

    //! 2 byte unsigned integer type.
    typedef unsigned short uint16;
    //! 4 byte unsigned integer type.
    typedef unsigned long  uint32;
    //! 2 byte signed integer type.
    typedef short          int16;
    //! 4 byte signed integer type.
    typedef long           int32;

    //! 8 byte unsigned rational type.
    typedef std::pair<uint32, uint32> URational;
    //! 8 byte signed rational type.
    typedef std::pair<int32, int32> Rational;

    //! Type for a function pointer for functions interpreting the tag value
    typedef std::ostream& (*PrintFct)(std::ostream&, const Value&);

    //! Type identifiers for IFD format types
    enum TypeId { invalid, unsignedByte, asciiString, unsignedShort, 
                  unsignedLong, unsignedRational, invalid6, undefined,
                  signedShort, signedLong, signedRational };

    //! Type to specify the IFD to which a metadata belongs
    enum IfdId { ifdIdNotSet, 
                 ifd0, exifIfd, gpsIfd, makerIfd, iopIfd, 
                 ifd1, ifd1ExifIfd, ifd1GpsIfd, ifd1MakerIfd, ifd1IopIfd,
                 lastIfdId};

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

    /*!
      @brief Very simple error class used for exceptions. It contains just an
             error message. An output operator is provided to print
             errors to a stream.
     */
    class Error {
    public:
        //! Constructor taking a (short) error message as argument
        Error(const std::string& message) : message_(message) {}
        /*!
          @brief Return the error message. Consider using the output operator
                 operator<<(std::ostream &os, const Error& error) instead.
         */
        std::string message() const { return message_; }
    private:
        std::string message_;
    };

    //! %Error output operator
    inline std::ostream& operator<<(std::ostream& os, const Error& error)
    {
        return os << error.message();
    }

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

    //! Description of the format of a metadatum
    struct TagFormat {
        //! Constructor
        TagFormat(TypeId typeId, const char* name, long size);
        TypeId typeId_;                         //!< Format type id
        const char* name_;                      //!< Name of the format 
        long size_;                             //!< Bytes per data entry 
    }; // struct TagFormat

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

    //! Container for %Exif tag information. Implemented as a static class.
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
        //! Return the name of the type
        static const char* typeName(TypeId typeId);
        //! Return the size in bytes of one element of this type
        static long typeSize(TypeId typeId);
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
          @brief Return a unique key for the tag and IFD id.
                 The key is of the form 'ifdItem.sectionName.tagName'.
         */
        static std::string makeKey(uint16 tag, IfdId ifdId);
        /*!
          @brief Return the unique tag and IFD id pair for the key.
          @return A pair consisting of the tag and IFD id.
          @throw Error ("Invalid key") if the key cannot be parsed into
                 item item, section name and tag name parts.
         */
        static std::pair<uint16, IfdId> decomposeKey(const std::string& key);
        //! Return the print function for the tag, IFD id combination
        static PrintFct printFct(uint16 tag, IfdId ifdId);
        //! Print a list of all tags to standart output
        static void taglist();
        
    private:
        static int tagInfoIdx(uint16 tag, IfdId ifdId);
        static int tagInfoIdx(const std::string& tagName, IfdId ifdId);

        static const IfdInfo     ifdInfo_[];
        static const SectionInfo sectionInfo_[];
        static const TagFormat   tagFormat_[];

        static const TagInfo*    tagInfos_[];

    };

// *****************************************************************************
// free functions

    //! Output operator for TagInfo
    std::ostream& operator<<(std::ostream& os, const TagInfo& ti);

    //! Output operator for our fake rational
    std::ostream& operator<<(std::ostream& os, const Rational& r);
    //! Input operator for our fake rational
    std::istream& operator>>(std::istream& is, Rational& r);
    //! Output operator for our fake unsigned rational
    std::ostream& operator<<(std::ostream& os, const URational& r);
    //! Input operator for our fake unsigned rational
    std::istream& operator>>(std::istream& is, URational& r);

    //! Template to determine the TypeId for a type T
    template<typename T> TypeId getType();

    //! Specialization for an unsigned short
    template<> inline TypeId getType<uint16>() { return unsignedShort; }
    //! Specialization for an unsigned long
    template<> inline TypeId getType<uint32>() { return unsignedLong; }
    //! Specialization for an unsigned rational
    template<> inline TypeId getType<URational>() { return unsignedRational; }
    //! Specialization for a signed short
    template<> inline TypeId getType<int16>() { return signedShort; }
    //! Specialization for a signed long
    template<> inline TypeId getType<int32>() { return signedLong; }
    //! Specialization for a signed rational
    template<> inline TypeId getType<Rational>() { return signedRational; }

    // No default implementation: let the compiler/linker complain
//    template<typename T> inline TypeId getType() { return invalid; }

    //! Utility function to convert the argument of any type to a string
    template<typename T> 
    std::string toString(T arg)
    {
        std::ostringstream os;
        os << arg;
        return os.str();
    }

    //! @name Functions printing interpreted tag values
    //@{
    //! NULL print function, prints a constant error message
    std::ostream& printNull(std::ostream& os, const Value& value);
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
    //! Print the YCbCrPositioning
    std::ostream& print0x0213(std::ostream& os, const Value& value);
    //! Print the Exposure time
    std::ostream& print0x829a(std::ostream& os, const Value& value);
    //! Print the F number
    std::ostream& print0x829d(std::ostream& os, const Value& value);
    //! Print the Exposure mode
    std::ostream& print0x8822(std::ostream& os, const Value& value);
    //! Print components configuration specific to compressed data
    std::ostream& print0x9101(std::ostream& os, const Value& value);
    //! Print the subject distance
    std::ostream& print0x9206(std::ostream& os, const Value& value);
    //! Print the metering mode
    std::ostream& print0x9207(std::ostream& os, const Value& value);
    //! Print the flash status
    std::ostream& print0x9209(std::ostream& os, const Value& value);
    //! Print the actual focal length of the lens
    std::ostream& print0x920a(std::ostream& os, const Value& value);
    //! Print color space information
    std::ostream& print0xa001(std::ostream& os, const Value& value);
    //! Print info on image sensor type on the camera or input device
    std::ostream& print0xa217(std::ostream& os, const Value& value);
    //! Print the exposure mode
    std::ostream& print0xa402(std::ostream& os, const Value& value);
    //! Print white balance information
    std::ostream& print0xa403(std::ostream& os, const Value& value);
    //! Print digital zoom ratio
    std::ostream& print0xa404(std::ostream& os, const Value& value);
    //! Print scene capture type
    std::ostream& print0xa406(std::ostream& os, const Value& value);

    // Todo: Copyright       (0x8298)
    // Todo: UserComment     (0x9286)

    //@}
}                                       // namespace Exif

#endif                                  // #ifndef TAGS_HPP_
