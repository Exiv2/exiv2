// SPDX-License-Identifier: GPL-2.0-or-later
// included header files
#include "easyaccess.hpp"
#include "utils.hpp"

// *****************************************************************************
namespace {
using namespace Exiv2;

/*!
  @brief Search \em ed for a Metadatum specified by the \em keys.
         The \em keys are searched in the order of their appearance, the
         first available Metadatum is returned.

  @param ed The %Exif metadata container to search
  @param keys Array of keys to look for
  @param count Number of elements in the array
 */
ExifData::const_iterator findMetadatum(const ExifData& ed, const char* const keys[], size_t count) {
  for (size_t i = 0; i < count; ++i) {
    auto pos = ed.findKey(ExifKey(keys[i]));
    if (pos != ed.end())
      return pos;
  }
  return ed.end();
}  // findMetadatum

}  // anonymous namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {
ExifData::const_iterator orientation(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Image.Orientation",       "Exif.Panasonic.Rotation",    "Exif.PanasonicRaw.Orientation",
      "Exif.MinoltaCs5D.Rotation",    "Exif.MinoltaCs5D.Rotation2", "Exif.MinoltaCs7D.Rotation",
      "Exif.Sony1MltCsA100.Rotation", "Exif.Sony1Cs.Rotation",      "Exif.Sony2Cs.Rotation",
      "Exif.Sony1Cs2.Rotation",       "Exif.Sony2Cs2.Rotation",     "Exif.Sony1MltCsA100.Rotation",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator isoSpeed(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.ISOSpeedRatings",
      "Exif.Image.ISOSpeedRatings",
      "Exif.CanonSi.ISOSpeed",
      "Exif.CanonCs.ISOSpeed",
      "Exif.Nikon1.ISOSpeed",
      "Exif.Nikon2.ISOSpeed",
      "Exif.Nikon3.ISOSettings",
      "Exif.Nikon3.ISOSpeed",
      "Exif.NikonIi.ISO",
      "Exif.NikonIi.ISO2",
      "Exif.NikonSiD300a.ISO",
      "Exif.MinoltaCsNew.ISOSetting",
      "Exif.MinoltaCsOld.ISOSetting",
      "Exif.MinoltaCs5D.ISOSpeed",
      "Exif.MinoltaCs7D.ISOSpeed",
      "Exif.Sony1Cs.ISOSetting",
      "Exif.Sony2Cs.ISOSetting",
      "Exif.Sony1Cs2.ISOSetting",
      "Exif.Sony2Cs2.ISOSetting",
      "Exif.Sony1MltCsA100.ISOSetting",
      "Exif.Pentax.ISO",
      "Exif.PentaxDng.ISO",
      "Exif.Olympus.ISOSpeed",
      "Exif.Samsung2.ISO",
      "Exif.Casio.ISO",
      "Exif.Casio2.ISO",
      "Exif.Casio2.ISOSpeed",
  };

  struct SensKeyNameList {
    int count;
    const char* keys[3];
  };

  // covers Exif.Phot.SensitivityType values 1-7. Note that SOS, REI and
  // ISO do differ in their meaning. Values coming first in a list (and
  // existing as a tag) are picked up first and used as the "ISO" value.
  static constexpr SensKeyNameList sensitivityKey[] = {
      {1, {"Exif.Photo.StandardOutputSensitivity"}},
      {1, {"Exif.Photo.RecommendedExposureIndex"}},
      {1, {"Exif.Photo.ISOSpeed"}},
      {2, {"Exif.Photo.RecommendedExposureIndex", "Exif.Photo.StandardOutputSensitivity"}},
      {2, {"Exif.Photo.ISOSpeed", "Exif.Photo.StandardOutputSensitivity"}},
      {2, {"Exif.Photo.ISOSpeed", "Exif.Photo.RecommendedExposureIndex"}},
      {3, {"Exif.Photo.ISOSpeed", "Exif.Photo.RecommendedExposureIndex", "Exif.Photo.StandardOutputSensitivity"}},
  };

  static constexpr const char* sensitivityType[] = {
      "Exif.Photo.SensitivityType",
  };

  // Find the first ISO value which is not "0"
  const size_t cnt = std::size(keys);
  auto md = ed.end();
  int64_t iso_val = -1;
  for (size_t idx = 0; idx < cnt;) {
    md = findMetadatum(ed, keys + idx, cnt - idx);
    if (md == ed.end())
      break;
    std::ostringstream os;
    md->write(os, &ed);
    bool ok = false;
    if (Internal::contains(os.str(), "inf"))
      break;
    iso_val = parseInt64(os.str(), ok);
    if (ok && iso_val > 0)
      break;
    while (md->key() != keys[idx++] && idx < cnt) {
    }
    md = ed.end();
  }

  // there is either a possible ISO "overflow" or no legacy
  // ISO tag at all. Check for SensitivityType tag and the referenced
  // ISO value (see EXIF 2.3 Annex G)
  int64_t iso_tmp_val = -1;
  while (iso_tmp_val == -1 && (iso_val == 65535 || md == ed.end())) {
    auto md_st = findMetadatum(ed, sensitivityType, std::size(sensitivityType));
    // no SensitivityType? exit with existing data
    if (md_st == ed.end())
      break;
    // otherwise pick up actual value and grab value accordingly
    std::ostringstream os;
    md_st->write(os, &ed);
    bool ok = false;
    const int64_t st_val = parseInt64(os.str(), ok);
    // SensitivityType out of range or cannot be parsed properly
    if (!ok || st_val < 1 || st_val > 7)
      break;
    // pick up list of ISO tags, and check for at least one of
    // them available.
    const SensKeyNameList* sensKeys = &sensitivityKey[st_val - 1];
    md_st = ed.end();
    for (int idx = 0; idx < sensKeys->count; md_st = ed.end()) {
      md_st = findMetadatum(ed, sensKeys->keys, sensKeys->count);
      if (md_st == ed.end())
        break;
      std::ostringstream os_iso;
      md_st->write(os_iso, &ed);
      ok = false;
      iso_tmp_val = parseInt64(os_iso.str(), ok);
      // something wrong with the value
      if (ok || iso_tmp_val > 0) {
        md = md_st;
        break;
      }
      while (md_st->key() != sensKeys->keys[idx++] && idx < sensKeys->count) {
      }
    }
    break;
  }

  return md;
}

