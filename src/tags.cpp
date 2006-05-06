// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005, 2006 Andreas Huggel <ahuggel@gmx.net>
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
/*
  File:      tags.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Jan-04, ahu: created
             21-Jan-05, ahu: added MakerNote TagInfo registry and related code
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "tags.hpp"
#include "error.hpp"
#include "types.hpp"
#include "ifd.hpp"
#include "value.hpp"
#include "makernote.hpp"
#include "mn.hpp"                // To ensure that all makernotes are registered

#include <iostream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <cstdlib>
#include <cassert>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    IfdInfo::IfdInfo(IfdId ifdId, const char* name, const char* item)
        : ifdId_(ifdId), name_(name), item_(item)
    {
    }

    // Todo: Allow to register new IfdInfo entries from elsewhere (the makernotes)
    // Important: IFD item must be unique!
    const IfdInfo ExifTags::ifdInfo_[] = {
        IfdInfo(ifdIdNotSet, "(Unknown IFD)", "(Unknown item)"),
        IfdInfo(ifd0Id, "IFD0", "Image"),
        IfdInfo(exifIfdId, "Exif", "Photo"),  // just to avoid 'Exif.Exif.*' keys
        IfdInfo(gpsIfdId, "GPSInfo", "GPSInfo"),
        IfdInfo(iopIfdId, "Iop", "Iop"),
        IfdInfo(ifd1Id, "IFD1", "Thumbnail"),
        IfdInfo(canonIfdId, "Makernote", "Canon"),
        IfdInfo(canonCs1IfdId, "Makernote", "CanonCs1"),
        IfdInfo(canonCs2IfdId, "Makernote", "CanonCs2"),
        IfdInfo(canonCfIfdId, "Makernote", "CanonCf"),
        IfdInfo(fujiIfdId, "Makernote", "Fujifilm"),
        IfdInfo(minoltaIfdId, "Makernote", "Minolta"),
        IfdInfo(nikon1IfdId, "Makernote", "Nikon1"),
        IfdInfo(nikon2IfdId, "Makernote", "Nikon2"),
        IfdInfo(nikon3IfdId, "Makernote", "Nikon3"),
        IfdInfo(olympusIfdId, "Makernote", "Olympus"),
        IfdInfo(panasonicIfdId, "Makernote", "Panasonic"),
        IfdInfo(sigmaIfdId, "Makernote", "Sigma"),
        IfdInfo(sonyIfdId, "Makernote", "Sony"),
        IfdInfo(lastIfdId, "(Last IFD info)", "(Last IFD item)")
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
        SectionInfo(makerTags, "Makernote", "Vendor specific information"),
        SectionInfo(lastSectionId, "(LastSection)", "Last section")
    };

    TagInfo::TagInfo(
        uint16_t tag,
        const char* name,
        const char* title,
        const char* desc,
        IfdId ifdId,
        SectionId sectionId,
        TypeId typeId,
        PrintFct printFct
    )
        : tag_(tag), name_(name), title_(title), desc_(desc), ifdId_(ifdId),
          sectionId_(sectionId), typeId_(typeId), printFct_(printFct)
    {
    }

    // Base IFD Tags (IFD0 and IFD1)
    static const TagInfo ifdTagInfo[] = {
        TagInfo(0x00fe, "NewSubfileType", "New Subfile Type", "A general indication of the kind of data contained in this subfile.", ifd0Id, imgStruct, unsignedLong, printValue), // TIFF tag
        TagInfo(0x0100, "ImageWidth", "Image Width", "Image width", ifd0Id, imgStruct, unsignedLong, printValue),
        TagInfo(0x0101, "ImageLength", "Image Length", "Image height", ifd0Id, imgStruct, unsignedLong, printValue),
        TagInfo(0x0102, "BitsPerSample", "Bits per Sample", "Number of bits per component", ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0103, "Compression", "Compression", "Compression scheme", ifd0Id, imgStruct, unsignedShort, print0x0103),
        TagInfo(0x0106, "PhotometricInterpretation", "Photometric Interpretation", "Pixel composition", ifd0Id, imgStruct, unsignedShort, print0x0106),
        TagInfo(0x010a, "FillOrder", "Fill Order", "The logical order of bits within a byte", ifd0Id, imgStruct, unsignedShort, printValue), // TIFF tag
        TagInfo(0x010d, "DocumentName", "Document Name", "The name of the document from which this image was scanned", ifd0Id, imgStruct, asciiString, printValue), // TIFF tag
        TagInfo(0x010e, "ImageDescription", "Image Description", "Image title", ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x010f, "Make", "Manufacturer", "Manufacturer of image input equipment", ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x0110, "Model", "Model", "Model of image input equipment", ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x0111, "StripOffsets", "Strip Offsets", "Image data location", ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0112, "Orientation", "Orientation", "Orientation of image", ifd0Id, imgStruct, unsignedShort, print0x0112),
        TagInfo(0x0115, "SamplesPerPixel", "Samples per Pixel", "Number of components", ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0116, "RowsPerStrip", "Rows per Strip", "Number of rows per strip", ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0117, "StripByteCounts", "Strip Byte Count", "Bytes per compressed strip", ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x011a, "XResolution", "x-Resolution", "Image resolution in width direction", ifd0Id, imgStruct, unsignedRational, printLong),
        TagInfo(0x011b, "YResolution", "y-Resolution", "Image resolution in height direction", ifd0Id, imgStruct, unsignedRational, printLong),
        TagInfo(0x011c, "PlanarConfiguration", "Planar Configuration", "Image data arrangement", ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0128, "ResolutionUnit", "Resolution Unit", "Unit of X and Y resolution", ifd0Id, imgStruct, unsignedShort, printUnit),
        TagInfo(0x012d, "TransferFunction", "Transfer Function", "Transfer function", ifd0Id, imgCharacter, unsignedShort, printValue),
        TagInfo(0x0131, "Software", "Software", "Software used", ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x0132, "DateTime", "Date and Time", "File change date and time", ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x013b, "Artist", "Artist", "Person who created the image", ifd0Id, otherTags, asciiString, printValue),
        TagInfo(0x013e, "WhitePoint", "White Point", "White point chromaticity", ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x013f, "PrimaryChromaticities", "Primary Chromaticities", "Chromaticities of primaries", ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x014a, "SubIFDs", "SubIFD Offsets", "Defined by Adobe Corporation to enable TIFF Trees within a TIFF file.", ifd0Id, otherTags, unsignedLong, printValue),
        TagInfo(0x0156, "TransferRange", "Transfer Range", "Expands the range of the TransferFunction", ifd0Id, imgCharacter, unsignedShort, printValue), // TIFF tag
        TagInfo(0x0200, "JPEGProc", "JPEGProc", "This field indicates the process used to produce the compressed data", ifd0Id, recOffset, unsignedLong, printValue), // TIFF tag
        TagInfo(0x0201, "JPEGInterchangeFormat", "JPEG Interchange Format", "Offset to JPEG SOI", ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0202, "JPEGInterchangeFormatLength", "JPEG Interchange Format Length", "Bytes of JPEG data", ifd0Id, recOffset, unsignedLong, printValue),
        TagInfo(0x0211, "YCbCrCoefficients", "YCbCr Coefficients", "Color space transformation matrix coefficients", ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x0212, "YCbCrSubSampling", "YCbCr Sub-Sampling", "Subsampling ratio of Y to C", ifd0Id, imgStruct, unsignedShort, printValue),
        TagInfo(0x0213, "YCbCrPositioning", "YCbCr Positioning", "Y and C positioning", ifd0Id, imgStruct, unsignedShort, print0x0213),
        TagInfo(0x0214, "ReferenceBlackWhite", "Reference Black/White", "Pair of black and white reference values", ifd0Id, imgCharacter, unsignedRational, printValue),
        TagInfo(0x02bc, "XMLPacket", "XML Packet", "XMP Metadata (Adobe technote 9-14-02)", ifd0Id, otherTags, unsignedByte, printValue),
        TagInfo(0x828d, "CFARepeatPatternDim", "CFARepeatPatternDim", "Contains two values representing the minimum rows and columns to define the repeating patterns of the color filter array", ifd0Id, otherTags, unsignedShort, printValue), // TIFF/EP Tag
        TagInfo(0x828e, "CFAPattern", "CFA Pattern", "Indicates the color filter array (CFA) geometric pattern of the image sensor when a one-chip color area sensor is used. It does not apply to all sensing methods", ifd0Id, otherTags, unsignedByte, printValue), // TIFF/EP Tag
        TagInfo(0x828f, "BatteryLevel", "Battery Level", "Contains a value of the battery level as a fraction or string", ifd0Id, otherTags, unsignedRational, printValue), // TIFF/EP Tag
        TagInfo(0x83bb, "IPTCNAA", "IPTC/NAA", "Contains an IPTC/NAA record", ifd0Id, otherTags, unsignedLong, printValue), // TIFF/EP Tag
        TagInfo(0x8298, "Copyright", "Copyright", "Copyright holder", ifd0Id, otherTags, asciiString, print0x8298),
        TagInfo(0x8649, "ImageResources", "Image Resources Block", "Contains information embedded by the Adobe Photoshop application", ifd0Id, otherTags, undefined, printValue),
        TagInfo(0x8769, "ExifTag", "ExifIFDPointer", "Exif IFD Pointer", ifd0Id, exifFormat, unsignedLong, printValue),
        TagInfo(0x8773, "InterColorProfile", "InterColorProfile", "Contains an InterColor Consortium (ICC) format color space characterization/profile", ifd0Id, otherTags, undefined, printValue),
        TagInfo(0x8825, "GPSTag", "GPSInfoIFDPointer", "GPSInfo IFD Pointer", ifd0Id, exifFormat, unsignedLong, printValue),
        TagInfo(0x9216, "TIFFEPStandardID", "TIFF/EP Standard ID", "Contains four ASCII characters representing the TIFF/EP standard version of a TIFF/EP file, eg '1', '0', '0', '0'", ifd0Id, otherTags, unsignedByte, printValue), // TIFF/EP Tag
        // End of list marker
        TagInfo(0xffff, "(UnknownIfdTag)", "Unknown IFD tag", "Unknown IFD tag", ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    // Exif IFD Tags
    static const TagInfo exifTagInfo[] = {
        TagInfo(0x829a, "ExposureTime", "Exposure Time", "Exposure time", exifIfdId, captureCond, unsignedRational, print0x829a),
        TagInfo(0x829d, "FNumber", "FNumber", "F number", exifIfdId, captureCond, unsignedRational, print0x829d),
        TagInfo(0x8822, "ExposureProgram", "ExposureProgram", "Exposure program", exifIfdId, captureCond, unsignedShort, print0x8822),
        TagInfo(0x8824, "SpectralSensitivity", "Spectral Sensitivity", "Spectral sensitivity", exifIfdId, captureCond, asciiString, printValue),
        TagInfo(0x8827, "ISOSpeedRatings", "ISO Speed Ratings", "ISO speed ratings", exifIfdId, captureCond, unsignedShort, print0x8827),
        TagInfo(0x8828, "OECF", "OECF", "Optoelectric coefficient", exifIfdId, captureCond, undefined, printValue),
        TagInfo(0x9000, "ExifVersion", "Exif Version", "Exif Version", exifIfdId, exifVersion, undefined, printValue),
        TagInfo(0x9003, "DateTimeOriginal", "Date and Time (original)", "Date and time original image was generated", exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9004, "DateTimeDigitized", "Date and Time (digitized)", "Date and time image was made digital data", exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9101, "ComponentsConfiguration", "ComponentsConfiguration", "Meaning of each component", exifIfdId, imgConfig, undefined, print0x9101),
        TagInfo(0x9102, "CompressedBitsPerPixel", "Compressed Bits per Pixel", "Image compression mode", exifIfdId, imgConfig, unsignedRational, printFloat),
        TagInfo(0x9201, "ShutterSpeedValue", "Shutter speed", "Shutter speed", exifIfdId, captureCond, signedRational, print0x9201),
        TagInfo(0x9202, "ApertureValue", "Aperture", "Aperture", exifIfdId, captureCond, unsignedRational, print0x9202),
        TagInfo(0x9203, "BrightnessValue", "Brightness", "Brightness", exifIfdId, captureCond, signedRational, printFloat),
        TagInfo(0x9204, "ExposureBiasValue", "Exposure Bias", "Exposure bias", exifIfdId, captureCond, signedRational, print0x9204),
        TagInfo(0x9205, "MaxApertureValue", "MaxApertureValue", "Maximum lens aperture", exifIfdId, captureCond, unsignedRational, print0x9202),
        TagInfo(0x9206, "SubjectDistance", "Subject Distance", "Subject distance", exifIfdId, captureCond, unsignedRational, print0x9206),
        TagInfo(0x9207, "MeteringMode", "Metering Mode", "Metering mode", exifIfdId, captureCond, unsignedShort, print0x9207),
        TagInfo(0x9208, "LightSource", "Light Source", "Light source", exifIfdId, captureCond, unsignedShort, print0x9208),
        TagInfo(0x9209, "Flash", "Flash", "Flash", exifIfdId, captureCond, unsignedShort, print0x9209),
        TagInfo(0x920a, "FocalLength", "Focal Length", "Lens focal length", exifIfdId, captureCond, unsignedRational, print0x920a),
        TagInfo(0x9214, "SubjectArea", "Subject Area", "Subject area", exifIfdId, captureCond, unsignedShort, printValue),
        TagInfo(0x927c, "MakerNote", "Maker Note", "Manufacturer notes", exifIfdId, userInfo, undefined, printValue),
        TagInfo(0x9286, "UserComment", "User Comment", "User comments", exifIfdId, userInfo, comment, print0x9286),
        TagInfo(0x9290, "SubSecTime", "SubsecTime", "DateTime subseconds", exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9291, "SubSecTimeOriginal", "SubSecTimeOriginal", "DateTimeOriginal subseconds", exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0x9292, "SubSecTimeDigitized", "SubSecTimeDigitized", "DateTimeDigitized subseconds", exifIfdId, dateTime, asciiString, printValue),
        TagInfo(0xa000, "FlashpixVersion", "FlashPixVersion", "Supported Flashpix version", exifIfdId, exifVersion, undefined, printValue),
        TagInfo(0xa001, "ColorSpace", "Color Space", "Color space information", exifIfdId, imgCharacter, unsignedShort, print0xa001),
        TagInfo(0xa002, "PixelXDimension", "PixelXDimension", "Valid image width", exifIfdId, imgConfig, unsignedLong, printValue),
        TagInfo(0xa003, "PixelYDimension", "PixelYDimension", "Valid image height", exifIfdId, imgConfig, unsignedLong, printValue),
        TagInfo(0xa004, "RelatedSoundFile", "RelatedSoundFile", "Related audio file", exifIfdId, relatedFile, asciiString, printValue),
        TagInfo(0xa005, "InteroperabilityTag", "InteroperabilityIFDPointer", "Interoperability IFD Pointer", exifIfdId, exifFormat, unsignedLong, printValue),
        TagInfo(0xa20b, "FlashEnergy", "Flash Energy", "Flash energy", exifIfdId, captureCond, unsignedRational, printValue),
        TagInfo(0xa20c, "SpatialFrequencyResponse", "Spatial Frequency Response", "Spatial frequency response", exifIfdId, captureCond, undefined, printValue),
        TagInfo(0xa20e, "FocalPlaneXResolution", "Focal Plane x-Resolution", "Focal plane X resolution", exifIfdId, captureCond, unsignedRational, printFloat),
        TagInfo(0xa20f, "FocalPlaneYResolution", "Focal Plane y-Resolution", "Focal plane Y resolution", exifIfdId, captureCond, unsignedRational, printFloat),
        TagInfo(0xa210, "FocalPlaneResolutionUnit", "Focal Plane Resolution Unit", "Focal plane resolution unit", exifIfdId, captureCond, unsignedShort, printUnit),
        TagInfo(0xa214, "SubjectLocation", "Subject Location", "Subject location", exifIfdId, captureCond, unsignedShort, printValue),
        TagInfo(0xa215, "ExposureIndex", "Exposure index", "Exposure index", exifIfdId, captureCond, unsignedRational, printValue),
        TagInfo(0xa217, "SensingMethod", "Sensing Method", "Sensing method", exifIfdId, captureCond, unsignedShort, print0xa217),
        TagInfo(0xa300, "FileSource", "File Source", "File source", exifIfdId, captureCond, undefined, print0xa300),
        TagInfo(0xa301, "SceneType", "Scene Type", "Scene type", exifIfdId, captureCond, undefined, print0xa301),
        TagInfo(0xa302, "CFAPattern", "CFA Pattern", "CFA pattern", exifIfdId, captureCond, undefined, printValue),
        TagInfo(0xa401, "CustomRendered", "Custom Rendered", "Custom image processing", exifIfdId, captureCond, unsignedShort, printValue),
        TagInfo(0xa402, "ExposureMode", "Exposure Mode", "Exposure mode", exifIfdId, captureCond, unsignedShort, print0xa402),
        TagInfo(0xa403, "WhiteBalance", "White Balance", "White balance", exifIfdId, captureCond, unsignedShort, print0xa403),
        TagInfo(0xa404, "DigitalZoomRatio", "Digital Zoom Ratio", "Digital zoom ratio", exifIfdId, captureCond, unsignedRational, print0xa404),
        TagInfo(0xa405, "FocalLengthIn35mmFilm", "Focal Length In 35mm Film", "Focal length in 35 mm film", exifIfdId, captureCond, unsignedShort, print0xa405),
        TagInfo(0xa406, "SceneCaptureType", "Scene Capture Type", "Scene capture type", exifIfdId, captureCond, unsignedShort, print0xa406),
        TagInfo(0xa407, "GainControl", "Gain Control", "Gain control", exifIfdId, captureCond, unsignedRational, print0xa407),
        TagInfo(0xa408, "Contrast", "Contrast", "Contrast", exifIfdId, captureCond, unsignedShort, print0xa408),
        TagInfo(0xa409, "Saturation", "Saturation", "Saturation", exifIfdId, captureCond, unsignedShort, print0xa409),
        TagInfo(0xa40a, "Sharpness", "Sharpness", "Sharpness", exifIfdId, captureCond, unsignedShort, print0xa40a),
        TagInfo(0xa40b, "DeviceSettingDescription", "Device Setting Description", "Device settings description", exifIfdId, captureCond, undefined, printValue),
        TagInfo(0xa40c, "SubjectDistanceRange", "Subject Distance Range", "Subject distance range", exifIfdId, captureCond, unsignedShort, print0xa40c),
        TagInfo(0xa420, "ImageUniqueID", "Image Unique ID", "Unique image ID", exifIfdId, otherTags, asciiString, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownExifTag)", "Unknown Exif tag", "Unknown Exif tag", ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    // GPS Info Tags
    static const TagInfo gpsTagInfo[] = {
        TagInfo(0x0000, "GPSVersionID", "GPSVersionID", "GPS tag version", gpsIfdId, gpsTags, unsignedByte, printValue),
        TagInfo(0x0001, "GPSLatitudeRef", "GPSLatitudeRef", "North or South Latitude", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0002, "GPSLatitude", "GPSLatitude", "Latitude", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0003, "GPSLongitudeRef", "GPSLongitudeRef", "East or West Longitude", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0004, "GPSLongitude", "GPSLongitude", "Longitude", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0005, "GPSAltitudeRef", "GPSAltitudeRef", "Altitude reference", gpsIfdId, gpsTags, unsignedByte, printValue),
        TagInfo(0x0006, "GPSAltitude", "GPSAltitude", "Altitude", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0007, "GPSTimeStamp", "GPSTimeStamp", "GPS time (atomic clock)", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0008, "GPSSatellites", "GPSSatellites", "GPS satellites used for measurement", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0009, "GPSStatus", "GPSStatus", "GPS receiver status", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x000a, "GPSMeasureMode", "GPSMeasureMode", "GPS measurement mode", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x000b, "GPSDOP", "GPSDOP", "Measurement precision", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x000c, "GPSSpeedRef", "GPSSpeedRef", "Speed unit", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x000d, "GPSSpeed", "GPSSpeed", "Speed of GPS receiver", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x000e, "GPSTrackRef", "GPSTrackRef", "Reference for direction of movement", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x000f, "GPSTrack", "GPSTrack", "Direction of movement", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0010, "GPSImgDirectionRef", "GPSImgDirectionRef", "Reference for direction of image", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0011, "GPSImgDirection", "GPSImgDirection", "Direction of image", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0012, "GPSMapDatum", "GPSMapDatum", "Geodetic survey data used", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0013, "GPSDestLatitudeRef", "GPSDestLatitudeRef", "Reference for latitude of destination", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0014, "GPSDestLatitude", "GPSDestLatitude", "Latitude of destination", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0015, "GPSDestLongitudeRef", "GPSDestLongitudeRef", "Reference for longitude of destination", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0016, "GPSDestLongitude", "GPSDestLongitude", "Longitude of destination", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0017, "GPSDestBearingRef", "GPSDestBearingRef", "Reference for bearing of destination", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x0018, "GPSDestBearing", "GPSDestBearing", "Bearing of destination", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x0019, "GPSDestDistanceRef", "GPSDestDistanceRef", "Reference for distance to destination", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x001a, "GPSDestDistance", "GPSDestDistance", "Distance to destination", gpsIfdId, gpsTags, unsignedRational, printValue),
        TagInfo(0x001b, "GPSProcessingMethod", "GPSProcessingMethod", "Name of GPS processing method", gpsIfdId, gpsTags, undefined, printValue),
        TagInfo(0x001c, "GPSAreaInformation", "GPSAreaInformation", "Name of GPS area", gpsIfdId, gpsTags, undefined, printValue),
        TagInfo(0x001d, "GPSDateStamp", "GPSDateStamp", "GPS date", gpsIfdId, gpsTags, asciiString, printValue),
        TagInfo(0x001e, "GPSDifferential", "GPSDifferential", "GPS differential correction", gpsIfdId, gpsTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownGpsTag)", "Unknown GPSInfo tag", "Unknown GPSInfo tag", ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    // Exif Interoperability IFD Tags
    static const TagInfo iopTagInfo[] = {
        TagInfo(0x0001, "InteroperabilityIndex", "InteroperabilityIndex", "Interoperability Identification", iopIfdId, iopTags, asciiString, printValue),
        TagInfo(0x0002, "InteroperabilityVersion", "InteroperabilityVersion", "Interoperability version", iopIfdId, iopTags, undefined, printValue),
        TagInfo(0x1000, "RelatedImageFileFormat", "RelatedImageFileFormat", "File format of image file", iopIfdId, iopTags, asciiString, printValue),
        TagInfo(0x1001, "RelatedImageWidth", "RelatedImageWidth", "Image width", iopIfdId, iopTags, unsignedLong, printValue),
        TagInfo(0x1002, "RelatedImageLength", "RelatedImageLength", "Image height", iopIfdId, iopTags, unsignedLong, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownIopTag)", "Unknown Exif Interoperability tag", "Unknown Exif Interoperability tag", ifdIdNotSet, sectionIdNotSet, invalidTypeId, printValue)
    };

    // Unknown Tag
    static const TagInfo unknownTag(0xffff, "Unknown tag", "Unknown tag", "Unknown tag", ifdIdNotSet, sectionIdNotSet, asciiString, printValue);

    std::ostream& TagTranslator::print(std::ostream& os, const Value& value) const
    {
        if (!pTagDetails_) return os << value;

        long l = value.toLong();

        long e = pTagDetails_[0].val_;
        int i = 1;
        for (; pTagDetails_[i].val_ != l && pTagDetails_[i].val_ != e; ++i) {}
        if (pTagDetails_[i].val_ == l) {
            os << pTagDetails_[i].label_;
        }
        else {
            os << "(" << l << ")";
        }
        return os;
    } // TagTranslator::print

    // Tag lookup lists with tag names, desc and where they (preferably) belong to;
    // this is an array with pointers to one list per IFD. The IfdId is used as the
    // index into the array.
    const TagInfo* ExifTags::tagInfos_[] = {
        0,
        ifdTagInfo, exifTagInfo, gpsTagInfo, iopTagInfo, ifdTagInfo,
        0
    };

    // Lookup list for registered makernote tag info tables
    const TagInfo* ExifTags::makerTagInfos_[];

    // All makernote ifd ids, in the same order as the tag infos in makerTagInfos_
    IfdId ExifTags::makerIfdIds_[];

    void ExifTags::registerBaseTagInfo(IfdId ifdId)
    {
        registerMakerTagInfo(ifdId, ifdTagInfo);
    }

    void ExifTags::registerMakerTagInfo(IfdId ifdId, const TagInfo* tagInfo)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS; ++i) {
            if (makerIfdIds_[i] == 0) {
                makerIfdIds_[i] = ifdId;
                makerTagInfos_[i] = tagInfo;
                break;
            }
        }
        if (i == MAX_MAKER_TAG_INFOS) throw Error(16);
    } // ExifTags::registerMakerTagInfo

    int ExifTags::tagInfoIdx(uint16_t tag, IfdId ifdId)
    {
        const TagInfo* tagInfo = tagInfos_[ifdId];
        if (tagInfo == 0) return -1;
        int idx;
        for (idx = 0; tagInfo[idx].tag_ != 0xffff; ++idx) {
            if (tagInfo[idx].tag_ == tag) return idx;
        }
        return -1;
    } // ExifTags::tagInfoIdx

    const TagInfo* ExifTags::makerTagInfo(uint16_t tag, IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        if (i == MAX_MAKER_TAG_INFOS) return 0;

        for (int k = 0; makerTagInfos_[i][k].tag_ != 0xffff; ++k) {
            if (makerTagInfos_[i][k].tag_ == tag) return &makerTagInfos_[i][k];
        }

        return 0;
    } // ExifTags::makerTagInfo

    const TagInfo* ExifTags::makerTagInfo(const std::string& tagName,
                                          IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        if (i == MAX_MAKER_TAG_INFOS) return 0;

        for (int k = 0; makerTagInfos_[i][k].tag_ != 0xffff; ++k) {
            if (makerTagInfos_[i][k].name_ == tagName) {
                return &makerTagInfos_[i][k];
            }
        }

        return 0;
    } // ExifTags::makerTagInfo

    bool ExifTags::isMakerIfd(IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        return i != MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != IfdId(0);
    }

    std::string ExifTags::tagName(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx != -1) return tagInfos_[ifdId][idx].name_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return tagInfo->name_;
        }
        std::ostringstream os;
        os << "0x" << std::setw(4) << std::setfill('0') << std::right
           << std::hex << tag;
        return os.str();
    } // ExifTags::tagName

    const char* ExifTags::tagTitle(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return unknownTag.title_;
            return tagInfos_[ifdId][idx].title_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return tagInfo->title_;
        }
        return "";
    } // ExifTags::tagTitle

    const char* ExifTags::tagDesc(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return unknownTag.desc_;
            return tagInfos_[ifdId][idx].desc_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return tagInfo->desc_;
        }
        return "";
    } // ExifTags::tagDesc

    const char* ExifTags::sectionName(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return sectionInfo_[unknownTag.sectionId_].name_;
            const TagInfo* tagInfo = tagInfos_[ifdId];
            return sectionInfo_[tagInfo[idx].sectionId_].name_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return sectionInfo_[tagInfo->sectionId_].name_;
        }
        return "";
    } // ExifTags::sectionName

    const char* ExifTags::sectionDesc(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx == -1) return sectionInfo_[unknownTag.sectionId_].desc_;
            const TagInfo* tagInfo = tagInfos_[ifdId];
            return sectionInfo_[tagInfo[idx].sectionId_].desc_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return sectionInfo_[tagInfo->sectionId_].desc_;
        }
        return "";
    } // ExifTags::sectionDesc

    uint16_t ExifTags::tag(const std::string& tagName, IfdId ifdId)
    {
        uint16_t tag = 0xffff;
        if (isExifIfd(ifdId)) {
            const TagInfo* tagInfo = tagInfos_[ifdId];
            if (tagInfo) {
                int idx;
                for (idx = 0; tagInfo[idx].tag_ != 0xffff; ++idx) {
                    if (tagInfo[idx].name_ == tagName) break;
                }
                tag = tagInfo[idx].tag_;
            }
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tagName, ifdId);
            if (tagInfo != 0) tag = tagInfo->tag_;
        }
        if (tag == 0xffff) {
            if (!isHex(tagName, 4, "0x")) throw Error(7, tagName, ifdId);
            std::istringstream is(tagName);
            is >> std::hex >> tag;
        }
        return tag;
    } // ExifTags::tag

    IfdId ExifTags::ifdIdByIfdItem(const std::string& ifdItem)
    {
        int i;
        for (i = int(lastIfdId) - 1; i > 0; --i) {
            if (ifdInfo_[i].item_ == ifdItem) break;
        }
        return IfdId(i);
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

    TypeId ExifTags::tagType(uint16_t tag, IfdId ifdId)
    {
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx != -1) return tagInfos_[ifdId][idx].typeId_;
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) return tagInfo->typeId_;
        }
        return unknownTag.typeId_;
    }

    std::ostream& ExifTags::printTag(std::ostream& os,
                                     uint16_t tag,
                                     IfdId ifdId,
                                     const Value& value)
    {
        if (value.count() == 0) return os;
        PrintFct fct = printValue;
        if (isExifIfd(ifdId)) {
            int idx = tagInfoIdx(tag, ifdId);
            if (idx != -1) {
                fct = tagInfos_[ifdId][idx].printFct_;
            }
        }
        if (isMakerIfd(ifdId)) {
            const TagInfo* tagInfo = makerTagInfo(tag, ifdId);
            if (tagInfo != 0) fct = tagInfo->printFct_;
        }
        return fct(os, value);
    } // ExifTags::printTag

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

    void ExifTags::makerTaglist(std::ostream& os, IfdId ifdId)
    {
        int i = 0;
        for (; i < MAX_MAKER_TAG_INFOS && makerIfdIds_[i] != ifdId; ++i);
        if (i != MAX_MAKER_TAG_INFOS) {
            const TagInfo* mnTagInfo = makerTagInfos_[i];
            for (int k=0; mnTagInfo[k].tag_ != 0xffff; ++k) {
                os << mnTagInfo[k] << "\n";
            }
        }
    } // ExifTags::makerTaglist

    const char* ExifKey::familyName_ = "Exif";

    ExifKey::ExifKey(const std::string& key)
        : tag_(0), ifdId_(ifdIdNotSet), ifdItem_(""),
          idx_(0), key_(key)
    {
        decomposeKey();
    }

    ExifKey::ExifKey(uint16_t tag, const std::string& ifdItem)
        : tag_(0), ifdId_(ifdIdNotSet), ifdItem_(""),
          idx_(0), key_("")
    {
        IfdId ifdId = ExifTags::ifdIdByIfdItem(ifdItem);
        if (ExifTags::isMakerIfd(ifdId)) {
            MakerNote::AutoPtr makerNote = MakerNoteFactory::create(ifdId);
            if (makerNote.get() == 0) throw Error(23, ifdId);
        }
        tag_ = tag;
        ifdId_ = ifdId;
        ifdItem_ = ifdItem;
        makeKey();
    }

    ExifKey::ExifKey(const Entry& e)
        : tag_(e.tag()), ifdId_(e.ifdId()),
          ifdItem_(ExifTags::ifdItem(e.ifdId())),
          idx_(e.idx()), key_("")
    {
        makeKey();
    }

    ExifKey::ExifKey(const ExifKey& rhs)
        : tag_(rhs.tag_), ifdId_(rhs.ifdId_), ifdItem_(rhs.ifdItem_),
          idx_(rhs.idx_), key_(rhs.key_)
    {
    }

    ExifKey::~ExifKey()
    {
    }

    ExifKey& ExifKey::operator=(const ExifKey& rhs)
    {
        if (this == &rhs) return *this;
        Key::operator=(rhs);
        tag_ = rhs.tag_;
        ifdId_ = rhs.ifdId_;
        ifdItem_ = rhs.ifdItem_;
        idx_ = rhs.idx_;
        key_ = rhs.key_;
        return *this;
    }

    std::string ExifKey::tagName() const
    {
        return ExifTags::tagName(tag_, ifdId_);
    }

    ExifKey::AutoPtr ExifKey::clone() const
    {
        return AutoPtr(clone_());
    }

    ExifKey* ExifKey::clone_() const
    {
        return new ExifKey(*this);
    }

    std::string ExifKey::sectionName() const
    {
        return ExifTags::sectionName(tag(), ifdId());
    }

    void ExifKey::decomposeKey()
    {
        // Get the family name, IFD name and tag name parts of the key
        std::string::size_type pos1 = key_.find('.');
        if (pos1 == std::string::npos) throw Error(6, key_);
        std::string familyName = key_.substr(0, pos1);
        if (familyName != std::string(familyName_)) {
            throw Error(6, key_);
        }
        std::string::size_type pos0 = pos1 + 1;
        pos1 = key_.find('.', pos0);
        if (pos1 == std::string::npos) throw Error(6, key_);
        std::string ifdItem = key_.substr(pos0, pos1 - pos0);
        if (ifdItem == "") throw Error(6, key_);
        std::string tagName = key_.substr(pos1 + 1);
        if (tagName == "") throw Error(6, key_);

        // Find IfdId
        IfdId ifdId = ExifTags::ifdIdByIfdItem(ifdItem);
        if (ifdId == ifdIdNotSet) throw Error(6, key_);
        if (ExifTags::isMakerIfd(ifdId)) {
            MakerNote::AutoPtr makerNote = MakerNoteFactory::create(ifdId);
            if (makerNote.get() == 0) throw Error(6, key_);
        }
        // Convert tag
        uint16_t tag = ExifTags::tag(tagName, ifdId);

        // Translate hex tag name (0xabcd) to a real tag name if there is one
        tagName = ExifTags::tagName(tag, ifdId);

        tag_ = tag;
        ifdId_ = ifdId;
        ifdItem_ = ifdItem;
        key_ = familyName + "." + ifdItem + "." + tagName;
    }

    void ExifKey::makeKey()
    {
        key_ =   std::string(familyName_)
               + "." + ifdItem_
               + "." + ExifTags::tagName(tag_, ifdId_);
    }

    // *************************************************************************
    // free functions

    bool isExifIfd(IfdId ifdId)
    {
        bool rc;
        switch (ifdId) {
        case ifd0Id:    rc = true; break;
        case exifIfdId: rc = true; break;
        case gpsIfdId:  rc = true; break;
        case iopIfdId:  rc = true; break;
        case ifd1Id:    rc = true; break;
        default:        rc = false; break;
        }
        return rc;
    } // isExifIfd

    std::ostream& operator<<(std::ostream& os, const TagInfo& ti)
    {
        ExifKey exifKey(ti.tag_, ExifTags::ifdItem(ti.ifdId_));
        return os << ExifTags::tagName(ti.tag_, ti.ifdId_) << ", "
                  << std::dec << ti.tag_ << ", "
                  << "0x" << std::setw(4) << std::setfill('0')
                  << std::right << std::hex << ti.tag_ << ", "
                  << ExifTags::ifdName(ti.ifdId_) << ", "
                  << exifKey.key() << ", "
                  << TypeInfo::typeName(
                      ExifTags::tagType(ti.tag_, ti.ifdId_)) << ", "
                  << ExifTags::tagDesc(ti.tag_, ti.ifdId_);
    }

    std::ostream& operator<<(std::ostream& os, const Rational& r)
    {
        return os << r.first << "/" << r.second;
    }

    std::istream& operator>>(std::istream& is, Rational& r)
    {
        int32_t nominator;
        int32_t denominator;
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
        uint32_t nominator;
        uint32_t denominator;
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
        Rational r = value.toRational();
        if (r.second != 0) return os << static_cast<long>(r.first) / r.second;
        return os << "(" << value << ")";
    } // printLong

    std::ostream& printFloat(std::ostream& os, const Value& value)
    {
        Rational r = value.toRational();
        if (r.second != 0) return os << static_cast<float>(r.first) / r.second;
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
            t.second = static_cast<uint32_t>(
                static_cast<float>(t.second) / t.first + 0.5);
            t.first = 1;
        }
        if (t.second > 1 && t.second < t.first) {
            t.first = static_cast<uint32_t>(
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
            std::ostringstream oss;
            oss.copyfmt(os);
            os << "F" << std::setprecision(2)
               << static_cast<float>(fnumber.first) / fnumber.second;
            os.copyfmt(oss);
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

    std::ostream& print0x9201(std::ostream& os, const Value& value)
    {
        URational ur = exposureTime(value.toFloat());
        os << ur.first;
        if (ur.second > 1) {
            os << "/" << ur.second;
        }
        return os << " s";
    }

    std::ostream& print0x9202(std::ostream& os, const Value& value)
    {
        std::ostringstream oss;
        oss.copyfmt(os);
        os << "F" << std::setprecision(2) << fnumber(value.toFloat());
        os.copyfmt(oss);

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
            int32_t d = gcd(bias.first, bias.second);
            int32_t num = std::abs(bias.first) / d;
            int32_t den = bias.second / d;
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
        else if (static_cast<uint32_t>(distance.first) == 0xffffffff) {
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

    // Todo: Implement this properly
    std::ostream& print0x9286(std::ostream& os, const Value& value)
    {
        if (value.size() > 8) {
            DataBuf buf(value.size());
            value.copy(buf.pData_, bigEndian);
            // Hack: Skip the leading 8-Byte character code, truncate
            // trailing '\0's and let the stream take care of the remainder
            std::string userComment(reinterpret_cast<char*>(buf.pData_) + 8, buf.size_ - 8);
            std::string::size_type pos = userComment.find_last_not_of('\0');
            os << userComment.substr(0, pos + 1);
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

    float fnumber(float apertureValue)
    {
        return static_cast<float>(std::exp(std::log(2.0) * apertureValue / 2));
    }

    URational exposureTime(float shutterSpeedValue)
    {
        URational ur(1, 1);
        double tmp = std::exp(std::log(2.0) * shutterSpeedValue);
        if (tmp > 1) {
            ur.second = static_cast<long>(tmp + 0.5);
        }
        else {
            ur.first = static_cast<long>(1/tmp + 0.5);
        }
        return ur;
    }

}                                       // namespace Exiv2
