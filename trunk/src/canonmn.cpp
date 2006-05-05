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
  File:      canonmn.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Patrice Boissonneault (pb) <patrice@zepto.ca>
  History:   18-Feb-04, ahu: created
             07-Mar-04, ahu: isolated as a separate component
             27-Mar-06, pb:  sync with ExifTool
  Credits:   Canon MakerNote implemented according to the specification
             "EXIF MakerNote of Canon" <http://www.burren.cx/david/canon.html>
             by David Burren
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#include "types.hpp"
#include "canonmn.hpp"
#include "makernote.hpp"
#include "value.hpp"
#include "ifd.hpp"

// + standard includes
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cstring>
#include <cmath>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! @cond IGNORE
    CanonMakerNote::RegisterMn::RegisterMn()
    {
        MakerNoteFactory::registerMakerNote("Canon", "*", createCanonMakerNote);

        MakerNoteFactory::registerMakerNote(canonIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonCsIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonFlIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonSiIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonPaIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonCl2IfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonCiIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonCfIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonPiIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonCf2IfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonPfIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonPvIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonFiIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonPcIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonCbIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonFgIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonMiIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonPrIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonSnIfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonCb2IfdId, MakerNote::AutoPtr(new CanonMakerNote));
        MakerNoteFactory::registerMakerNote(canonClIfdId, MakerNote::AutoPtr(new CanonMakerNote));

        ExifTags::registerMakerTagInfo(canonIfdId, tagInfo_);
        ExifTags::registerMakerTagInfo(canonCsIfdId, tagInfoCs_);
        ExifTags::registerMakerTagInfo(canonFlIfdId, tagInfoFl_);
        ExifTags::registerMakerTagInfo(canonSiIfdId, tagInfoSi_);
        ExifTags::registerMakerTagInfo(canonPaIfdId, tagInfoPa_);
        ExifTags::registerMakerTagInfo(canonCl2IfdId, tagInfoCl2_);
        ExifTags::registerMakerTagInfo(canonCiIfdId, tagInfoCi_);
        ExifTags::registerMakerTagInfo(canonCfIfdId, tagInfoCf_);
        ExifTags::registerMakerTagInfo(canonPiIfdId, tagInfoPi_);
        ExifTags::registerMakerTagInfo(canonCf2IfdId, tagInfoCf2_);
        ExifTags::registerMakerTagInfo(canonPfIfdId, tagInfoPf_);
        ExifTags::registerMakerTagInfo(canonPvIfdId, tagInfoPv_);
        ExifTags::registerMakerTagInfo(canonFiIfdId, tagInfoFi_);
        ExifTags::registerMakerTagInfo(canonPcIfdId, tagInfoPc_);
        ExifTags::registerMakerTagInfo(canonCbIfdId, tagInfoCb_);
        ExifTags::registerMakerTagInfo(canonFgIfdId, tagInfoFg_);
        ExifTags::registerMakerTagInfo(canonMiIfdId, tagInfoMi_);
        ExifTags::registerMakerTagInfo(canonPrIfdId, tagInfoPr_);
        ExifTags::registerMakerTagInfo(canonSnIfdId, tagInfoSn_);
        ExifTags::registerMakerTagInfo(canonCb2IfdId, tagInfoCb2_);
        ExifTags::registerMakerTagInfo(canonClIfdId, tagInfoCl_);
    }
    //! @endcond

    // Canon MakerNote Tag Info
    const TagInfo CanonMakerNote::tagInfo_[] = {
        TagInfo(0x0000, "0x0000", "0x0000", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0001, "CameraSettings", "Camera Settings", "Various camera settings", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "FocalLength", "Focal Length", "Focal Length", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0003, "FlashInfo", "Flash Info", "Flash Info", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "ShotInfo", "Shot Info", "Various shot info", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0005, "Panorama", "Panorama", "Panorama", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0006, "ImageType", "Image Type", "Image type", canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0007, "FirmwareVersion", "Firmware Version", "Firmware version", canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x0008, "FileNumber", "File Number", "File number", canonIfdId, makerTags, unsignedLong, print0x0008),
        TagInfo(0x0009, "OwnerName", "Owner Name", "Owner Name", canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x000a, "ColorInfo", "Color Info", "Color Info (D30)", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000c, "SerialNumber", "Serial Number", "Camera serial number", canonIfdId, makerTags, unsignedLong, print0x000c),
        TagInfo(0x000d, "CameraInfo", "Camera Info", "Camera Info", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000e, "FileLength", "File Length", "File Length", canonIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x000f, "CustomFunctions", "Custom Functions", "Custom Functions (1D, 5D, 10D, 20D, 30D, 350D, D30, D60, Unknown)", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0010, "ModelID", "Model ID", "Model ID", canonIfdId, makerTags, unsignedLong, print0x0010),
        TagInfo(0x0012, "PictureInfo", "Picture Info", "Picture info", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0015, "SerialNumberFormat", "Serial Number Format", "Serial Number Format", canonIfdId, makerTags, unsignedLong, print0x0015),
        TagInfo(0x0090, "CustomFunctions2", "Custom Functions 2", "Custom Functions 2 (1D)", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0091, "PersonalFunctions", "Personal Functions", "Personal Functions", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0092, "PersonalFunctionValues", "Personal Function Values", "Personal Function Values", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0093, "FileInfo", "File Info", "File Info", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0094, "AFPointsUsed", "AF Points Used", "AF Points Used", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0095, "LensType", "Lens Type", "Lens Type", canonIfdId, makerTags, asciiString, printValue),
        TagInfo(0x00a0, "ProccessingInfo", "Proccessing Info", "Proccessing Info", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00a1, "ToneCurveTable", "Tone Curve Table", "Tone Curve Table", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00a2, "SharpnessTable", "Sharpness Table", "Sharpness Table", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00a3, "SharpnessFreqTable", "Sharpness Freq Table", "Sharpness Freq Table", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00a4, "WhiteBalanceTable", "White Balance Table", "White Balance Table", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00a9, "ColorBalance", "Color Balance", "Color Balance", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00ae, "ColorTemperature", "Color Temperature", "Color Temperature", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00b0, "Flags", "Flags", "Flags", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00b1, "ModifiedInfo", "Modified Info", "Modified Info", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00b2, "ToneCurveMatching", "Tone Curve Matching", "Tone Curve Matching", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00b3, "WhiteBalanceMatching", "White Balance Matching", "White Balance Matching", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00b4, "ColorSpace", "Color Space", "Color Space", canonIfdId, makerTags, unsignedShort, print0x00b4),
        TagInfo(0x00b5, "0x00b5", "0x00b5", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00b6, "PreviewImageInfo", "Preview Image Info", "Preview Image Info", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00c0, "0x00c0", "0x00c0", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00c1, "0x00c1", "0x00c1", "Unknown", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x00e0, "SensorInfo", "Sensor Info", "Sensor Info", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x4001, "ColorBalance", "Color Balance", "Color Balance (1, 2, 3, Unknown)", canonIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x4003, "ColorInfo", "Color Info", "Color Info", canonIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonMakerNoteTag)", "(UnknownCanonMakerNoteTag)", "Unknown CanonMakerNote tag", canonIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Camera Settings Tag Info
    const TagInfo CanonMakerNote::tagInfoCs_[] = {
        TagInfo(0x0001, "Macro", "Macro Mode", "Macro mode", canonCsIfdId, makerTags, unsignedShort, printCs0x0001),
        TagInfo(0x0002, "Selftimer", "Self-timer", "Self-timer", canonCsIfdId, makerTags, unsignedShort, printCs0x0002),
        TagInfo(0x0003, "Quality", "Quality", "Quality", canonCsIfdId, makerTags, unsignedShort, printCs0x0003),
        TagInfo(0x0004, "FlashMode", "Flash Mode", "Flash mode setting", canonCsIfdId, makerTags, unsignedShort, printCs0x0004),
        TagInfo(0x0005, "DriveMode", "Drive Mode", "Drive mode setting", canonCsIfdId, makerTags, unsignedShort, printCs0x0005),
        TagInfo(0x0006, "0x0006", "0x0006", "Unknown", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "FocusMode", "Focus Mode", "Focus mode setting", canonCsIfdId, makerTags, unsignedShort, printCs0x0007),
        TagInfo(0x0008, "0x0008", "0x0008", "Unknown", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0009, "0x0009", "0x0009", "Unknown", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000a, "ImageSize", "Image Size", "Image size", canonCsIfdId, makerTags, unsignedShort, printCs0x000a),
        TagInfo(0x000b, "EasyMode", "Easy Mode", "Easy shooting mode", canonCsIfdId, makerTags, unsignedShort, printCs0x000b),
        TagInfo(0x000c, "DigitalZoom", "Digital Zoom", "Digital zoom", canonCsIfdId, makerTags, unsignedShort, printCs0x000c),
        TagInfo(0x000d, "Contrast", "Contrast", "Contrast setting", canonCsIfdId, makerTags, unsignedShort, printCsLnh),
        TagInfo(0x000e, "Saturation", "Saturation", "Saturation setting", canonCsIfdId, makerTags, unsignedShort, printCsLnh),
        TagInfo(0x000f, "Sharpness", "Sharpness", "Sharpness setting", canonCsIfdId, makerTags, unsignedShort, printCsLnh),
        TagInfo(0x0010, "ISOSpeed", "Camera ISO Speed", "Camera ISO speed setting", canonCsIfdId, makerTags, unsignedShort, printCs0x0010),
        TagInfo(0x0011, "MeteringMode", "Metering Mode", "Metering mode setting", canonCsIfdId, makerTags, unsignedShort, printCs0x0011),
        TagInfo(0x0012, "FocusRange", "Focus Range", "Focus type setting", canonCsIfdId, makerTags, unsignedShort, printCs0x0012),
        TagInfo(0x0013, "AFPoint", "AF Point", "AF point selected", canonCsIfdId, makerTags, unsignedShort, printCs0x0013),
        TagInfo(0x0014, "ExposureMode", "Exposure Mode", "Exposure mode setting", canonCsIfdId, makerTags, unsignedShort, printCs0x0014),
        TagInfo(0x0015, "0x0015", "0x0015", "Unknown", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0016, "LensType", "Lens Type", "Lens Type", canonCsIfdId, makerTags, unsignedShort, printCs0x0016),
        TagInfo(0x0017, "LongFocal", "Long Focal", "Long Focal", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0018, "ShortFocal", "Short Focal", "Short Focal", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0019, "FocalUnits", "Focal Units", "Focal Units", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001a, "MaxAperture", "Max Aperture", "Max Aperture", canonCsIfdId, makerTags, unsignedShort, printCsAper),
        TagInfo(0x001b, "MinAperture", "Min Aperture", "Min Aperture", canonCsIfdId, makerTags, unsignedShort, printCsAper),
        TagInfo(0x001c, "FlashActivity", "Flash Activity", "Flash activity", canonCsIfdId, makerTags, unsignedShort, printCs0x001c),
        TagInfo(0x001d, "FlashDetails", "Flash Details", "Flash details", canonCsIfdId, makerTags, unsignedShort, printCs0x001d),
        TagInfo(0x001e, "0x001e", "0x001e", "Unknown", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001f, "0x001f", "0x001f", "Unknown", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0020, "FocusContinuous", "Focus Continuous", "Focus Continuous setting", canonCsIfdId, makerTags, unsignedShort, printCs0x0020),
        TagInfo(0x0021, "AESetting", "AE Setting", "AE Setting", canonCsIfdId, makerTags, unsignedShort, printCs0x0021),
        TagInfo(0x0022, "ImageStabilization", "Image Stabilization", "Image Stabilization", canonCsIfdId, makerTags, unsignedShort, printCs0x0022),
        TagInfo(0x0023, "DisplayAperture", "Display Aperture", "Display Aperture", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0024, "ZoomSourceWidth", "Zoom Source Width", "Zoom Source Width", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0025, "ZoomTargetWidth", "Zoom Target Width", "Zoom Target Width", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0026, "PhotoEffect", "Photo Effect", "Photo Effect", canonCsIfdId, makerTags, unsignedShort, printCs0x0026),
        TagInfo(0x0027, "0x0027", "0x0027", "Unknown", canonCsIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0028, "ColorTone", "Color Tone", "Color Tone", canonCsIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCsTag)", "(UnknownCanonCsTag)", "Unknown Canon Camera Settings tag", canonCsIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Focal length Tag Info
    const TagInfo CanonMakerNote::tagInfoFl_[] = {
        TagInfo(0x0000, "FocalType", "Focal Type", "Focal Type", canonFlIfdId, makerTags, unsignedShort, printFl0x0000),
        TagInfo(0x0001, "FocalLength", "Focal Length", "Focal Length", canonFlIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "FocalPlaneXSize", "Focal PlaneX Size", "Focal PlaneX Size", canonFlIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0003, "FocalPlaneYSize", "Focal PlaneY Size", "Focal PlaneY Size", canonFlIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonFlTag)", "(UnknownCanonFlTag)", "Unknown Focal Length tag", canonFlIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Shot Info Tag Info
    const TagInfo CanonMakerNote::tagInfoSi_[] = {
        TagInfo(0x0001, "0x0001", "0x0001", "Unknown", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "ISOSpeed", "ISO Speed Used", "ISO speed used", canonSiIfdId, makerTags, unsignedShort, printSi0x0002),
        TagInfo(0x0003, "0x0003", "0x0003", "Unknown", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "TargetAperture", "Target Aperture", "Target Aperture", canonSiIfdId, makerTags, unsignedShort, printCsAper),
        TagInfo(0x0005, "TargetExposureTime", "Target Exposure Time", "Target exposure time", canonSiIfdId, makerTags, unsignedShort, printSiExp),
        TagInfo(0x0006, "ExposureCompensation", "Exposure Compensation", "Exposure Compensation", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "WhiteBalance", "White Balance", "White balance setting", canonSiIfdId, makerTags, unsignedShort, printSi0x0007),
        TagInfo(0x0008, "SlowShutter", "Slow Shutter", "Slow Shutter", canonSiIfdId, makerTags, unsignedShort, printSi0x0008),
        TagInfo(0x0009, "SequenceNumber", "Sequence Number", "Sequence number (if in a continuous burst)", canonSiIfdId, makerTags, unsignedShort, printSi0x0009),
        TagInfo(0x000a, "0x000a", "0x000a", "Unknown", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000b, "0x000b", "0x000b", "Unknown", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000c, "0x000c", "0x000c", "Unknown", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000d, "FlashGuideNumber", "Flash Guide Number", "Flash Guide Number", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000e, "AFPointUsed", "AF Point Used", "AF point used", canonSiIfdId, makerTags, unsignedShort, printSi0x000e),
        TagInfo(0x000f, "FlashBias", "Flash Bias", "Flash exposure compensation", canonSiIfdId, makerTags, unsignedShort, printSi0x000f),
        TagInfo(0x0010, "AutoExposureBracketing", "Auto Exposure Bracketing", "Auto Exposure Bracketing", canonSiIfdId, makerTags, unsignedShort, printSi0x0010),
        TagInfo(0x0011, "AEBBracketValue", "AEB Bracket Value", "AEB Bracket Value", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0012, "0x0012", "0x0012", "Unknown", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0013, "FocusDistanceUpper", "Focus Distance Upper", "Focus Distance Upper (units are not clear)", canonSiIfdId, makerTags, unsignedShort, printSi0x0013),
        TagInfo(0x0014, "FocusDistanceLower", "Focus Distance Lower", "Focus Distance Lower (units are not clear)", canonSiIfdId, makerTags, unsignedShort, printSi0x0013),
        TagInfo(0x0015, "FNumber", "F Number", "F Number", canonSiIfdId, makerTags, unsignedShort, printCsAper),
        TagInfo(0x0016, "ExposureTime", "Exposure Time", "Exposure time", canonSiIfdId, makerTags, unsignedShort, printSiExp),
        TagInfo(0x0017, "0x0017", "0x0017", "Unknown", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0018, "BulbDuration", "Bulb Duration", "Bulb duration", canonSiIfdId, makerTags, unsignedLong, printValue),
        TagInfo(0x0019, "0x0019", "0x0019", "Unknown", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001a, "0x001a", "0x001a", "Unknown", canonSiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x001b, "AutoRotate", "Auto Rotate", "Auto rotate", canonSiIfdId, makerTags, unsignedShort, printSi0x001b),
        TagInfo(0x001d, "Selftimer", "Self-timer", "Self-timer", canonSiIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonSiTag)", "(UnknownCanonSiTag)", "Unknown Canon Shot Info tag", canonSiIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Panorama Tag Info
    const TagInfo CanonMakerNote::tagInfoPa_[] = {
        TagInfo(0x0001, "PanoramaFrame2", "Panorama Frame 2", "Panorama Frame 2", canonPaIfdId, makerTags, unsignedShort, printPa0x0001),
        TagInfo(0x0002, "PanoramaFrame", "Panorama Frame", "Panorama Frame", canonPaIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0005, "PanoramaDirection", "Panorama Direction", "Panorama Direction", canonPaIfdId, makerTags, unsignedShort, printPa0x0005),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonPaTag)", "(UnknownCanonPaTag)", "Unknown Panorama tag", canonPaIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Color Info 2 Tag Info
    const TagInfo CanonMakerNote::tagInfoCl2_[] = {
        TagInfo(0x0009, "ColorTemperature", "Color Temperature", "Color Temperature", canonCl2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000a, "ColorMatrix", "Color Matrix", "Color Matrix", canonCl2IfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCl2Tag)", "(UnknownCanonCl2Tag)", "Unknown Color info 2 tag", canonCl2IfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Camera Info Tag Info (used by the 1D, 1DS, 1DmkII, 1DSmkII and 5D)
    const TagInfo CanonMakerNote::tagInfoCi_[] = {
        TagInfo(4, "ExposureTime", "ExposureTime", "ExposureTime", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(10, "FocalLength", "FocalLength", "FocalLength", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(13, "LensType", "LensType", "LensType", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(18, "ShortFocal", "ShortFocal", "ShortFocal", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(20, "LongFocal", "LongFocal", "LongFocal", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(45, "FocalType", "FocalType", "FocalType", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(56, "AFPointsUsed5D", "AFPointsUsed5D", "AFPointsUsed5D", canonCiIfdId, makerTags, undefined, printValue),
        TagInfo(108, "PictureStyle", "PictureStyle", "PictureStyle", canonCiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(164, "FirmwareRevision", "FirmwareRevision", "FirmwareRevision", canonCiIfdId, makerTags, asciiString, printValue),
        TagInfo(172, "ShortOwnerName", "ShortOwnerName", "ShortOwnerName", canonCiIfdId, makerTags, asciiString, printValue),
        TagInfo(208, "ImageNumber", "ImageNumber", "ImageNumber", canonCiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(232, "ContrastStandard", "ContrastStandard", "ContrastStandard", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(233, "ContrastPortrait", "ContrastPortrait", "ContrastPortrait", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(234, "ContrastLandscape", "ContrastLandscape", "ContrastLandscape", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(235, "ContrastNeutral", "ContrastNeutral", "ContrastNeutral", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(236, "ContrastFaithful", "ContrastFaithful", "ContrastFaithful", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(237, "ContrastMonochrome", "ContrastMonochrome", "ContrastMonochrome", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(238, "ContrastUserDef1", "ContrastUserDef1", "ContrastUserDef1", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(239, "ContrastUserDef2", "ContrastUserDef2", "ContrastUserDef2", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(240, "ContrastUserDef3", "ContrastUserDef3", "ContrastUserDef3", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(241, "SharpnessStandard", "SharpnessStandard", "SharpnessStandard", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(242, "SharpnessPortrait", "SharpnessPortrait", "SharpnessPortrait", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(243, "SharpnessLandscape", "SharpnessLandscape", "SharpnessLandscape", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(244, "SharpnessNeutral", "SharpnessNeutral", "SharpnessNeutral", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(245, "SharpnessFaithful", "SharpnessFaithful", "SharpnessFaithful", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(246, "SharpnessMonochrome", "SharpnessMonochrome", "SharpnessMonochrome", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(247, "SharpnessUserDef1", "SharpnessUserDef1", "SharpnessUserDef1", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(248, "SharpnessUserDef2", "SharpnessUserDef2", "SharpnessUserDef2", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(249, "SharpnessUserDef3", "SharpnessUserDef3", "SharpnessUserDef3", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(250, "SaturationStandard", "SaturationStandard", "SaturationStandard", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(251, "SaturationPortrait", "SaturationPortrait", "SaturationPortrait", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(252, "SaturationLandscape", "SaturationLandscape", "SaturationLandscape", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(253, "SaturationNeutral", "SaturationNeutral", "SaturationNeutral", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(254, "SaturationFaithful", "SaturationFaithful", "SaturationFaithful", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(255, "FilterEffectMonochrome", "FilterEffectMonochrome", "FilterEffectMonochrome", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(256, "SaturationUserDef1", "SaturationUserDef1", "SaturationUserDef1", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(257, "SaturationUserDef2", "SaturationUserDef2", "SaturationUserDef2", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(258, "SaturationUserDef3", "SaturationUserDef3", "SaturationUserDef3", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(259, "ColorToneStandard", "ColorToneStandard", "ColorToneStandard", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(260, "ColorTonePortrait", "ColorTonePortrait", "ColorTonePortrait", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(261, "ColorToneLandscape", "ColorToneLandscape", "ColorToneLandscape", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(262, "ColorToneNeutral", "ColorToneNeutral", "ColorToneNeutral", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(263, "ColorToneFaithful", "ColorToneFaithful", "ColorToneFaithful", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(264, "ToningEffectMonochrome", "ToningEffectMonochrome", "ToningEffectMonochrome", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(265, "ColorToneUserDef1", "ColorToneUserDef1", "ColorToneUserDef1", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(266, "ColorToneUserDef2", "ColorToneUserDef2", "ColorToneUserDef2", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(267, "ColorToneUserDef3", "ColorToneUserDef3", "ColorToneUserDef3", canonCiIfdId, makerTags, unsignedByte, printValue),
        TagInfo(268, "UserDef1PictureStyle", "UserDef1PictureStyle", "UserDef1PictureStyle", canonCiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(270, "UserDef2PictureStyle", "UserDef2PictureStyle", "UserDef2PictureStyle", canonCiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(272, "UserDef3PictureStyle", "UserDef3PictureStyle", "UserDef3PictureStyle", canonCiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(284, "TimeStamp", "TimeStamp", "TimeStamp", canonCiIfdId, makerTags, unsignedLong, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCl2Tag)", "(UnknownCanonCl2Tag)", "Unknown Color info 2 tag", canonCiIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Custom Function Tag Info
    const TagInfo CanonMakerNote::tagInfoCf_[] = {
        TagInfo(0x0001, "NoiseReduction", "NoiseReduction", "Long exposure noise reduction", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0002, "ShutterAeLock", "ShutterAeLock", "Shutter/AE lock buttons", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0003, "MirrorLockup", "MirrorLockup", "Mirror lockup", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0004, "ExposureLevelIncrements", "ExposureLevelIncrements", "Tv/Av and exposure level", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0005, "AFAssist", "AFAssist", "AF assist light", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0006, "FlashSyncSpeedAv", "FlashSyncSpeedAv", "Shutter speed in Av mode", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0007, "AEBSequence", "AEBSequence", "AEB sequence/auto cancellation", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0008, "ShutterCurtainSync", "ShutterCurtainSync", "Shutter curtain sync", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x0009, "LensAFStopButton", "LensAFStopButton", "Lens AF stop button Fn. Switch", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000a, "FillFlashAutoReduction", "FillFlashAutoReduction", "Auto reduction of fill flash", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000b, "MenuButtonReturn", "MenuButtonReturn", "Menu button return position", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000c, "SetButtonFunction", "SetButtonFunction", "SET button func. when shooting", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000d, "SensorCleaning", "SensorCleaning", "Sensor cleaning", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000e, "SuperimposedDisplay", "SuperimposedDisplay", "Superimposed display", canonCfIfdId, makerTags, unsignedShort, printValue),
        TagInfo(0x000f, "ShutterReleaseNoCFCard", "ShutterReleaseNoCFCard", "Shutter Release W/O CF Card", canonCfIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCfTag)", "(UnknownCanonCfTag)", "Unknown Canon Custom Function tag", canonCfIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Picture Info Tag Info
    const TagInfo CanonMakerNote::tagInfoPi_[] = {
        TagInfo(2, "ImageWidth", "Canon Image Width", "Canon Image Width", canonPiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(3, "ImageHeight", "Canon Image Height", "Canon Image Height", canonPiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(4, "ImageWidthAsShot", "Canon Image Width As Shot", "Canon Image Width As Shot", canonPiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(5, "ImageHeightAsShot", "Canon Image Height As Shot", "Canon Image Height As Shot", canonPiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(22, "AFPointsUsed", "AF Points Used", "AF Points Used", canonPiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(26, "AFPointsUsed20D", "AF Points Used 20D", "AF Points Used2 0D", canonPiIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonPiTag)", "(UnknownCanonPiTag)", "Unknown Pi tag", canonPiIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Custom Functions 2 Tag Info
    const TagInfo CanonMakerNote::tagInfoCf2_[] = {
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCf2Tag)", "(UnknownCanonCf2Tag)", "Unknown Cf2 tag", canonCf2IfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Personal Functions Tag Info
    const TagInfo CanonMakerNote::tagInfoPf_[] = {
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonPfTag)", "(UnknownCanonPfTag)", "Unknown Pf tag", canonPfIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Personal Values Tag Info
    const TagInfo CanonMakerNote::tagInfoPv_[] = {
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonPvTag)", "(UnknownCanonPvTag)", "Unknown Pv tag", canonPvIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon File Info Tag Info
    const TagInfo CanonMakerNote::tagInfoFi_[] = {
        TagInfo(1, "FileNumber", "File Number", "File Number", canonFiIfdId, makerTags, unsignedLong, printValue),
        TagInfo(2, "ShutterCount", "Shutter Count", "Shutter Count", canonFiIfdId, makerTags, unsignedLong, printValue),
        TagInfo(3, "BracketMode", "Bracket Mode", "Bracket Mode", canonFiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(4, "BracketValue", "Bracket Value", "Bracket Value", canonFiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(5, "BracketShotNumber", "Bracket Shot Number", "Bracket Shot Number", canonFiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(6, "RawJpgQuality", "Raw Jpg Quality", "Raw Jpg Quality", canonFiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(7, "RawJpgSize", "Raw Jpg Size", "Raw Jpg Size", canonFiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(8, "NoiseReduction", "Noise Reduction", "Noise Reduction", canonFiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(9, "WBBracketMode", "WB Bracket Mode", "WB Bracket Mode", canonFiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(12, "WBBracketValueAB", "WB Bracket Value AB", "WB Bracket Value AB", canonFiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(13, "WBBracketValueGM", "WB Bracket Value GM", "WB Bracket Value GM", canonFiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(14, "FilterEffect", "Filter Effect", "Filter Effect", canonFiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(15, "ToningEffect", "Toning Effect", "Toning Effect", canonFiIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonFiTag)", "(UnknownCanonFiTag)", "Unknown Fi tag", canonFiIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Processing Tag Info
    const TagInfo CanonMakerNote::tagInfoPc_[] = {
        TagInfo(1, "ToneCurve", "ToneCurve", "ToneCurve", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(2, "Sharpness", "Sharpness", "Sharpness", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(3, "SharpnessFrequency", "SharpnessFrequency", "SharpnessFrequency", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(4, "SensorRedLevel", "SensorRedLevel", "SensorRedLevel", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(5, "SensorBlueLevel", "SensorBlueLevel", "SensorBlueLevel", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(6, "WhiteBalanceRed", "WhiteBalanceRed", "WhiteBalanceRed", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(7, "WhiteBalanceBlue", "WhiteBalanceBlue", "WhiteBalanceBlue", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(8, "WhiteBalance", "WhiteBalance", "WhiteBalance", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(9, "ColorTemperature", "ColorTemperature", "ColorTemperature", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(10, "PictureStyle", "PictureStyle", "PictureStyle", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(11, "DigitalGain", "DigitalGain", "DigitalGain", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(12, "WBShiftAB", "WBShiftAB", "WBShiftAB", canonPcIfdId, makerTags, unsignedShort, printValue),
        TagInfo(13, "WBShiftGM", "WBShiftGM", "WBShiftGM", canonPcIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonPcTag)", "(UnknownCanonPcTag)", "Unknown Pc tag", canonPcIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Color Balance Tag Info
    const TagInfo CanonMakerNote::tagInfoCb_[] = {
        TagInfo(0, "WB_RGGBLevelsAuto", "WB_RGGBLevelsAuto", "WB_RGGBLevelsAuto", canonCbIfdId, makerTags, unsignedShort, printValue),
        TagInfo(4, "WB_RGGBLevelsDaylight", "WB_RGGBLevelsDaylight", "WB_RGGBLevelsDaylight", canonCbIfdId, makerTags, unsignedShort, printValue),
        TagInfo(8, "WB_RGGBLevelsShade", "WB_RGGBLevelsShade", "WB_RGGBLevelsShade", canonCbIfdId, makerTags, unsignedShort, printValue),
        TagInfo(12, "WB_RGGBLevelsCloudy", "WB_RGGBLevelsCloudy", "WB_RGGBLevelsCloudy", canonCbIfdId, makerTags, unsignedShort, printValue),
        TagInfo(16, "WB_RGGBLevelsTungsten", "WB_RGGBLevelsTungsten", "WB_RGGBLevelsTungsten", canonCbIfdId, makerTags, unsignedShort, printValue),
        TagInfo(20, "WB_RGGBLevelsFluorescent", "WB_RGGBLevelsFluorescent", "WB_RGGBLevelsFluorescent", canonCbIfdId, makerTags, unsignedShort, printValue),
        TagInfo(24, "WB_RGGBLevelsFlash", "WB_RGGBLevelsFlash", "WB_RGGBLevelsFlash", canonCbIfdId, makerTags, unsignedShort, printValue),
        TagInfo(28, "WB_RGGBLevelsCustom", "WB_RGGBLevelsCustom", "WB_RGGBLevelsCustom", canonCbIfdId, makerTags, unsignedShort, printValue),
        TagInfo(32, "WB_RGGBLevelsKelvin", "WB_RGGBLevelsKelvin", "WB_RGGBLevelsKelvin", canonCbIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCbTag)", "(UnknownCanonCbTag)", "Unknown Cb tag", canonCbIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Flags Tag Info
    const TagInfo CanonMakerNote::tagInfoFg_[] = {
        TagInfo(1, "ModifiedParamFlag", "ModifiedParamFlag", "ModifiedParamFlag", canonFgIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonFgTag)", "(UnknownCanonFgTag)", "Unknown Fg tag", canonFgIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Modified Info Tag Info
    const TagInfo CanonMakerNote::tagInfoMi_[] = {
        TagInfo(1, "ModifiedToneCurve", "ModifiedToneCurve", "ModifiedToneCurve", canonMiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(2, "ModifiedSharpness", "ModifiedSharpness", "ModifiedSharpness", canonMiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(3, "ModifiedSharpnessFreq", "ModifiedSharpnessFreq", "ModifiedSharpnessFreq", canonMiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(4, "ModifiedSensorRedLevel", "ModifiedSensorRedLevel", "ModifiedSensorRedLevel", canonMiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(5, "ModifiedSensorBlueLevel", "ModifiedSensorBlueLevel", "ModifiedSensorBlueLevel", canonMiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(6, "ModifiedWhiteBalanceRed", "ModifiedWhiteBalanceRed", "ModifiedWhiteBalanceRed", canonMiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(7, "ModifiedWhiteBalanceBlue", "ModifiedWhiteBalanceBlue", "ModifiedWhiteBalanceBlue", canonMiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(8, "ModifiedWhiteBalance", "ModifiedWhiteBalance", "ModifiedWhiteBalance", canonMiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(9, "ModifiedColorTemp", "ModifiedColorTemp", "ModifiedColorTemp", canonMiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(10, "ModifiedPictureStyle", "ModifiedPictureStyle", "ModifiedPictureStyle", canonMiIfdId, makerTags, unsignedShort, printValue),
        TagInfo(11, "ModifiedDigitalGain", "ModifiedDigitalGain", "ModifiedDigitalGain", canonMiIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonMiTag)", "(UnknownCanonMiTag)", "Unknown Mi tag", canonMiIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Preview Image Info Tag Info
    const TagInfo CanonMakerNote::tagInfoPr_[] = {
        TagInfo(1, "PreviewQuality", "PreviewQuality", "PreviewQuality", canonPrIfdId, makerTags, unsignedLong, printValue),
        TagInfo(2, "PreviewImageLength", "PreviewImageLength", "PreviewImageLength", canonPrIfdId, makerTags, unsignedLong, printValue),
        TagInfo(3, "PreviewImageWidth", "PreviewImageWidth", "PreviewImageWidth", canonPrIfdId, makerTags, unsignedLong, printValue),
        TagInfo(4, "PreviewImageHeight", "PreviewImageHeight", "PreviewImageHeight", canonPrIfdId, makerTags, unsignedLong, printValue),
        TagInfo(5, "PreviewImageStart", "PreviewImageStart", "PreviewImageStart", canonPrIfdId, makerTags, unsignedLong, printValue),
        TagInfo(6, "PreviewFocalPlaneXResolution", "PreviewFocalPlaneXResolution", "PreviewFocalPlaneXResolution", canonPrIfdId, makerTags, unsignedRational, printValue),
        TagInfo(8, "PreviewFocalPlaneYResolution", "PreviewFocalPlaneYResolution", "PreviewFocalPlaneYResolution", canonPrIfdId, makerTags, unsignedRational, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonPrTag)", "(UnknownCanonPrTag)", "Unknown Pr tag", canonPrIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Sensor Info Tag Info
    const TagInfo CanonMakerNote::tagInfoSn_[] = {
        TagInfo(1, "SensorWidth", "SensorWidth", "SensorWidth", canonSnIfdId, makerTags, unsignedShort, printValue),
        TagInfo(2, "SensorHeight", "SensorHeight", "SensorHeight", canonSnIfdId, makerTags, unsignedShort, printValue),
        TagInfo(5, "SensorLeftBorder", "SensorLeftBorder", "SensorLeftBorder", canonSnIfdId, makerTags, unsignedShort, printValue),
        TagInfo(6, "SensorTopBorder", "SensorTopBorder", "SensorTopBorder", canonSnIfdId, makerTags, unsignedShort, printValue),
        TagInfo(7, "SensorRightBorder", "SensorRightBorder", "SensorRightBorder", canonSnIfdId, makerTags, unsignedShort, printValue),
        TagInfo(8, "SensorBottomBorder", "SensorBottomBorder", "SensorBottomBorder", canonSnIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonSnTag)", "(UnknownCanonSnTag)", "Unknown Sn tag", canonSnIfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Color Balance 2 Tag Info
    const TagInfo CanonMakerNote::tagInfoCb2_[] = {
        // this table is used by the 20D and 350D
        TagInfo(25, "WB_RGGBLevelsAsShot", "WB_RGGBLevelsAsShot", "WB_RGGBLevelsAsShot", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(29, "ColorTempAsShot", "ColorTempAsShot", "ColorTempAsShot", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(30, "WB_RGGBLevelsAuto", "WB_RGGBLevelsAuto", "WB_RGGBLevelsAuto", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(34, "ColorTempAuto", "ColorTempAuto", "ColorTempAuto", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(35, "WB_RGGBLevelsDaylight", "WB_RGGBLevelsDaylight", "WB_RGGBLevelsDaylight", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(39, "ColorTempDaylight", "ColorTempDaylight", "ColorTempDaylight", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(40, "WB_RGGBLevelsShade", "WB_RGGBLevelsShade", "WB_RGGBLevelsShade", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(44, "ColorTempShade", "ColorTempShade", "ColorTempShade", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(45, "WB_RGGBLevelsCloudy", "WB_RGGBLevelsCloudy", "WB_RGGBLevelsCloudy", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(49, "ColorTempCloudy", "ColorTempCloudy", "ColorTempCloudy", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(50, "WB_RGGBLevelsTungsten", "WB_RGGBLevelsTungsten", "WB_RGGBLevelsTungsten", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(54, "ColorTempTungsten", "ColorTempTungsten", "ColorTempTungsten", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(55, "WB_RGGBLevelsFluorescent", "WB_RGGBLevelsFluorescent", "WB_RGGBLevelsFluorescent", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(59, "ColorTempFluorescent", "ColorTempFluorescent", "ColorTempFluorescent", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(60, "WB_RGGBLevelsFlash", "WB_RGGBLevelsFlash", "WB_RGGBLevelsFlash", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(64, "ColorTempFlash", "ColorTempFlash", "ColorTempFlash", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(65, "WB_RGGBLevelsCustom1", "WB_RGGBLevelsCustom1", "WB_RGGBLevelsCustom1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(69, "ColorTempCustom1", "ColorTempCustom1", "ColorTempCustom1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(70, "WB_RGGBLevelsCustom2", "WB_RGGBLevelsCustom2", "WB_RGGBLevelsCustom2", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(74, "ColorTempCustom2", "ColorTempCustom2", "ColorTempCustom2", canonCb2IfdId, makerTags, unsignedShort, printValue),
        // this table is used by the 1DmkII and 1DSmkII
        TagInfo(24, "WB_RGGBLevelsAsShot", "WB_RGGBLevelsAsShot", "WB_RGGBLevelsAsShot", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(28, "ColorTempAsShot", "ColorTempAsShot", "ColorTempAsShot", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(29, "WB_RGGBLevelsUnknow", "WB_RGGBLevelsUnknow", "WB_RGGBLevelsUnknow", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(33, "ColorTempUnknow", "ColorTempUnknow", "ColorTempUnknow", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(34, "WB_RGGBLevelsAuto", "WB_RGGBLevelsAuto", "WB_RGGBLevelsAuto", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(38, "ColorTempAuto", "ColorTempAuto", "ColorTempAuto", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(39, "WB_RGGBLevelsDaylight", "WB_RGGBLevelsDaylight", "WB_RGGBLevelsDaylight", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(43, "ColorTempDaylight", "ColorTempDaylight", "ColorTempDaylight", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(44, "WB_RGGBLevelsShade", "WB_RGGBLevelsShade", "WB_RGGBLevelsShade", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(48, "ColorTempShade", "ColorTempShade", "ColorTempShade", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(49, "WB_RGGBLevelsCloudy", "WB_RGGBLevelsCloudy", "WB_RGGBLevelsCloudy", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(53, "ColorTempCloudy", "ColorTempCloudy", "ColorTempCloudy", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(54, "WB_RGGBLevelsTungsten", "WB_RGGBLevelsTungsten", "WB_RGGBLevelsTungsten", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(58, "ColorTempTungsten", "ColorTempTungsten", "ColorTempTungsten", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(59, "WB_RGGBLevelsFluorescent", "WB_RGGBLevelsFluorescent", "WB_RGGBLevelsFluorescent", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(63, "ColorTempFluorescent", "ColorTempFluorescent", "ColorTempFluorescent", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(64, "WB_RGGBLevelsKelvin", "WB_RGGBLevelsKelvin", "WB_RGGBLevelsKelvin", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(68, "ColorTempKelvin", "ColorTempKelvin", "ColorTempKelvin", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(69, "WB_RGGBLevelsFlash", "WB_RGGBLevelsFlash", "WB_RGGBLevelsFlash", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(73, "ColorTempFlash", "ColorTempFlash", "ColorTempFlash", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(74, "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(78, "ColorTempUnknown", "ColorTempUnknown", "ColorTempUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(79, "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(83, "ColorTempUnknown", "ColorTempUnknown", "ColorTempUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(84, "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(88, "ColorTempUnknown", "ColorTempUnknown", "ColorTempUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(89, "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(93, "ColorTempUnknown", "ColorTempUnknown", "ColorTempUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(94, "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(98, "ColorTempUnknown", "ColorTempUnknown", "ColorTempUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(99, "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(103, "ColorTempUnknown", "ColorTempUnknown", "ColorTempUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(104, "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(108, "ColorTempUnknown", "ColorTempUnknown", "ColorTempUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(109, "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", "WB_RGGBLevelsUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(113, "ColorTempUnknown", "ColorTempUnknown", "ColorTempUnknown", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(114, "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(118, "ColorTempUnknown1", "ColorTempUnknown1", "ColorTempUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(119, "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(123, "ColorTempUnknown1", "ColorTempUnknown1", "ColorTempUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(124, "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(128, "ColorTempUnknown1", "ColorTempUnknown1", "ColorTempUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(129, "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(133, "ColorTempUnknown1", "ColorTempUnknown1", "ColorTempUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(134, "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(138, "ColorTempUnknown1", "ColorTempUnknown1", "ColorTempUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(139, "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(143, "ColorTempUnknown1", "ColorTempUnknown1", "ColorTempUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(144, "WB_RGGBLevelsPC1", "WB_RGGBLevelsPC1", "WB_RGGBLevelsPC1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(148, "ColorTempPC1", "ColorTempPC1", "ColorTempPC1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(149, "WB_RGGBLevelsPC2", "WB_RGGBLevelsPC2", "WB_RGGBLevelsPC2", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(153, "ColorTempPC2", "ColorTempPC2", "ColorTempPC2", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(154, "WB_RGGBLevelsPC3", "WB_RGGBLevelsPC3", "WB_RGGBLevelsPC3", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(158, "ColorTempPC3", "ColorTempPC3", "ColorTempPC3", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(159, "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", "WB_RGGBLevelsUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(163, "ColorTempUnknown1", "ColorTempUnknown1", "ColorTempUnknown1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        // this table is used by the 5D and 1DmkIIN
        TagInfo(63, "WB_RGGBLevelsAsShot", "WB_RGGBLevelsAsShot", "WB_RGGBLevelsAsShot", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(67, "ColorTempAsShot", "ColorTempAsShot", "ColorTempAsShot", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(68, "WB_RGGBLevelsAuto", "WB_RGGBLevelsAuto", "WB_RGGBLevelsAuto", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(72, "ColorTempAuto", "ColorTempAuto", "ColorTempAuto", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(73, "WB_RGGBLevelsMeasured", "WB_RGGBLevelsMeasured", "WB_RGGBLevelsMeasured", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(77, "ColorTempMeasured", "ColorTempMeasured", "ColorTempMeasured", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(78, "WB_RGGBLevelsDaylight", "WB_RGGBLevelsDaylight", "WB_RGGBLevelsDaylight", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(82, "ColorTempDaylight", "ColorTempDaylight", "ColorTempDaylight", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(83, "WB_RGGBLevelsShade", "WB_RGGBLevelsShade", "WB_RGGBLevelsShade", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(87, "ColorTempShade", "ColorTempShade", "ColorTempShade", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(88, "WB_RGGBLevelsCloudy", "WB_RGGBLevelsCloudy", "WB_RGGBLevelsCloudy", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(92, "ColorTempCloudy", "ColorTempCloudy", "ColorTempCloudy", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(93, "WB_RGGBLevelsTungsten", "WB_RGGBLevelsTungsten", "WB_RGGBLevelsTungsten", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(97, "ColorTempTungsten", "ColorTempTungsten", "ColorTempTungsten", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(98, "WB_RGGBLevelsFluorescent", "WB_RGGBLevelsFluorescent", "WB_RGGBLevelsFluorescent", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(102, "ColorTempFluorescent", "ColorTempFluorescent", "ColorTempFluorescent", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(103, "WB_RGGBLevelsKelvin", "WB_RGGBLevelsKelvin", "WB_RGGBLevelsKelvin", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(107, "ColorTempKelvin", "ColorTempKelvin", "ColorTempKelvin", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(108, "WB_RGGBLevelsFlash", "WB_RGGBLevelsFlash", "WB_RGGBLevelsFlash", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(112, "ColorTempFlash", "ColorTempFlash", "ColorTempFlash", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(113, "WB_RGGBLevelsPC1", "WB_RGGBLevelsPC1", "WB_RGGBLevelsPC1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(117, "ColorTempPC1", "ColorTempPC1", "ColorTempPC1", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(118, "WB_RGGBLevelsPC2", "WB_RGGBLevelsPC2", "WB_RGGBLevelsPC2", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(122, "ColorTempPC2", "ColorTempPC2", "ColorTempPC2", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(123, "WB_RGGBLevelsPC3", "WB_RGGBLevelsPC3", "WB_RGGBLevelsPC3", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(127, "ColorTempPC3", "ColorTempPC3", "ColorTempPC3", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(128, "WB_RGGBLevelsCustom", "WB_RGGBLevelsCustom", "WB_RGGBLevelsCustom", canonCb2IfdId, makerTags, unsignedShort, printValue),
        TagInfo(132, "ColorTempCustom", "ColorTempCustom", "ColorTempCustom", canonCb2IfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonCb2Tag)", "(UnknownCanonCb2Tag)", "Unknown Cb2 tag", canonCb2IfdId, makerTags, invalidTypeId, printValue)
    };

    // Canon Color Info Tag Info
    const TagInfo CanonMakerNote::tagInfoCl_[] = {
        TagInfo(1, "Saturation", "Saturation", "Saturation", canonClIfdId, makerTags, unsignedShort, printValue),
        TagInfo(2, "ColorHue", "ColorHue", "ColorHue", canonClIfdId, makerTags, unsignedShort, printValue),
        TagInfo(3, "ColorSpace", "ColorSpace", "ColorSpace", canonClIfdId, makerTags, unsignedShort, printValue),
        // End of list marker
        TagInfo(0xffff, "(UnknownCanonClTag)", "(UnknownCanonClTag)", "Unknown Cl tag", canonClIfdId, makerTags, invalidTypeId, printValue)
    };

    int CanonMakerNote::read(const byte* buf,
                             long len,
                             long start,
                             ByteOrder byteOrder,
                             long shift)
    {
        int rc = IfdMakerNote::read(buf, len, start, byteOrder, shift);
        if (rc) return rc;

        // Decode camera settings and add settings as additional entries
        Entries::iterator cs = ifd_.findTag(0x0001);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                if (c == 22 && cs->count() > 27) {
                    // add related lens info into same tag
                    addCsEntry(canonCsIfdId, c, cs->offset() + c*2, cs->data() + c*2, 6);
                }
                else {
                    addCsEntry(canonCsIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1);
                }
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x0002);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) { 
                addCsEntry(canonFlIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x0004);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonSiIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x0005);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonPaIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x000a);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonCl2IfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x000d);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonCiIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x000f);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonCfIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x0012);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonPiIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x0090);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonCf2IfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x0091);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonPfIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x0092);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonPvIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x0093);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonFiIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x00a0);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonPcIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x00a9);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonCbIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x00b0);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonFgIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x00b1);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonMiIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x00b6);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonPrIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x00e0);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonSnIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x4001);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonCb2IfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        cs = ifd_.findTag(0x4003);
        if (cs != ifd_.end()) {
            for (uint16_t c = 0; cs->count() > c; ++c) {
                addCsEntry(canonClIfdId, c, cs->offset() + c*2, cs->data() + c*2, 1); 
            }
            ifd_.erase(cs); // Discard the original entry
        }

        // Copy remaining ifd entries
        entries_.insert(entries_.begin(), ifd_.begin(), ifd_.end());

        // Set idx
        int idx = 0;
        Entries::iterator e = entries_.end();
        for (Entries::iterator i = entries_.begin(); i != e; ++i) {
            i->setIdx(++idx);
        }

        return 0;
    }

    void CanonMakerNote::addCsEntry(IfdId ifdId,
                                    uint16_t tag,
                                    long offset,
                                    const byte* data,
                                    int count)
    {
        Entry e(false);
        e.setIfdId(ifdId);
        e.setTag(tag);
        e.setOffset(offset);
        e.setValue(unsignedShort, count, data, 2*count);
        add(e);
    }

    void CanonMakerNote::add(const Entry& entry)
    {
        assert(alloc_ == entry.alloc());
        assert(   entry.ifdId() == canonIfdId
               || entry.ifdId() == canonCsIfdId
               || entry.ifdId() == canonFlIfdId
               || entry.ifdId() == canonSiIfdId
               || entry.ifdId() == canonPaIfdId
               || entry.ifdId() == canonCl2IfdId
               || entry.ifdId() == canonCiIfdId
               || entry.ifdId() == canonCfIfdId
               || entry.ifdId() == canonPiIfdId
               || entry.ifdId() == canonCf2IfdId
               || entry.ifdId() == canonPfIfdId
               || entry.ifdId() == canonPvIfdId
               || entry.ifdId() == canonFiIfdId
               || entry.ifdId() == canonPcIfdId
               || entry.ifdId() == canonCbIfdId
               || entry.ifdId() == canonFgIfdId
               || entry.ifdId() == canonMiIfdId
               || entry.ifdId() == canonPrIfdId
               || entry.ifdId() == canonSnIfdId
               || entry.ifdId() == canonCb2IfdId
               || entry.ifdId() == canonClIfdId);

        // allow duplicates
        entries_.push_back(entry);
    }

    long CanonMakerNote::copy(byte* buf, ByteOrder byteOrder, long offset)
    {
        if (byteOrder_ == invalidByteOrder) byteOrder_ = byteOrder;

        assert(ifd_.alloc());
        ifd_.clear();

        // Add all standard Canon entries to the IFD
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == canonIfdId) {
                ifd_.add(*i);
            }
        }

        // Collect different sub-entries and add the original Canon tag
        Entry cs;
        if (assemble(cs, canonCsIfdId, 0x0001, byteOrder_)) {
            ifd_.erase(0x0001); ifd_.add(cs); 
        }
        Entry fl;
        if (assemble(fl, canonFlIfdId, 0x0002, byteOrder_)) {
            ifd_.erase(0x0002); ifd_.add(fl); 
        }
        Entry si;
        if (assemble(si, canonSiIfdId, 0x0004, byteOrder_)) {
            ifd_.erase(0x0004); ifd_.add(si); 
        }
        Entry pa;
        if (assemble(pa, canonPaIfdId, 0x0005, byteOrder_)) {
            ifd_.erase(0x0005); ifd_.add(pa); 
        }
        Entry cl2;
        if (assemble(cl2, canonCl2IfdId, 0x000a, byteOrder_)) {
            ifd_.erase(0x000a); ifd_.add(cl2); 
        }
        Entry ci;
        if (assemble(ci, canonCiIfdId, 0x000d, byteOrder_)) {
            ifd_.erase(0x000d); ifd_.add(ci); 
        }
        Entry cf;
        if (assemble(cf, canonCfIfdId, 0x000f, byteOrder_)) {
            ifd_.erase(0x000f); ifd_.add(cf); 
        }
        Entry pi;
        if (assemble(pi, canonPiIfdId, 0x0012, byteOrder_)) {
            ifd_.erase(0x0012); ifd_.add(pi); 
        }
        Entry cf2;
        if (assemble(cf2, canonCf2IfdId, 0x0090, byteOrder_)) {
            ifd_.erase(0x0090); ifd_.add(cf2); 
        }
        Entry pf;
        if (assemble(pf, canonPfIfdId, 0x0091, byteOrder_)) {
            ifd_.erase(0x0091); ifd_.add(pf); 
        }
        Entry pv;
        if (assemble(pv, canonPvIfdId, 0x0092, byteOrder_)) {
            ifd_.erase(0x0092); ifd_.add(pv); 
        }
        Entry fi;
        if (assemble(fi, canonFiIfdId, 0x0093, byteOrder_)) {
            ifd_.erase(0x0093); ifd_.add(fi); 
        }
        Entry pc;
        if (assemble(pc, canonPcIfdId, 0x00a0, byteOrder_)) {
            ifd_.erase(0x00a0); ifd_.add(pc); 
        }
        Entry cb;
        if (assemble(cb, canonCbIfdId, 0x00a9, byteOrder_)) {
            ifd_.erase(0x00a9); ifd_.add(cb); 
        }
        Entry fg;
        if (assemble(fg, canonFgIfdId, 0x00b0, byteOrder_)) {
            ifd_.erase(0x00b0); ifd_.add(fg); 
        }
        Entry mi;
        if (assemble(mi, canonMiIfdId, 0x00b1, byteOrder_)) {
            ifd_.erase(0x00b1); ifd_.add(mi); 
        }
        Entry pr;
        if (assemble(pr, canonPrIfdId, 0x00b6, byteOrder_)) {
            ifd_.erase(0x00b6); ifd_.add(pr); 
        }
        Entry sn;
        if (assemble(sn, canonSnIfdId, 0x00e0, byteOrder_)) {
            ifd_.erase(0x00e0); ifd_.add(sn); 
        }
        Entry cb2;
        if (assemble(cb2, canonCb2IfdId, 0x4001, byteOrder_)) {
            ifd_.erase(0x4001); ifd_.add(cb2); 
        }
        Entry cl;
        if (assemble(cl, canonClIfdId, 0x4003, byteOrder_)) {
            ifd_.erase(0x4003); ifd_.add(cl); 
        }

        return IfdMakerNote::copy(buf, byteOrder_, offset);
    } // CanonMakerNote::copy

    void CanonMakerNote::updateBase(byte* pNewBase)
    {
        byte* pBase = ifd_.updateBase(pNewBase);
        if (absShift_ && !alloc_) {
            Entries::iterator end = entries_.end();
            for (Entries::iterator pos = entries_.begin(); pos != end; ++pos) {
                pos->updateBase(pBase, pNewBase);
            }
        }
    } // CanonMakerNote::updateBase

    long CanonMakerNote::size() const
    {
        Ifd ifd(canonIfdId, 0, alloc_); // offset doesn't matter

        // Add all standard Canon entries to the IFD
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == canonIfdId) {
                ifd.add(*i);
            }
        }

        // Collect different sub-entries and add the original Canon tag
        Entry cs(alloc_);
        if (assemble(cs, canonCsIfdId, 0x0001, littleEndian)) {
            ifd.erase(0x0001); ifd.add(cs); 
        }
        Entry fl(alloc_);
        if (assemble(fl, canonFlIfdId, 0x0002, littleEndian)) {
            ifd.erase(0x0002); ifd.add(fl); 
        }
        Entry si(alloc_);
        if (assemble(si, canonSiIfdId, 0x0004, littleEndian)) {
            ifd.erase(0x0004); ifd.add(si); 
        }
        Entry pa(alloc_);
        if (assemble(pa, canonPaIfdId, 0x0005, littleEndian)) {
            ifd.erase(0x0005); ifd.add(pa); 
        }
        Entry cl2(alloc_);
        if (assemble(cl2, canonCl2IfdId, 0x000a, littleEndian)) {
            ifd.erase(0x000a); ifd.add(cl2); 
        }
        Entry ci(alloc_);
        if (assemble(ci, canonCiIfdId, 0x000d, littleEndian)) {
            ifd.erase(0x000d); ifd.add(ci); 
        }
        Entry cf(alloc_);
        if (assemble(cf, canonCfIfdId, 0x000f, littleEndian)) {
            ifd.erase(0x000f); ifd.add(cf); 
        }
        Entry pi(alloc_);
        if (assemble(pi, canonPiIfdId, 0x0012, littleEndian)) {
            ifd.erase(0x0012); ifd.add(pi); 
        }
        Entry cf2(alloc_);
        if (assemble(cf2, canonCf2IfdId, 0x0090, littleEndian)) {
            ifd.erase(0x0090); ifd.add(cf2); 
        }
        Entry pf(alloc_);
        if (assemble(pf, canonPfIfdId, 0x0091, littleEndian)) {
            ifd.erase(0x0091); ifd.add(pf); 
        }
        Entry pv(alloc_);
        if (assemble(pv, canonPvIfdId, 0x0092, littleEndian)) {
            ifd.erase(0x0092); ifd.add(pv); 
        }
        Entry fi(alloc_);
        if (assemble(fi, canonFiIfdId, 0x0093, littleEndian)) {
            ifd.erase(0x0093); ifd.add(fi); 
        }
        Entry pc(alloc_);
        if (assemble(pc, canonPcIfdId, 0x00a0, littleEndian)) {
            ifd.erase(0x00a0); ifd.add(pc); 
        }
        Entry cb(alloc_);
        if (assemble(cb, canonCbIfdId, 0x00a9, littleEndian)) {
            ifd.erase(0x00a9); ifd.add(cb); 
        }
        Entry fg(alloc_);
        if (assemble(fg, canonFgIfdId, 0x00b0, littleEndian)) {
            ifd.erase(0x00b0); ifd.add(fg); 
        }
        Entry mi(alloc_);
        if (assemble(mi, canonMiIfdId, 0x00b1, littleEndian)) {
            ifd.erase(0x00b1); ifd.add(mi); 
        }
        Entry pr(alloc_);
        if (assemble(pr, canonPrIfdId, 0x00b6, littleEndian)) {
            ifd.erase(0x00b6); ifd.add(pr); 
        }
        Entry sn(alloc_);
        if (assemble(sn, canonSnIfdId, 0x00e0, littleEndian)) {
            ifd.erase(0x00e0); ifd.add(sn); 
        }
        Entry cb2(alloc_);
        if (assemble(cb2, canonCb2IfdId, 0x4001, littleEndian)) {
            ifd.erase(0x4001); ifd.add(cb2); 
        }
        Entry cl(alloc_);
        if (assemble(cl, canonClIfdId, 0x4003, littleEndian)) {
            ifd.erase(0x4003); ifd.add(cl); 
        }

        return headerSize() + ifd.size() + ifd.dataSize();
    } // CanonMakerNote::size

    long CanonMakerNote::assemble(Entry& e,
                                  IfdId ifdId,
                                  uint16_t tag,
                                  ByteOrder byteOrder) const
    {
        DataBuf buf(1024);
        memset(buf.pData_, 0x0, 1024);
        uint16_t len = 0;
        Entries::const_iterator end = entries_.end();
        for (Entries::const_iterator i = entries_.begin(); i != end; ++i) {
            if (i->ifdId() == ifdId) {
                uint16_t pos = i->tag() * 2;
                uint16_t size = pos + static_cast<uint16_t>(i->size());
                assert(size <= 1024);
                memcpy(buf.pData_ + pos, i->data(), i->size());
                if (len < size) len = size;
            }
        }
        if (len > 0) {
            // Number of shorts in the buffer (rounded up)
            uint16_t s = (len+1) / 2;
            us2Data(buf.pData_, s*2, byteOrder);

            e.setIfdId(canonIfdId);
            e.setIdx(0); // don't care
            e.setTag(tag);
            e.setOffset(0); // will be calculated when the IFD is written
            e.setValue(unsignedShort, s, buf.pData_, s*2);
        }

        return len;
    } // CanonMakerNote::assemble

    Entries::const_iterator CanonMakerNote::findIdx(int idx) const
    {
        return std::find_if(entries_.begin(), entries_.end(),
                            FindEntryByIdx(idx));
    }

    CanonMakerNote::CanonMakerNote(bool alloc)
        : IfdMakerNote(canonIfdId, alloc)
    {
    }

    CanonMakerNote::CanonMakerNote(const CanonMakerNote& rhs)
        : IfdMakerNote(rhs)
    {
        entries_ = rhs.entries_;
    }

    CanonMakerNote::AutoPtr CanonMakerNote::create(bool alloc) const
    {
        return AutoPtr(create_(alloc));
    }

    CanonMakerNote* CanonMakerNote::create_(bool alloc) const
    {
        return new CanonMakerNote(alloc);
    }

    CanonMakerNote::AutoPtr CanonMakerNote::clone() const
    {
        return AutoPtr(clone_());
    }

    CanonMakerNote* CanonMakerNote::clone_() const
    {
        return new CanonMakerNote(*this);
    }

    std::ostream& CanonMakerNote::print0x0008(std::ostream& os,
                                              const Value& value)
    {
        std::string n = value.toString();
        if (n.length() < 4) return os << "(" << n << ")";
        return os << n.substr(0, n.length() - 4) << "-"
                  << n.substr(n.length() - 4);
    }

    std::ostream& CanonMakerNote::print0x000c(std::ostream& os,
                                              const Value& value)
    {
        std::istringstream is(value.toString());
        uint32_t l;
        is >> l;
        return os << std::setw(4) << std::setfill('0') << std::hex
                  << ((l & 0xffff0000) >> 16)
                  << std::setw(5) << std::setfill('0') << std::dec
                  << (l & 0x0000ffff);
    }

    std::ostream& CanonMakerNote::print0x0010(std::ostream& os,
                                              const Value& value)
    {
        if (value.typeId() != unsignedLong) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0x1010000: os << "PowerShot A30"; break;
        case 0x1040000: os << "PowerShot S300 / Digital IXUS 300 / IXY Digital 300"; break;
        case 0x1060000: os << "PowerShot A20"; break;
        case 0x1080000: os << "PowerShot A10"; break;
        case 0x1090000: os << "PowerShot S110 / Digital IXUS v / IXY Digital 200"; break;
        case 0x1100000: os << "PowerShot G2"; break;
        case 0x1110000: os << "PowerShot S40"; break;
        case 0x1120000: os << "PowerShot S30"; break;
        case 0x1130000: os << "PowerShot A40"; break;
        case 0x1140000: os << "EOS D30"; break;
        case 0x1150000: os << "PowerShot A100"; break;
        case 0x1160000: os << "PowerShot S200 / Digital IXUS v2 / IXY Digital 200a"; break;
        case 0x1170000: os << "PowerShot A200"; break;
        case 0x1180000: os << "PowerShot S330 / Digital IXUS 330 / IXY Digital 300a"; break;
        case 0x1190000: os << "PowerShot G3"; break;
        case 0x1210000: os << "PowerShot S45"; break;
        case 0x1230000: os << "PowerShot SD100 / Digital IXUS II / IXY Digital 30"; break;
        case 0x1240000: os << "PowerShot S230 / Digital IXUS v3 / IXY Digital 320"; break;
        case 0x1250000: os << "PowerShot A70"; break;
        case 0x1260000: os << "PowerShot A60"; break;
        case 0x1270000: os << "PowerShot S400 / Digital IXUS 400 / IXY Digital 400"; break;
        case 0x1290000: os << "PowerShot G5"; break;
        case 0x1300000: os << "PowerShot A300"; break;
        case 0x1310000: os << "PowerShot S50"; break;
        case 0x1340000: os << "PowerShot A80"; break;
        case 0x1350000: os << "PowerShot SD10 / Digital IXUS i / IXY Digital L"; break;
        case 0x1360000: os << "PowerShot S1 IS"; break;
        case 0x1370000: os << "PowerShot Pro1"; break;
        case 0x1380000: os << "PowerShot S70"; break;
        case 0x1390000: os << "PowerShot S60"; break;
        case 0x1400000: os << "PowerShot G6"; break;
        case 0x1410000: os << "PowerShot S500 / Digital IXUS 500 / IXY Digital 500"; break;
        case 0x1420000: os << "PowerShot A75"; break;
        case 0x1440000: os << "PowerShot SD110 / Digital IXUS IIs / IXY Digital 30a"; break;
        case 0x1450000: os << "PowerShot A400"; break;
        case 0x1470000: os << "PowerShot A310"; break;
        case 0x1490000: os << "PowerShot A85"; break;
        case 0x1520000: os << "PowerShot S410 / Digital IXUS 430 / IXY Digital 450"; break;
        case 0x1530000: os << "PowerShot A95"; break;
        case 0x1540000: os << "PowerShot SD300 / Digital IXUS 40 / IXY Digital 50"; break;
        case 0x1550000: os << "PowerShot SD200 / Digital IXUS 30 / IXY Digital 40"; break;
        case 0x1560000: os << "PowerShot A520"; break;
        case 0x1570000: os << "PowerShot A510"; break;
        case 0x1590000: os << "PowerShot SD20 / Digital IXUS i5 / IXY Digital L2"; break;
        case 0x1640000: os << "PowerShot S2 IS"; break;
        case 0x1650000: os << "PowerShot SD430 / IXUS Wireless / IXY Wireless"; break;
        case 0x1660000: os << "PowerShot SD500 / Digital IXUS 700 / IXY Digital 600"; break;
        case 0x1668000: os << "EOS D60"; break;
        case 0x1700000: os << "PowerShot SD30 / Digital IXUS i zoom / IXY Digital L3"; break;
        case 0x1740000: os << "PowerShot A430"; break;
        case 0x1750000: os << "PowerShot A410"; break;
        case 0x1760000: os << "PowerShot S80"; break;
        case 0x1780000: os << "PowerShot A620"; break;
        case 0x1790000: os << "PowerShot A610"; break;
        case 0x1800000: os << "PowerShot SD630 / Digital IXUS 65 / IXY Digital 80"; break;
        case 0x1810000: os << "PowerShot SD450 / Digital IXUS 55 / IXY Digital 60"; break;
        case 0x1870000: os << "PowerShot SD400 / Digital IXUS 50 / IXY Digital 55"; break;
        case 0x1880000: os << "PowerShot A420"; break;
        case 0x1900000: os << "PowerShot SD550 / Digital IXUS 750 / IXY Digital 700"; break;
        case 0x1920000: os << "PowerShot A700"; break;
        case 0x1940000: os << "PowerShot SD700 IS / Digital IXUS 800 IS"; break;
        case 0x1950000: os << "PowerShot S3 IS"; break;
        case 0x1960000: os << "PowerShot A540"; break;
        case 0x1970000: os << "PowerShot SD600 / Digital IXUS 60 / IXY Digital 70"; break;
        case 0x1990000: os << "PowerShot A530"; break;
        case 0x3010000: os << "PowerShot Pro90 IS"; break;
        case 0x4040000: os << "PowerShot G1"; break;
        case 0x6040000: os << "PowerShot S100 / Digital IXUS / IXY Digital"; break;
        case 0x80000001: os << "EOS-1D"; break;
        case 0x80000167: os << "EOS-1DS"; break;
        case 0x80000168: os << "EOS 10D"; break;
        case 0x80000170: os << "EOS Digital Rebel / 300D / Kiss Digital"; break;
        case 0x80000174: os << "EOS-1D Mark II"; break;
        case 0x80000175: os << "EOS 20D"; break;
        case 0x80000188: os << "EOS-1Ds Mark II"; break;
        case 0x80000189: os << "EOS Digital Rebel XT / 350D / Kiss Digital N"; break;
        case 0x80000213: os << "EOS 5D"; break;
        case 0x80000232: os << "EOS-1D Mark II N"; break;
        case 0x80000234: os << "EOS 30D"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x0015(std::ostream& os,
                                              const Value& value)
    {
        if (value.typeId() != unsignedLong) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0x90000000: os << "Format 1"; break;
        case 0xa0000000: os << "Format 2"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::print0x00b4(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 1: os << "sRGB"; break;
        case 2: os << "Adobe RGB"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0001(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 1: os << "On"; break;
        case 2: os << "Off"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0002(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        if (l == 0) {
            os << "Off";
        }
        else {
				// Todo: custom mode.
            os << l / 10.0 << " s";
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0003(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 1: os << "Economy"; break;
        case 2: os << "Normal"; break;
        case 3: os << "Fine"; break;
        case 4: os << "RAW"; break;
        case 5: os << "Superfine"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0004(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Off"; break;
        case 1: os << "Auto"; break;
        case 2: os << "On"; break;
        case 3: os << "Red-eye reduction"; break;
        case 4: os << "Slow-sync"; break;
        case 5: os << "Red-eye reduction (Auto)"; break;
        case 6: os << "Red-eye reduction (On)"; break;
        case 16: os << "External flash"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0005(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Single"; break;
        case 1: os << "Continuous"; break;
        case 2: os << "Movie"; break;
        case 3: os << "Continuous, Speed Priority"; break;
        case 4: os << "Continuous, Low"; break;
        case 5: os << "Continuous, High"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0007(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "One-shot AF"; break;
        case 1: os << "AI Servo AF"; break;
        case 2: os << "AI Focus AF"; break;
        case 3: os << "Manual Focus"; break;
        case 4: os << "Single"; break;
        case 5: os << "Continuous"; break;
        case 6: os << "Manual Focus"; break;
        case 16: os << "Pan Focus"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x000a(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Large"; break;
        case 1: os << "Medium"; break;
        case 2: os << "Small"; break;
        case 5: os << "Medium 1"; break;
        case 6: os << "Medium 2"; break;
        case 7: os << "Medium 3"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x000b(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case  0: os << "Full auto"; break;
        case  1: os << "Manual"; break;
        case  2: os << "Landscape"; break;
        case  3: os << "Fast shutter"; break;
        case  4: os << "Slow shutter"; break;
        case  5: os << "Night Scene"; break;
        case  6: os << "Gray Scale"; break;
        case  7: os << "Sepia"; break;
        case  8: os << "Portrait"; break;
        case  9: os << "Sports"; break;
        case 10: os << "Macro"; break;
        case 11: os << "Black & White"; break;
        case 12: os << "Pan focus"; break;
        case 13: os << "Vivid"; break;
        case 14: os << "Neutral"; break;
        case 15: os << "Flash Off"; break;
        case 16: os << "Long shutter"; break;
        case 17: os << "Super Macro"; break;
        case 18: os << "Foliage"; break;
        case 19: os << "Indoor"; break;
        case 20: os << "Fireworks"; break;
        case 21: os << "Beach"; break;
        case 22: os << "Underwater"; break;
        case 23: os << "Snow"; break;
        case 24: os << "Kids & Pets"; break;
        case 25: os << "Night SnapShot"; break;
        case 26: os << "Digital Macro"; break;
        case 27: os << "My Colors"; break;
        case 28: os << "Movie"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x000c(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "None"; break;
        case 1: os << "2x"; break;
        case 2: os << "4x"; break;
        case 3: os << "Other"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCsLnh(std::ostream& os,
                                              const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0xffff: os << "Low"; break;
        case 0x0000: os << "Normal"; break;
        case 0x0001: os << "High"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0010(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case  0: os << "n/a"; break;
        case 15: os << "Auto"; break;
        case 16: os << "50"; break;
        case 17: os << "100"; break;
        case 18: os << "200"; break;
        case 19: os << "400"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0011(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Default"; break;
        case 1: os << "Spot"; break;
        case 2: os << "Average"; break;
        case 3: os << "Evaluative"; break;
        case 4: os << "Partial"; break;
        case 5: os << "Center-weighted averaging"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0012(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Manual"; break;
        case 1: os << "Auto"; break;
        case 2: os << "Auto-2"; break;
        case 3: os << "Macro"; break;
        case 4: os << "Very Close"; break;
        case 5: os << "Close"; break;
        case 6: os << "Middle Range"; break;
        case 7: os << "Far Range"; break;
        case 8: os << "Pan Focus"; break;
        case 9: os << "Super Macro"; break;
        case 10: os << "Infinity"; break;
        case 11: os << "Super Macro"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0013(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
//seems wrong        case 0x0000: os << "Right"; break;
//seems wrong        case 0x0001: os << "Center"; break;
//seems wrong        case 0x0002: os << "Left"; break;
        case 0x2005: os << "Manual AF point selection"; break;
        case 0x3000: os << "None (MF)"; break;
        case 0x3001: os << "Auto-selected"; break;
        case 0x3002: os << "Right"; break;
        case 0x3003: os << "Center"; break;
        case 0x3004: os << "Left"; break;
        case 0x4001: os << "Auto AF point selection"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0014(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Easy (Auto)"; break;
        case 1: os << "Program (P)"; break;
        case 2: os << "Shutter Speed Priority (Tv)"; break;
        case 3: os << "Aperture Priority (Av)"; break;
        case 4: os << "Manual (M)"; break;
        case 5: os << "Depth-of-field AE"; break;
        case 6: os << "M-DEP"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0016(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        if (value.count() < 6) return os << value;

        // first, try reading the official lens value
        long l = value.toLong(0);
        switch (l) {
        case 1: os << "Canon EF 50mm f/1.8"; break;
        case 2: os << "Canon EF 28mm f/2.8"; break;
        case 4: os << "Sigma UC Zoom 35-135mm f/4-5.6"; break;
        case 6: os << "Tokina AF193-2 19-35mm f/3.5-4.5"; break;
        case 10: os << "Canon EF 50mm f/2.5 Macro or Sigma 50mm f/2.8 EX or 28mm f/1.8"; break;
        case 11: os << "Canon EF 35mm f/2"; break;
        case 13: os << "Canon EF 15mm f/2.8"; break;
        case 21: os << "Canon EF 80-200mm f/2.8L"; break;
        case 26: os << "Cosina 100mm f/3.5 Macro AF"; break;
        case 28: os << "Tamron AF Aspherical 28-200mm f/3.8-5.6 or 28-200mm f/3.8-5.6"; break;
        case 29: os << "Canon EF 50mm f/1.8 MkII"; break;
        case 32: os << "Canon EF 24mm f/2.8 or Sigma 15mm f/2.8 EX Fisheye"; break;
        case 39: os << "Canon EF 75-300mm f/4-5.6"; break;
        case 40: os << "Canon EF 28-80mm f/3.5-5.6"; break;
        case 43: os << "Canon EF 28-105mm f/4-5.6"; break;
        case 124: os << "Canon MP-E 65mm f/2.8 1-5x Macro Photo"; break;
        case 125: os << "Canon TS-E 24mm f/3.5L"; break;
        case 130: os << "Canon EF 50mm 5/1.0 USM"; break;
        case 131: os << "Sigma 17-35mm f2.8-4 EX Aspherical HSM"; break;
        case 135: os << "Canon EF 200mm f/1.8L"; break;
        case 136: os << "Canon EF 300mm f/2.8L"; break;
        case 137: os << "Canon EF 85mm f/1.2L"; break;
        case 139: os << "Canon EF 400mm f/2.8L"; break;
        case 141: os << "Canon EF 500mm f/4.5L"; break;
        case 149: os << "Canon EF 100mm f/2"; break;
        case 150: os << "Canon EF 14mm f/2.8L or Sigma 20mm EX f/1.8"; break;
        case 151: os << "Canon EF 200mm f/2.8L"; break;
        case 153: os << "Canon EF 35-350mm f/3.5-5.6L"; break;
        case 155: os << "Canon EF 85mm f/1.8 USM"; break;
        case 156: os << "Canon EF 28-105mm f/3.5-4.5 USM"; break;
        case 160: os << "Canon EF 20-35mm f/3.5-4.5 USM"; break;
        case 161: os << "Canon EF 28-70mm f/2.8L or Sigma 24-70mm EX f/2.8"; break;
        case 165: os << "Canon EF 70-200mm f/2.8 L"; break;
        case 166: os << "Canon EF 70-200mm f/2.8 L + x1.4"; break;
        case 167: os << "Canon EF 70-200mm f/2.8 L + x2"; break;
        case 169: os << "Sigma 15-30mm f/3.5-4.5 EX DG Aspherical"; break;
        case 170: os << "Canon EF 200mm f/2.8L II"; break;
        case 173: os << "Canon EF 180mm Macro f/3.5L or Sigma 180mm EX HSM Macro f/3.5"; break;
        case 174: os << "Canon EF 135mm f/2L"; break;
        case 176: os << "Canon EF 24-85mm f/3.5-4.5 USM"; break;
        case 177: os << "Canon EF 300mm f/4L IS"; break;
        case 178: os << "Canon EF 28-135mm f/3.5-5.6 IS"; break;
        case 180: os << "Canon EF 35mm f/1.4L"; break;
        case 182: os << "Canon EF 100-400mm f/4.5-5.6L IS + x2"; break;
        case 183: os << "Canon EF 100-400mm f/4.5-5.6L IS"; break;
        case 186: os << "Canon EF 70-200mm f/4L"; break;
        case 190: os << "Canon EF 100mm f/2.8 Macro"; break;
        case 191: os << "Canon EF 400mm f/4 DO IS"; break;
        case 197: os << "Canon EF 75-300mm f/4-5.6 IS"; break;
        case 198: os << "Canon EF 50mm f/1.4 USM"; break;
        case 202: os << "Canon EF 28-80 f/3.5-5.6 USM IV"; break;
        case 213: os << "Canon EF 90-300mm f/4.5-5.6"; break;
        case 224: os << "Canon EF 70-200mm f/2.8L IS USM"; break;
        case 225: os << "Canon EF 70-200mm f/2.8L IS USM + x1.4"; break;
        case 229: os << "Canon EF 16-35mm f/2.8L"; break;
        case 230: os << "Canon EF 24-70mm f/2.8L"; break;
        case 231: os << "Canon EF 17-40mm f/4L"; break;
        default:
            // if not found, build a lens description with the next 5 tags
            // (long and short focal, focal units, max and min aperture)
            float fu = value.toFloat(3);
            if (fu == 0.0) return os << value;
            float len1 = value.toLong(1) / fu;
            float len2 = value.toLong(2) / fu;
            std::ostringstream oss;
            oss.copyfmt(os);
            os << std::fixed << std::setprecision(1)
               << len2;
            if( len2 != len1 )
                os << " - " << len1;
            os << " mm";

            long ap1 = value.toLong(4);
            long ap2 = value.toLong(5);
            if( ap1 != 65535 && ap2 != 65535 )
                os << ", F" << fnumber(canonEv(ap1)) << "/" << fnumber(canonEv(ap2));
            os.copyfmt(oss);
            break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCsAper(std::ostream& os,
                                              const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;

        std::ostringstream oss;
        oss.copyfmt(os);
        os << std::setprecision(2)
           << "F" << fnumber(canonEv(value.toLong()));
        os.copyfmt(oss);

        return os;
    }

    std::ostream& CanonMakerNote::printCs0x001c(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Did not fire"; break;
        case 1: os << "Fired"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x001d(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        bool coma = false;
        if (l & 0x4000) {
            if (coma) os << ", ";
            os << "External";
            coma = true;
        }
        if (l & 0x2000) {
            if (coma) os << ", ";
            os << "Built-in";
            coma = true;
        }
        if (l & 0x0800) {
            if (coma) os << ", ";
            os << "FP sync used";
            coma = true;
        }
        if (l & 0x0080) {
            if (coma) os << ", ";
            os << "2nd-curtain sync used";
            coma = true;
        }
        if (l & 0x0010) {
            if (coma) os << ", ";
            os << "FP sync enabled";
            coma = true;
        }
        if (l & 0x0008) {
            if (coma) os << ", ";
            os << "E-TTL";
            coma = true;
        }
        if (l & 0x0004) {
            if (coma) os << ", ";
            os << "A-TTL";
            coma = true;
        }
        if (l & 0x0002) {
            if (coma) os << ", ";
            os << "TTL";
            coma = true;
        }
        if (l & 0x0001) {
            if (coma) os << ", ";
            os << "Manual";
            coma = true;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0020(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Single"; break;
        case 1: os << "Continuous"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0021(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Normal AE"; break;
        case 1: os << "Exposure Compensation"; break;
        case 2: os << "AE Lock"; break;
        case 3: os << "AE Lock + Exposure Comp."; break;
        case 4: os << "No AE"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0022(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Off"; break;
        case 1: os << "On"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printCs0x0026(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Off"; break;
        case 1: os << "Vivid"; break;
        case 2: os << "Neutral"; break;
        case 3: os << "Smooth"; break;
        case 4: os << "Sepia"; break;
        case 5: os << "B&W"; break;
        case 6: os << "Custom"; break;
        case 100: os << "My Color Data"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printFl0x0000(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 1: os << "Fixed"; break;
        case 2: os << "Zoom"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0002(std::ostream& os,
                                                 const Value& value)
    {
        // Ported from Exiftool by Will Stokes
        return os << exp(canonEv(value.toLong()) * log(2.0)) * 100.0 / 32.0;
    }

    std::ostream& CanonMakerNote::printSi0x0007(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Auto"; break;
        case 1: os << "Day Light"; break;
        case 2: os << "Cloudy"; break;
        case 3: os << "Tungsten"; break;
        case 4: os << "Fluorescent"; break;
        case 5: os << "Flash"; break;
        case 6: os << "Custom"; break;
        case 7: os << "Black & White"; break;
        case 8: os << "Shade"; break;
        case 9: os << "Manual Temperature (Kelvin)"; break;
        case 10: os << "PC Set1"; break;
        case 11: os << "PC Set2"; break;
        case 12: os << "PC Set3"; break;
        case 14: os << "Fluorescent H (Daylight)"; break;
        case 15: os << "Custom 1"; break;
        case 16: os << "Custom 2"; break;
        case 17: os << "Underwater"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0008(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Off"; break;
        case 1: os << "Night Scene"; break;
        case 2: os << "On"; break;
        case 3: os << "None"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0009(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        os << l << "";
        // Todo: determine unit
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x000e(std::ostream& os,
                                                 const Value& value)
    {
      /* commented by Patrice, see comment below
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        long num = (l & 0xf000) >> 12;
        os << num << " focus points; ";
        long used = l & 0x0fff;
        if (used == 0) {
            os << "none";
        }
        else {
            bool coma = false;
            if (l & 0x0004) {
                if (coma) os << ", ";
                os << "left";
                coma = true;
            }
            if (l & 0x0002) {
                if (coma) os << ", ";
                os << "center";
                coma = true;
            }
            if (l & 0x0001) {
                if (coma) os << ", ";
                os << "right";
                coma = true;
            }
        }
        os << " used";
        return os;
      */

        // Exiftool way, which one is right?  
        // (used by D30, D60 and some PowerShot/Ixus models)
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0x3000: os << "None (MF)"; break;
        case 0x3001: os << "Right"; break;
        case 0x3002: os << "Center"; break;
        case 0x3003: os << "Center+Right"; break;
        case 0x3004: os << "Left"; break;
        case 0x3005: os << "Left+Right"; break;
        case 0x3006: os << "Left+Center"; break;
        case 0x3007: os << "All"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x000f(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0xffc0: os << "-2 EV"; break;
        case 0xffcc: os << "-1.67 EV"; break;
        case 0xffd0: os << "-1.50 EV"; break;
        case 0xffd4: os << "-1.33 EV"; break;
        case 0xffe0: os << "-1 EV"; break;
        case 0xffec: os << "-0.67 EV"; break;
        case 0xfff0: os << "-0.50 EV"; break;
        case 0xfff4: os << "-0.33 EV"; break;
        case 0x0000: os << "0 EV"; break;
        case 0x000c: os << "0.33 EV"; break;
        case 0x0010: os << "0.50 EV"; break;
        case 0x0014: os << "0.67 EV"; break;
        case 0x0020: os << "1 EV"; break;
        case 0x002c: os << "1.33 EV"; break;
        case 0x0030: os << "1.50 EV"; break;
        case 0x0034: os << "1.67 EV"; break;
        case 0x0040: os << "2 EV"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0010(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0xffff: os << "On"; break;
        case 0: os << "Off"; break;
        case 1: os << "On (shot 1)"; break;
        case 2: os << "On (shot 2)"; break;
        case 3: os << "On (shot 3)"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x0013(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        if (l == 0xffff || l == 0xfffa || l == 0x1999) {
            os << "Infinite";
        }
        else {
            os << l << "";
        }
        return os;
    }

    std::ostream& CanonMakerNote::printSi0x001b(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0xffff: os << "Rotated by Software"; break;
        case 0: os << "None"; break;
        case 1: os << "Rotate 90 CW"; break;
        case 2: os << "Rotate 180"; break;
        case 3: os << "Rotate 270 CW"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

    std::ostream& CanonMakerNote::printPa0x0001(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
           os << l / 256 + 1;
        return os;
    }

    std::ostream& CanonMakerNote::printPa0x0005(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;
        long l = value.toLong();
        switch (l) {
        case 0: os << "Left to Right"; break;
        case 1: os << "Right to Left"; break;
        case 2: os << "Bottom to Top"; break;
        case 3: os << "Top to Bottom"; break;
        case 4: os << "2x2 Matrix (Clockwise)"; break;
        default: os << "(" << l << ")"; break;
        }
        return os;
    }

	 std::ostream& CanonMakerNote::printSiExp(std::ostream& os,
                                                 const Value& value)
    {
        if (value.typeId() != unsignedShort) return os << value;

        URational ur = exposureTime(canonEv(value.toLong()));
        os << ur.first;
        if (ur.second > 1) {
            os << "/" << ur.second;
        }
        return os << " s";
    }

// *****************************************************************************
// free functions

    MakerNote::AutoPtr createCanonMakerNote(bool alloc,
                                            const byte* buf,
                                            long len,
                                            ByteOrder byteOrder,
                                            long offset)
    {
        return MakerNote::AutoPtr(new CanonMakerNote(alloc));
    }

    float canonEv(long val)
    {
        return (float)((val/32)+((val/32.0)-(val/32)));

      /*
        Patrice Boissonneault : There seems to have a bug in the canonEv
        function below, returning value not exactely as reality the new function
        above seems more precise (at least for Canon Powershot S2 IS).  Comments
        welcomed.  

        Note: the translaction from ExifTool was properly done.  This bug is
        also present in ExifTool.

        // temporarily remove sign
        int sign = 1;
        if (val < 0) {
            sign = -1;
            val = -val;
        }
        // remove fraction
        float frac = static_cast<float>(val & 0x1f);
        val -= long(frac);
        // convert 1/3 (0x0c) and 2/3 (0x14) codes
        if (frac == 0x0c) {
            frac = 32.0f / 3;
        }
        else if (frac == 0x14) {
            frac = 64.0f / 3;
        }
        return sign * (val + frac) / 32.0f;
      */
    }

}                                       // namespace Exiv2