ExifData::const_iterator dateTimeOriginal(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.DateTimeOriginal",
      "Exif.Image.DateTimeOriginal",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator flashBias(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.CanonSi.FlashBias",           "Exif.Panasonic.FlashBias",       "Exif.Olympus.FlashBias",
      "Exif.OlympusCs.FlashExposureComp", "Exif.Minolta.FlashExposureComp", "Exif.SonyMinolta.FlashExposureComp",
      "Exif.Sony1.FlashExposureComp",     "Exif.Sony2.FlashExposureComp",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator exposureMode(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.ExposureProgram",     "Exif.Image.ExposureProgram",       "Exif.CanonCs.ExposureProgram",
      "Exif.MinoltaCs7D.ExposureMode",  "Exif.MinoltaCs5D.ExposureMode",    "Exif.MinoltaCsNew.ExposureMode",
      "Exif.MinoltaCsOld.ExposureMode", "Exif.OlympusCs.ExposureMode",      "Exif.Sony1.ExposureMode",
      "Exif.Sony2.ExposureMode",        "Exif.Sony1Cs.ExposureProgram",     "Exif.Sony1Cs2.ExposureProgram",
      "Exif.Sony2Cs.ExposureProgram",   "Exif.Sony1MltCsA100.ExposureMode", "Exif.SonyMisc2b.ExposureProgram",
      "Exif.Sigma.ExposureMode",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator sceneMode(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.CanonCs.EasyMode",
      "Exif.Fujifilm.PictureMode",
      "Exif.MinoltaCsNew.SubjectProgram",
      "Exif.MinoltaCsOld.SubjectProgram",
      "Exif.Minolta.SceneMode",
      "Exif.SonyMinolta.SceneMode",
      "Exif.Sony1.SceneMode",
      "Exif.Sony2.SceneMode",
      "Exif.OlympusCs.SceneMode",
      "Exif.Panasonic.ShootingMode",
      "Exif.Panasonic.SceneMode",
      "Exif.Pentax.PictureMode",
      "Exif.PentaxDng.PictureMode",
      "Exif.Photo.SceneCaptureType",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator macroMode(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.CanonCs.Macro",       "Exif.Fujifilm.Macro",  "Exif.Olympus.Macro",          "Exif.Olympus2.Macro",
      "Exif.OlympusCs.MacroMode", "Exif.Panasonic.Macro", "Exif.MinoltaCsNew.MacroMode", "Exif.MinoltaCsOld.MacroMode",
      "Exif.Sony1.Macro",         "Exif.Sony2.Macro",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator imageQuality(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.CanonCs.Quality",        "Exif.Fujifilm.Quality",    "Exif.Sigma.Quality",
      "Exif.Nikon1.Quality",         "Exif.Nikon2.Quality",      "Exif.Nikon3.Quality",
      "Exif.Olympus.Quality",        "Exif.Olympus2.Quality",    "Exif.OlympusCs.Quality",
      "Exif.Panasonic.Quality",      "Exif.Pentax.Quality",      "Exif.PentaxDng.Quality",
      "Exif.Sony1.JPEGQuality",      "Exif.Sony1.Quality",       "Exif.Sony1.Quality2",
      "Exif.Sony1Cs.Quality",        "Exif.Sony2.JPEGQuality",   "Exif.Sony2.Quality",
      "Exif.Sony2.Quality2",         "Exif.Sony2Cs.Quality",     "Exif.SonyMinolta.Quality",
      "Exif.SonyMisc3c.Quality2",    "Exif.Minolta.Quality",     "Exif.MinoltaCsNew.Quality",
      "Exif.MinoltaCsOld.Quality",   "Exif.MinoltaCs5D.Quality", "Exif.MinoltaCs7D.Quality",
      "Exif.Sony1MltCsA100.Quality", "Exif.Casio.Quality",       "Exif.Casio2.QualityMode",
      "Exif.Casio2.Quality",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator whiteBalance(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.CanonSi.WhiteBalance",      "Exif.Fujifilm.WhiteBalance",    "Exif.Sigma.WhiteBalance",
      "Exif.Nikon1.WhiteBalance",       "Exif.Nikon2.WhiteBalance",      "Exif.Nikon3.WhiteBalance",
      "Exif.Olympus.WhiteBalance",      "Exif.OlympusCs.WhiteBalance",   "Exif.Panasonic.WhiteBalance",
      "Exif.MinoltaCs5D.WhiteBalance",  "Exif.MinoltaCs7D.WhiteBalance", "Exif.MinoltaCsNew.WhiteBalance",
      "Exif.MinoltaCsOld.WhiteBalance", "Exif.Minolta.WhiteBalance",     "Exif.Pentax.WhiteBalance",
      "Exif.PentaxDng.WhiteBalance",    "Exif.Sony1.WhiteBalance",       "Exif.Sony2.WhiteBalance",
      "Exif.Sony1.WhiteBalance2",       "Exif.Sony2.WhiteBalance2",      "Exif.Sony1MltCsA100.WhiteBalance",
      "Exif.SonyMinolta.WhiteBalance",  "Exif.Casio.WhiteBalance",       "Exif.Casio2.WhiteBalance",
      "Exif.Casio2.WhiteBalance2",      "Exif.Photo.WhiteBalance",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator lensName(const ExifData& ed) {
  static constexpr const char* keys[] = {
      // Try Exif.CanonCs.LensType first.
      "Exif.CanonCs.LensType",      "Exif.Photo.LensModel",
      "Exif.Canon.LensModel",       "Exif.NikonLd1.LensIDNumber",
      "Exif.NikonLd2.LensIDNumber", "Exif.NikonLd3.LensIDNumber",
      "Exif.NikonLd4.LensID",       "Exif.NikonLd4.LensIDNumber",
      "Exif.Pentax.LensType",       "Exif.PentaxDng.LensType",
      "Exif.Minolta.LensID",        "Exif.SonyMinolta.LensID",
      "Exif.Sony1.LensID",          "Exif.Sony2.LensID",
      "Exif.Sony1.LensSpec",        "Exif.Sony2.LensSpec",
      "Exif.OlympusEq.LensType",    "Exif.Panasonic.LensType",
      "Exif.Samsung2.LensType",     "Exif.Photo.LensSpecification",
      "Exif.Nikon3.Lens",
  };

  for (const auto& key : keys) {
    auto pos = ed.findKey(ExifKey(key));
    if (pos != ed.end()) {
      // Exif.NikonLd4.LensID and Exif.NikonLd4.LensIDNumber are usually together included,
      // one of them has value 0 (which means undefined), so skip tag with value 0
      if (strncmp(key, "Exif.NikonLd4", 13) != 0 || pos->getValue()->toInt64(0) > 0)
        return pos;
    }
  }
  return ed.end();
}

ExifData::const_iterator saturation(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.Saturation",        "Exif.CanonCs.Saturation",     "Exif.MinoltaCsNew.Saturation",
      "Exif.MinoltaCsOld.Saturation", "Exif.MinoltaCs7D.Saturation", "Exif.MinoltaCs5D.Saturation",
      "Exif.Fujifilm.Color",          "Exif.Nikon3.Saturation",      "Exif.NikonPc.Saturation",
      "Exif.Panasonic.Saturation",    "Exif.Pentax.Saturation",      "Exif.PentaxDng.Saturation",
      "Exif.Sigma.Saturation",        "Exif.Sony1.Saturation",       "Exif.Sony2.Saturation",
      "Exif.Casio.Saturation",        "Exif.Casio2.Saturation",      "Exif.Casio2.Saturation2",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator sharpness(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.Sharpness",        "Exif.CanonCs.Sharpness",       "Exif.Fujifilm.Sharpness",
      "Exif.MinoltaCsNew.Sharpness", "Exif.MinoltaCsOld.Sharpness",  "Exif.MinoltaCs7D.Sharpness",
      "Exif.MinoltaCs5D.Sharpness",  "Exif.Olympus.SharpnessFactor", "Exif.Panasonic.Sharpness",
      "Exif.Pentax.Sharpness",       "Exif.PentaxDng.Sharpness",     "Exif.Sigma.Sharpness",
      "Exif.Sony1.Sharpness",        "Exif.Sony2.Sharpness",         "Exif.Casio.Sharpness",
      "Exif.Casio2.Sharpness",       "Exif.Casio2.Sharpness2",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator contrast(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.Contrast",        "Exif.CanonCs.Contrast",      "Exif.Fujifilm.Tone",
      "Exif.MinoltaCsNew.Contrast", "Exif.MinoltaCsOld.Contrast", "Exif.MinoltaCs7D.Contrast",
      "Exif.MinoltaCs5D.Contrast",  "Exif.Olympus.Contrast",      "Exif.Panasonic.Contrast",
      "Exif.Pentax.Contrast",       "Exif.PentaxDng.Contrast",    "Exif.Sigma.Contrast",
      "Exif.Sony1.Contrast",        "Exif.Sony2.Contrast",        "Exif.Casio.Contrast",
      "Exif.Casio2.Contrast",       "Exif.Casio2.Contrast2",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator sceneCaptureType(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.SceneCaptureType",
      "Exif.Olympus.SpecialMode",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator meteringMode(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.MeteringMode",       "Exif.Image.MeteringMode",        "Exif.CanonCs.MeteringMode",
      "Exif.MinoltaCs5D.MeteringMode", "Exif.MinoltaCsOld.MeteringMode", "Exif.OlympusCs.MeteringMode",
      "Exif.Pentax.MeteringMode",      "Exif.PentaxDng.MeteringMode",    "Exif.Sigma.MeteringMode",
      "Exif.Sony1.MeteringMode2",      "Exif.Sony1Cs.MeteringMode",      "Exif.Sony1Cs2.MeteringMode",
      "Exif.Sony2.MeteringMode2",      "Exif.Sony2Cs.MeteringMode",      "Exif.Sony1MltCsA100.MeteringMode",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator make(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Image.Make",
      "Exif.PanasonicRaw.Make",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator model(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Image.Model",
      "Exif.MinoltaCsOld.MinoltaModel",
      "Exif.MinoltaCsNew.MinoltaModel",
      "Exif.PanasonicRaw.Model",
      "Exif.Pentax.ModelID",
      "Exif.PentaxDng.ModelID",
      "Exif.Sony1.SonyModelID",
      "Exif.Sony2.SonyModelID",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator exposureTime(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.ExposureTime",     "Exif.Image.ExposureTime",    "Exif.Pentax.ExposureTime",
      "Exif.PentaxDng.ExposureTime", "Exif.Samsung2.ExposureTime",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator fNumber(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.FNumber",     "Exif.Image.FNumber",    "Exif.Pentax.FNumber",
      "Exif.PentaxDng.FNumber", "Exif.Samsung2.FNumber",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator shutterSpeedValue(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.ShutterSpeedValue",
      "Exif.Image.ShutterSpeedValue",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator apertureValue(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.ApertureValue",
      "Exif.Image.ApertureValue",
      "Exif.CanonSi.ApertureValue",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator brightnessValue(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.BrightnessValue",
      "Exif.Image.BrightnessValue",
      "Exif.Sony1.Brightness",
      "Exif.Sony2.Brightness",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator exposureBiasValue(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.ExposureBiasValue",     "Exif.Image.ExposureBiasValue",      "Exif.MinoltaCs5D.ExposureManualBias",
      "Exif.OlympusRd.ExposureBiasValue", "Exif.OlympusRd2.ExposureBiasValue",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator maxApertureValue(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.MaxApertureValue",
      "Exif.Image.MaxApertureValue",
      "Exif.CanonCs.MaxAperture",
      "Exif.NikonLd4.MaxAperture",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator subjectDistance(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.SubjectDistance",      "Exif.Image.SubjectDistance",      "Exif.CanonSi.SubjectDistance",
      "Exif.CanonFi.FocusDistanceUpper", "Exif.CanonFi.FocusDistanceLower", "Exif.MinoltaCsNew.FocusDistance",
      "Exif.Nikon1.FocusDistance",       "Exif.Nikon3.FocusDistance",       "Exif.NikonLd2.FocusDistance",
      "Exif.NikonLd3.FocusDistance",     "Exif.NikonLd4.FocusDistance",     "Exif.Olympus.FocusDistance",
      "Exif.OlympusFi.FocusDistance",    "Exif.Casio.ObjectDistance",       "Exif.Casio2.ObjectDistance",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator lightSource(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.LightSource",
      "Exif.Image.LightSource",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator flash(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.Flash",     "Exif.Image.Flash",       "Exif.Pentax.Flash",
      "Exif.PentaxDng.Flash", "Exif.Sony1.FlashAction", "Exif.Sony2.FlashAction",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator serialNumber(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Image.CameraSerialNumber", "Exif.Photo.BodySerialNumber", "Exif.Canon.SerialNumber",
      "Exif.Nikon3.SerialNumber",      "Exif.Nikon3.SerialNO",        "Exif.Fujifilm.SerialNumber",
      "Exif.Olympus.SerialNumber2",    "Exif.OlympusEq.SerialNumber", "Exif.Pentax.SerialNumber",
      "Exif.PentaxDng.SerialNumber",   "Exif.Sigma.SerialNumber",     "Exif.Sony1.SerialNumber",
      "Exif.Sony2.SerialNumber",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator focalLength(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.FocalLength",    "Exif.Image.FocalLength",     "Exif.Canon.FocalLength",
      "Exif.NikonLd2.FocalLength", "Exif.NikonLd3.FocalLength",  "Exif.MinoltaCsNew.FocalLength",
      "Exif.Pentax.FocalLength",   "Exif.PentaxDng.FocalLength", "Exif.Casio2.FocalLength",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator subjectArea(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.SubjectArea",
      "Exif.Image.SubjectLocation",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator flashEnergy(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.FlashEnergy",
      "Exif.Image.FlashEnergy",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator exposureIndex(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.ExposureIndex",
      "Exif.Image.ExposureIndex",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator sensingMethod(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.Photo.SensingMethod",
      "Exif.Image.SensingMethod",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

ExifData::const_iterator afPoint(const ExifData& ed) {
  static constexpr const char* keys[] = {
      "Exif.CanonPi.AFPointsUsed",
      "Exif.CanonPi.AFPointsUsed20D",
      "Exif.CanonSi.AFPointUsed",
      "Exif.CanonCs.AFPoint",
      "Exif.MinoltaCs7D.AFPoints",
      "Exif.Nikon1.AFFocusPos",
      "Exif.NikonAf.AFPoint",
      "Exif.NikonAf.AFPointsInFocus",
      "Exif.NikonAf2.AFPointsUsed",
      "Exif.NikonAf2.PrimaryAFPoint",
      "Exif.OlympusFi.AFPoint",
      "Exif.Pentax.AFPoint",
      "Exif.Pentax.AFPointInFocus",
      "Exif.PentaxDng.AFPoint",
      "Exif.PentaxDng.AFPointInFocus",
      "Exif.Sony1.AFPointSelected",
      "Exif.Sony2.AFPointSelected",
      "Exif.Sony1Cs.LocalAFAreaPoint",
      "Exif.Sony2Cs.LocalAFAreaPoint",
      "Exif.Sony1Cs2.LocalAFAreaPoint",
      "Exif.Sony2Cs2.LocalAFAreaPoint",
      "Exif.Sony1MltCsA100.LocalAFAreaPoint",
      "Exif.Casio.AFPoint",
      "Exif.Casio2.AFPointPosition",
  };
  return findMetadatum(ed, keys, std::size(keys));
}

}  // namespace Exiv2
