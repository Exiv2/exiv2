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
/*
  File:      tags.cpp
  Version:   $Name:  $ $Revision: 1.8 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Jan-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.8 $ $RCSfile: tags.cpp,v $")

// *****************************************************************************
// included header files
#include "tags.hpp"

#include <iostream>

// *****************************************************************************
// class member definitions
namespace Exif {

    IfdInfo::IfdInfo(IfdId ifdId, const char* name, const char* item)
        : ifdId_(ifdId), name_(name), item_(item)
    {
    }

    const IfdInfo ExifTags::ifdInfo_[] = {
        IfdInfo(ifdIdNotSet, "(Unknown IFD)", "(Unknown data area)"),
        IfdInfo(ifd0, "IFD0", "Image"),
        IfdInfo(exifIfd, "Exif", "Image"),
        IfdInfo(gpsIfd, "GPSInfo", "Image"),
        IfdInfo(makerIfd, "MakerNote", "Image"),
        IfdInfo(iopIfd, "Iop", "Image"),
        IfdInfo(ifd1, "IFD1", "Thumbnail"),
        IfdInfo(ifd1ExifIfd, "Exif", "Thumbnail"),
        IfdInfo(ifd1GpsIfd, "GPSInfo", "Thumbnail"),
        IfdInfo(ifd1MakerIfd, "MakerNote", "Thumbnail"),
        IfdInfo(ifd1IopIfd, "Iop", "Thumbnail"),
        IfdInfo(lastIfdId, "(Last IFD info)", "(Last IFD info)")
    };

    SectionInfo::SectionInfo(
        SectionId sectionId,
        const char* name,
        const char* desc
    )
        : sectionId_(sectionId), name_(name), desc_(desc)
    {
    }

    const SectionInfo ExifTags::sectionInfo_[] = {
        SectionInfo(sectionIdNotSet, "(UnknownSection)", "Unknown section"),
        SectionInfo(imgStruct, "ImageStructure", "Image data structure"),
        SectionInfo(recOffset, "RecordingOffset", "Recording offset"),
        SectionInfo(imgCharacter, "ImageCharacteristics", "Image data characteristics"),
        SectionInfo(otherTags, "OtherTags", "Other data"),
        SectionInfo(exifFormat, "ExifFormat", "Exif data structure"),
        SectionInfo(exifVersion, "ExifVersion", "Exif Version"),
        SectionInfo(imgConfig, "ImageConfig", "Image configuration"),
        SectionInfo(userInfo, "UserInfo", "User information"),
        SectionInfo(relatedFile, "RelatedFile", "Related file"),
        SectionInfo(dateTime, "DateTime", "Date and time"),
        SectionInfo(captureCond, "CaptureConditions", "Picture taking conditions"),
        SectionInfo(gpsTags, "GPS", "GPS information"),
        SectionInfo(iopTags, "Interoperability", "Interoperability information"),
        SectionInfo(lastSectionId, "(LastSection)", "Last section")
    };

    TagFormat::TagFormat(TypeId typeId, const char* name, long size)
        : typeId_(typeId), name_(name), size_(size)
    {
    }

    //! Lookup list of IFD tag data formats and their properties
    const TagFormat ExifTags::tagFormat_[] = {
        TagFormat(invalid,          "invalid",           0),
        TagFormat(unsignedByte,     "unsigned byte",     1),
        TagFormat(asciiString,      "ascii strings",     1),
        TagFormat(unsignedShort,    "unsigned short",    2),
        TagFormat(unsignedLong,     "unsigned long",     4),
        TagFormat(unsignedRational, "unsigned rational", 8),
        TagFormat(invalid6,         "invalid (6)",       1),
        TagFormat(undefined,        "undefined",         1),
        TagFormat(signedShort,      "signed short",      2),
        TagFormat(signedLong,       "signed long",       4),
        TagFormat(signedRational,   "signed rational",   8)
    };

    TagInfo::TagInfo(
        uint16 tag, 
        const char* name, 
        const char* desc, 
        IfdId ifdId, 
        SectionId sectionId
    )
        : tag_(tag), name_(name), desc_(desc), 
          ifdId_(ifdId), sectionId_(sectionId)
    {
    }

    // Base IFD Tags (IFD0 and IFD1)
    static const TagInfo ifdTagInfo[] = {
        TagInfo(0x0100, "ImageWidth", "Image width", ifd0, imgStruct),
        TagInfo(0x0101, "ImageLength", "Image height", ifd0, imgStruct),
        TagInfo(0x0102, "BitsPerSample", "Number of bits per component", ifd0, imgStruct),
        TagInfo(0x0103, "Compression", "Compression scheme", ifd0, imgStruct),
        TagInfo(0x0106, "PhotometricInterpretation", "Pixel composition", ifd0, imgStruct),
        TagInfo(0x010e, "ImageDescription", "Image title", ifd0, otherTags),
        TagInfo(0x010f, "Make", "Manufacturer of image input equipment", ifd0, otherTags),
        TagInfo(0x0110, "Model", "Model of image input equipment", ifd0, otherTags),
        TagInfo(0x0111, "StripOffsets", "Image data location", ifd0, recOffset),
        TagInfo(0x0112, "Orientation", "Orientation of image", ifd0, imgStruct),
        TagInfo(0x0115, "SamplesPerPixel", "Number of components", ifd0, imgStruct),
        TagInfo(0x0116, "RowsPerStrip", "Number of rows per strip", ifd0, recOffset),
        TagInfo(0x0117, "StripByteCounts", "Bytes per compressed strip", ifd0, recOffset),
        TagInfo(0x011a, "XResolution", "Image resolution in width direction", ifd0, imgStruct),
        TagInfo(0x011b, "YResolution", "Image resolution in height direction", ifd0, imgStruct),
        TagInfo(0x011c, "PlanarConfiguration", "Image data arrangement", ifd0, imgStruct),
        TagInfo(0x0128, "ResolutionUnit", "Unit of X and Y resolution", ifd0, imgStruct),
        TagInfo(0x012d, "TransferFunction", "Transfer function", ifd0, imgCharacter),
        TagInfo(0x0131, "Software", "Software used", ifd0, otherTags),
        TagInfo(0x0132, "DateTime", "File change date and time", ifd0, otherTags),
        TagInfo(0x013b, "Artist", "Person who created the image", ifd0, otherTags),
        TagInfo(0x013e, "WhitePoint", "White point chromaticity", ifd0, imgCharacter),
        TagInfo(0x013f, "PrimaryChromaticities", "Chromaticities of primaries", ifd0, imgCharacter),
        TagInfo(0x0201, "JPEGInterchangeFormat", "Offset to JPEG SOI", ifd0, recOffset),
        TagInfo(0x0202, "JPEGInterchangeFormatLength", "Bytes of JPEG data", ifd0, recOffset),
        TagInfo(0x0211, "YCbCrCoefficients", "Color space transformation matrix coefficients", ifd0, imgCharacter),
        TagInfo(0x0212, "YCbCrSubSampling", "Subsampling ratio of Y to C", ifd0, imgStruct),
        TagInfo(0x0213, "YCbCrPositioning", "Y and C positioning", ifd0, imgStruct),
        TagInfo(0x0214, "ReferenceBlackWhite", "Pair of black and white reference values", ifd0, imgCharacter),
        TagInfo(0x8298, "Copyright", "Copyright holder", ifd0, otherTags),
        TagInfo(0x8769, "ExifTag", "Exif IFD Pointer", ifd0, exifFormat),
        TagInfo(0x8825, "GPSTag", "GPSInfo IFD Pointer", ifd0, exifFormat),
        // End of list marker
        TagInfo(0xffff, "(UnknownIfdTag)", "Unknown IFD tag", ifdIdNotSet, sectionIdNotSet)
    };

    // Exif IFD Tags
    static const TagInfo exifTagInfo[] = {
        TagInfo(0x829a, "ExposureTime", "Exposure time", exifIfd, captureCond),
        TagInfo(0x829d, "FNumber", "F number", exifIfd, captureCond),
        TagInfo(0x8822, "ExposureProgram", "Exposure program", exifIfd, captureCond),
        TagInfo(0x8824, "SpectralSensitivity", "Spectral sensitivity", exifIfd, captureCond),
        TagInfo(0x8827, "ISOSpeedRatings", "ISO speed ratings", exifIfd, captureCond),
        TagInfo(0x8828, "OECF", "Optoelectric coefficient", exifIfd, captureCond),
        TagInfo(0x9000, "ExifVersion", "Exif Version", exifIfd, exifVersion),
        TagInfo(0x9003, "DateTimeOriginal", "Date and time original image was generated", exifIfd, dateTime),
        TagInfo(0x9004, "DateTimeDigitized", "Date and time image was made digital data", exifIfd, dateTime),
        TagInfo(0x9101, "ComponentsConfiguration", "Meaning of each component", exifIfd, imgConfig),
        TagInfo(0x9102, "CompressedBitsPerPixel", "Image compression mode", exifIfd, imgConfig),
        TagInfo(0x9201, "ShutterSpeedValue", "Shutter speed", exifIfd, captureCond),
        TagInfo(0x9202, "ApertureValue", "Aperture", exifIfd, captureCond),
        TagInfo(0x9203, "BrightnessValue", "Brightness", exifIfd, captureCond),
        TagInfo(0x9204, "ExposureBiasValue", "Exposure bias", exifIfd, captureCond),
        TagInfo(0x9205, "MaxApertureValue", "Maximum lens aperture", exifIfd, captureCond),
        TagInfo(0x9206, "SubjectDistance", "Subject distance", exifIfd, captureCond),
        TagInfo(0x9207, "MeteringMode", "Metering mode", exifIfd, captureCond),
        TagInfo(0x9208, "LightSource", "Light source", exifIfd, captureCond),
        TagInfo(0x9209, "Flash", "Flash", exifIfd, captureCond),
        TagInfo(0x920a, "FocalLength", "Lens focal length", exifIfd, captureCond),
        TagInfo(0x9214, "SubjectArea", "Subject area", exifIfd, captureCond),
        TagInfo(0x927c, "MakerNote", "Manufacturer notes", exifIfd, userInfo),
        TagInfo(0x9286, "UserComment", "User comments", exifIfd, userInfo),
        TagInfo(0x9290, "SubSecTime", "DateTime subseconds", exifIfd, dateTime),
        TagInfo(0x9291, "SubSecTimeOriginal", "DateTimeOriginal subseconds", exifIfd, dateTime),
        TagInfo(0x9292, "SubSecTimeDigitized", "DateTimeDigitized subseconds", exifIfd, dateTime),
        TagInfo(0xa000, "FlashpixVersion", "Supported Flashpix version", exifIfd, exifVersion),
        TagInfo(0xa001, "ColorSpace", "Color space information", exifIfd, imgCharacter),
        TagInfo(0xa002, "PixelXDimension", "Valid image width", exifIfd, imgConfig),
        TagInfo(0xa003, "PixelYDimension", "Valid image height", exifIfd, imgConfig),
        TagInfo(0xa004, "RelatedSoundFile", "Related audio file", exifIfd, relatedFile),
        TagInfo(0xa005, "InteroperabilityTag", "Interoperability IFD Pointer", exifIfd, exifFormat),
        TagInfo(0xa20b, "FlashEnergy", "Flash energy", exifIfd, captureCond),
        TagInfo(0xa20c, "SpatialFrequencyResponse", "Spatial frequency response", exifIfd, captureCond),
        TagInfo(0xa20e, "FocalPlaneXResolution", "Focal plane X resolution", exifIfd, captureCond),
        TagInfo(0xa20f, "FocalPlaneYResolution", "Focal plane Y resolution", exifIfd, captureCond),
        TagInfo(0xa210, "FocalPlaneResolutionUnit", "Focal plane resolution unit", exifIfd, captureCond),
        TagInfo(0xa214, "SubjectLocation", "Subject location", exifIfd, captureCond),
        TagInfo(0xa215, "ExposureIndex", "Exposure index", exifIfd, captureCond),
        TagInfo(0xa217, "SensingMethod", "Sensing method", exifIfd, captureCond),
        TagInfo(0xa300, "FileSource", "File source", exifIfd, captureCond),
        TagInfo(0xa301, "SceneType", "Scene type", exifIfd, captureCond),
        TagInfo(0xa302, "CFAPattern", "CFA pattern", exifIfd, captureCond),
        TagInfo(0xa401, "CustomRendered", "Custom image processing", exifIfd, captureCond),
        TagInfo(0xa402, "ExposureMode", "Exposure mode", exifIfd, captureCond),
        TagInfo(0xa403, "WhiteBalance", "White balance", exifIfd, captureCond),
        TagInfo(0xa404, "DigitalZoomRatio", "Digital zoom ratio", exifIfd, captureCond),
        TagInfo(0xa405, "FocalLengthIn35mmFilm", "Focal length in 35 mm film", exifIfd, captureCond),
        TagInfo(0xa406, "SceneCaptureType", "Scene capture type", exifIfd, captureCond),
        TagInfo(0xa407, "GainControl", "Gain control", exifIfd, captureCond),
        TagInfo(0xa408, "Contrast", "Contrast", exifIfd, captureCond),
        TagInfo(0xa409, "Saturation", "Saturation", exifIfd, captureCond),
        TagInfo(0xa40a, "Sharpness", "Sharpness", exifIfd, captureCond),
        TagInfo(0xa40b, "DeviceSettingDescription", "Device settings description", exifIfd, captureCond),
        TagInfo(0xa40c, "SubjectDistanceRange", "Subject distance range", exifIfd, captureCond),
        TagInfo(0xa420, "ImageUniqueID", "Unique image ID", exifIfd, otherTags),
        // End of list marker
        TagInfo(0xffff, "(UnknownExifTag)", "Unknown Exif tag", ifdIdNotSet, sectionIdNotSet)
    };

    // GPS Info Tags
    static const TagInfo gpsTagInfo[] = {
        TagInfo(0x0000, "GPSVersionID", "GPS tag version", gpsIfd, gpsTags),
        TagInfo(0x0001, "GPSLatitudeRef", "North or South Latitude", gpsIfd, gpsTags),
        TagInfo(0x0002, "GPSLatitude", "Latitude", gpsIfd, gpsTags),
        TagInfo(0x0003, "GPSLongitudeRef", "East or West Longitude", gpsIfd, gpsTags),
        TagInfo(0x0004, "GPSLongitude", "Longitude", gpsIfd, gpsTags),
        TagInfo(0x0005, "GPSAltitudeRef", "Altitude reference", gpsIfd, gpsTags),
        TagInfo(0x0006, "GPSAltitude", "Altitude", gpsIfd, gpsTags),
        TagInfo(0x0007, "GPSTimeStamp", "GPS time (atomic clock)", gpsIfd, gpsTags),
        TagInfo(0x0008, "GPSSatellites", "GPS satellites used for measurement", gpsIfd, gpsTags),
        TagInfo(0x0009, "GPSStatus", "GPS receiver status", gpsIfd, gpsTags),
        TagInfo(0x000a, "GPSMeasureMode", "GPS measurement mode", gpsIfd, gpsTags),
        TagInfo(0x000b, "GPSDOP", "Measurement precision", gpsIfd, gpsTags),
        TagInfo(0x000c, "GPSSpeedRef", "Speed unit", gpsIfd, gpsTags),
        TagInfo(0x000d, "GPSSpeed", "Speed of GPS receiver", gpsIfd, gpsTags),
        TagInfo(0x000e, "GPSTrackRef", "Reference for direction of movement", gpsIfd, gpsTags),
        TagInfo(0x000f, "GPSTrack", "Direction of movement", gpsIfd, gpsTags),
        TagInfo(0x0010, "GPSImgDirectionRef", "Reference for direction of image", gpsIfd, gpsTags),
        TagInfo(0x0011, "GPSImgDirection", "Direction of image", gpsIfd, gpsTags),
        TagInfo(0x0012, "GPSMapDatum", "Geodetic survey data used", gpsIfd, gpsTags),
        TagInfo(0x0013, "GPSDestLatitudeRef", "Reference for latitude of destination", gpsIfd, gpsTags),
        TagInfo(0x0014, "GPSDestLatitude", "Latitude of destination", gpsIfd, gpsTags),
        TagInfo(0x0015, "GPSDestLongitudeRef", "Reference for longitude of destination", gpsIfd, gpsTags),
        TagInfo(0x0016, "GPSDestLongitude", "Longitude of destination", gpsIfd, gpsTags),
        TagInfo(0x0017, "GPSDestBearingRef", "Reference for bearing of destination", gpsIfd, gpsTags),
        TagInfo(0x0018, "GPSDestBearing", "Bearing of destination", gpsIfd, gpsTags),
        TagInfo(0x0019, "GPSDestDistanceRef", "Reference for distance to destination", gpsIfd, gpsTags),
        TagInfo(0x001a, "GPSDestDistance", "Distance to destination", gpsIfd, gpsTags),
        TagInfo(0x001b, "GPSProcessingMethod", "Name of GPS processing method", gpsIfd, gpsTags),
        TagInfo(0x001c, "GPSAreaInformation", "Name of GPS area", gpsIfd, gpsTags),
        TagInfo(0x001d, "GPSDateStamp", "GPS date", gpsIfd, gpsTags),
        TagInfo(0x001e, "GPSDifferential", "GPS differential correction", gpsIfd, gpsTags),
        // End of list marker
        TagInfo(0xffff, "(UnknownGpsTag)", "Unknown GPSInfo tag", ifdIdNotSet, sectionIdNotSet)
    };
    
    // Exif Interoperability IFD Tags
    static const TagInfo iopTagInfo[] = {
        TagInfo(0x0001, "InteroperabilityIndex", "Interoperability Identification", iopIfd, iopTags),
        TagInfo(0x0002, "InteroperabilityVersion", "Interoperability version", iopIfd, iopTags),
        TagInfo(0x1000, "RelatedImageFileFormat", "File format of image file", iopIfd, iopTags),
        TagInfo(0x1001, "RelatedImageWidth", "Image width", iopIfd, iopTags),
        TagInfo(0x1002, "RelatedImageLength", "Image height", iopIfd, iopTags),
        // End of list marker
        TagInfo(0xffff, "(UnknownIopTag)", "Unknown Exif Interoperability tag", ifdIdNotSet, sectionIdNotSet)
    };

    // Tag lookup lists with tag names, desc and where they (preferably) belong to;
    // this is an array with pointers to one list per IFD. The IfdId is used as the
    // index into the array.
    const TagInfo* ExifTags::tagInfos_[] = {
        0, 
        ifdTagInfo, exifTagInfo, gpsTagInfo, 0, iopTagInfo,
        ifdTagInfo, exifTagInfo, gpsTagInfo, 0, iopTagInfo
    };

    int ExifTags::tagInfoIdx(uint16 tag, IfdId ifdId)
    {
	const TagInfo* tagInfo = tagInfos_[ifdId];
        if (tagInfo == 0) return -1;
        int idx;
        for (idx = 0; tagInfo[idx].tag_ != 0xffff; ++idx) {
            if (tagInfo[idx].tag_ == tag) break;
        }
        return idx;
    }

    int ExifTags::tagInfoIdx(const std::string& tagName, IfdId ifdId)
    {
	const TagInfo* tagInfo = tagInfos_[ifdId];
        if (tagInfo == 0) return -1;
        int idx;
        for (idx = 0; tagInfo[idx].tag_ != 0xffff; ++idx) {
            if (tagInfo[idx].name_ == tagName) break;
        }
        return idx;
    }

    const char* ExifTags::tagName(uint16 tag, IfdId ifdId)
    {
        int idx = tagInfoIdx(tag, ifdId);
        if (idx == -1) throw Error("No taginfo for IFD");
        return tagInfos_[ifdId][idx].name_;
    }

    const char* ExifTags::sectionName(uint16 tag, IfdId ifdId)
    {
        int idx = tagInfoIdx(tag, ifdId);
        if (idx == -1) throw Error("No taginfo for IFD");
	const TagInfo* tagInfo = tagInfos_[ifdId];
        return sectionInfo_[tagInfo[idx].sectionId_].name_;
    }

    const char* ExifTags::typeName(TypeId typeId)
    {
        return tagFormat_[typeId].name_;
    }

    long ExifTags::typeSize(TypeId typeId)
    {
        return tagFormat_[typeId].size_;
    }

    const char* ExifTags::ifdName(IfdId ifdId)
    {
        return ifdInfo_[ifdId].name_;
    }

    const char* ExifTags::ifdItem(IfdId ifdId)
    {
        return ifdInfo_[ifdId].item_;
    }

    const char* ExifTags::sectionName(SectionId sectionId)
    {
        return sectionInfo_[sectionId].name_;
    }

    SectionId ExifTags::sectionId(const std::string& sectionName)
    {
        int i;
        for (i = int(lastSectionId) - 1; i > 0; --i) {
            if (sectionInfo_[i].name_ == sectionName) break;
        }
        return SectionId(i);
    }

    // The uniqueness that we promise in this 'database lookup' function
    // holds only implicitely. The function returns the first match that
    // we find, it doesn't verify the uniqueness.
    std::pair<IfdId, uint16> ExifTags::ifdAndTag(const std::string& ifdItem, 
                                                 const std::string& sectionName, 
                                                 const std::string& tagName)
    {
        IfdId ifdId = ifdIdNotSet;
        uint16 tag = 0xffff;
        
        SectionId s = sectionId(sectionName);
        if (s == sectionIdNotSet) return std::make_pair(ifdId, tag);

        for (int i = 0; i < lastIfdId; ++i) {
            if (ifdInfo_[i].item_ == ifdItem) {
                ifdId = ifdInfo_[i].ifdId_;
                int k = tagInfoIdx(tagName, ifdId);
                if (k != -1 && tagInfos_[ifdId][k].sectionId_ == s) {
                    tag = tagInfos_[ifdId][k].tag_;
                    break;
                }
            }
        }
        return std::make_pair(ifdId, tag);
    }

    // *************************************************************************
    // free functions

    std::ostream& operator<<(std::ostream& os, const Rational& r) 
    {
        return os << r.first << "/" << r.second;
    }

    std::istream& operator>>(std::istream& is, Rational& r) 
    { 
        int32 nominator;
        int32 denominator;
        char c;
        is >> nominator >> c >> denominator; 
        if (is && c == '/') r = std::make_pair(nominator, denominator);
        return is;
    }

    std::ostream& operator<<(std::ostream& os, const URational& r) 
    { 
        return os << r.first << "/" << r.second;
    }

    std::istream& operator>>(std::istream& is, URational& r) 
    {
        uint32 nominator;
        uint32 denominator;
        char c;
        is >> nominator >> c >> denominator; 
        if (is && c == '/') r = std::make_pair(nominator, denominator);
        return is;
    }

}                                       // namespace Exif
