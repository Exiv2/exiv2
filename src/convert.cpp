// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
  File:      convert.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Vladimir Nadvornik (vn) <nadvornik@suse.cz>
  History:   17-Mar-08, ahu: created basic converter framework
             20-May-08, vn:  added actual conversion logic
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "types.hpp"
#include "exif.hpp"
#include "iptc.hpp"
#include "xmp.hpp"
#include "convert.hpp"

// Adobe XMP Toolkit
#ifdef EXV_HAVE_XMP_TOOLKIT
# define TXMP_STRING_TYPE std::string
# include <XMP.hpp>
# include <XMP.incl_cpp>
# include <../src/MD5.h>
#endif // EXV_HAVE_XMP_TOOLKIT

// + standard includes

#include <iostream>
#include <iomanip>

// *****************************************************************************
namespace {

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    //! Metadata conversions.
    class Converter {
    public:
        /*!
          @brief Type for metadata converter functions, taking two key strings,
                 \em from and \em to.

          These functions have access to both the source and destination metadata
          containers and store the result directly in the destination container.
         */
        typedef void (Converter::*ConvertFct)(const char* from, const char* to);
        //! Structure to define conversions between two keys.
        struct Conversion {
            MetadataId  metadataId; //!< Type of metadata for the first key.
            const char* key1;       //!< First metadata key.
            const char* key2;       //!< Second metadata key (always an XMP key for now).
            ConvertFct  key1ToKey2; //!< Conversion from first to second key.
            ConvertFct  key2ToKey1; //!< Conversion from second to first key.
        };
    public:
        //! @name Creators
        //@{
        //! Constructor for Exif tags and XMP properties.
        Converter(ExifData& exifData, XmpData& xmpData);
        //! Constructor for Iptc tags and XMP properties.
        Converter(IptcData& iptcData, XmpData& xmpData);
        //@}
        //! @name Manipulators
        //@{
        //! Convert Exif or Iptc tags to XMP properties according to the conversion table.
        void cnvToXmp();
        //! Convert XMP properties to Exif or Iptc tags according to the conversion table.
        void cnvFromXmp();
        /*!
          @brief Set the erase flag.

          This flag indicates whether successfully converted source records are erased.
         */
        void setErase(bool onoff =true) { erase_ = onoff; }
        /*!
          @brief Set the overwrite flag.

          This flag indicates whether existing target records are overwritten.
         */
        void setOverwrite(bool onoff =true) { overwrite_ = onoff; }
        //@}
        //! @name Conversion functions (manipulators)
        //@{
        /*!
          @brief Simple Exif to XMP conversion function.

          Sets the XMP property to an XmpText value containing the Exif value string.
         */
        void cnvExifValue(const char* from, const char* to);
        /*!
          @brief Converts Exif tag with multiple components to XMP array.

          Converts Exif tag with multiple components to XMP array. This function is 
	  used for ComponentsConfiguration tag.
         */
        void cnvExifArray(const char* from, const char* to);
        /*!
          @brief Exif date to XMP conversion function.

          Sets the XMP property to an XmpText value containing date and time. This function
	  combines values from multiple Exif tags as described in XMP specification. It
	  is used for DateTime, DateTimeOriginal, DateTimeDigitized and GPSTimeStamp.
         */
        void cnvExifDate(const char* from, const char* to);
	/*!
          @brief Exif version to XMP conversion function.

          Converts ExifVersion tag to XmpText value.
         */
        void cnvExifVersion(const char* from, const char* to);
	/*!
          @brief Exif GPS version to XMP conversion function.

          Converts GPSVersionID tag to XmpText value.
         */
        void cnvExifGPSVersion(const char* from, const char* to);
	/*!
          @brief Exif Flash to XMP conversion function.

          Converts Flash tag to XMP structure.
         */
        void cnvExifFlash(const char* from, const char* to);
	/*!
          @brief Exif GPS coordinate to XMP conversion function.

          Converts GPS coordinates tag to XmpText value. It combines multiple exif tags
	  as described in XMP specification.
         */
        void cnvExifGPSCoord(const char* from, const char* to);
        /*!
          @brief Simple XMP to Exif conversion function.

          Sets the Exif tag according to the XMP property.
         */
        void cnvXmpValue(const char* from, const char* to);
        /*!
          @brief Converts XMP array to Exif tag with multiple components.

          Converts XMP array to Exif tag with multiple components. This function is 
	  used for ComponentsConfiguration tag.
         */
        void cnvXmpArray(const char* from, const char* to);
        /*!
          @brief XMP to Exif date conversion function.

          Converts the XmpText value to Exif date and time. This function
	  sets multiple Exif tags as described in XMP specification. It
	  is used for DateTime, DateTimeOriginal, DateTimeDigitized and GPSTimeStamp.
         */
        void cnvXmpDate(const char* from, const char* to);
	/*!
          @brief XMP to Exif version conversion function.

          Converts XmpText value to ExifVersion tag.
         */
	void cnvXmpVersion(const char* from, const char* to);
	/*!
          @brief XMP to Exif GPS version conversion function.

          Converts XmpText value to GPSVersionID tag.
         */
	void cnvXmpGPSVersion(const char* from, const char* to);
	/*!
          @brief XMP to Exif Flash conversion function.

          Converts XMP structure to Flash tag.
         */
        void cnvXmpFlash(const char* from, const char* to);
	/*!
          @brief XMP to Exif GPS coordinate conversion function.

          Converts XmpText value to GPS coordinates tags. It sets multiple Exif tags
	  as described in XMP specification.
         */
        void cnvXmpGPSCoord(const char* from, const char* to);


