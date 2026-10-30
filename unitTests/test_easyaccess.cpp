// SPDX-License-Identifier: GPL-2.0-or-later

// Tests of the easy access functions: which key each one finds in the Exif
// data of a file, and which key it falls back to when that one is removed.

#include <gtest/gtest.h>

#include <exiv2/easyaccess.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/image.hpp>
#include <exiv2/tags.hpp>

#include <algorithm>
#include <initializer_list>
#include <string>

using namespace Exiv2;

namespace {
using EasyAccessFct = ExifData::const_iterator (*)(const ExifData&);

struct Found {
  EasyAccessFct fct;
  const char* key;
  const char* value;  //!< as printed by Exifdatum::print()
};

#define EASY_ACCESS_FCT(name) {&name, #name}
const struct {
  EasyAccessFct fct;
  const char* name;
} allFcts[] = {
    EASY_ACCESS_FCT(orientation),
    EASY_ACCESS_FCT(isoSpeed),
    EASY_ACCESS_FCT(dateTimeOriginal),
    EASY_ACCESS_FCT(flashBias),
    EASY_ACCESS_FCT(exposureMode),
    EASY_ACCESS_FCT(sceneMode),
    EASY_ACCESS_FCT(macroMode),
    EASY_ACCESS_FCT(imageQuality),
    EASY_ACCESS_FCT(whiteBalance),
    EASY_ACCESS_FCT(lensName),
    EASY_ACCESS_FCT(saturation),
    EASY_ACCESS_FCT(sharpness),
    EASY_ACCESS_FCT(contrast),
    EASY_ACCESS_FCT(sceneCaptureType),
    EASY_ACCESS_FCT(meteringMode),
    EASY_ACCESS_FCT(make),
    EASY_ACCESS_FCT(model),
    EASY_ACCESS_FCT(exposureTime),
    EASY_ACCESS_FCT(fNumber),
    EASY_ACCESS_FCT(shutterSpeedValue),
    EASY_ACCESS_FCT(apertureValue),
    EASY_ACCESS_FCT(brightnessValue),
    EASY_ACCESS_FCT(exposureBiasValue),
    EASY_ACCESS_FCT(maxApertureValue),
    EASY_ACCESS_FCT(subjectDistance),
    EASY_ACCESS_FCT(lightSource),
    EASY_ACCESS_FCT(flash),
    EASY_ACCESS_FCT(serialNumber),
    EASY_ACCESS_FCT(focalLength),
    EASY_ACCESS_FCT(subjectArea),
    EASY_ACCESS_FCT(flashEnergy),
    EASY_ACCESS_FCT(exposureIndex),
    EASY_ACCESS_FCT(sensingMethod),
    EASY_ACCESS_FCT(afPoint),
};
#undef EASY_ACCESS_FCT

const char* nameOf(EasyAccessFct fct) {
  auto it = std::find_if(std::begin(allFcts), std::end(allFcts), [fct](const auto& f) { return f.fct == fct; });
  return it == std::end(allFcts) ? "?" : it->name;
}

ExifData readExif(const std::string& file) {
  auto image = ImageFactory::open(TESTDATA_PATH "/" + file);
  image->readMetadata();
  return image->exifData();
}

void erase(ExifData& exif, const char* key) {
  auto pos = exif.findKey(ExifKey(key));
  ASSERT_NE(pos, exif.end()) << key;
  exif.erase(pos);
}

void expectFound(const ExifData& exif, EasyAccessFct fct, const char* key, const char* value) {
  SCOPED_TRACE(nameOf(fct));
  auto pos = fct(exif);
  ASSERT_NE(pos, exif.end());
  EXPECT_EQ(pos->key(), key);
  EXPECT_EQ(pos->print(&exif), value);
}

void expectNotFound(const ExifData& exif, EasyAccessFct fct) {
  auto pos = fct(exif);
  EXPECT_TRUE(pos == exif.end()) << nameOf(fct) << " found " << pos->key();
}

//! Checks all easy access functions, those not in @p found must find nothing
void expectFound(const ExifData& exif, std::initializer_list<Found> found) {
  for (const auto& f : allFcts) {
    auto it = std::find_if(found.begin(), found.end(), [&f](const Found& e) { return e.fct == f.fct; });
    if (it == found.end())
      expectNotFound(exif, f.fct);
    else
      expectFound(exif, it->fct, it->key, it->value);
  }
}
}  // namespace

TEST(EasyAccess, Empty) {
  auto exif = readExif("exiv2-empty.jpg");
  expectFound(exif, {});
}

