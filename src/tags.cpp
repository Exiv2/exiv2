// ***************************************************************** -*- C++ -*-
/*
 * Copyright (c) 2004 Andreas Huggel
 *
 * Todo: Insert license blabla here
 *
 */
/*
  Author(s): Andreas Huggel (ahu)
  History:
   15-Jan-04, ahu: created

  RCS information
   $Name:  $
   $Revision: 1.2 $
 */
// *****************************************************************************
// included header files
#include "tags.hpp"

// *****************************************************************************
// class member definitions
namespace Exif {

    IfdInfo::IfdInfo(IfdId ifdId, const char* name, const char* item)
        : ifdId_(ifdId), name_(name), item_(item)
    {
    }

    const IfdInfo ExifTags::ifdInfo_[] = {
        IfdInfo(IfdIdNotSet, "(Unknown IFD)", "(Unknown data area)"),
        IfdInfo(ifd0, "IFD0", "Image"),
        IfdInfo(exifIfd, "Exif", "Image"),
        IfdInfo(gpsIfd, "GPSInfo", "Image"),
        IfdInfo(makerIfd, "MakerNote", "Image"),
        IfdInfo(iopIfd, "Iop", "Image"),
        IfdInfo(ifd1, "IFD1", "Thumbnail"),
        IfdInfo(ifd1ExifIfd, "Exif", "Thumbnail"),
        IfdInfo(ifd1GpsIfd, "GPSInfo", "Thumbnail"),
        IfdInfo(ifd1MakerIfd, "MakerNote", "Thumbnail"),
        IfdInfo(ifd1IopIfd, "Iop", "Thumbnail")
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
        SectionInfo(SectionIdNotSet, "(UnknownSection)", "Unknown section"),
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
        SectionInfo(iopTags, "Interoperability", "Interoperability information")
    };

    TagFormat::TagFormat(uint16 type, const char* name, long size)
        : type_(type), name_(name), size_(size)
    {
    }

    //! Lookup list of IFD tag data formats and their properties
    const TagFormat ExifTags::tagFormat_[] = {
        TagFormat( 0, "invalid",           0),
        TagFormat( 1, "unsigned byte",     1),
        TagFormat( 2, "ascii strings",     1),
        TagFormat( 3, "unsigned short",    2),
        TagFormat( 4, "unsigned long",     4),
        TagFormat( 5, "unsigned rational", 8),
        TagFormat( 6, "signed byte",       1),
        TagFormat( 7, "undefined",         1),
        TagFormat( 8, "signed short",      2),
        TagFormat( 9, "signed long",       4),
        TagFormat(10, "signed rational",   8),
        TagFormat(11, "single float",      4),
        TagFormat(12, "double float",      8)
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
        TagInfo(0xffff, "(UnknownIfdTag)", "Unknown IFD tag", IfdIdNotSet, SectionIdNotSet)
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
        TagInfo(0xffff, "(UnknownExifTag)", "Unknown Exif tag", IfdIdNotSet, SectionIdNotSet)
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
        TagInfo(0xffff, "(UnknownGpsTag)", "Unknown GPSInfo tag", IfdIdNotSet, SectionIdNotSet)
    };
    
    // Exif Interoperability IFD Tags
    static const TagInfo iopTagInfo[] = {
        TagInfo(0x0001, "InteroperabilityIndex", "Interoperability Identification", iopIfd, iopTags),
        TagInfo(0x0002, "InteroperabilityVersion", "Interoperability version", iopIfd, iopTags),
        TagInfo(0x1000, "RelatedImageFileFormat", "File format of image file", iopIfd, iopTags),
        TagInfo(0x1001, "RelatedImageWidth", "Image width", iopIfd, iopTags),
        TagInfo(0x1002, "RelatedImageLength", "Image height", iopIfd, iopTags),
        // End of list marker
        TagInfo(0xffff, "(UnknownIopTag)", "Unknown Exif Interoperability tag", IfdIdNotSet, SectionIdNotSet)
    };

    // Tag lookup lists with tag names, desc and where they (preferrably) belong to;
    // this is an array with pointers to one list per IFD. The IfdId is used as the
    // index into the array.
    const TagInfo* ExifTags::tagInfos_[] = {
        0, 
        ifdTagInfo, exifTagInfo, gpsTagInfo, 0, iopTagInfo,
        ifdTagInfo, exifTagInfo, gpsTagInfo, 0, iopTagInfo
    };

    int ExifTags::tagInfoIdx(uint16 tag, IfdId ifdId)
    {
        // Todo: implement a better (more efficient) algorithm
	const TagInfo* tagInfo = tagInfos_[ifdId];
        int idx;
        for (idx = 0; tagInfo[idx].tag_ != 0xffff; ++idx) {
            if (tagInfo[idx].tag_ == tag) break;
        }
        return idx;
    }

    const char* ExifTags::tagName(uint16 tag, IfdId ifdId)
    {
        return tagInfos_[ifdId][tagInfoIdx(tag, ifdId)].name_;
    }

    const char* ExifTags::sectionName(uint16 tag, IfdId ifdId)
    {
	const TagInfo* tagInfo = tagInfos_[ifdId];
        return sectionInfo_[tagInfo[tagInfoIdx(tag, ifdId)].sectionId_].name_;
    }

    const char* ExifTags::typeName(uint16 type)
    {
        return tagFormat_[type].name_;
    }

    long ExifTags::typeSize(uint16 type)
    {
        return tagFormat_[type].size_;
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

}                                       // namespace Exif