	void cnvIptcValue(const char* from, const char* to);
	void cnvXmpValueToIptc(const char* from, const char* to);


	/*!
          @brief Write exif:NativeDigest and tiff:NativeDigest properties to XMP.

	  Compute digests from Exif values and write them to  exif:NativeDigest 
	  and tiff:NativeDigest properties. This should be compatible with XMP SDK.
         */
	void writeExifDigest();
	/*!
          @brief Copies metadata in appropriate direction.

	  From values of exif:NativeDigest and tiff:NativeDigest detects which of
	  XMP and Exif was updated more recently and copies metadata in appropriate direction.
         */
	void syncExifWithXmp();
        //@}
        //! @name Accessors
        //@{
        //! Get the value of the erase flag, see also setErase(bool on).
        bool erase() const { return erase_; }
        //! Get the value of the overwrite flag, see also setOverwrite(bool on).
        bool overwrite() const { return overwrite_; }
        //@}
    private:
        bool prepareExifTarget(const char* to, bool force = false);
        bool prepareIptcTarget(const char* to, bool force = false);
        bool prepareXmpTarget(const char* to, bool force = false);
	std::string computeExifDigest(bool tiff);
	std::string computeIptcDigest();
        // DATA
        static const Conversion conversion_[];  //<! Conversion rules
        bool erase_;
	bool overwrite_;
        ExifData *exifData_;
        IptcData *iptcData_;
        XmpData  *xmpData_;

    }; // class Converter