TEST(EasyAccess, Canon1) {
  auto exif = readExif("exiv2-canon-eos-300d.jpg");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "right, top"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "100"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2003:09:26 08:19:51"},
                        {&flashBias, "Exif.CanonSi.FlashBias", "0 EV"},
                        {&exposureMode, "Exif.CanonCs.ExposureProgram", "Program (P)"},
                        {&sceneMode, "Exif.CanonCs.EasyMode", "Manual"},
                        {&macroMode, "Exif.CanonCs.Macro", "(0)"},
                        {&imageQuality, "Exif.CanonCs.Quality", "Fine"},
                        {&whiteBalance, "Exif.CanonSi.WhiteBalance", "Auto"},
                        {&lensName, "Exif.CanonCs.LensType", "n/a"},
                        {&saturation, "Exif.CanonCs.Saturation", "High"},
                        {&sharpness, "Exif.CanonCs.Sharpness", "High"},
                        {&contrast, "Exif.CanonCs.Contrast", "High"},
                        {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Standard"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "Canon"},
                        {&model, "Exif.Image.Model", "Canon EOS 300D DIGITAL"},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/80 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F5.6"},
                        {&shutterSpeedValue, "Exif.Photo.ShutterSpeedValue", "1/80 s"},
                        {&apertureValue, "Exif.Photo.ApertureValue", "F5.6"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.Photo.MaxApertureValue", "F3.5"},
                        {&subjectDistance, "Exif.CanonSi.SubjectDistance", "Infinite"},
                        {&flash, "Exif.Photo.Flash", "No flash"},
                        {&serialNumber, "Exif.Canon.SerialNumber", "570004681"},
                        {&focalLength, "Exif.Photo.FocalLength", "18.0 mm"},
                        {&sensingMethod, "Exif.Photo.SensingMethod", "One-chip color area"},
                        {&afPoint, "Exif.CanonPi.AFPointsUsed", "center"},
                    });
  erase(exif, "Exif.Photo.MeteringMode");
  expectFound(exif, &meteringMode, "Exif.CanonCs.MeteringMode", "Evaluative");
  erase(exif, "Exif.Photo.ApertureValue");
  expectFound(exif, &apertureValue, "Exif.CanonSi.ApertureValue", "F5.4");
  erase(exif, "Exif.Photo.FocalLength");
  expectFound(exif, &focalLength, "Exif.Canon.FocalLength", "18.0 mm");
}

TEST(EasyAccess, Canon2) {
  auto exif = readExif("20220610_MG_7238.exv");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "top, left"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "800"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2022:06:10 13:03:43"},
                        {&flashBias, "Exif.CanonSi.FlashBias", "0 EV"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Aperture priority"},
                        {&sceneMode, "Exif.CanonCs.EasyMode", "Manual"},
                        {&macroMode, "Exif.CanonCs.Macro", "Off"},
                        {&imageQuality, "Exif.CanonCs.Quality", "RAW"},
                        {&whiteBalance, "Exif.CanonSi.WhiteBalance", "Auto"},
                        {&lensName, "Exif.CanonCs.LensType", "Canon EF 35-80mm f/4-5.6"},
                        {&saturation, "Exif.CanonCs.Saturation", "Normal"},
                        {&sharpness, "Exif.CanonCs.Sharpness", "(32767)"},
                        {&contrast, "Exif.CanonCs.Contrast", "Normal"},
                        {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Standard"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "Canon"},
                        {&model, "Exif.Image.Model", "Canon EOS 6D"},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/100 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F5.6"},
                        {&shutterSpeedValue, "Exif.Photo.ShutterSpeedValue", "1/99 s"},
                        {&apertureValue, "Exif.Photo.ApertureValue", "F5.7"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.CanonCs.MaxAperture", "F4"},
                        {&subjectDistance, "Exif.CanonSi.SubjectDistance", "0 m"},
                        {&flash, "Exif.Photo.Flash", "No, compulsory"},
                        {&serialNumber, "Exif.Photo.BodySerialNumber", "113053000536"},
                        {&focalLength, "Exif.Photo.FocalLength", "35.0 mm"},
                        {&afPoint, "Exif.CanonSi.AFPointUsed", "0 focus points; none used"},
                    });
  erase(exif, "Exif.Photo.ISOSpeedRatings");
  expectFound(exif, &isoSpeed, "Exif.CanonSi.ISOSpeed", "800");
}

TEST(EasyAccess, Canon2_FujiTagsDRangeAutoRating1) {
  auto exif = readExif("FujiTagsDRangeAutoRating1.jpg");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "top, left"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "800"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Manual"},
                        {&sceneMode, "Exif.Fujifilm.PictureMode", "Manual"},
                        {&imageQuality, "Exif.Fujifilm.Quality", "NORMAL "},
                        {&whiteBalance, "Exif.Fujifilm.WhiteBalance", "Auto"},
                        {&lensName, "Exif.Photo.LensModel", "XF50mmF2 R WR"},
                        {&saturation, "Exif.Fujifilm.Color", "0 (normal)"},
                        {&sharpness, "Exif.Photo.Sharpness", "Normal"},
                        {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Standard"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "FUJIFILM"},
                        {&model, "Exif.Image.Model", "X-T3"},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/8000 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F2"},
                        {&shutterSpeedValue, "Exif.Photo.ShutterSpeedValue", "1/8192 s"},
                        {&apertureValue, "Exif.Photo.ApertureValue", "F2"},
                        {&brightnessValue, "Exif.Photo.BrightnessValue", "2.53"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.Photo.MaxApertureValue", "F2"},
                        {&lightSource, "Exif.Photo.LightSource", "Unknown"},
                        {&flash, "Exif.Photo.Flash", "No flash"},
                        {&focalLength, "Exif.Photo.FocalLength", "50.0 mm"},
                        {&sensingMethod, "Exif.Photo.SensingMethod", "One-chip color area"},
                    });
  erase(exif, "Exif.Photo.LensModel");
  expectFound(exif, &lensName, "Exif.Photo.LensSpecification", "50mm F2");
}

TEST(EasyAccess, Nikon1) {
  auto exif = readExif("NikonD1.exv");
  expectFound(exif, {
                        {&isoSpeed, "Exif.Nikon1.ISOSpeed", "400"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2005:12:16 08:25:45"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Auto"},
                        {&imageQuality, "Exif.Nikon1.Quality", "NORMAL "},
                        {&whiteBalance, "Exif.Nikon1.WhiteBalance", "AUTO        "},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "NIKON CORPORATION"},
                        {&model, "Exif.Image.Model", "NIKON D1 "},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/125 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F5.3"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.Photo.MaxApertureValue", "F4.4"},
                        {&focalLength, "Exif.Photo.FocalLength", "78.0 mm"},
                        {&sensingMethod, "Exif.Photo.SensingMethod", "One-chip color area"},
                        {&afPoint, "Exif.Nikon1.AFFocusPos", "Dynamic area; Center"},
                    });
}

