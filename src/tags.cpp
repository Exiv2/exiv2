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
   $Revision: 1.1 $
 */
// *****************************************************************************
// included header files
#include "tags.hpp"

// + standard includes

// *****************************************************************************
// local declarations
namespace {

}

// *****************************************************************************
// class member definitions
namespace Exif {

    IfdInfo::IfdInfo(IfdId ifdId, const char* name)
        : ifdId_(ifdId), name_(name)
    {
    }

    const IfdInfo ExifTags::ifdInfo_[] = {
        IfdInfo(IfdIdNotSet, "(Unknown IFD)"),

        IfdInfo(ifd0, "IFD0"),
        IfdInfo(exifIfd, "Exif"),
        IfdInfo(gpsIfd, "GPSInfo"),
        IfdInfo(exifMakerIfd, "MakerNote"),
        IfdInfo(exifIopIfd, "Exif Iop."),
        IfdInfo(ifd1, "IFD1"),

        IfdInfo(ifd1ExifIfd, "Exif (at IFD1)"),
        IfdInfo(ifd1GpsIfd, "GPSInfo (at IFD1)"),
        IfdInfo(ifd1MakerIfd, "MakerNote (at IFD1)"),
        IfdInfo(ifd1ExifIopIfd, "Exif Iop. (at IFD1)")
    };

    SectionInfo::SectionInfo(SectionId sectionId, const char* name)
        : sectionId_(sectionId), name_(name)
    {
    }

