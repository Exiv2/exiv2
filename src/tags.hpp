// ***************************************************************** -*- C++ -*-
/*
 * Copyright (c) 2004 Andreas Huggel. All rights reserved.
 * 
 * Todo: Insert license blabla here
 *
 */
/*!
  @file    tags.hpp
  @brief   %Exif tag and type information
  @version $Name:  $ $Revision: 1.4 $
  @author  Andreas Huggel (ahu)
  @date    15-Jan-03, ahu: created
 */
#ifndef _TAGS_HPP_
#define _TAGS_HPP_

// *****************************************************************************
// included header files

// + standard includes
#include <utility>                              // for std::pair
#include <iosfwd>

// *****************************************************************************
// namespace extensions
namespace Exif {

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

    //! Type identifiers for IFD format types
    enum TypeId { invalid, unsignedByte, asciiString, unsignedShort, 
                  unsignedLong, unsignedRational, invalid6, undefined,
                  signedShort, signedLong, signedRational };

    //! Type to specify the IFD to which a metadata belongs
    enum IfdId { IfdIdNotSet, 
                 ifd0, exifIfd, gpsIfd, makerIfd, iopIfd, 
                 ifd1, ifd1ExifIfd, ifd1GpsIfd, ifd1MakerIfd, ifd1IopIfd };

    /*!
      @brief Section identifiers to logically group tags. A section consists
             of nothing more than a name, based on the Exif standard.
     */
    enum SectionId { SectionIdNotSet, 
                     imgStruct, recOffset, imgCharacter, otherTags, exifFormat, 
                     exifVersion, imgConfig, userInfo, relatedFile, dateTime,
                     captureCond, gpsTags, iopTags };

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
            SectionId sectionId
        );
        uint16 tag_;                            //!< Tag
        const char* name_;                      //!< One word tag label
        const char* desc_;                      //!< Short tag description
        IfdId ifdId_;                           //!< Link to the IFD
        SectionId sectionId_;                   //!< Section id
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
        //! Returns the name of the tag
        static const char* tagName(uint16 tag, IfdId ifdId);
        //! Returns the name of the type
        static const char* typeName(TypeId typeId);
        //! Returns the size in bytes of one element of this type
        static long typeSize(TypeId typeId);
        //! Returns the name of the IFD
        static const char* ifdName(IfdId ifdId);
        //! Returns the related image item (image or thumbnail)
        static const char* ifdItem(IfdId ifdId);
        //! Returns the name of the section
        static const char* sectionName(SectionId sectionId);
        //! Returns the name of the section
        static const char* sectionName(uint16 tag, IfdId ifdId);

    private:
        static int tagInfoIdx(uint16 tag, IfdId ifdId);

        static const IfdInfo     ifdInfo_[];
        static const SectionInfo sectionInfo_[];
        static const TagFormat   tagFormat_[];

        static const TagInfo*    tagInfos_[];

    };

// *****************************************************************************
// free functions

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

}                                       // namespace Exif

#endif                                  // #ifndef _TAGS_HPP_