TEST(EasyAccess, KonicaMinolta1) {
  auto exif = readExif("KonicaMinoltaDYNAX5D.exv");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "top, left"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "100"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2005:10:17 14:47:57"},
                        {&flashBias, "Exif.Minolta.FlashExposureComp", "0 EV"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Aperture priority"},
                        {&sceneMode, "Exif.Minolta.SceneMode", "Standard"},
                        {&imageQuality, "Exif.Minolta.Quality", "Extra Fine"},
                        {&whiteBalance, "Exif.MinoltaCs5D.WhiteBalance", "Auto"},
                        {&lensName, "Exif.Minolta.LensID",
                         "Minolta/Sony AF DT 18-70mm F3.5-5.6 (D) | Sony AF DT 18-200mm F3.5-6.3"},
                        {&saturation, "Exif.Photo.Saturation", "Normal"},
                        {&sharpness, "Exif.Photo.Sharpness", "Normal"},
                        {&contrast, "Exif.Photo.Contrast", "Normal"},
                        {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Standard"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "KONICA MINOLTA "},
                        {&model, "Exif.Image.Model", "DYNAX 5D"},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/125 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F8"},
                        {&brightnessValue, "Exif.Photo.BrightnessValue", "8"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.Photo.MaxApertureValue", "F4.5"},
                        {&lightSource, "Exif.Photo.LightSource", "Unknown"},
                        {&flash, "Exif.Photo.Flash", "No, compulsory"},
                        {&focalLength, "Exif.Photo.FocalLength", "24.0 mm"},
                        {&subjectArea, "Exif.Photo.SubjectArea", "1504 1000 256 304"},
                    });
  erase(exif, "Exif.Image.Orientation");
  expectFound(exif, &orientation, "Exif.MinoltaCs5D.Rotation", "Horizontal (normal)");
  erase(exif, "Exif.Photo.ISOSpeedRatings");
  expectFound(exif, &isoSpeed, "Exif.MinoltaCs5D.ISOSpeed", "100");
}

TEST(EasyAccess, Nikon2) {
  auto exif = readExif("exiv2-nikon-e950.jpg");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "top, left"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "80"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2001:04:06 11:51:40"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Auto"},
                        {&imageQuality, "Exif.Nikon2.Quality", "(12)"},
                        {&whiteBalance, "Exif.Nikon2.WhiteBalance", "Auto"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "NIKON"},
                        {&model, "Exif.Image.Model", "E950"},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/77 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F5.5"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.Photo.MaxApertureValue", "F2.5"},
                        {&lightSource, "Exif.Photo.LightSource", "Unknown"},
                        {&flash, "Exif.Photo.Flash", "No flash"},
                        {&focalLength, "Exif.Photo.FocalLength", "12.8 mm"},
                    });
  erase(exif, "Exif.Photo.ISOSpeedRatings");
  expectFound(exif, &isoSpeed, "Exif.Nikon2.ISOSpeed", "80");
}