    const SectionInfo ExifTags::sectionInfo_[] = {
        SectionInfo(SectionIdNotSet, "(Unknown Section)"),

        SectionInfo(ifd0Tiff, "IFD0"),
        SectionInfo(exifIfdSection, "Exif"),
        SectionInfo(gpsIfdSection, "GPSInfo"),
        SectionInfo(exifIopIfdSection, "Exif Iop."),
        SectionInfo(ifd1Section, "IFD1")

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

    //! Lookup list with tags, their names and where they belong to
    const TagInfo ExifTags::tagInfo_[] = {
        // Exif Interoperability IFD Tags
        TagInfo(0x0001, "InteroperabilityIndex", "Interoperability Identification", exifIopIfd, exifIopIfdSection),
        TagInfo(0x0002, "InteroperabilityVersion", "Interoperability version", exifIopIfd, exifIopIfdSection),
        TagInfo(0x1000, "RelatedImageFileFormat", "File format of image file", exifIopIfd, exifIopIfdSection),
        TagInfo(0x1001, "RelatedImageWidth", "Image width", exifIopIfd, exifIopIfdSection),
        TagInfo(0x1002, "RelatedImageLength", "Image height", exifIopIfd, exifIopIfdSection),

        // GPS Info Tags
        TagInfo(0x0000, "GPSVersionID", "GPS tag version", gpsIfd, gpsIfdSection),
        TagInfo(0x0001, "GPSLatitudeRef", "North or South Latitude", gpsIfd, gpsIfdSection),
        TagInfo(0x0002, "GPSLatitude", "Latitude", gpsIfd, gpsIfdSection),
        TagInfo(0x0003, "GPSLongitudeRef", "East or West Longitude", gpsIfd, gpsIfdSection),
        TagInfo(0x0004, "GPSLongitude", "Longitude", gpsIfd, gpsIfdSection),
        TagInfo(0x0005, "GPSAltitudeRef", "Altitude reference", gpsIfd, gpsIfdSection),
        TagInfo(0x0006, "GPSAltitude", "Altitude", gpsIfd, gpsIfdSection),
        TagInfo(0x0007, "GPSTimeStamp", "GPS time (atomic clock)", gpsIfd, gpsIfdSection),
        TagInfo(0x0008, "GPSSatellites", "GPS satellites used for measurement", gpsIfd, gpsIfdSection),
        TagInfo(0x0009, "GPSStatus", "GPS receiver status", gpsIfd, gpsIfdSection),
        TagInfo(0x000a, "GPSMeasureMode", "GPS measurement mode", gpsIfd, gpsIfdSection),
        TagInfo(0x000b, "GPSDOP", "Measurement precision", gpsIfd, gpsIfdSection),
        TagInfo(0x000c, "GPSSpeedRef", "Speed unit", gpsIfd, gpsIfdSection),
        TagInfo(0x000d, "GPSSpeed", "Speed of GPS receiver", gpsIfd, gpsIfdSection),
        TagInfo(0x000e, "GPSTrackRef", "Reference for direction of movement", gpsIfd, gpsIfdSection),
        TagInfo(0x000f, "GPSTrack", "Direction of movement", gpsIfd, gpsIfdSection),
        TagInfo(0x0010, "GPSImgDirectionRef", "Reference for direction of image", gpsIfd, gpsIfdSection),
        TagInfo(0x0011, "GPSImgDirection", "Direction of image", gpsIfd, gpsIfdSection),
        TagInfo(0x0012, "GPSMapDatum", "Geodetic survey data used", gpsIfd, gpsIfdSection),
        TagInfo(0x0013, "GPSDestLatitudeRef", "Reference for latitude of destination", gpsIfd, gpsIfdSection),
        TagInfo(0x0014, "GPSDestLatitude", "Latitude of destination", gpsIfd, gpsIfdSection),
        TagInfo(0x0015, "GPSDestLongitudeRef", "Reference for longitude of destination", gpsIfd, gpsIfdSection),
        TagInfo(0x0016, "GPSDestLongitude", "Longitude of destination", gpsIfd, gpsIfdSection),
        TagInfo(0x0017, "GPSDestBearingRef", "Reference for bearing of destination", gpsIfd, gpsIfdSection),
        TagInfo(0x0018, "GPSDestBearing", "Bearing of destination", gpsIfd, gpsIfdSection),
        TagInfo(0x0019, "GPSDestDistanceRef", "Reference for distance to destination", gpsIfd, gpsIfdSection),
        TagInfo(0x001a, "GPSDestDistance", "Distance to destination", gpsIfd, gpsIfdSection),
        TagInfo(0x001b, "GPSProcessingMethod", "Name of GPS processing method", gpsIfd, gpsIfdSection),
        TagInfo(0x001c, "GPSAreaInformation", "Name of GPS area", gpsIfd, gpsIfdSection),
        TagInfo(0x001d, "GPSDateStamp", "GPS date", gpsIfd, gpsIfdSection),
        TagInfo(0x001e, "GPSDifferential", "GPS differential correction", gpsIfd, gpsIfdSection),

        // IFD0 Tags
        TagInfo(0x0100, "ImageWidth", "Image width", ifd0, ifd0Tiff),
        TagInfo(0x0101, "ImageLength", "Image height", ifd0, ifd0Tiff),
        TagInfo(0x0102, "BitsPerSample", "Number of bits per component", ifd0, ifd0Tiff),
        TagInfo(0x0103, "Compression", "Compression scheme", ifd0, ifd0Tiff),
        TagInfo(0x0106, "PhotometricInterpretation", "Pixel composition", ifd0, ifd0Tiff),
        TagInfo(0x010e, "ImageDescription", "Image title", ifd0, ifd0Tiff),
        TagInfo(0x010f, "Make", "Manufacturer of image input equipment", ifd0, ifd0Tiff),
        TagInfo(0x0110, "Model", "Model of image input equipment", ifd0, ifd0Tiff),
        TagInfo(0x0111, "StripOffsets", "Image data location", ifd0, ifd0Tiff),
        TagInfo(0x0112, "Orientation", "Orientation of image", ifd0, ifd0Tiff),
        TagInfo(0x0115, "SamplesPerPixel", "Number of components", ifd0, ifd0Tiff),
        TagInfo(0x0116, "RowsPerStrip", "Number of rows per strip", ifd0, ifd0Tiff),
        TagInfo(0x0117, "StripByteCounts", "Bytes per compressed strip", ifd0, ifd0Tiff),
        TagInfo(0x011a, "XResolution", "Image resolution in width direction", ifd0, ifd0Tiff),
        TagInfo(0x011b, "YResolution", "Image resolution in height direction", ifd0, ifd0Tiff),
        TagInfo(0x011c, "PlanarConfiguration", "Image data arrangement", ifd0, ifd0Tiff),
        TagInfo(0x0128, "ResolutionUnit", "Unit of X and Y resolution", ifd0, ifd0Tiff),
        TagInfo(0x012d, "TransferFunction", "Transfer function", ifd0, ifd0Tiff),
        TagInfo(0x0131, "Software", "Software used", ifd0, ifd0Tiff),
        TagInfo(0x0132, "DateTime", "File change date and time", ifd0, ifd0Tiff),
        TagInfo(0x013b, "Artist", "Person who created the image", ifd0, ifd0Tiff),
        TagInfo(0x013e, "WhitePoint", "White point chromaticity", ifd0, ifd0Tiff),
        TagInfo(0x013f, "PrimaryChromaticities", "Chromaticities of primaries", ifd0, ifd0Tiff),
        TagInfo(0x0201, "JPEGInterchangeFormat", "Offset to JPEG SOI", ifd0, ifd0Tiff),
        TagInfo(0x0202, "JPEGInterchangeFormatLength", "Bytes of JPEG data", ifd0, ifd0Tiff),
        TagInfo(0x0211, "YCbCrCoefficients", "Color space transformation matrix coefficients", ifd0, ifd0Tiff),
        TagInfo(0x0212, "YCbCrSubSampling", "Subsampling ratio of Y to C", ifd0, ifd0Tiff),
        TagInfo(0x0213, "YCbCrPositioning", "Y and C positioning", ifd0, ifd0Tiff),
        TagInfo(0x0214, "ReferenceBlackWhite", "Pair of black and white reference values", ifd0, ifd0Tiff),
        TagInfo(0x8298, "Copyright", "Copyright holder", ifd0, ifd0Tiff),
        TagInfo(0x8769, "ExifTag", "Exif IFD Pointer", ifd0, ifd0Tiff),
        TagInfo(0x8825, "GPSTag", "GPSInfo IFD Pointer", ifd0, ifd0Tiff),

        // IFD1 Tags
        TagInfo(0x0100, "ImageWidth", "Image width", ifd1, ifd1Section),
        TagInfo(0x0101, "ImageLength", "Image height", ifd1, ifd1Section),
        TagInfo(0x0102, "BitsPerSample", "Number of bits per component", ifd1, ifd1Section),
        TagInfo(0x0103, "Compression", "Compression scheme", ifd1, ifd1Section),
        TagInfo(0x0106, "PhotometricInterpretation", "Pixel composition", ifd1, ifd1Section),
        TagInfo(0x010e, "ImageDescription", "Image title", ifd1, ifd1Section),
        TagInfo(0x010f, "Make", "Manufacturer of image input equipment", ifd1, ifd1Section),
        TagInfo(0x0110, "Model", "Model of image input equipment", ifd1, ifd1Section),
        TagInfo(0x0111, "StripOffsets", "Image data location", ifd1, ifd1Section),
        TagInfo(0x0112, "Orientation", "Orientation of image", ifd1, ifd1Section),
        TagInfo(0x0115, "SamplesPerPixel", "Number of components", ifd1, ifd1Section),
        TagInfo(0x0116, "RowsPerStrip", "Number of rows per strip", ifd1, ifd1Section),
        TagInfo(0x0117, "StripByteCounts", "Bytes per compressed strip", ifd1, ifd1Section),
        TagInfo(0x011a, "XResolution", "Image resolution in width direction", ifd1, ifd1Section),
        TagInfo(0x011b, "YResolution", "Image resolution in height direction", ifd1, ifd1Section),
        TagInfo(0x011c, "PlanarConfiguration", "Image data arrangement", ifd1, ifd1Section),
        TagInfo(0x0128, "ResolutionUnit", "Unit of X and Y resolution", ifd1, ifd1Section),
        TagInfo(0x012d, "TransferFunction", "Transfer function", ifd1, ifd1Section),
        TagInfo(0x0131, "Software", "Software used", ifd1, ifd1Section),
        TagInfo(0x0132, "DateTime", "File change date and time", ifd1, ifd1Section),
        TagInfo(0x013b, "Artist", "Person who created the image", ifd1, ifd1Section),
        TagInfo(0x013e, "WhitePoint", "White point chromaticity", ifd1, ifd1Section),
        TagInfo(0x013f, "PrimaryChromaticities", "Chromaticities of primaries", ifd1, ifd1Section),
        TagInfo(0x0201, "JPEGInterchangeFormat", "Offset to JPEG SOI", ifd1, ifd1Section),
        TagInfo(0x0202, "JPEGInterchangeFormatLength", "Bytes of JPEG data", ifd1, ifd1Section),
        TagInfo(0x0211, "YCbCrCoefficients", "Color space transformation matrix coefficients", ifd1, ifd1Section),
        TagInfo(0x0212, "YCbCrSubSampling", "Subsampling ratio of Y to C", ifd1, ifd1Section),
        TagInfo(0x0213, "YCbCrPositioning", "Y and C positioning", ifd1, ifd1Section),
        TagInfo(0x0214, "ReferenceBlackWhite", "Pair of black and white reference values", ifd1, ifd1Section),
        TagInfo(0x8298, "Copyright", "Copyright holder", ifd1, ifd1Section),
        TagInfo(0x8769, "ExifTag", "Exif IFD Pointer", ifd1, ifd1Section),
        TagInfo(0x8825, "GPSTag", "GPSInfo IFD Pointer", ifd1, ifd1Section),

        // Exif IFD Tags
        TagInfo(0x829a, "ExposureTime", "Exposure time", exifIfd, exifIfdSection),
        TagInfo(0x829d, "FNumber", "F number", exifIfd, exifIfdSection),
        TagInfo(0x8822, "ExposureProgram", "Exposure program", exifIfd, exifIfdSection),
        TagInfo(0x8824, "SpectralSensitivity", "Spectral sensitivity", exifIfd, exifIfdSection),
        TagInfo(0x8827, "ISOSpeedRatings", "ISO speed ratings", exifIfd, exifIfdSection),
        TagInfo(0x8828, "OECF", "Optoelectric coefficient", exifIfd, exifIfdSection),
        TagInfo(0x9000, "ExifVersion", "Exif Version", exifIfd, exifIfdSection),
        TagInfo(0x9003, "DateTimeOriginal", "Date and time original image was generated", exifIfd, exifIfdSection),
        TagInfo(0x9004, "DateTimeDigitized", "Date and time image was made digital data", exifIfd, exifIfdSection),
        TagInfo(0x9101, "ComponentsConfiguration", "Meaning of each component", exifIfd, exifIfdSection),
        TagInfo(0x9102, "CompressedBitsPerPixel", "Image compression mode", exifIfd, exifIfdSection),
        TagInfo(0x9201, "ShutterSpeedValue", "Shutter speed", exifIfd, exifIfdSection),
        TagInfo(0x9202, "ApertureValue", "Aperture", exifIfd, exifIfdSection),
        TagInfo(0x9203, "BrightnessValue", "Brightness", exifIfd, exifIfdSection),
        TagInfo(0x9204, "ExposureBiasValue", "Exposure bias", exifIfd, exifIfdSection),
        TagInfo(0x9205, "MaxApertureValue", "Maximum lens aperture", exifIfd, exifIfdSection),
        TagInfo(0x9206, "SubjectDistance", "Subject distance", exifIfd, exifIfdSection),
        TagInfo(0x9207, "MeteringMode", "Metering mode", exifIfd, exifIfdSection),
        TagInfo(0x9208, "LightSource", "Light source", exifIfd, exifIfdSection),
        TagInfo(0x9209, "Flash", "Flash", exifIfd, exifIfdSection),
        TagInfo(0x920a, "FocalLength", "Lens focal length", exifIfd, exifIfdSection),
        TagInfo(0x9214, "SubjectArea", "Subject area", exifIfd, exifIfdSection),
        TagInfo(0x927c, "MakerNote", "Manufacturer notes", exifIfd, exifIfdSection),
        TagInfo(0x9286, "UserComment", "User comments", exifIfd, exifIfdSection),
        TagInfo(0x9290, "SubSecTime", "DateTime subseconds", exifIfd, exifIfdSection),
        TagInfo(0x9291, "SubSecTimeOriginal", "DateTimeOriginal subseconds", exifIfd, exifIfdSection),
        TagInfo(0x9292, "SubSecTimeDigitized", "DateTimeDigitized subseconds", exifIfd, exifIfdSection),
        TagInfo(0xa000, "FlashpixVersion", "Supported Flashpix version", exifIfd, exifIfdSection),
        TagInfo(0xa001, "ColorSpace", "Color space information", exifIfd, exifIfdSection),
        TagInfo(0xa002, "PixelXDimension", "Valid image width", exifIfd, exifIfdSection),
        TagInfo(0xa003, "PixelYDimension", "Valid image height", exifIfd, exifIfdSection),
        TagInfo(0xa004, "RelatedSoundFile", "Related audio file", exifIfd, exifIfdSection),
        TagInfo(0xa005, "InteroperabilityTag", "Interoperability IFD Pointer", exifIfd, exifIfdSection),
        TagInfo(0xa20b, "FlashEnergy", "Flash energy", exifIfd, exifIfdSection),
        TagInfo(0xa20c, "SpatialFrequencyResponse", "Spatial frequency response", exifIfd, exifIfdSection),
        TagInfo(0xa20e, "FocalPlaneXResolution", "Focal plane X resolution", exifIfd, exifIfdSection),
        TagInfo(0xa20f, "FocalPlaneYResolution", "Focal plane Y resolution", exifIfd, exifIfdSection),
        TagInfo(0xa210, "FocalPlaneResolutionUnit", "Focal plane resolution unit", exifIfd, exifIfdSection),
        TagInfo(0xa214, "SubjectLocation", "Subject location", exifIfd, exifIfdSection),
        TagInfo(0xa215, "ExposureIndex", "Exposure index", exifIfd, exifIfdSection),
        TagInfo(0xa217, "SensingMethod", "Sensing method", exifIfd, exifIfdSection),
        TagInfo(0xa300, "FileSource", "File source", exifIfd, exifIfdSection),
        TagInfo(0xa301, "SceneType", "Scene type", exifIfd, exifIfdSection),
        TagInfo(0xa302, "CFAPattern", "CFA pattern", exifIfd, exifIfdSection),
        TagInfo(0xa401, "CustomRendered", "Custom image processing", exifIfd, exifIfdSection),
        TagInfo(0xa402, "ExposureMode", "Exposure mode", exifIfd, exifIfdSection),
        TagInfo(0xa403, "WhiteBalance", "White balance", exifIfd, exifIfdSection),
        TagInfo(0xa404, "DigitalZoomRatio", "Digital zoom ratio", exifIfd, exifIfdSection),
        TagInfo(0xa405, "FocalLengthIn35mmFilm", "Focal length in 35 mm film", exifIfd, exifIfdSection),
        TagInfo(0xa406, "SceneCaptureType", "Scene capture type", exifIfd, exifIfdSection),
        TagInfo(0xa407, "GainControl", "Gain control", exifIfd, exifIfdSection),
        TagInfo(0xa408, "Contrast", "Contrast", exifIfd, exifIfdSection),
        TagInfo(0xa409, "Saturation", "Saturation", exifIfd, exifIfdSection),
        TagInfo(0xa40a, "Sharpness", "Sharpness", exifIfd, exifIfdSection),
        TagInfo(0xa40b, "DeviceSettingDescription", "Device settings description", exifIfd, exifIfdSection),
        TagInfo(0xa40c, "SubjectDistanceRange", "Subject distance range", exifIfd, exifIfdSection),
        TagInfo(0xa420, "ImageUniqueID", "Unique image ID", exifIfd, exifIfdSection),

        // End of list marker
        TagInfo(0xffff, "(Unknown)", "Unknown tag", IfdIdNotSet, SectionIdNotSet)
    };

    int ExifTags::tagInfoIdx(uint16 tag, IfdId ifdId)
    {
        // Todo: implement a better algorithm
        int idx;
        for (idx = 0; tagInfo_[idx].tag_ != 0xffff; ++idx) {
            if (   tagInfo_[idx].tag_   == tag
                && tagInfo_[idx].ifdId_ == ifdId) break;
        }
        return idx;
    }

    const char* ExifTags::tagName(uint16 tag, IfdId ifdId)
    {
        return tagInfo_[tagInfoIdx(tag, ifdId)].name_;
    }

    const char* ExifTags::sectionName(uint16 tag, IfdId ifdId)
    {
        return sectionInfo_[tagInfo_[tagInfoIdx(tag, ifdId)].sectionId_].name_;
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

    const char* ExifTags::sectionName(SectionId sectionId)
    {
        return sectionInfo_[sectionId].name_;
    }

    // *************************************************************************
    // free functions

}                                       // namespace Exif

// *****************************************************************************
// local definitions
namespace {

}