    // order is important for computing digests
    const Converter::Conversion Converter::conversion_[] = {
        { mdExif, "Exif.Image.ImageWidth",                "Xmp.tiff.ImageWidth",                &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.ImageLength",               "Xmp.tiff.ImageLength",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.BitsPerSample",             "Xmp.tiff.BitsPerSample",             &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.Compression",               "Xmp.tiff.Compression",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.PhotometricInterpretation", "Xmp.tiff.PhotometricInterpretation", &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.Orientation",               "Xmp.tiff.Orientation",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.SamplesPerPixel",           "Xmp.tiff.SamplesPerPixe",            &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.PlanarConfiguration",       "Xmp.tiff.PlanarConfiguration",       &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.YCbCrSubSampling",          "Xmp.tiff.YCbCrSubSampling",          &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.YCbCrPositioning",          "Xmp.tiff.YCbCrPositioning",          &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.XResolution",               "Xmp.tiff.XResolution",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.YResolution",               "Xmp.tiff.YResolution",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.ResolutionUnit",            "Xmp.tiff.ResolutionUnit",            &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.TransferFunction",          "Xmp.tiff.TransferFunction",          &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.WhitePoint",                "Xmp.tiff.WhitePoint",                &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.PrimaryChromaticities",     "Xmp.tiff.PrimaryChromaticities",     &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.YCbCrCoefficients",         "Xmp.tiff.YCbCrCoefficients",         &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.ReferenceBlackWhite",       "Xmp.tiff.ReferenceBlackWhite",       &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.DateTime",                  "Xmp.tiff.DateTime",                  &Converter::cnvExifDate , &Converter::cnvXmpDate  },
        { mdExif, "Exif.Image.ImageDescription",          "Xmp.dc.description",                 &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.Make",                      "Xmp.tiff.Make",                      &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.Model",                     "Xmp.tiff.Model",                     &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.Software",                  "Xmp.tiff.Software",                  &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.Artist",                    "Xmp.dc.creator",                     &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Image.Copyright",                 "Xmp.dc.rights",                      &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.ExifVersion",               "Xmp.exif.ExifVersion",               &Converter::cnvExifVersion, &Converter::cnvXmpVersion },
        { mdExif, "Exif.Photo.FlashpixVersion",           "Xmp.exif.FlashpixVersion",           &Converter::cnvExifVersion, &Converter::cnvXmpVersion },
        { mdExif, "Exif.Photo.ColorSpace",                "Xmp.exif.ColorSpace",                &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.ComponentsConfiguration",   "Xmp.exif.ComponentsConfiguration",   &Converter::cnvExifArray, &Converter::cnvXmpArray },
        { mdExif, "Exif.Photo.CompressedBitsPerPixel",    "Xmp.exif.CompressedBitsPerPixel",    &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.PixelXDimension",           "Xmp.exif.PixelXDimension",           &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.PixelYDimension",           "Xmp.exif.PixelYDimension",           &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.UserComment",               "Xmp.exif.UserComment",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.RelatedSoundFile",          "Xmp.exif.RelatedSoundFile",          &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.DateTimeOriginal",          "Xmp.exif.DateTimeOriginal",          &Converter::cnvExifDate,  &Converter::cnvXmpDate  },
        { mdExif, "Exif.Photo.DateTimeDigitized",         "Xmp.exif.DateTimeDigitized",         &Converter::cnvExifDate,  &Converter::cnvXmpDate  },
        { mdExif, "Exif.Photo.ExposureTime",              "Xmp.exif.ExposureTime",              &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.FNumber",                   "Xmp.exif.FNumber",                   &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.ExposureProgram",           "Xmp.exif.ExposureProgram",           &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.SpectralSensitivity",       "Xmp.exif.SpectralSensitivity",       &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.ISOSpeedRatings",           "Xmp.exif.ISOSpeedRatings",           &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.OECF",                      "Xmp.exif.OECF",                      &Converter::cnvExifValue, &Converter::cnvXmpValue }, // FIXME ?
        { mdExif, "Exif.Photo.ShutterSpeedValue",         "Xmp.exif.ShutterSpeedValue",         &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.ApertureValue",             "Xmp.exif.ApertureValue",             &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.BrightnessValue",           "Xmp.exif.BrightnessValue",           &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.ExposureBiasValue",         "Xmp.exif.ExposureBiasValue",         &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.MaxApertureValue",          "Xmp.exif.MaxApertureValue",          &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.SubjectDistance",           "Xmp.exif.SubjectDistance",           &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.MeteringMode",              "Xmp.exif.MeteringMode",              &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.LightSource",               "Xmp.exif.LightSource",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.Flash",                     "Xmp.exif.Flash",                     &Converter::cnvExifFlash, &Converter::cnvXmpFlash },
        { mdExif, "Exif.Photo.FocalLength",               "Xmp.exif.FocalLength",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.SubjectArea",               "Xmp.exif.SubjectArea",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.FlashEnergy",               "Xmp.exif.FlashEnergy",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.SpatialFrequencyResponse",  "Xmp.exif.SpatialFrequencyResponse",  &Converter::cnvExifValue, &Converter::cnvXmpValue }, // FIXME ?
        { mdExif, "Exif.Photo.FocalPlaneXResolution",     "Xmp.exif.FocalPlaneXResolution",     &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.FocalPlaneYResolution",     "Xmp.exif.FocalPlaneYResolution",     &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.FocalPlaneResolutionUnit",  "Xmp.exif.FocalPlaneResolutionUnit",  &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.SubjectLocation",           "Xmp.exif.SubjectLocation",           &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.ExposureIndex",             "Xmp.exif.ExposureIndex",             &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.SensingMethod",             "Xmp.exif.SensingMethod",             &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.FileSource",                "Xmp.exif.FileSource",                &Converter::cnvExifValue, &Converter::cnvXmpValue }, // FIXME ?
        { mdExif, "Exif.Photo.SceneType",                 "Xmp.exif.SceneType",                 &Converter::cnvExifValue, &Converter::cnvXmpValue }, // FIXME ?
        { mdExif, "Exif.Photo.CFAPattern",                "Xmp.exif.CFAPattern",                &Converter::cnvExifValue, &Converter::cnvXmpValue }, // FIXME ?
        { mdExif, "Exif.Photo.CustomRendered",            "Xmp.exif.CustomRendered",            &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.ExposureMode",              "Xmp.exif.ExposureMode",              &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.WhiteBalance",              "Xmp.exif.WhiteBalance",              &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.DigitalZoomRatio",          "Xmp.exif.DigitalZoomRatio",          &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.FocalLengthIn35mmFilm",     "Xmp.exif.FocalLengthIn35mmFilm",     &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.SceneCaptureType",          "Xmp.exif.SceneCaptureType",          &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.GainControl",               "Xmp.exif.GainControl",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.Contrast",                  "Xmp.exif.Contrast",                  &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.Saturation",                "Xmp.exif.Saturation",                &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.Sharpness",                 "Xmp.exif.Sharpness",                 &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.DeviceSettingDescription",  "Xmp.exif.DeviceSettingDescription",  &Converter::cnvExifValue, &Converter::cnvXmpValue }, // FIXME ?
        { mdExif, "Exif.Photo.SubjectDistanceRange",      "Xmp.exif.SubjectDistanceRange",      &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.Photo.ImageUniqueID",             "Xmp.exif.ImageUniqueID",             &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSVersionID",            "Xmp.exif.GPSVersionID",              &Converter::cnvExifGPSVersion, &Converter::cnvXmpGPSVersion },
        { mdExif, "Exif.GPSInfo.GPSLatitude",             "Xmp.exif.GPSLatitude",               &Converter::cnvExifGPSCoord, &Converter::cnvXmpGPSCoord },
        { mdExif, "Exif.GPSInfo.GPSLongitude",            "Xmp.exif.GPSLongitude",              &Converter::cnvExifGPSCoord, &Converter::cnvXmpGPSCoord },
        { mdExif, "Exif.GPSInfo.GPSAltitudeRef",          "Xmp.exif.GPSAltitudeRef",            &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSAltitude",             "Xmp.exif.GPSAltitude",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSTimeStamp",            "Xmp.exif.GPSTimeStamp",              &Converter::cnvExifDate,  &Converter::cnvXmpDate  }, // FIXME ?
        { mdExif, "Exif.GPSInfo.GPSSatellites",           "Xmp.exif.GPSSatellites",             &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSStatus",               "Xmp.exif.GPSStatus",                 &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSMeasureMode",          "Xmp.exif.GPSMeasureMode",            &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSDOP",                  "Xmp.exif.GPSDOP",                    &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSSpeedRef",             "Xmp.exif.GPSSpeedRef",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSSpeed",                "Xmp.exif.GPSSpeed",                  &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSTrackRef",             "Xmp.exif.GPSTrackRef",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSTrack",                "Xmp.exif.GPSTrack",                  &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSImgDirectionRef",      "Xmp.exif.GPSImgDirectionRef",        &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSImgDirection",         "Xmp.exif.GPSImgDirection",           &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSMapDatum",             "Xmp.exif.GPSMapDatum",               &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSDestLatitude",         "Xmp.exif.GPSDestLatitude",           &Converter::cnvExifGPSCoord, &Converter::cnvXmpGPSCoord },
        { mdExif, "Exif.GPSInfo.GPSDestLongitude",        "Xmp.exif.GPSDestLongitude",          &Converter::cnvExifGPSCoord, &Converter::cnvXmpGPSCoord },
        { mdExif, "Exif.GPSInfo.GPSDestBearingRef",       "Xmp.exif.GPSDestBearingRef",         &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSDestBearing",          "Xmp.exif.GPSDestBearing",            &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSDestDistanceRef",      "Xmp.exif.GPSDestDistanceRef",        &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSDestDistance",         "Xmp.exif.GPSDestDistance",           &Converter::cnvExifValue, &Converter::cnvXmpValue },
        { mdExif, "Exif.GPSInfo.GPSProcessingMethod",     "Xmp.exif.GPSProcessingMethod",       &Converter::cnvExifValue, &Converter::cnvXmpValue }, // FIXME ?
        { mdExif, "Exif.GPSInfo.GPSAreaInformation",      "Xmp.exif.GPSAreaInformation",        &Converter::cnvExifValue, &Converter::cnvXmpValue }, // FIXME ?
        { mdExif, "Exif.GPSInfo.GPSDifferential",         "Xmp.exif.GPSDifferential",           &Converter::cnvExifValue, &Converter::cnvXmpValue },

        { mdIptc, "Iptc.Application2.ObjectName",         "Xmp.dc.title",                       &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.Urgency",            "Xmp.photoshop.Urgency",              &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.Category",           "Xmp.photoshop.Category",             &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.SuppCategory",       "Xmp.photoshop.SupplementalCategory", &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.Keywords",           "Xmp.dc.subject",                     &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.LocationName",       "Xmp.iptc.Location",                  &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.SpecialInstructions","Xmp.photoshop.Instruction",          &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.DateCreated",        "Xmp.photoshop.DateCreated",          &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.Byline",             "Xmp.dc.creator",                     &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.BylineTitle",        "Xmp.photoshop.AuthorsPosition",      &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.City",               "Xmp.photoshop.City",                 &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.ProvinceState",      "Xmp.photoshop.State",                &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.CountryCode",        "Xmp.iptc.CountryCode",               &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.CountryName",        "Xmp.photoshop.Country",              &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.TransmissionReference", "Xmp.photoshop.TransmissionReference", &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.Headline",            "Xmp.photoshop.Headline",            &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.Credit",             "Xmp.photoshop.Credit",               &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.Source",             "Xmp.photoshop.Source",               &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.Copyright",          "Xmp.dc.rights",                      &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.Caption",            "Xmp.dc.description",                 &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc },
        { mdIptc, "Iptc.Application2.Writer",             "Xmp.photoshop.CaptionWriter",        &Converter::cnvIptcValue, &Converter::cnvXmpValueToIptc }

    };

    Converter::Converter(ExifData& exifData, XmpData& xmpData)
        : erase_(false), overwrite_(true), exifData_(&exifData), iptcData_(NULL), xmpData_(&xmpData)
    {
    }

    Converter::Converter(IptcData& iptcData, XmpData& xmpData)
        : erase_(false), overwrite_(true), exifData_(NULL), iptcData_(&iptcData), xmpData_(&xmpData)
    {
    }

    void Converter::cnvToXmp()
    {
        for (unsigned int i = 0; i < EXV_COUNTOF(conversion_); ++i) {
            const Conversion& c = conversion_[i];
            if (   (c.metadataId == mdExif && exifData_)
                || (c.metadataId == mdIptc && iptcData_)) {
                EXV_CALL_MEMBER_FN(*this, c.key1ToKey2)(c.key1, c.key2);
            }
        }
    }

    void Converter::cnvFromXmp()
    {
        for (unsigned int i = 0; i < EXV_COUNTOF(conversion_); ++i) {
            const Conversion& c = conversion_[i];
            if (   (c.metadataId == mdExif && exifData_)
                || (c.metadataId == mdIptc && iptcData_)) {
                EXV_CALL_MEMBER_FN(*this, c.key2ToKey1)(c.key2, c.key1);
            }
        }
    }

    bool Converter::prepareExifTarget(const char* to, bool force)
    {
        Exiv2::ExifData::iterator pos = exifData_->findKey(ExifKey(to));
        if (pos == exifData_->end()) return true;
	if (!overwrite_ && !force) return false;
	exifData_->erase(pos);
	return true;
    }

    bool Converter::prepareIptcTarget(const char* to, bool force)
    {
        Exiv2::IptcData::iterator pos = iptcData_->findKey(IptcKey(to));
        if (pos == iptcData_->end()) return true;
	if (!overwrite_ && !force) return false;
	while ((pos = iptcData_->findKey(IptcKey(to))) != iptcData_->end())
	    iptcData_->erase(pos);
	return true;
    }

    bool Converter::prepareXmpTarget(const char* to, bool force)
    {
        Exiv2::XmpData::iterator pos = xmpData_->findKey(XmpKey(to));
        if (pos == xmpData_->end()) return true;
	if (!overwrite_ && !force) return false;
	xmpData_->erase(pos);
	return true;
    }

    void Converter::cnvExifValue(const char* from, const char* to)
    {
        Exiv2::ExifData::iterator pos = exifData_->findKey(ExifKey(from));
        if (pos == exifData_->end()) return;
        std::string value = pos->value().toString();
        if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
            return;
        }
	if (!prepareXmpTarget(to)) return;
        (*xmpData_)[to] = value;
        if (erase_) exifData_->erase(pos);
    }

    void Converter::cnvExifArray(const char* from, const char* to)
    {
        Exiv2::ExifData::iterator pos = exifData_->findKey(ExifKey(from));
        if (pos == exifData_->end()) return;
	if (!prepareXmpTarget(to)) return;
	int i;
	for (i = 0; i < pos->value().count(); i++) {
	    std::string value = pos->value().toString(i);
	    if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
        	std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
        	return;
	    }
	    (*xmpData_)[to] = value;
	}
        if (erase_) exifData_->erase(pos);
    }

    void Converter::cnvExifDate(const char* from, const char* to)
    {
        Exiv2::ExifData::iterator pos = exifData_->findKey(ExifKey(from));
        if (pos == exifData_->end()) return;
	if (!prepareXmpTarget(to)) return;
	int year, month, day, hour, min, sec;
        std::string subsec = "";
	char buf[30];

	if (std::string(from) != "Exif.GPSInfo.GPSTimeStamp") {
	    std::string value = pos->value().toString();
	    if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
		std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
		return;
	    }
	    if (sscanf(value.c_str(), "%d:%d:%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec) != 6) {
#ifndef SUPPRESS_WARNINGS
		std::cerr << "Warning: Failed to convert " << from << " to " << to 
			  << ", unable to parse '" << value << "'\n";
#endif            
		return;
	    }
	}
	else { // "Exif.GPSInfo.GPSTimeStamp"
	    double dhour = pos->value().toFloat(0);
	    double dmin = pos->value().toFloat(1);
	    double dsec = pos->value().toFloat(2);

	    if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
		std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
		return;
	    }
	
	    dsec = dhour * 3600.0 + dmin * 60.0 + dsec;
	
	    hour = (int)(dsec / 3600.0);
	    dsec -= hour * 3600;
	    min = (int)(dsec / 60.0);
	    dsec -= min * 60;
	    sec = (int)dsec;
	    dsec -= sec;
	    
	    snprintf(buf, sizeof(buf), "%.9f", dsec);
	    buf[sizeof(buf) - 1] = 0;
	    subsec = buf + 1; 

	    Exiv2::ExifData::iterator date_pos = exifData_->findKey(ExifKey("Exif.GPSInfo.GPSDateStamp"));
    	    if (date_pos == exifData_->end())
		date_pos = exifData_->findKey(ExifKey("Exif.Photo.DateTimeOriginal"));
    	    if (date_pos == exifData_->end())
		date_pos = exifData_->findKey(ExifKey("Exif.Photo.DateTimeDigitized"));
    	    if (date_pos == exifData_->end()) {
#ifndef SUPPRESS_WARNINGS
		std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
		return;
	    }
	    std::string value = date_pos->value().toString();
	    if (sscanf(value.c_str(), "%d:%d:%d", &year, &month, &day) != 3) {
#ifndef SUPPRESS_WARNINGS
		std::cerr << "Warning: Failed to convert " << from << " to " << to 
			  << ", unable to parse '" << value << "'\n";
#endif            
		return;
	    }
	}	     

	const char *subsec_tag = NULL;
	if (std::string(from) == "Exif.Image.DateTime") {
	    subsec_tag = "Exif.Photo.SubSecTime";
	}
	else if (std::string(from) == "Exif.Photo.DateTimeOriginal") {
	    subsec_tag = "Exif.Photo.SubSecTimeOriginal";
	}
	else if (std::string(from) == "Exif.Photo.DateTimeDigitized") {
	    subsec_tag = "Exif.Photo.SubSecTimeDigitized";
	}

	if (subsec_tag) {
	    Exiv2::ExifData::iterator subsec_pos = exifData_->findKey(ExifKey(subsec_tag));
    	    if (subsec_pos != exifData_->end() && !subsec_pos->value().toString().empty())
    		subsec = std::string(".") + subsec_pos->value().toString();
    	    if (erase_) exifData_->erase(subsec_pos);
	}
	
	
	snprintf(buf, sizeof(buf), "%4d-%02d-%02dT%02d:%02d:%02d%s", year, month, day, hour, min, sec, subsec.c_str());
	buf[sizeof(buf) - 1] = 0;
	
        (*xmpData_)[to] = buf;
        if (erase_) exifData_->erase(pos);
    }

    void Converter::cnvExifVersion(const char* from, const char* to)
    {
        Exiv2::ExifData::iterator pos = exifData_->findKey(ExifKey(from));
        if (pos == exifData_->end()) return;
	if (!prepareXmpTarget(to)) return;
	int i;
	std::ostringstream value;
	for (i=0; i < pos->value().count(); i++)
	    value << (char)pos->value().toLong(i);
        (*xmpData_)[to] = value.str();
        if (erase_) exifData_->erase(pos);
    }

    void Converter::cnvExifGPSVersion(const char* from, const char* to)
    {
        Exiv2::ExifData::iterator pos = exifData_->findKey(ExifKey(from));
        if (pos == exifData_->end()) return;
	if (!prepareXmpTarget(to)) return;
	int i;
	std::ostringstream value;
	for (i=0; i < pos->value().count(); i++) {
	    if (i > 0) value << '.';
	    value << pos->value().toLong(i);
	}
        (*xmpData_)[to] = value.str();
        if (erase_) exifData_->erase(pos);
    }

    void Converter::cnvExifFlash(const char* from, const char* to)
    {
        Exiv2::ExifData::iterator pos = exifData_->findKey(ExifKey(from));
        if (pos == exifData_->end()) return;
	if (!prepareXmpTarget(to)) return;
        int value = pos->value().toLong();
        if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
            return;
        }
	
	(*xmpData_)["Xmp.exif.Flash/exif:Fired"] = (bool)(value & 1);
	(*xmpData_)["Xmp.exif.Flash/exif:Return"] = (value >> 1) & 3;
	(*xmpData_)["Xmp.exif.Flash/exif:Mode"] = (value >> 3) & 3;
	(*xmpData_)["Xmp.exif.Flash/exif:Function"] = (bool)((value >> 5) & 1);
	(*xmpData_)["Xmp.exif.Flash/exif:RedEyeMode"] = (bool)((value >> 6) & 1);

        if (erase_) exifData_->erase(pos);
    }

    void Converter::cnvExifGPSCoord(const char* from, const char* to)
    {
        Exiv2::ExifData::iterator pos = exifData_->findKey(ExifKey(from));
        if (pos == exifData_->end()) return;
	if (!prepareXmpTarget(to)) return;
        if (pos->value().count() != 3) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
            return;
        }
        Exiv2::ExifData::iterator ref_pos = exifData_->findKey(ExifKey(std::string(from) + "Ref"));
        if (ref_pos == exifData_->end()) return;

	double deg = pos->value().toFloat(0);
	double min = pos->value().toFloat(1);
	double sec = pos->value().toFloat(2);
	
	sec = deg * 3600.0 + min * 60.0 + sec;
	
	int ideg = (int)(sec / 3600.0);
	sec -= ideg * 3600;
	int imin = (int)(sec / 60.0);
	sec -= imin * 60;
	
	char buf[30];
	
	snprintf(buf, sizeof(buf), "%d,%d,%.2f%c", ideg, imin, sec, ref_pos->value().toString().c_str()[0]);
	buf[sizeof(buf) - 1] = 0;

        (*xmpData_)[to] = buf;
        if (erase_) exifData_->erase(pos);
        if (erase_) exifData_->erase(ref_pos);
    }

    void Converter::cnvXmpValue(const char* from, const char* to)
    {
        Exiv2::XmpData::iterator pos = xmpData_->findKey(XmpKey(from));
        if (pos == xmpData_->end()) return;
	if (!prepareExifTarget(to)) return;
        std::string value = pos->value().toString();
        if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
            return;
        }
        (*exifData_)[to] = value;
        if (erase_) xmpData_->erase(pos);
    }

    void Converter::cnvXmpArray(const char* from, const char* to)
    {
	if (!prepareExifTarget(to)) return;
        Exiv2::XmpData::iterator pos = xmpData_->findKey(XmpKey(from));
        if (pos == xmpData_->end()) return;
	int i;
	std::ostringstream array;
	
	for (i = 0; i < pos->value().count(); i++) {
	    std::string value = pos->value().toString(i);
	    if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
		std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
		return;
	    }
	    array << value << " ";
	}
	(*exifData_)[to] = array.str();
        if (erase_) xmpData_->erase(pos);
    }

