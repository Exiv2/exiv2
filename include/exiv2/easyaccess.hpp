// SPDX-License-Identifier: GPL-2.0-or-later

/*!
  @brief   Provides easy (high-level) access to some Exif meta data.
  @author  Carsten Pfeiffer <pfeiffer@kde.org>
  @date    28-Feb-09, gis: created
 */
#ifndef EASYACCESS_HPP_
#define EASYACCESS_HPP_

// *****************************************************************************
#include "exiv2lib_export.h"

// included header files
#include "exif.hpp"

namespace Exiv2 {
// *****************************************************************************
// class declarations
class ExifData;

//! Return the orientation of the image
EXIV2API ExifData::const_iterator orientation(const ExifData& ed);
//! Return the ISO speed used to shoot the image
EXIV2API ExifData::const_iterator isoSpeed(const ExifData& ed);
//! Return the date and time when the original image data was generated
EXIV2API ExifData::const_iterator dateTimeOriginal(const ExifData& ed);
//! Return the flash bias value
EXIV2API ExifData::const_iterator flashBias(const ExifData& ed);
//! Return the exposure mode setting
EXIV2API ExifData::const_iterator exposureMode(const ExifData& ed);
//! Return the scene mode setting
EXIV2API ExifData::const_iterator sceneMode(const ExifData& ed);
//! Return the macro mode setting
EXIV2API ExifData::const_iterator macroMode(const ExifData& ed);
//! Return the image quality setting
EXIV2API ExifData::const_iterator imageQuality(const ExifData& ed);
//! Return the white balance setting
EXIV2API ExifData::const_iterator whiteBalance(const ExifData& ed);
//! Return the name of the lens used
EXIV2API ExifData::const_iterator lensName(const ExifData& ed);
//! Return the saturation level
EXIV2API ExifData::const_iterator saturation(const ExifData& ed);
//! Return the sharpness level
EXIV2API ExifData::const_iterator sharpness(const ExifData& ed);
//! Return the contrast level
EXIV2API ExifData::const_iterator contrast(const ExifData& ed);
//! Return the scene capture type
EXIV2API ExifData::const_iterator sceneCaptureType(const ExifData& ed);
//! Return the metering mode setting
EXIV2API ExifData::const_iterator meteringMode(const ExifData& ed);
//! Return the camera make
EXIV2API ExifData::const_iterator make(const ExifData& ed);
//! Return the camera model
EXIV2API ExifData::const_iterator model(const ExifData& ed);
//! Return the exposure time
EXIV2API ExifData::const_iterator exposureTime(const ExifData& ed);
//! Return the F number
EXIV2API ExifData::const_iterator fNumber(const ExifData& ed);
//! Return the shutter speed value
EXIV2API ExifData::const_iterator shutterSpeedValue(const ExifData& ed);
//! Return the aperture value
EXIV2API ExifData::const_iterator apertureValue(const ExifData& ed);
//! Return the brightness value
EXIV2API ExifData::const_iterator brightnessValue(const ExifData& ed);
//! Return the exposure bias value
EXIV2API ExifData::const_iterator exposureBiasValue(const ExifData& ed);
//! Return the max aperture value
EXIV2API ExifData::const_iterator maxApertureValue(const ExifData& ed);
//! Return the subject distance
EXIV2API ExifData::const_iterator subjectDistance(const ExifData& ed);
//! Return the kind of light source
EXIV2API ExifData::const_iterator lightSource(const ExifData& ed);
//! Return the status of flash
EXIV2API ExifData::const_iterator flash(const ExifData& ed);
//! Return the camera serial number
EXIV2API ExifData::const_iterator serialNumber(const ExifData& ed);
//! Return the focal length setting
EXIV2API ExifData::const_iterator focalLength(const ExifData& ed);
//! Return the subject location and area
EXIV2API ExifData::const_iterator subjectArea(const ExifData& ed);
//! Return the flash energy
EXIV2API ExifData::const_iterator flashEnergy(const ExifData& ed);
//! Return the exposure index
EXIV2API ExifData::const_iterator exposureIndex(const ExifData& ed);
//! Return the image sensor type
EXIV2API ExifData::const_iterator sensingMethod(const ExifData& ed);
//! Return the AF point
EXIV2API ExifData::const_iterator afPoint(const ExifData& ed);

}  // namespace Exiv2

#endif  // EASYACCESS_HPP_