TEST(EasyAccess, NikonLd4LensIDx) {
  auto exif = readExif("CH0_0174.exv");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "top, left"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "12800"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2020:12:11 19:05:49"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Aperture priority"},
                        {&imageQuality, "Exif.Nikon3.Quality", "RAW    "},
                        {&whiteBalance, "Exif.Nikon3.WhiteBalance", "AUTO1       "},
                        {&lensName, "Exif.Photo.LensModel", ""},
                        {&saturation, "Exif.Photo.Saturation", "Normal"},
                        {&sharpness, "Exif.Photo.Sharpness", "Normal"},
                        {&contrast, "Exif.Photo.Contrast", "Normal"},
                        {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Standard"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "NIKON CORPORATION"},
                        {&model, "Exif.Image.Model", "NIKON Z 6"},
                        {&exposureTime, "Exif.Photo.ExposureTime", "0.769231 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F4"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.NikonLd4.MaxAperture", "n/a"},
                        {&subjectDistance, "Exif.NikonLd4.FocusDistance", "2.66 m"},
                        {&lightSource, "Exif.Photo.LightSource", "Unknown"},
                        {&flash, "Exif.Photo.Flash", "No flash"},
                        {&serialNumber, "Exif.Photo.BodySerialNumber", "6078248"},
                        {&focalLength, "Exif.Photo.FocalLength", "102.0 mm"},
                        {&sensingMethod, "Exif.Photo.SensingMethod", "One-chip color area"},
                    });
  erase(exif, "Exif.Photo.LensModel");
  expectFound(exif, &lensName, "Exif.NikonLd4.LensIDNumber", "Nikon AF-S VR Zoom-Nikkor 70-200mm f/2.8G IF-ED");
  exif["Exif.NikonLd4.LensID"].setValue("13");
  expectFound(exif, &lensName, "Exif.NikonLd4.LensID", "Nikon Nikkor Z 24-70mm f/2.8 S");
  erase(exif, "Exif.Photo.ISOSpeedRatings");
  expectFound(exif, &isoSpeed, "Exif.NikonIi.ISO", "12800");
  erase(exif, "Exif.Photo.DateTimeOriginal");
  expectFound(exif, &dateTimeOriginal, "Exif.Image.DateTimeOriginal", "2020:12:11 19:05:49");
  erase(exif, "Exif.Photo.Saturation");
  expectFound(exif, &saturation, "Exif.NikonPc.Saturation", "Normal");
  erase(exif, "Exif.Photo.BodySerialNumber");
  expectFound(exif, &serialNumber, "Exif.Nikon3.SerialNumber", "6078248");
}

TEST(EasyAccess, NikonLd4FocalLength2) {
  auto exif = readExif("NikonZ6.exv");
  expectFound(exif, &focalLength, "Exif.Photo.FocalLength", "35.0 mm");
  erase(exif, "Exif.Photo.FocalLength");
  expectFound(exif, &focalLength, "Exif.NikonLd4.FocalLength2", "35 mm");
  expectFound(exif, &subjectDistance, "Exif.NikonLd4.FocusDistance2", "9.92 m");
}

TEST(EasyAccess, Nikon3Saturation2) {
  auto exif = readExif("exiv2-nikon-d70.jpg");
  expectFound(exif, &saturation, "Exif.Photo.Saturation", "Normal");
  erase(exif, "Exif.Photo.Saturation");
  expectFound(exif, &saturation, "Exif.Nikon3.Saturation2", "NORMAL         ");
}

