// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2009 Andreas Huggel <ahuggel@gmx.net>
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
  File:      easyaccess.cpp
  Version:   $Rev$
  Author(s): Carsten Pfeiffer <pfeiffer@kde.org>
             Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   28-Feb-09, gis: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "easyaccess.hpp"

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
    ExifData::const_iterator findMetadatum(const ExifData& ed,
                                           const char* keys[],
                                           int count)
    {
        for (int i = 0; i < count; ++i) {
            ExifData::const_iterator pos = ed.findKey(ExifKey(keys[i]));
            if (pos != ed.end()) return pos;
        }
        return ed.end();
    } // findMetadatum

} // anonymous namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    ExifData::const_iterator orientation(const ExifData& ed)
    {
        static const char* keys[] = {
            "Exif.Image.Orientation",
            "Exif.Panasonic.Rotation",
            "Exif.MinoltaCs5D.Rotation",
            "Exif.MinoltaCs7D.Rotation"
        };
        return findMetadatum(ed, keys, EXV_COUNTOF(keys));
    }

    ExifData::const_iterator isoSpeed(const ExifData& ed)
    {
        static const char* keys[] = {
            "Exif.Photo.ISOSpeedRatings",
            "Exif.CanonSi.ISOSpeed",
            "Exif.CanonCs.ISOSpeed",
            "Exif.Nikon1.ISOSpeed",
            "Exif.Nikon2.ISOSpeed",
            "Exif.Nikon3.ISOSpeed",
            "Exif.MinoltaCsNew.ISOSpeed",
            "Exif.MinoltaCsOld.ISOSpeed",
            "Exif.MinoltaCs5D.ISOSpeed",
            "Exif.MinoltaCs7D.ISOSpeed",
            "Exif.Pentax.ISO",
            "Exif.Olympus.ISOSpeed"
        };
        return findMetadatum(ed, keys, EXV_COUNTOF(keys));
    }

    ExifData::const_iterator flashBias(const ExifData& ed)
    {
        static const char* keys[] = {
            "Exif.CanonSi.FlashBias",
            "Exif.Panasonic.FlashBias",
            "Exif.Olympus.FlashBias",
            "Exif.OlympusCs.FlashExposureComp"
        };
        return findMetadatum(ed, keys, EXV_COUNTOF(keys));
    }

    ExifData::const_iterator exposureMode(const ExifData& ed)
    {
        static const char* keys[] = {
            "Exif.Photo.ExposureProgram",
            "Exif.CanonCs.ExposureProgram",
            "Exif.MinoltaCs7D.ExposureMode",
            "Exif.MinoltaCs5D.ExposureMode",
            "Exif.MinoltaCsNew.ExposureMode",
            "Exif.MinoltaCsOld.ExposureMode",
            "Exif.Sigma.ExposureMode"
        };
        return findMetadatum(ed, keys, EXV_COUNTOF(keys));
    }

    ExifData::const_iterator sceneMode(const ExifData& ed)
    {
        static const char* keys[] = {
            "Exif.CanonCs.EasyMode",
            "Exif.Fujifilm.PictureMode",
            "Exif.MinoltaCsNew.SubjectProgram",
            "Exif.OlympusCs.SceneMode",
            "Exif.Panasonic.ShootingMode",
            "Exif.Panasonic.SceneMode",
            "Exif.Pentax.PictureMode"
        };
        return findMetadatum(ed, keys, EXV_COUNTOF(keys));
    }

    ExifData::const_iterator macroMode(const ExifData& ed)
    {
        static const char* keys[] = {
            "Exif.CanonCs.Macro",
            "Exif.Fujifilm.Macro",
            "Exif.Olympus.Macro",
            "Exif.OlympusCs.MacroMode",
            "Exif.Panasonic.Macro",
            "Exif.MinoltaCsNew.MacroMode",
            "Exif.MinoltaCsOld.MacroMode"
        };
        return findMetadatum(ed, keys, EXV_COUNTOF(keys));
    }

    ExifData::const_iterator imageQuality(const ExifData& ed)
    {
        static const char* keys[] = {
            "Exif.CanonCs.Quality",
            "Exif.Fujifilm.Quality",
            "Exif.Sigma.Quality",
            "Exif.Nikon1.Quality",
            "Exif.Nikon2.Quality",
            "Exif.Nikon3.Quality",
            "Exif.Olympus.Quality",
            "Exif.OlympusCs.Quality",
            "Exif.Panasonic.Quality",
            "Exif.Minolta.Quality",
            "Exif.MinoltaCsNew.Quality",
            "Exif.MinoltaCsOld.Quality",
            "Exif.MinoltaCs5D.Quality",
            "Exif.MinoltaCs7D.Quality"
        };
        return findMetadatum(ed, keys, EXV_COUNTOF(keys));
    }

    ExifData::const_iterator whiteBalance(const ExifData& ed)
    {
        static const char* keys[] = {
            "Exif.CanonSi.WhiteBalance",
            "Exif.Fujifilm.WhiteBalance",
            "Exif.Sigma.WhiteBalance",
            "Exif.Nikon1.WhiteBalance",
            "Exif.Nikon2.WhiteBalance",
            "Exif.Nikon3.WhiteBalance",
            "Exif.Olympus.WhiteBalance",
            "Exif.OlympusCs.WhiteBalance",
            "Exif.Panasonic.WhiteBalance",
            "Exif.MinoltaCs5D.WhiteBalance",
            "Exif.MinoltaCs7D.WhiteBalance",
            "Exif.MinoltaCsNew.WhiteBalance",
            "Exif.MinoltaCsOld.WhiteBalance",
            "Exif.Photo.WhiteBalance"
        };
        return findMetadatum(ed, keys, EXV_COUNTOF(keys));
    }

    ExifData::const_iterator lensName(const ExifData& ed)
    {
        static const char* keys[] = {
            "Exif.CanonCs.LensType",
            "Exif.Nikon3.LensData",
            "Exif.Minolta.LensID",
            "Exif.Pentax.LensType"
        };
        return findMetadatum(ed, keys, EXV_COUNTOF(keys));
    }

}                                       // namespace Exiv2
