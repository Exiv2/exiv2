// ***************************************************************** -*- C++ -*-
/*
 * Copyright (c) 2004 Andreas Huggel. All rights reserved.
 * 
 * Todo: Insert license blabla here
 *
 */
/*!
  @file    tags.h
  @brief   %Exif tag and type information
  @version $Name:  $ $Revision: 1.2 $
  @author  Andreas Huggel (ahu)
  @date    15-Jan-03, ahu: created
 */
#ifndef _TAGS_H_
#define _TAGS_H_

// *****************************************************************************
// included header files

// + standard includes

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

    //! Type to specify the IFD to which a metadata belongs
    enum IfdId { IfdIdNotSet, 
                 ifd0, exifIfd, gpsIfd, exifMakerIfd, exifIopIfd, 
                 ifd1, ifd1ExifIfd, ifd1GpsIfd, ifd1MakerIfd, ifd1ExifIopIfd };

    //! Section identifiers to logically group tags 
    enum SectionId { SectionIdNotSet, 
                     ifd0Tiff, exifIfdSection, gpsIfdSection, 
                     exifIopIfdSection, ifd1Section };

// *****************************************************************************
// class definitions

    //! Contains information pertaining to one IFD
    struct IfdInfo {
        //! Constructor
        IfdInfo(IfdId ifdId, const char* name);
        IfdId ifdId_;                           //!< IFD id
        const char* name_;                      //!< IFD name
    };

    //! Contains information pertaining to one section
    struct SectionInfo {
        //! Constructor
        SectionInfo(SectionId sectionId, const char* name);
        SectionId sectionId_;                   //!< Section id
        const char* name_;                      //!< Section name
    };

    //! Description of the format of a metadatum
    struct TagFormat {
        //! Constructor
        TagFormat(uint16 type, const char* name, long size);
        uint16 type_;                           //!< Format type id
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
        static const char* typeName(uint16 type);
        //! Returns the size in bytes of one element of this type
        static long typeSize(uint16 type);
        //! Returns the name of the IFD
        static const char* ifdName(IfdId ifdId);
        //! Returns the name of the section
        static const char* sectionName(SectionId sectionId);
        //! Returns the name of the section
        static const char* sectionName(uint16 tag, IfdId ifdId);

    private:
        static int tagInfoIdx(uint16 tag, IfdId ifdId);

        static const IfdInfo     ifdInfo_[];
        static const SectionInfo sectionInfo_[];
        static const TagFormat   tagFormat_[];
        static const TagInfo     tagInfo_[];
    };

// *****************************************************************************
// free functions

}                                       // namespace Exif

#endif                                  // #ifndef _TAGS_H_