    void Converter::cnvXmpDate(const char* from, const char* to)
    {
#ifdef EXV_HAVE_XMP_TOOLKIT	
        Exiv2::XmpData::iterator pos = xmpData_->findKey(XmpKey(from));
        if (pos == xmpData_->end()) return;
	if (!prepareExifTarget(to)) return;
        std::string value = pos->value().toString();
        if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
            return;
        }
	XMP_DateTime datetime;
	SXMPUtils::ConvertToDate(value, &datetime);
	
	char buf[30];
	
	if (std::string(to) != "Exif.GPSInfo.GPSTimeStamp") {

//	    SXMPUtils::ConvertToLocalTime(&datetime);
	
	    snprintf(buf, sizeof(buf), "%4d:%02d:%02d %02d:%02d:%02d", (int)datetime.year, (int)datetime.month, 
	    	     (int)datetime.day, (int)datetime.hour, (int)datetime.minute, (int)datetime.second);
	    buf[sizeof(buf) - 1] = 0;
    	    (*exifData_)[to] = buf;

	    const char *subsec_tag = NULL;
	    if (std::string(to) == "Exif.Image.DateTime") {
	        subsec_tag = "Exif.Photo.SubSecTime";
	    }
	    else if (std::string(to) == "Exif.Photo.DateTimeOriginal") {
		subsec_tag = "Exif.Photo.SubSecTimeOriginal";
	    }
	    else if (std::string(to) == "Exif.Photo.DateTimeDigitized") {
		subsec_tag = "Exif.Photo.SubSecTimeDigitized";
	    }

	    if (subsec_tag) {
		prepareExifTarget(subsec_tag, true);

		if (datetime.nanoSecond) {
		    snprintf(buf, sizeof(buf), "%09d", (int)datetime.nanoSecond);
		    (*exifData_)[subsec_tag] = buf;
		}
	    }
	}
	else { // "Exif.GPSInfo.GPSTimeStamp"
	    Rational rhour = floatToRationalCast(datetime.hour);
	    Rational rmin = floatToRationalCast(datetime.minute);
	    Rational rsec = floatToRationalCast((float)datetime.second + datetime.nanoSecond / 1000000000.0);

	    std::ostringstream array;
	    array << rhour << " " << rmin << " " << rsec;
	    (*exifData_)[to] = array.str();

	    prepareExifTarget("Exif.GPSInfo.GPSDateStamp", true);
	    snprintf(buf, sizeof(buf), "%4d:%02d:%02d", (int)datetime.year, (int)datetime.month, (int)datetime.day);
	    buf[sizeof(buf) - 1] = 0;
    	    (*exifData_)["Exif.GPSInfo.GPSDateStamp"] = buf;
	}

