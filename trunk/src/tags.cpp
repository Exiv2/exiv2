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
  Version:   $Name:  $ $Revision: 1.26 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Jan-04, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.26 $ $RCSfile: tags.cpp,v $")

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "error.hpp"
#include "types.hpp"
#include "value.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cstdlib>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    IfdInfo::IfdInfo(IfdId ifdId, const char* name, const char* item)
        : ifdId_(ifdId), name_(name), item_(item)
    {
    }

    const IfdInfo ExifTags::ifdInfo_[] = {
        IfdInfo(ifdIdNotSet, "(Unknown IFD)", "(Unknown data area)"),
        IfdInfo(ifd0, "IFD0", "Image"),
        IfdInfo(exifIfd, "Exif", "Image"),
        IfdInfo(gpsIfd, "GPSInfo", "Image"),
        IfdInfo(makerIfd, "Makernote", "Makernote"),
        IfdInfo(iopIfd, "Iop", "Image"),
        IfdInfo(ifd1, "IFD1", "Thumbnail"),
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

    TagInfo::TagInfo(
        uint16 tag, 
        const char* name, 
        const char* desc, 
        IfdId ifdId, 
        SectionId sectionId,
        PrintFct printFct        
    )
        : tag_(tag), name_(name), desc_(desc), ifdId_(ifdId),
          sectionId_(sectionId), printFct_(printFct)
    {
    }

    // Base IFD Tags (IFD0 and IFD1)
    static const TagInfo ifdTagInfo[] = {
        TagInfo(0x0100, "ImageWidth", "Image width", ifd0, imgStruct, printValue),
        TagInfo(0x0101, "ImageLength", "Image height", ifd0, imgStruct, printValue),
        TagInfo(0x0102, "BitsPerSample", "Number of bits per component", ifd0, imgStruct, printValue),
        TagInfo(0x0103, "Compression", "Compression scheme", ifd0, imgStruct, print0x0103),
        TagInfo(0x0106, "PhotometricInterpretation", "Pixel composition", ifd0, imgStruct, print0x0106),
        TagInfo(0x010e, "ImageDescription", "Image title", ifd0, otherTags, printValue),
        TagInfo(0x010f, "Make", "Manufacturer of image input equipment", ifd0, otherTags, printValue),
        TagInfo(0x0110, "Model", "Model of image input equipment", ifd0, otherTags, printValue),
        TagInfo(0x0111, "StripOffsets", "Image data location", ifd0, recOffset, printValue),
        TagInfo(0x0112, "Orientation", "Orientation of image", ifd0, imgStruct, print0x0112),
        TagInfo(0x0115, "SamplesPerPixel", "Number of components", ifd0, imgStruct, printValue),
        TagInfo(0x0116, "RowsPerStrip", "Number of rows per strip", ifd0, recOffset, printValue),
        TagInfo(0x0117, "StripByteCounts", "Bytes per compressed strip", ifd0, recOffset, printValue),
        TagInfo(0x011a, "XResolution", "Image resolution in width direction", ifd0, imgStruct, printLong),
        TagInfo(0x011b, "YResolution", "Image resolution in height direction", ifd0, imgStruct, printLong),
        TagInfo(0x011c, "PlanarConfiguration", "Image data arrangement", ifd0, imgStruct, printValue),
        TagInfo(0x0128, "ResolutionUnit", "Unit of X and Y resolution", ifd0, imgStruct, printUnit),
        TagInfo(0x012d, "TransferFunction", "Transfer function", ifd0, imgCharacter, printValue),
        TagInfo(0x0131, "Software", "Software used", ifd0, otherTags, printValue),
        TagInfo(0x0132, "DateTime", "File change date and time", ifd0, otherTags, printValue),
        TagInfo(0x013b, "Artist", "Person who created the image", ifd0, otherTags, printValue),
        TagInfo(0x013e, "WhitePoint", "White point chromaticity", ifd0, imgCharacter, printValue),
        TagInfo(0x013f, "PrimaryChromaticities", "Chromaticities of primaries", ifd0, imgCharacter, printValue),
        TagInfo(0x0201, "JPEGInterchangeFormat", "Offset to JPEG SOI", ifd0, recOffset, printValue),
        TagInfo(0x0202, "JPEGInterchangeFormatLength", "Bytes of JPEG data", ifd0, recOffset, printValue),
        TagInfo(0x0211, "YCbCrCoefficients", "Color space transformation matrix coefficients", ifd0, imgCharacter, printValue),
        TagInfo(0x0212, "YCbCrSubSampling", "Subsampling ratio of Y to C", ifd0, imgStruct, printValue),
        TagInfo(0x0213, "YCbCrPositioning", "Y and C positioning", ifd0, imgStruct, print0x0213),
        TagInfo(0x0214, "ReferenceBlackWhite", "Pair of black and white reference values", ifd0, imgCharacter, printValue),
        TagInfo(0x8298, "Copyright", "Copyright holder", ifd0, otherTags, print0x8298),
        TagInfo(0x8769, "ExifTag", "Exif IFD Pointer", ifd0, exifFormat, printValue),
        TagInfo(0x8825, "GPSTag", "GPSInfo IFD Pointer", ifd0, exifFormat, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownIfdTag)", "Unknown IFD tag", ifdIdNotSet, sectionIdNotSet, printValue)
    };

    // Exif IFD Tags
    static const TagInfo exifTagInfo[] = {
        TagInfo(0x829a, "ExposureTime", "Exposure time", exifIfd, captureCond, print0x829a),
        TagInfo(0x829d, "FNumber", "F number", exifIfd, captureCond, print0x829d),
        TagInfo(0x8822, "ExposureProgram", "Exposure program", exifIfd, captureCond, print0x8822),
        TagInfo(0x8824, "SpectralSensitivity", "Spectral sensitivity", exifIfd, captureCond, printValue),
        TagInfo(0x8827, "ISOSpeedRatings", "ISO speed ratings", exifIfd, captureCond, print0x8827),
        TagInfo(0x8828, "OECF", "Optoelectric coefficient", exifIfd, captureCond, printValue),
        TagInfo(0x9000, "ExifVersion", "Exif Version", exifIfd, exifVersion, printValue),
        TagInfo(0x9003, "DateTimeOriginal", "Date and time original image was generated", exifIfd, dateTime, printValue),
        TagInfo(0x9004, "DateTimeDigitized", "Date and time image was made digital data", exifIfd, dateTime, printValue),
        TagInfo(0x9101, "ComponentsConfiguration", "Meaning of each component", exifIfd, imgConfig, print0x9101),
        TagInfo(0x9102, "CompressedBitsPerPixel", "Image compression mode", exifIfd, imgConfig, printFloat),
        TagInfo(0x9201, "ShutterSpeedValue", "Shutter speed", exifIfd, captureCond, printFloat),
        TagInfo(0x9202, "ApertureValue", "Aperture", exifIfd, captureCond, printFloat),
        TagInfo(0x9203, "BrightnessValue", "Brightness", exifIfd, captureCond, printFloat),
        TagInfo(0x9204, "ExposureBiasValue", "Exposure bias", exifIfd, captureCond, print0x9204),
        TagInfo(0x9205, "MaxApertureValue", "Maximum lens aperture", exifIfd, captureCond, printFloat),
        TagInfo(0x9206, "SubjectDistance", "Subject distance", exifIfd, captureCond, print0x9206),
        TagInfo(0x9207, "MeteringMode", "Metering mode", exifIfd, captureCond, print0x9207),
        TagInfo(0x9208, "LightSource", "Light source", exifIfd, captureCond, print0x9208),
        TagInfo(0x9209, "Flash", "Flash", exifIfd, captureCond, print0x9209),
        TagInfo(0x920a, "FocalLength", "Lens focal length", exifIfd, captureCond, print0x920a),
        TagInfo(0x9214, "SubjectArea", "Subject area", exifIfd, captureCond, printValue),
        TagInfo(0x927c, "MakerNote", "Manufacturer notes", exifIfd, userInfo, printValue),
        TagInfo(0x9286, "UserComment", "User comments", exifIfd, userInfo, print0x9286),
        TagInfo(0x9290, "SubSecTime", "DateTime subseconds", exifIfd, dateTime, printValue),
        TagInfo(0x9291, "SubSecTimeOriginal", "DateTimeOriginal subseconds", exifIfd, dateTime, printValue),
        TagInfo(0x9292, "SubSecTimeDigitized", "DateTimeDigitized subseconds", exifIfd, dateTime, printValue),
        TagInfo(0xa000, "FlashpixVersion", "Supported Flashpix version", exifIfd, exifVersion, printValue),
        TagInfo(0xa001, "ColorSpace", "Color space information", exifIfd, imgCharacter, print0xa001),
        TagInfo(0xa002, "PixelXDimension", "Valid image width", exifIfd, imgConfig, printValue),
        TagInfo(0xa003, "PixelYDimension", "Valid image height", exifIfd, imgConfig, printValue),
        TagInfo(0xa004, "RelatedSoundFile", "Related audio file", exifIfd, relatedFile, printValue),
        TagInfo(0xa005, "InteroperabilityTag", "Interoperability IFD Pointer", exifIfd, exifFormat, printValue),
        TagInfo(0xa20b, "FlashEnergy", "Flash energy", exifIfd, captureCond, printValue),
        TagInfo(0xa20c, "SpatialFrequencyResponse", "Spatial frequency response", exifIfd, captureCond, printValue),
        TagInfo(0xa20e, "FocalPlaneXResolution", "Focal plane X resolution", exifIfd, captureCond, printFloat),
        TagInfo(0xa20f, "FocalPlaneYResolution", "Focal plane Y resolution", exifIfd, captureCond, printFloat),
        TagInfo(0xa210, "FocalPlaneResolutionUnit", "Focal plane resolution unit", exifIfd, captureCond, printUnit),
        TagInfo(0xa214, "SubjectLocation", "Subject location", exifIfd, captureCond, printValue),
        TagInfo(0xa215, "ExposureIndex", "Exposure index", exifIfd, captureCond, printValue),
        TagInfo(0xa217, "SensingMethod", "Sensing method", exifIfd, captureCond, print0xa217),
        TagInfo(0xa300, "FileSource", "File source", exifIfd, captureCond, print0xa300),
        TagInfo(0xa301, "SceneType", "Scene type", exifIfd, captureCond, print0xa301),
        TagInfo(0xa302, "CFAPattern", "CFA pattern", exifIfd, captureCond, printValue),
        TagInfo(0xa401, "CustomRendered", "Custom image processing", exifIfd, captureCond, printValue),
        TagInfo(0xa402, "ExposureMode", "Exposure mode", exifIfd, captureCond, print0xa402),
        TagInfo(0xa403, "WhiteBalance", "White balance", exifIfd, captureCond, print0xa403),
        TagInfo(0xa404, "DigitalZoomRatio", "Digital zoom ratio", exifIfd, captureCond, print0xa404),
        TagInfo(0xa405, "FocalLengthIn35mmFilm", "Focal length in 35 mm film", exifIfd, captureCond, print0xa405),
        TagInfo(0xa406, "SceneCaptureType", "Scene capture type", exifIfd, captureCond, print0xa406),
        TagInfo(0xa407, "GainControl", "Gain control", exifIfd, captureCond, print0xa407),
        TagInfo(0xa408, "Contrast", "Contrast", exifIfd, captureCond, print0xa408),
        TagInfo(0xa409, "Saturation", "Saturation", exifIfd, captureCond, print0xa409),
        TagInfo(0xa40a, "Sharpness", "Sharpness", exifIfd, captureCond, print0xa40a),
        TagInfo(0xa40b, "DeviceSettingDescription", "Device settings description", exifIfd, captureCond, printValue),
        TagInfo(0xa40c, "SubjectDistanceRange", "Subject distance range", exifIfd, captureCond, print0xa40c),
        TagInfo(0xa420, "ImageUniqueID", "Unique image ID", exifIfd, otherTags, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownExifTag)", "Unknown Exif tag", ifdIdNotSet, sectionIdNotSet, printValue)
    };

    // GPS Info Tags
    static const TagInfo gpsTagInfo[] = {
        TagInfo(0x0000, "GPSVersionID", "GPS tag version", gpsIfd, gpsTags, printValue),
        TagInfo(0x0001, "GPSLatitudeRef", "North or South Latitude", gpsIfd, gpsTags, printValue),
        TagInfo(0x0002, "GPSLatitude", "Latitude", gpsIfd, gpsTags, printValue),
        TagInfo(0x0003, "GPSLongitudeRef", "East or West Longitude", gpsIfd, gpsTags, printValue),
        TagInfo(0x0004, "GPSLongitude", "Longitude", gpsIfd, gpsTags, printValue),
        TagInfo(0x0005, "GPSAltitudeRef", "Altitude reference", gpsIfd, gpsTags, printValue),
        TagInfo(0x0006, "GPSAltitude", "Altitude", gpsIfd, gpsTags, printValue),
        TagInfo(0x0007, "GPSTimeStamp", "GPS time (atomic clock)", gpsIfd, gpsTags, printValue),
        TagInfo(0x0008, "GPSSatellites", "GPS satellites used for measurement", gpsIfd, gpsTags, printValue),
        TagInfo(0x0009, "GPSStatus", "GPS receiver status", gpsIfd, gpsTags, printValue),
        TagInfo(0x000a, "GPSMeasureMode", "GPS measurement mode", gpsIfd, gpsTags, printValue),
        TagInfo(0x000b, "GPSDOP", "Measurement precision", gpsIfd, gpsTags, printValue),
        TagInfo(0x000c, "GPSSpeedRef", "Speed unit", gpsIfd, gpsTags, printValue),
        TagInfo(0x000d, "GPSSpeed", "Speed of GPS receiver", gpsIfd, gpsTags, printValue),
        TagInfo(0x000e, "GPSTrackRef", "Reference for direction of movement", gpsIfd, gpsTags, printValue),
        TagInfo(0x000f, "GPSTrack", "Direction of movement", gpsIfd, gpsTags, printValue),
        TagInfo(0x0010, "GPSImgDirectionRef", "Reference for direction of image", gpsIfd, gpsTags, printValue),
        TagInfo(0x0011, "GPSImgDirection", "Direction of image", gpsIfd, gpsTags, printValue),
        TagInfo(0x0012, "GPSMapDatum", "Geodetic survey data used", gpsIfd, gpsTags, printValue),
        TagInfo(0x0013, "GPSDestLatitudeRef", "Reference for latitude of destination", gpsIfd, gpsTags, printValue),
        TagInfo(0x0014, "GPSDestLatitude", "Latitude of destination", gpsIfd, gpsTags, printValue),
        TagInfo(0x0015, "GPSDestLongitudeRef", "Reference for longitude of destination", gpsIfd, gpsTags, printValue),
        TagInfo(0x0016, "GPSDestLongitude", "Longitude of destination", gpsIfd, gpsTags, printValue),
        TagInfo(0x0017, "GPSDestBearingRef", "Reference for bearing of destination", gpsIfd, gpsTags, printValue),
        TagInfo(0x0018, "GPSDestBearing", "Bearing of destination", gpsIfd, gpsTags, printValue),
        TagInfo(0x0019, "GPSDestDistanceRef", "Reference for distance to destination", gpsIfd, gpsTags, printValue),
        TagInfo(0x001a, "GPSDestDistance", "Distance to destination", gpsIfd, gpsTags, printValue),
        TagInfo(0x001b, "GPSProcessingMethod", "Name of GPS processing method", gpsIfd, gpsTags, printValue),
        TagInfo(0x001c, "GPSAreaInformation", "Name of GPS area", gpsIfd, gpsTags, printValue),
        TagInfo(0x001d, "GPSDateStamp", "GPS date", gpsIfd, gpsTags, printValue),
        TagInfo(0x001e, "GPSDifferential", "GPS differential correction", gpsIfd, gpsTags, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownGpsTag)", "Unknown GPSInfo tag", ifdIdNotSet, sectionIdNotSet, printValue)
    };
    
    // Exif Interoperability IFD Tags
    static const TagInfo iopTagInfo[] = {
        TagInfo(0x0001, "InteroperabilityIndex", "Interoperability Identification", iopIfd, iopTags, printValue),
        TagInfo(0x0002, "InteroperabilityVersion", "Interoperability version", iopIfd, iopTags, printValue),
        TagInfo(0x1000, "RelatedImageFileFormat", "File format of image file", iopIfd, iopTags, printValue),
        TagInfo(0x1001, "RelatedImageWidth", "Image width", iopIfd, iopTags, printValue),
        TagInfo(0x1002, "RelatedImageLength", "Image height", iopIfd, iopTags, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownIopTag)", "Unknown Exif Interoperability tag", ifdIdNotSet, sectionIdNotSet, printValue)
    };

    // MakerNote Tags
    static const TagInfo makerNoteTagInfo[] = {
        // End of list marker
        TagInfo(0xffff, "(UnknownMakerNoteTag)", "Unknown MakerNote tag", ifdIdNotSet, sectionIdNotSet, printValue)
    };

    // Tag lookup lists with tag names, desc and where they (preferably) belong to;
    // this is an array with pointers to one list per IFD. The IfdId is used as the
    // index into the array.
    const TagInfo* ExifTags::tagInfos_[] = {
        0, 
        ifdTagInfo, exifTagInfo, gpsTagInfo, makerNoteTagInfo, iopTagInfo, ifdTagInfo, 
        0
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

    const char* ExifTags::tagDesc(uint16 tag, IfdId ifdId)
    {
        int idx = tagInfoIdx(tag, ifdId);
        if (idx == -1) throw Error("No taginfo for IFD");
        return tagInfos_[ifdId][idx].desc_;
    }

    const char* ExifTags::sectionName(uint16 tag, IfdId ifdId)
    {
        int idx = tagInfoIdx(tag, ifdId);
        if (idx == -1) throw Error("No taginfo for IFD");
	const TagInfo* tagInfo = tagInfos_[ifdId];
        return sectionInfo_[tagInfo[idx].sectionId_].name_;
    }

    const char* ExifTags::sectionDesc(uint16 tag, IfdId ifdId)
    {
        int idx = tagInfoIdx(tag, ifdId);
        if (idx == -1) throw Error("No taginfo for IFD");
	const TagInfo* tagInfo = tagInfos_[ifdId];
        return sectionInfo_[tagInfo[idx].sectionId_].desc_;
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

    std::string ExifTags::makeKey(uint16 tag, IfdId ifdId)
    {
        return std::string(ifdItem(ifdId)) 
            + "." + std::string(sectionName(tag, ifdId)) 
            + "." + std::string(tagName(tag, ifdId));
    }

    // This 'database lookup' function returns the first match that
    // we find, it doesn't verify whether this is the only match.
    std::pair<uint16, IfdId> ExifTags::decomposeKey(const std::string& key)
    {
        // Get the IFD, section name and tag name parts of the key
        std::string::size_type pos1 = key.find('.');
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string ifdItem = key.substr(0, pos1);
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key.find('.', pos0);
        if (pos1 == std::string::npos) throw Error("Invalid key");
        std::string sectionName = key.substr(pos0, pos1 - pos0);
        pos0 = pos1 + 1;
        std::string tagName = key.substr(pos0);
        if (tagName == "") throw Error("Invalid key");

        // Check if this is a MakerNote key, stop processing if it is
        if (ifdItem == ifdInfo_[makerIfd].item_) {
            return std::make_pair(0xffff, makerIfd);
        }

        // Use the parts of the key to find tag and IFD id
        IfdId ifdId = ifdIdNotSet;
        uint16 tag = 0xffff;

        SectionId s = sectionId(sectionName);
        if (s == sectionIdNotSet) return std::make_pair(tag, ifdId);

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
        return std::make_pair(tag, ifdId);
    } // ExifTags::decomposeKey

    std::ostream& ExifTags::printTag(std::ostream& os,
                                     uint16 tag, 
                                     IfdId ifdId,
                                     const Value& value)
    {
        int idx = tagInfoIdx(tag, ifdId);
        if (idx == -1) throw Error("No taginfo for IFD");
        PrintFct fct = tagInfos_[ifdId][idx].printFct_;
        return fct(os, value);
    }

    void ExifTags::taglist(std::ostream& os)
    {
        for (int i=0; ifdTagInfo[i].tag_ != 0xffff; ++i) {
            os << ifdTagInfo[i] << "\n";
        }
        for (int i=0; exifTagInfo[i].tag_ != 0xffff; ++i) {
            os << exifTagInfo[i] << "\n";
        }
        for (int i=0; iopTagInfo[i].tag_ != 0xffff; ++i) {
            os << iopTagInfo[i] << "\n";
        }
        for (int i=0; gpsTagInfo[i].tag_ != 0xffff; ++i) {
            os << gpsTagInfo[i] << "\n";
        }
    } // ExifTags::taglist
    
    // *************************************************************************
    // free functions

    std::ostream& operator<<(std::ostream& os, const TagInfo& ti) 
    {
        return os << ExifTags::tagName(ti.tag_, ti.ifdId_) << ", "
                  << std::dec << ti.tag_ << ", "
                  << "0x" << std::setw(4) << std::setfill('0') 
                  << std::right << std::hex << ti.tag_ << ", "
                  << ExifTags::ifdName(ti.ifdId_) << ", "
                  << ExifTags::makeKey(ti.tag_, ti.ifdId_) << ", " 
                  << ExifTags::tagDesc(ti.tag_, ti.ifdId_);
    }

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

    std::ostream& printValue(std::ostream& os, const Value& value)
    {
        return os << value;
    }

    std::ostream& printLong(std::ostream& os, const Value& value)
    {
        return os << value.toLong();
    }

    std::ostream& printFloat(std::ostream& os, const Value& value)
    {
        Rational r = value.toRational();
        if (r.second != 0) return os << (float)r.first / r.second;
        return os << "(" << value << ")";
    } // printFloat

    std::ostream& printUnit(std::ostream& os, const Value& value)
    {
        long unit = value.toLong();
        switch (unit) {
        case 2:  os << "inch"; break;
        case 3:  os << "cm"; break;
        default: os << "(" << unit << ")"; break;
        }
        return os;
    }

    std::ostream& print0x0103(std::ostream& os, const Value& value)
    {
        long compression = value.toLong();
        switch (compression) {
        case 1:  os << "TIFF"; break;
        case 6:  os << "JPEG"; break;
        default: os << "(" << compression << ")"; break;
        }
        return os;
    }

    std::ostream& print0x0106(std::ostream& os, const Value& value)
    {
        long photo = value.toLong();
        switch (photo) {
        case 2:  os << "RGB"; break;
        case 6:  os << "YCbCr"; break;
        default: os << "(" << photo << ")"; break;
        }
        return os;
    }

    std::ostream& print0x0112(std::ostream& os, const Value& value)
    {
        long orientation = value.toLong();
        switch (orientation) {
        case 1:  os << "top, left"; break;
        case 2:  os << "top, right"; break;
        case 3:  os << "bottom, right"; break;
        case 4:  os << "bottom, left"; break;
        case 5:  os << "left, top"; break;
        case 6:  os << "right, top"; break;
        case 7:  os << "right, bottom"; break;
        case 8:  os << "left, bottom"; break;
        default: os << "(" << orientation << ")"; break;
        }
        return os;
    }

    std::ostream& print0x0213(std::ostream& os, const Value& value)
    {
        long position = value.toLong();
        switch (position) {
        case 1:  os << "Centered"; break;
        case 2:  os << "Co-sited"; break;
        default: os << "(" << position << ")"; break;
        }
        return os;
    }

    std::ostream& print0x8298(std::ostream& os, const Value& value)
    {
        // Print the copyright information in the format Photographer, Editor
        std::string val = value.toString();
        std::string::size_type pos = val.find('\0');
        if (pos != std::string::npos) {
            std::string photographer(val, 0, pos);
            if (photographer != " ") os << photographer;
            std::string editor(val, pos + 1);
            if (editor != "") {
                if (photographer != " ") os << ", ";
                os << editor;
            }
        }
        else {
            os << val;
        }
        return os;
    }

    std::ostream& print0x829a(std::ostream& os, const Value& value)
    {
        Rational t = value.toRational();
        if (t.first > 1 && t.second > 1 && t.second >= t.first) {
            t.second = static_cast<uint32>(
                static_cast<float>(t.second) / t.first + 0.5);
            t.first = 1;
        }
        if (t.second > 1 && t.second < t.first) {
            t.first = static_cast<uint32>(
                static_cast<float>(t.first) / t.second + 0.5);
            t.second = 1;
        }
        if (t.second == 1) {
            os << t.first << " s";
        }
        else {
            os << t.first << "/" << t.second << " s";
        }
        return os;
    }

    std::ostream& print0x829d(std::ostream& os, const Value& value)
    {
        Rational fnumber = value.toRational();
        if (fnumber.second != 0) {
            os << "F" << (float)fnumber.first / fnumber.second;
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& print0x8822(std::ostream& os, const Value& value)
    {
        long program = value.toLong();
        switch (program) {
        case 0:  os << "Not defined"; break;
        case 1:  os << "Manual"; break;
        case 2:  os << "Auto"; break;
        case 3:  os << "Aperture priority"; break;
        case 4:  os << "Shutter priority"; break;
        case 5:  os << "Creative program"; break;
        case 6:  os << "Action program"; break;
        case 7:  os << "Portrait mode"; break;
        case 8:  os << "Landscape mode"; break;
        default: os << "(" << program << ")"; break;
        }
        return os;
    }

    std::ostream& print0x8827(std::ostream& os, const Value& value)
    {
        return os << value.toLong();
    }

    std::ostream& print0x9101(std::ostream& os, const Value& value)
    {
        for (long i = 0; i < value.count(); ++i) {
            long l = value.toLong(i);
            switch (l) {
            case 0:  break;
            case 1:  os << "Y"; break;
            case 2:  os << "Cb"; break;
            case 3:  os << "Cr"; break;
            case 4:  os << "R"; break;
            case 5:  os << "G"; break;
            case 6:  os << "B"; break;
            default: os << "(" << l << ")"; break;
            }
        }
        return os;
    }

    std::ostream& print0x9204(std::ostream& os, const Value& value)
    {
        Rational bias = value.toRational();
        if (bias.second <= 0) {
            os << "(" << bias.first << "/" << bias.second << ")";
        }
        else if (bias.first == 0) {
            os << "0";
        }
        else {
            long d = lgcd(labs(bias.first), bias.second);
            long num = labs(bias.first) / d;
            long den = bias.second / d;
            os << (bias.first < 0 ? "-" : "+") << num;
            if (den != 1) {
                os << "/" << den;
            }
        }
        return os;
    }

    std::ostream& print0x9206(std::ostream& os, const Value& value)
    {
        Rational distance = value.toRational();
        if (distance.first == 0) {
            os << "Unknown";
        }
        else if (static_cast<uint32>(distance.first) == 0xffffffff) {
            os << "Infinity";
        }
        else if (distance.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(2)
               << (float)distance.first / distance.second
               << " m";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;        
    }

    std::ostream& print0x9207(std::ostream& os, const Value& value)
    {
        long mode = value.toLong();
        switch (mode) {
        case 0:  os << "Unknown"; break;
        case 1:  os << "Average"; break;
        case 2:  os << "Center weighted"; break;
        case 3:  os << "Spot"; break;
        case 4:  os << "Multispot"; break;
        case 5:  os << "Matrix"; break;
        case 6:  os << "Partial"; break;
        default: os << "(" << mode << ")"; break;
        }
        return os;
    }

    std::ostream& print0x9208(std::ostream& os, const Value& value)
    {
        long source = value.toLong();
        switch (source) {
        case   0: os << "Unknown"; break;
        case   1: os << "Daylight"; break;
        case   2: os << "Fluorescent"; break;
        case   3: os << "Tungsten (incandescent light)"; break;
        case   4: os << "Flash"; break;
        case   9: os << "Fine weather"; break;
        case  10: os << "Cloudy weather"; break;
        case  11: os << "Shade"; break;
        case  12: os << "Daylight fluorescent (D 5700 - 7100K)"; break;
        case  13: os << "Day white fluorescent (N 4600 - 5400K)"; break;
        case  14: os << "Cool white fluorescent (W 3900 - 4500K)"; break;
        case  15: os << "White fluorescent (WW 3200 - 3700K)"; break;
        case  17: os << "Standard light A"; break;
        case  18: os << "Standard light B"; break;
        case  19: os << "Standard light C"; break;
        case  20: os << "D55"; break;
        case  21: os << "D65"; break;
        case  22: os << "D75"; break;
        case  23: os << "D50"; break;
        case  24: os << "ISO studio tungsten"; break;
        case 255: os << "other light source"; break;
        default:  os << "(" << source << ")"; break;
        }
        return os;
    }

    std::ostream& print0x9209(std::ostream& os, const Value& value)
    {
        long flash = value.toLong();
        switch (flash) {
        case 0x00: os << "No"; break;
        case 0x01: os << "Yes"; break;
        case 0x05: os << "Strobe return light not detected"; break;
        case 0x07: os << "Strobe return light detected"; break;
        case 0x09: os << "Yes, compulsory"; break;
        case 0x0d: os << "Yes, compulsory, return light not detected"; break;
        case 0x0f: os << "Yes, compulsory, return light detected"; break;
        case 0x10: os << "No, compulsory"; break;
        case 0x18: os << "No, auto"; break;
        case 0x19: os << "Yes, auto"; break;
        case 0x1d: os << "Yes, auto, return light not detected"; break;
        case 0x1f: os << "Yes, auto, return light detected"; break;
        case 0x20: os << "No flash function"; break;
        case 0x41: os << "Yes, red-eye reduction"; break;
        case 0x45: os << "Yes, red-eye reduction, return light not detected"; break;
        case 0x47: os << "Yes, red-eye reduction, return light detected"; break;
        case 0x49: os << "Yes, compulsory, red-eye reduction"; break;
        case 0x4d: os << "Yes, compulsory, red-eye reduction, return light not detected"; break;
        case 0x4f: os << "Yes, compulsory, red-eye reduction, return light detected"; break;
        case 0x59: os << "Yes, auto, red-eye reduction"; break;
        case 0x5d: os << "Yes, auto, red-eye reduction, return light not detected"; break;
        case 0x5f: os << "Yes, auto, red-eye reduction, return light detected"; break;
        default:   os << "(" << flash << ")"; break;
        }
        return os;
    }

    std::ostream& print0x920a(std::ostream& os, const Value& value)
    {
        Rational length = value.toRational();
        if (length.second != 0) {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)length.first / length.second
               << " mm";
            os.copyfmt(oss);
        }
        else {
            os << "(" << value << ")";
        }
        return os;
    }

    std::ostream& print0x9286(std::ostream& os, const Value& value)
    {
        if (value.size() > 8) {
            DataBuf buf(value.size());
            value.copy(buf.pData_, bigEndian);
            // Hack: Simply skip the leading 8-Byte character code and let
            // the stream handle the comment
            std::string userComment(buf.pData_ + 8, buf.size_ - 8);
            os << userComment;
        }
        return os;
    }

    std::ostream& print0xa001(std::ostream& os, const Value& value)
    {
        long space = value.toLong();
        switch (space) {
        case 1:      os << "sRGB"; break;
        case 0xffff: os << "Uncalibrated"; break;
        default:     os << "(" << space << ")"; break;
        }
        return os;
    }

    std::ostream& print0xa217(std::ostream& os, const Value& value)
    {
        long method = value.toLong();
        switch (method) {
        case 1:  os << "Not defined"; break;
        case 2:  os << "One-chip color area"; break;
        case 3:  os << "Two-chip color area"; break;
        case 4:  os << "Three-chip color area"; break;
        case 5:  os << "Color sequential area"; break;
        case 7:  os << "Trilinear sensor"; break;
        case 8:  os << "Color sequential linear"; break;
        default: os << "(" << method << ")"; break;
        }
        return os;
    }

    std::ostream& print0xa300(std::ostream& os, const Value& value)
    {
        long source = value.toLong();
        switch (source) {
        case 3:      os << "Digital still camera"; break;
        default:     os << "(" << source << ")"; break;
        }
        return os;
    }

    std::ostream& print0xa301(std::ostream& os, const Value& value)
    {
        long scene = value.toLong();
        switch (scene) {
        case 1:      os << "Directly photographed"; break;
        default:     os << "(" << scene << ")"; break;
        }
        return os;
    }

    std::ostream& print0xa402(std::ostream& os, const Value& value)
    {
        long mode = value.toLong();
        switch (mode) {
        case 0: os << "Auto"; break;
        case 1: os << "Manual"; break;
        case 2: os << "Auto bracket"; break;
        default: os << "(" << mode << ")"; break;
        }
        return os;
    }

    std::ostream& print0xa403(std::ostream& os, const Value& value)
    {
        long wb = value.toLong();
        switch (wb) {
        case 0: os << "Auto"; break;
        case 1: os << "Manual"; break;
        default: os << "(" << wb << ")"; break;
        }
        return os;
    }

    std::ostream& print0xa404(std::ostream& os, const Value& value)
    {
        Rational zoom = value.toRational();
        if (zoom.second == 0) {
            os << "Digital zoom not used";
        }
        else {
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << (float)zoom.first / zoom.second;
            os.copyfmt(oss);
        }
        return os;
    }

    std::ostream& print0xa405(std::ostream& os, const Value& value)
    {
        long length = value.toLong();
        if (length == 0) {
            os << "Unknown";
        }
        else {
            os << length << ".0 mm";
        }
        return os;
    }

    std::ostream& print0xa406(std::ostream& os, const Value& value)
    {
        long scene = value.toLong();
        switch (scene) {
        case 0: os << "Standard"; break;
        case 1: os << "Landscape"; break;
        case 2: os << "Portrait"; break;
        case 3: os << "Night scene"; break;
        default: os << "(" << scene << ")"; break;
        }
        return os;
    }

    std::ostream& print0xa407(std::ostream& os, const Value& value)
    {
        long gain = value.toLong();
        switch (gain) {
        case 0: os << "None"; break;
        case 1: os << "Low gain up"; break;
        case 2: os << "High gain up"; break;
        case 3: os << "Low gain down"; break;
        case 4: os << "High gain down"; break;
        default: os << "(" << gain << ")"; break;
        }
        return os;
    }

    std::ostream& print0xa408(std::ostream& os, const Value& value)
    {
        long contrast = value.toLong();
        switch (contrast) {
        case 0: os << "Normal"; break;
        case 1: os << "Soft"; break;
        case 2: os << "Hard"; break;
        default: os << "(" << contrast << ")"; break;
        }
        return os;
    }

    std::ostream& print0xa409(std::ostream& os, const Value& value)
    {
        long saturation = value.toLong();
        switch (saturation) {
        case 0: os << "Normal"; break;
        case 1: os << "Low"; break;
        case 2: os << "High"; break;
        default: os << "(" << saturation << ")"; break;
        }
        return os;
    }

    std::ostream& print0xa40a(std::ostream& os, const Value& value)
    {
        long sharpness = value.toLong();
        switch (sharpness) {
        case 0: os << "Normal"; break;
        case 1: os << "Soft"; break;
        case 2: os << "Hard"; break;
        default: os << "(" << sharpness << ")"; break;
        }
        return os;
    }

    std::ostream& print0xa40c(std::ostream& os, const Value& value)
    {
        long distance = value.toLong();
        switch (distance) {
        case 0: os << "Unknown"; break;
        case 1: os << "Macro"; break;
        case 2: os << "Close view"; break;
        case 3: os << "Distant view"; break;
        default: os << "(" << distance << ")"; break;
        }
        return os;
    }

}                                       // namespace Exiv2