TEST(EasyAccess, Pentax1) {
  auto exif = readExif("RAW_PENTAX_K100.exv");
  expectFound(exif,
              {
                  {&orientation, "Exif.Image.Orientation", "top, left"},
                  {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "400"},
                  {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2009:12:05 13:26:11"},
                  {&exposureMode, "Exif.Photo.ExposureProgram", "Aperture priority"},
                  {&sceneMode, "Exif.Pentax.PictureMode", "Aperture Priority"},
                  {&imageQuality, "Exif.Pentax.Quality", "RAW"},
                  {&whiteBalance, "Exif.Pentax.WhiteBalance", "Auto"},
                  {&lensName, "Exif.Pentax.LensType", "Tamron AF 18-250mm F3.5-6.3 Di II LD Aspherical [IF] Macro"},
                  {&saturation, "Exif.Photo.Saturation", "High"},
                  {&sharpness, "Exif.Photo.Sharpness", "Hard"},
                  {&contrast, "Exif.Photo.Contrast", "Hard"},
                  {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Standard"},
                  {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                  {&make, "Exif.Image.Make", "PENTAX Corporation "},
                  {&model, "Exif.Image.Model", "PENTAX K100D       "},
                  {&exposureTime, "Exif.Photo.ExposureTime", "1/750 s"},
                  {&fNumber, "Exif.Photo.FNumber", "F3.5"},
                  {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                  {&flash, "Exif.Photo.Flash", "No, compulsory"},
                  {&focalLength, "Exif.Photo.FocalLength", "18.0 mm"},
                  {&sensingMethod, "Exif.Photo.SensingMethod", "One-chip color area"},
                  {&afPoint, "Exif.Pentax.AFPoint", "Fixed Center"},
              });
  erase(exif, "Exif.Photo.ISOSpeedRatings");
  expectFound(exif, &isoSpeed, "Exif.Pentax.ISO", "400");
  erase(exif, "Exif.Pentax.WhiteBalance");
  expectFound(exif, &whiteBalance, "Exif.Photo.WhiteBalance", "Auto");
  erase(exif, "Exif.Photo.Saturation");
  expectFound(exif, &saturation, "Exif.Pentax.Saturation", "Med High");
  erase(exif, "Exif.Photo.Sharpness");
  expectFound(exif, &sharpness, "Exif.Pentax.Sharpness", "Med Hard");
  erase(exif, "Exif.Photo.Contrast");
  expectFound(exif, &contrast, "Exif.Pentax.Contrast", "Med High");
  erase(exif, "Exif.Photo.MeteringMode");
  expectFound(exif, &meteringMode, "Exif.Pentax.MeteringMode", "Multi Segment");
  erase(exif, "Exif.Image.Model");
  expectFound(exif, &model, "Exif.Pentax.ModelID", "K100D");
  erase(exif, "Exif.Photo.ExposureTime");
  expectFound(exif, &exposureTime, "Exif.Pentax.ExposureTime", "1.33 ms");
  erase(exif, "Exif.Photo.FNumber");
  expectFound(exif, &fNumber, "Exif.Pentax.FNumber", "F3.5");
  erase(exif, "Exif.Photo.Flash");
  expectFound(exif, &flash, "Exif.Pentax.Flash", "Off, Did not fire");
  erase(exif, "Exif.Photo.FocalLength");
  expectFound(exif, &focalLength, "Exif.Pentax.FocalLength", "18.0 mm");
}

TEST(EasyAccess, Pentax2) {
  auto exif = readExif("RAW_PENTAX_K30.exv");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "top, left"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "100"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2012:07:31 08:56:59"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Landscape mode"},
                        {&sceneMode, "Exif.PentaxDng.PictureMode", "Unknown (0x010601)"},
                        {&imageQuality, "Exif.PentaxDng.Quality", "RAW"},
                        {&whiteBalance, "Exif.PentaxDng.WhiteBalance", "Auto"},
                        {&lensName, "Exif.PentaxDng.LensType", "smc PENTAX-DA 18-135mm F3.5-5.6 ED AL [IF] DC WR"},
                        {&saturation, "Exif.Photo.Saturation", "Normal"},
                        {&sharpness, "Exif.Photo.Sharpness", "Hard"},
                        {&contrast, "Exif.Photo.Contrast", "Hard"},
                        {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Landscape"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "PENTAX             "},
                        {&model, "Exif.Image.Model", "PENTAX K-30        "},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/60 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F11"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&flash, "Exif.Photo.Flash", "No, compulsory"},
                        {&serialNumber, "Exif.PentaxDng.SerialNumber", "4364069"},
                        {&focalLength, "Exif.Photo.FocalLength", "31.0 mm"},
                        {&sensingMethod, "Exif.Photo.SensingMethod", "One-chip color area"},
                        {&afPoint, "Exif.PentaxDng.AFPoint", "Auto"},
                    });
  erase(exif, "Exif.Photo.ISOSpeedRatings");
  expectFound(exif, &isoSpeed, "Exif.PentaxDng.ISO", "100");
  erase(exif, "Exif.PentaxDng.WhiteBalance");
  expectFound(exif, &whiteBalance, "Exif.Photo.WhiteBalance", "Auto");
  erase(exif, "Exif.Photo.Saturation");
  expectFound(exif, &saturation, "Exif.PentaxDng.Saturation", "Normal");
  erase(exif, "Exif.Photo.Sharpness");
  expectFound(exif, &sharpness, "Exif.PentaxDng.Sharpness", "Med Hard");
  erase(exif, "Exif.Photo.Contrast");
  expectFound(exif, &contrast, "Exif.PentaxDng.Contrast", "Med High");
  erase(exif, "Exif.Photo.MeteringMode");
  expectFound(exif, &meteringMode, "Exif.PentaxDng.MeteringMode", "Multi Segment");
  erase(exif, "Exif.Image.Model");
  expectFound(exif, &model, "Exif.PentaxDng.ModelID", "K-30");
  erase(exif, "Exif.Photo.ExposureTime");
  expectFound(exif, &exposureTime, "Exif.PentaxDng.ExposureTime", "16.66 ms");
  erase(exif, "Exif.Photo.FNumber");
  expectFound(exif, &fNumber, "Exif.PentaxDng.FNumber", "F11");
  erase(exif, "Exif.Photo.Flash");
  expectFound(exif, &flash, "Exif.PentaxDng.Flash", "Off, Did not fire");
  erase(exif, "Exif.Photo.FocalLength");
  expectFound(exif, &focalLength, "Exif.PentaxDng.FocalLength", "31.0 mm");
}

TEST(EasyAccess, Sony1) {
  auto exif = readExif("exiv2-SonyDSC-HX60V.exv");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "top, left"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "250"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2014:01:01 00:52:22"},
                        {&flashBias, "Exif.Sony1.FlashExposureComp", "0 EV"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Auto"},
                        {&sceneMode, "Exif.Sony1.SceneMode", "Standard"},
                        {&macroMode, "Exif.Sony1.Macro", "Off"},
                        {&imageQuality, "Exif.Sony1.JPEGQuality", "Fine"},
                        {&whiteBalance, "Exif.Sony1.WhiteBalance", "Auto"},
                        {&lensName, "Exif.Sony1.LensSpec", "24-720mm F3.5-6.3"},
                        {&saturation, "Exif.Photo.Saturation", "Normal"},
                        {&sharpness, "Exif.Photo.Sharpness", "Normal"},
                        {&contrast, "Exif.Photo.Contrast", "Normal"},
                        {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Standard"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "SONY"},
                        {&model, "Exif.Image.Model", "DSC-HX60V"},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/250 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F6.3"},
                        {&brightnessValue, "Exif.Photo.BrightnessValue", "7.81016"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.Photo.MaxApertureValue", "F6.3"},
                        {&lightSource, "Exif.Photo.LightSource", "Unknown"},
                        {&flash, "Exif.Photo.Flash", "No, compulsory"},
                        {&focalLength, "Exif.Photo.FocalLength", "129.0 mm"},
                        {&afPoint, "Exif.Sony1.AFPointSelected", "n/a"},
                    });
  erase(exif, "Exif.Photo.ISOSpeedRatings");
  expectFound(exif, &isoSpeed, "Exif.Photo.RecommendedExposureIndex", "250");
  erase(exif, "Exif.Photo.ExposureProgram");
  expectFound(exif, &exposureMode, "Exif.Sony1.ExposureMode", "Program AE");
  erase(exif, "Exif.Sony1.ExposureMode");
  expectFound(exif, &exposureMode, "Exif.SonyMisc2b.ExposureProgram", "Program AE");
  erase(exif, "Exif.Sony1.JPEGQuality");
  expectFound(exif, &imageQuality, "Exif.Sony1.Quality", "Fine");
  erase(exif, "Exif.Sony1.Quality");
  expectFound(exif, &imageQuality, "Exif.SonyMisc3c.Quality2", "JPEG");
  erase(exif, "Exif.Sony1.WhiteBalance");
  expectFound(exif, &whiteBalance, "Exif.Sony1.WhiteBalance2", "Auto");
}

TEST(EasyAccess, Sony2) {
  auto exif = readExif("SonyDSLR-A100.exv");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "top, left"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "100"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2006:06:09 11:20:37"},
                        {&flashBias, "Exif.SonyMinolta.FlashExposureComp", "0 EV"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Auto"},
                        {&sceneMode, "Exif.SonyMinolta.SceneMode", "Auto"},
                        {&macroMode, "Exif.Sony1.Macro", "n/a"},
                        {&imageQuality, "Exif.Sony1.JPEGQuality", "n/a"},
                        {&whiteBalance, "Exif.Sony1MltCsA100.WhiteBalance", "Auto"},
                        {&lensName, "Exif.SonyMinolta.LensID",
                         "Minolta/Sony AF DT 18-70mm F3.5-5.6 (D) | Sony AF DT 18-200mm F3.5-6.3"},
                        {&saturation, "Exif.Photo.Saturation", "Normal"},
                        {&sharpness, "Exif.Photo.Sharpness", "Normal"},
                        {&contrast, "Exif.Photo.Contrast", "Normal"},
                        {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Standard"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "SONY           "},
                        {&model, "Exif.Image.Model", "DSLR-A100"},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/200 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F10"},
                        {&brightnessValue, "Exif.Photo.BrightnessValue", "9.37"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.Photo.MaxApertureValue", "F3.5"},
                        {&lightSource, "Exif.Photo.LightSource", "Unknown"},
                        {&flash, "Exif.Photo.Flash", "No, compulsory"},
                        {&focalLength, "Exif.Photo.FocalLength", "18.0 mm"},
                        {&subjectArea, "Exif.Photo.SubjectArea", "1448 968 246 294"},
                        {&afPoint, "Exif.Sony1MltCsA100.LocalAFAreaPoint", "Center"},
                    });
  erase(exif, "Exif.SonyMinolta.LensID");
  expectFound(exif, &lensName, "Exif.Sony1.LensID",
              "Minolta/Sony AF DT 18-70mm F3.5-5.6 (D) | Sony AF DT 18-200mm F3.5-6.3");
}

TEST(EasyAccess, Sony3) {
  auto exif = readExif("exiv2-pr906.exv");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "top, left"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "100"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2018:10:17 08:55:12"},
                        {&flashBias, "Exif.Sony2.FlashExposureComp", "0 EV"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Manual"},
                        {&sceneMode, "Exif.Sony2.SceneMode", "Standard"},
                        {&imageQuality, "Exif.Sony2.Quality", "RAW + JPEG/HEIF"},
                        {&whiteBalance, "Exif.Sony2.WhiteBalance", "Custom"},
                        {&lensName, "Exif.Photo.LensModel", "FE 85mm F1.4 GM"},
                        {&saturation, "Exif.Photo.Saturation", "Normal"},
                        {&sharpness, "Exif.Photo.Sharpness", "Normal"},
                        {&contrast, "Exif.Photo.Contrast", "Normal"},
                        {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Standard"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "SONY"},
                        {&model, "Exif.Image.Model", "ILCE-7M3"},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/100 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F8"},
                        {&brightnessValue, "Exif.Photo.BrightnessValue", "6.59922"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.Photo.MaxApertureValue", "F1.4"},
                        {&lightSource, "Exif.Photo.LightSource", "Other light source"},
                        {&flash, "Exif.Photo.Flash", "No, compulsory"},
                        {&focalLength, "Exif.Photo.FocalLength", "85.0 mm"},
                        {&afPoint, "Exif.Sony2.AFPointSelected", "Center Zone"},
                    });
  erase(exif, "Exif.Photo.ExposureProgram");
  expectFound(exif, &exposureMode, "Exif.Sony2.ExposureMode", "Manual");
}

TEST(EasyAccess, Sony4) {
  auto exif = readExif("exiv2-pr2323_coverage3.exv");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "top, left"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "200"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2018:06:14 11:35:45"},
                        {&flashBias, "Exif.Sony1.FlashExposureComp", "0 EV"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Aperture priority"},
                        {&sceneMode, "Exif.Sony1.SceneMode", "Standard"},
                        {&imageQuality, "Exif.Sony1.Quality", "RAW + JPEG/HEIF"},
                        {&whiteBalance, "Exif.Sony1.WhiteBalance", "Auto"},
                        {&lensName, "Exif.Sony1.LensSpec", "24-70mm F1.8-2.8"},
                        {&saturation, "Exif.Photo.Saturation", "Normal"},
                        {&sharpness, "Exif.Photo.Sharpness", "Normal"},
                        {&contrast, "Exif.Photo.Contrast", "Normal"},
                        {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Standard"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "SONY"},
                        {&model, "Exif.Image.Model", "DSC-RX100M5A"},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/125 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F3.2"},
                        {&brightnessValue, "Exif.Photo.BrightnessValue", "5.22031"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.Photo.MaxApertureValue", "F2.8"},
                        {&lightSource, "Exif.Photo.LightSource", "Unknown"},
                        {&flash, "Exif.Photo.Flash", "No, compulsory"},
                        {&focalLength, "Exif.Photo.FocalLength", "18.5 mm"},
                        {&afPoint, "Exif.Sony1.AFPointSelected", "n/a"},
                    });
  erase(exif, "Exif.Sony1.Quality");
  expectFound(exif, &imageQuality, "Exif.Sony1.Quality2", "RAW + extra fine");
  erase(exif, "Exif.Photo.Saturation");
  expectFound(exif, &saturation, "Exif.Sony1.Saturation", "0");
  erase(exif, "Exif.Photo.Sharpness");
  expectFound(exif, &sharpness, "Exif.Sony1.Sharpness", "0");
  erase(exif, "Exif.Photo.Contrast");
  expectFound(exif, &contrast, "Exif.Sony1.Contrast", "0");
  erase(exif, "Exif.Photo.MeteringMode");
  expectFound(exif, &meteringMode, "Exif.Sony1.MeteringMode2", "Multi-segment");
  erase(exif, "Exif.Image.Model");
  expectFound(exif, &model, "Exif.Sony1.SonyModelID", "DSC-RX100M5A");
  erase(exif, "Exif.Photo.BrightnessValue");
  expectFound(exif, &brightnessValue, "Exif.Sony1.Brightness", "0");
  erase(exif, "Exif.Photo.Flash");
  expectFound(exif, &flash, "Exif.Sony1.FlashAction", "Did not fire");
}

TEST(EasyAccess, Sony5) {
  auto exif = readExif("exiv2-pr906.exv");
  expectFound(exif, {
                        {&orientation, "Exif.Image.Orientation", "top, left"},
                        {&isoSpeed, "Exif.Photo.ISOSpeedRatings", "100"},
                        {&dateTimeOriginal, "Exif.Photo.DateTimeOriginal", "2018:10:17 08:55:12"},
                        {&flashBias, "Exif.Sony2.FlashExposureComp", "0 EV"},
                        {&exposureMode, "Exif.Photo.ExposureProgram", "Manual"},
                        {&sceneMode, "Exif.Sony2.SceneMode", "Standard"},
                        {&imageQuality, "Exif.Sony2.Quality", "RAW + JPEG/HEIF"},
                        {&whiteBalance, "Exif.Sony2.WhiteBalance", "Custom"},
                        {&lensName, "Exif.Photo.LensModel", "FE 85mm F1.4 GM"},
                        {&saturation, "Exif.Photo.Saturation", "Normal"},
                        {&sharpness, "Exif.Photo.Sharpness", "Normal"},
                        {&contrast, "Exif.Photo.Contrast", "Normal"},
                        {&sceneCaptureType, "Exif.Photo.SceneCaptureType", "Standard"},
                        {&meteringMode, "Exif.Photo.MeteringMode", "Multi-segment"},
                        {&make, "Exif.Image.Make", "SONY"},
                        {&model, "Exif.Image.Model", "ILCE-7M3"},
                        {&exposureTime, "Exif.Photo.ExposureTime", "1/100 s"},
                        {&fNumber, "Exif.Photo.FNumber", "F8"},
                        {&brightnessValue, "Exif.Photo.BrightnessValue", "6.59922"},
                        {&exposureBiasValue, "Exif.Photo.ExposureBiasValue", "0 EV"},
                        {&maxApertureValue, "Exif.Photo.MaxApertureValue", "F1.4"},
                        {&lightSource, "Exif.Photo.LightSource", "Other light source"},
                        {&flash, "Exif.Photo.Flash", "No, compulsory"},
                        {&focalLength, "Exif.Photo.FocalLength", "85.0 mm"},
                        {&afPoint, "Exif.Sony2.AFPointSelected", "Center Zone"},
                    });
  erase(exif, "Exif.Sony2.Quality");
  expectFound(exif, &imageQuality, "Exif.Sony2.Quality2", "RAW + extra fine");
  erase(exif, "Exif.Sony2.WhiteBalance");
  expectFound(exif, &whiteBalance, "Exif.Sony2.WhiteBalance2", "Manual");
  erase(exif, "Exif.Photo.LensModel");
  expectFound(exif, &lensName, "Exif.Sony2.LensID", "Manual lens");
  erase(exif, "Exif.Sony2.LensID");
  expectFound(exif, &lensName, "Exif.Sony2.LensSpec", "FE 85mm F1.4");
  erase(exif, "Exif.Photo.Saturation");
  expectFound(exif, &saturation, "Exif.Sony2.Saturation", "0");
  erase(exif, "Exif.Photo.Sharpness");
  expectFound(exif, &sharpness, "Exif.Sony2.Sharpness", "0");
  erase(exif, "Exif.Photo.Contrast");
  expectFound(exif, &contrast, "Exif.Sony2.Contrast", "0");
  erase(exif, "Exif.Photo.MeteringMode");
  expectFound(exif, &meteringMode, "Exif.Sony2.MeteringMode2", "Multi-segment");
  erase(exif, "Exif.Image.Model");
  expectFound(exif, &model, "Exif.Sony2.SonyModelID", "ILCE-7M3");
  erase(exif, "Exif.Photo.BrightnessValue");
  expectFound(exif, &brightnessValue, "Exif.Sony2.Brightness", "0");
  erase(exif, "Exif.Photo.Flash");
  expectFound(exif, &flash, "Exif.Sony2.FlashAction", "Did not fire");
}

TEST(EasyAccess, ImageISOSpeedRatings) {
  auto exif = readExif("exiv2-bug479.jpg");
  expectFound(exif, &isoSpeed, "Exif.Image.ISOSpeedRatings", "250");
}

TEST(EasyAccess, Sony2SerialNumber) {
  auto exif = readExif("exiv2-pr2323_coverage1.exv");
  expectFound(exif, &serialNumber, "Exif.Sony1.SerialNumber", "12345678");
}