        if (erase_) xmpData_->erase(pos);
#endif
    }

    void Converter::cnvXmpVersion(const char* from, const char* to)
    {
        Exiv2::XmpData::iterator pos = xmpData_->findKey(XmpKey(from));
        if (pos == xmpData_->end()) return;
	if (!prepareExifTarget(to)) return;
        std::string value = pos->value().toString();
        if (!pos->value().ok() || value.length() < 4) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
            return;
        }
	std::ostringstream array;
	
	array << (int)value[0] << " " << (int)value[1] << " " << (int)value[2] << " " << (int)value[3];

        (*exifData_)[to] = array.str();
        if (erase_) xmpData_->erase(pos);
	
    }

    void Converter::cnvXmpGPSVersion(const char* from, const char* to)
    {
        Exiv2::XmpData::iterator pos = xmpData_->findKey(XmpKey(from));
        if (pos == xmpData_->end()) return;
	if (!prepareExifTarget(to)) return;
        std::string value = pos->value().toString();
        if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
            return;
        }

	unsigned i;
	for (i = 0; i < value.length(); i++)
	    if (value[i] == '.') value[i] = ' ';

        (*exifData_)[to] = value;
        if (erase_) xmpData_->erase(pos);
	
    }

    void Converter::cnvXmpFlash(const char* from, const char* to)
    {
        Exiv2::XmpData::iterator pos = xmpData_->findKey(XmpKey(std::string(from) + "/exif:Fired"));
        if (pos == xmpData_->end()) return;
	if (!prepareExifTarget(to)) return;
	unsigned short value = 0;
        
        if (pos != xmpData_->end()) {
	    int fired = pos->value().toLong();
	    if (pos->value().ok()) 
		value |= fired & 1;
#ifndef SUPPRESS_WARNINGS
	    else
        	std::cerr << "Warning: Failed to convert " << std::string(from) + "/exif:Fired" << " to " << to << "\n";
#endif            
	}
        pos = xmpData_->findKey(XmpKey(std::string(from) + "/exif:Return"));
        if (pos != xmpData_->end()) {
	    int ret = pos->value().toLong();
	    if (pos->value().ok()) 
		value |= (ret & 3) << 1;
#ifndef SUPPRESS_WARNINGS
	    else
        	std::cerr << "Warning: Failed to convert " << std::string(from) + "/exif:Return" << " to " << to << "\n";
#endif            
	}
        pos = xmpData_->findKey(XmpKey(std::string(from) + "/exif:Mode"));
        if (pos != xmpData_->end()) {
	    int mode = pos->value().toLong();
	    if (pos->value().ok()) 
		value |= (mode & 3) << 3;
#ifndef SUPPRESS_WARNINGS
	    else
        	std::cerr << "Warning: Failed to convert " << std::string(from) + "/exif:Mode" << " to " << to << "\n";
#endif            
	}
        pos = xmpData_->findKey(XmpKey(std::string(from) + "/exif:Function"));
        if (pos != xmpData_->end()) {
	    int function = pos->value().toLong();
	    if (pos->value().ok()) 
		value |= (function & 1) << 5;
#ifndef SUPPRESS_WARNINGS
	    else
        	std::cerr << "Warning: Failed to convert " << std::string(from) + "/exif:Function" << " to " << to << "\n";
#endif            
	}
        pos = xmpData_->findKey(XmpKey(std::string(from) + "/exif:RedEyeMode"));
        if (pos != xmpData_->end()) {
	    int red = pos->value().toLong();
	    if (pos->value().ok()) 
		value |= (red & 1) << 6;
#ifndef SUPPRESS_WARNINGS
	    else
        	std::cerr << "Warning: Failed to convert " << std::string(from) + "/exif:RedEyeMode" << " to " << to << "\n";
#endif            
	}
	
        (*exifData_)[to] = value;
        if (erase_) xmpData_->erase(pos);
    }

    void Converter::cnvXmpGPSCoord(const char* from, const char* to)
    {
        Exiv2::XmpData::iterator pos = xmpData_->findKey(XmpKey(from));
        if (pos == xmpData_->end()) return;
	if (!prepareExifTarget(to)) return;
        std::string value = pos->value().toString();
        if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
            return;
        }

	float deg, min, sec;
	char ref, sep1, sep2;
	
	ref = value[value.length() - 1];
	value.erase(value.length() - 1);
	
	std::istringstream in(value);
	
	in >> deg >> sep1 >> min >> sep2;
	
	if (sep2 == ',') {
	    in >> sec;
	}
	else {
	    sec = (min - (int)min) * 60.0;
	    min = (int)min;
	    sep2 = ',';
	}

        if (in.bad() || 
	    !(ref == 'N' || ref == 'S' || ref == 'E' || ref == 'W') ||
	    sep1 != ',' || sep2 != ',' ||
	    !in.eof()) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
            return;
        }

	    
	Rational rdeg = floatToRationalCast(deg);
	Rational rmin = floatToRationalCast(min);
	Rational rsec = floatToRationalCast(sec);

	std::ostringstream array;
	array << rdeg << " " << rmin << " " << rsec;
        (*exifData_)[to] = array.str();

	prepareExifTarget((std::string(to) + "Ref").c_str(), true);
	char ref_str[2] = {ref, 0};
        (*exifData_)[std::string(to) + "Ref"] = ref_str;

        if (erase_) xmpData_->erase(pos);
    }

    void Converter::cnvIptcValue(const char* from, const char* to)
    {
        Exiv2::IptcData::iterator pos = iptcData_->findKey(IptcKey(from));
        if (pos == iptcData_->end()) return;
	if (!prepareXmpTarget(to)) return;
	while (pos != iptcData_->end()) {
	    if (pos->key() == from) {
		std::string value = pos->value().toString();
		if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
		    std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
		    ++pos;
		    continue;
		}
		(*xmpData_)[to] = value;
		if (erase_) {
		    pos = iptcData_->erase(pos);
		    continue;
		}
	    }
	    ++pos;
	}
    }

    void Converter::cnvXmpValueToIptc(const char* from, const char* to)
    {
        Exiv2::XmpData::iterator pos = xmpData_->findKey(XmpKey(from));
        if (pos == xmpData_->end()) return;
	if (!prepareIptcTarget(to)) return;
	int count = pos->value().count();
	int i;
	for (i = 0; i < count; i++) {
    	    std::string value = pos->value().toString();
    	    if (!pos->value().ok()) {
#ifndef SUPPRESS_WARNINGS
        	std::cerr << "Warning: Failed to convert " << from << " to " << to << "\n";
#endif            
        	continue;
	    }
	    (*iptcData_)[to] = value;
	}
        if (erase_) xmpData_->erase(pos);
    }

    std::string Converter::computeExifDigest(bool tiff)
    {
#ifdef EXV_HAVE_XMP_TOOLKIT	
	std::ostringstream res;
	MD5_CTX    context;
	unsigned char digest[16];

	MD5Init ( &context );
        for (unsigned int i = 0; i < EXV_COUNTOF(conversion_); ++i) {
            const Conversion& c = conversion_[i];
            if (c.metadataId == mdExif) {
		Exiv2::ExifKey key(c.key1);
		if (tiff && key.groupName() != "Image") continue;
		if (!tiff && key.groupName() == "Image") continue;
		
		if (!res.str().empty()) res << ',';
		res << key.tag();
    		Exiv2::ExifData::iterator pos = exifData_->findKey(key);
    		if (pos == exifData_->end()) continue;
		DataBuf data(pos->value().size());
		pos->value().copy(data.pData_, littleEndian /* FIXME ? */);
		MD5Update ( &context, data.pData_, data.size_);
            }
        }
	MD5Final ( digest, &context );
	res << ';';
	res << std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
	int i;
	for (i = 0; i < 16; i++)
	    res << (int)digest[i];

	return res.str();	
#else
	return std::string("");
#endif
    }

    void Converter::writeExifDigest()
    {
#ifdef EXV_HAVE_XMP_TOOLKIT	
	(*xmpData_)["Xmp.tiff.NativeDigest"] = computeExifDigest(true);
	(*xmpData_)["Xmp.exif.NativeDigest"] = computeExifDigest(false);
#endif
    }
    
    void Converter::syncExifWithXmp()
    {
        Exiv2::XmpData::iterator td = xmpData_->findKey(XmpKey("Xmp.tiff.NativeDigest"));
        Exiv2::XmpData::iterator ed = xmpData_->findKey(XmpKey("Xmp.exif.NativeDigest"));
        if (td != xmpData_->end() && ed != xmpData_->end()) {
	    if (td->value().toString() == computeExifDigest(true) &&
	        ed->value().toString() == computeExifDigest(false)) {
		// we have both digest and the values matches
		// xmp is up-to-date, we should update exif
		
		setOverwrite(true);
		setErase(false);
		
		cnvFromXmp();
		writeExifDigest();
		return;
	    }
	    else {
		// we have both digest and the values does not match
		// exif was modified after xmp, we should update xmp
		setOverwrite(true);
		setErase(false);
		
		cnvToXmp();
		writeExifDigest();
		return;
	    }
	}
	else {
	    // we don't have digest, it is probably the first conversion to xmp
	    setOverwrite(false); // to be safe
	    setErase(false);
		
	    cnvToXmp();
	    writeExifDigest();
	    return;
	}
    }

    std::string Converter::computeIptcDigest()
    {
#ifdef EXV_HAVE_XMP_TOOLKIT	
	std::ostringstream res;
	MD5_CTX    context;
	unsigned char digest[16];

	MD5Init ( &context );
	
	DataBuf data = iptcData_->copy();
	MD5Update ( &context, data.pData_, data.size_);
	MD5Final ( digest, &context );
	res << std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
	int i;
	for (i = 0; i < 16; i++)
	    res << (int)digest[i];

	return res.str();	
#else
	return std::string("");
#endif
    }

    // *************************************************************************
    // free functions
    void copyExifToXmp(const ExifData& exifData, XmpData& xmpData)
    {
        Converter converter(const_cast<ExifData&>(exifData), xmpData);
        converter.cnvToXmp();
    }

    void moveExifToXmp(ExifData& exifData, XmpData& xmpData)
    {
        Converter converter(const_cast<ExifData&>(exifData), xmpData);
        converter.setErase();
        converter.cnvToXmp();
    }

    void copyXmpToExif(const XmpData& xmpData, ExifData& exifData)
    {
        Converter converter(exifData, const_cast<XmpData&>(xmpData));
        converter.cnvFromXmp();
    }

    void moveXmpToExif(XmpData& xmpData, ExifData& exifData)
    {
        Converter converter(exifData, const_cast<XmpData&>(xmpData));
        converter.setErase();
        converter.cnvFromXmp();
    }

    void syncExifWithXmp(ExifData& exifData, XmpData& xmpData)
    {
        Converter converter(exifData, const_cast<XmpData&>(xmpData));
	converter.syncExifWithXmp();
    }

    void copyIptcToXmp(const IptcData& iptcData, XmpData& xmpData)
    {
        Converter converter(const_cast<IptcData&>(iptcData), xmpData);
        converter.cnvToXmp();
    }

    void moveIptcToXmp(IptcData& iptcData, XmpData& xmpData)
    {
        Converter converter(const_cast<IptcData&>(iptcData), xmpData);
        converter.setErase();
        converter.cnvToXmp();
    }

    void copyXmpToIptc(const XmpData& xmpData, IptcData& iptcData)
    {
        Converter converter(iptcData, const_cast<XmpData&>(xmpData));
        converter.cnvFromXmp();
    }

    void moveXmpToIptc(XmpData& xmpData, IptcData& iptcData)
    {
        Converter converter(iptcData, const_cast<XmpData&>(xmpData));
        converter.setErase();
        converter.cnvFromXmp();
    }

}                                       // namespace Exiv2

// *****************************************************************************
// local definitions
namespace {


}
