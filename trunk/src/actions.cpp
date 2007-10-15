// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2007 Andreas Huggel <ahuggel@gmx.net>
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
  File:      actions.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   08-Dec-03, ahu: created
             30-Apr-06, Roger Larsson: Print filename if processing multiple files
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#ifndef EXV_HAVE_TIMEGM
# include "timegm.h"
#endif
#include "actions.hpp"
#include "exiv2.hpp"
#include "image.hpp"
#include "jpgimage.hpp"
#include "utils.hpp"
#include "types.hpp"
#include "exif.hpp"
#include "canonmn.hpp"
#include "iptc.hpp"
#include "xmp.hpp"
#include "futils.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdio>
#include <ctime>
#include <cmath>
#include <cassert>
#include <sys/types.h>                  // for stat()
#include <sys/stat.h>                   // for stat()
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                    // for stat()
#endif
#ifdef _MSC_VER
# include <sys/utime.h>
#else
# include <utime.h>
#endif

// *****************************************************************************
// local declarations
namespace {

    //! Helper class to set the timestamp of a file to that of another file
    class Timestamp {
    public:
        //! C'tor
        Timestamp() : actime_(0), modtime_(0) {}
        //! Read the timestamp of a file
        int read(const std::string& path);
        //! Read the timestamp from a broken-down time in buffer \em tm.
        int read(struct tm* tm);
        //! Set the timestamp of a file
        int touch(const std::string& path);

    private:
        time_t actime_;
        time_t modtime_;
    };

    // Convert a string "YYYY:MM:DD HH:MI:SS" to a struct tm type,
    // returns 0 if successful
    int str2Tm(const std::string& timeStr, struct tm* tm);

    // Convert a string "YYYY:MM:DD HH:MI:SS" to a UTC time, -1 on error
    time_t str2Time(const std::string& timeStr);

    // Convert a UTC time to a string "YYYY:MM:DD HH:MI:SS", "" on error
    std::string time2Str(time_t time);

    /*!
      @brief Copy metadata from source to target according to Params::copyXyz

      @param source Source file path
      @param target Target file path. An *.exv file is created if target doesn't
                    exist.
      @param preserve Indicates if existing metadata in the target file should
                    be kept.
      @return 0 if successful, else an error code
    */
    int metacopy(const std::string& source,
                 const std::string& target,
                 bool preserve);

    /*!
      @brief Rename a file according to a timestamp value.

      @param path The original file path. Contains the new path on exit.
      @param tm   Pointer to a buffer with the broken-down time to rename
                  the file to.
      @return 0 if successful, -1 if the file was skipped, 1 on error.
    */
    int renameFile(std::string& path, const struct tm* tm);

    /*!
      @brief Make a file path from the current file path, destination
             directory (if any) and the filename extension passed in.
      @param path Path of the existing file
      @param ext New filename extension (incl. the dot '.' if required)
      @return 0 if successful, 1 if the new file exists and the user
             chose not to overwrite it.
     */
    std::string newFilePath(const std::string& path, const std::string& ext);

    /*!
      @brief Check if file \em path exists and whether it should be
             overwritten. Ask user if necessary. Return 1 if the file 
             exists and shouldn't be overwritten, else 0.
     */
    int dontOverwrite(const std::string& path);
}

// *****************************************************************************
// class member definitions
namespace Action {

    Task::AutoPtr Task::clone() const
    {
        return AutoPtr(clone_());
    }

    TaskFactory* TaskFactory::instance_ = 0;

    TaskFactory& TaskFactory::instance()
    {
        if (0 == instance_) {
            instance_ = new TaskFactory;
        }
        return *instance_;
    } // TaskFactory::instance

    void TaskFactory::cleanup()
    {
        if (instance_ != 0) {
            Registry::iterator e = registry_.end();
            for (Registry::iterator i = registry_.begin(); i != e; ++i) {
                delete i->second;
            }
            delete instance_;
            instance_ = 0;
        }
    } //TaskFactory::cleanup

    void TaskFactory::registerTask(TaskType type, Task::AutoPtr task)
    {
        Registry::iterator i = registry_.find(type);
        if (i != registry_.end()) {
            delete i->second;
        }
        registry_[type] = task.release();
    } // TaskFactory::registerTask

    TaskFactory::TaskFactory()
    {
        // Register a prototype of each known task
        registerTask(adjust,  Task::AutoPtr(new Adjust));
        registerTask(print,   Task::AutoPtr(new Print));
        registerTask(rename,  Task::AutoPtr(new Rename));
        registerTask(erase,   Task::AutoPtr(new Erase));
        registerTask(extract, Task::AutoPtr(new Extract));
        registerTask(insert,  Task::AutoPtr(new Insert));
        registerTask(modify,  Task::AutoPtr(new Modify));
        registerTask(fixiso,  Task::AutoPtr(new FixIso));
    } // TaskFactory c'tor

    Task::AutoPtr TaskFactory::create(TaskType type)
    {
        Registry::const_iterator i = registry_.find(type);
        if (i != registry_.end() && i->second != 0) {
            Task* t = i->second;
            return t->clone();
        }
        return Task::AutoPtr(0);
    } // TaskFactory::create

    int Print::run(const std::string& path)
    try {
        path_ = path;
        int rc = 0;
        switch (Params::instance().printMode_) {
        case Params::pmSummary:     rc = printSummary(); break;
        case Params::pmList:        rc = printList(); break;
        case Params::pmIptc:        rc = printIptc(); break;
        case Params::pmXmp:         rc = printXmp(); break;
        case Params::pmComment:     rc = printComment(); break;
        }
        return rc;
    }
    catch(const Exiv2::AnyError& e) {
        std::cerr << "Exiv2 exception in print action for file "
                  << path << ":\n" << e << "\n";
        return 1;
    } // Print::run

    int Print::printSummary()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_ << ": "
                      << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path_ << ": "
                      << _("No Exif data found in the file\n");
            return -3;
        }
        align_ = 16;

        // Filename
        printLabel(_("File name"));
        std::cout << path_ << std::endl;

        // Filesize
        struct stat buf;
        if (0 == stat(path_.c_str(), &buf)) {
            printLabel(_("File size"));
            std::cout << buf.st_size << " " << _("Bytes") << std::endl;
        }

        // Camera make
        printTag(exifData, "Exif.Image.Make", _("Camera make"));

        // Camera model
        printTag(exifData, "Exif.Image.Model", _("Camera model"));

        // Image Timestamp
        printTag(exifData, "Exif.Photo.DateTimeOriginal", _("Image timestamp"));

        // Image number
        // Todo: Image number for cameras other than Canon
        printTag(exifData, "Exif.Canon.ImageNumber", _("Image number"));

        // Exposure time
        // From ExposureTime, failing that, try ShutterSpeedValue
        bool done = false;
        printLabel(_("Exposure time"));
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.ExposureTime");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.ShutterSpeedValue");
        }
        std::cout << std::endl;

        // Aperture
        // Get if from FNumber and, failing that, try ApertureValue
        done = false;
        printLabel(_("Aperture"));
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.FNumber");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.ApertureValue");
        }
        std::cout << std::endl;

        // Exposure bias
        printTag(exifData, "Exif.Photo.ExposureBiasValue", _("Exposure bias"));

        // Flash
        printTag(exifData, "Exif.Photo.Flash", _("Flash"));

        // Todo: Flash bias, flash energy
        // Todo: Implement this for other cameras
        done = false;
        printLabel(_("Flash bias"));
        if (!done) {
            done = 0 != printTag(exifData, "Exif.CanonSi.FlashBias");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Panasonic.FlashBias");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Olympus.FlashBias");
        }
        std::cout << std::endl;

        // Actual focal length and 35 mm equivalent
        // Todo: Calculate 35 mm equivalent a la jhead
        Exiv2::ExifData::const_iterator md;
        printLabel(_("Focal length"));
        if (1 == printTag(exifData, "Exif.Photo.FocalLength")) {
            md = exifData.findKey(
                Exiv2::ExifKey("Exif.Photo.FocalLengthIn35mmFilm"));
            if (md != exifData.end()) {
                std::cout << " ("<< _("35 mm equivalent") << ": " << *md << ")";
            }
        }
        std::cout << std::endl;

        // Subject distance
        printLabel(_("Subject distance"));
        done = false;
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.SubjectDistance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.CanonSi.SubjectDistance");
        }
        std::cout << std::endl;

        // ISO speed
        // from ISOSpeedRatings or the Makernote
        printLabel(_("ISO speed"));
        done = false;
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.ISOSpeedRatings");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.CanonSi.ISOSpeed");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Nikon1.ISOSpeed");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Nikon2.ISOSpeed");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Nikon3.ISOSpeed");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCsNew.ISOSpeed");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCsOld.ISOSpeed");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCs5D.ISOSpeed");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCs7D.ISOSpeed");
        }
        std::cout << std::endl;

        // Exposure mode
        // From ExposureProgram or Canon Makernote
        printLabel(_("Exposure mode"));
        done = false;
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Photo.ExposureProgram");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.CanonCs.ExposureProgram");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCs7D.ExposureMode");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCs5D.ExposureMode");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCsNew.ExposureMode");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCsOld.ExposureMode");
        }
        std::cout << std::endl;

        // Metering mode
        printTag(exifData, "Exif.Photo.MeteringMode", _("Metering mode"));

        // Macro mode
        // Todo: Implement this for other cameras
        printLabel(_("Macro mode"));
        done = false;
        if (!done) {
            done = 0 != printTag(exifData, "Exif.CanonCs.Macro");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Fujifilm.Macro");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Olympus.Macro");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Panasonic.Macro");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCsNew.MacroMode");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCsOld.MacroMode");
        }
        std::cout << std::endl;

        // Image quality setting (compression)
        // Todo: Implement this for other cameras
        printLabel(_("Image quality"));
        done = false;
        if (!done) {
            done = 0 != printTag(exifData, "Exif.CanonCs.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Fujifilm.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Sigma.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Nikon1.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Nikon2.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Nikon3.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Olympus.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Panasonic.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Minolta.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCsNew.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCsOld.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCs5D.Quality");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCs7D.Quality");
        }
        std::cout << std::endl;

        // Exif Resolution
        printLabel(_("Exif Resolution"));
        long xdim = 0;
        long ydim = 0;
        md = exifData.findKey(Exiv2::ExifKey("Exif.Image.ImageWidth"));
        if (md == exifData.end()) {
            md = exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
        }
        if (md != exifData.end()) {
            xdim = md->toLong();
        }
        md = exifData.findKey(Exiv2::ExifKey("Exif.Image.ImageLength"));
        if (md == exifData.end()) {
            md = exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
        }
        if (md != exifData.end()) {
            ydim = md->toLong();
        }
        if (xdim != 0 && ydim != 0) {
            std::cout << xdim << " x " << ydim;
        }
        std::cout << std::endl;

        // White balance
        // Todo: Implement this for other cameras
        printLabel(_("White balance"));
        done = false;
        if (!done) {
            done = 0 != printTag(exifData, "Exif.CanonSi.WhiteBalance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Fujifilm.WhiteBalance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Sigma.WhiteBalance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Nikon1.WhiteBalance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Nikon2.WhiteBalance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Nikon3.WhiteBalance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Olympus.WhiteBalance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Panasonic.WhiteBalance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCs5D.WhiteBalance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCs7D.WhiteBalance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCsNew.WhiteBalance");
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.MinoltaCsOld.WhiteBalance");
        }
        std::cout << std::endl;

        // Thumbnail
        printLabel(_("Thumbnail"));
        std::string thumbExt = exifData.thumbnailExtension();
        if (thumbExt.empty()) {
            std::cout << _("None");
        }
        else {
            Exiv2::DataBuf buf = exifData.copyThumbnail();
            std::cout << exifData.thumbnailFormat() << ", "
                      << buf.size_ << " " << _("Bytes");
        }
        std::cout << std::endl;

        // Copyright
        printTag(exifData, "Exif.Image.Copyright", _("Copyright"));

        // Exif Comment
        printTag(exifData, "Exif.Photo.UserComment", _("Exif comment"));
        std::cout << std::endl;

        return 0;
    } // Print::printSummary

    void Print::printLabel(const std::string& label) const
    {
        std::cout << std::setfill(' ') << std::left;
        if (Params::instance().files_.size() > 1) {
            std::cout << std::setw(20) << path_ << " ";
        }
        std::cout << std::setw(align_)
                  << label << ": ";
    }

    int Print::printTag(const Exiv2::ExifData& exifData,
                        const std::string& key,
                        const std::string& label) const
    {
        int rc = 0;
        if (!label.empty()) {
            printLabel(label);
        }
        Exiv2::ExifKey ek(key);
        Exiv2::ExifData::const_iterator md = exifData.findKey(ek);
        if (md != exifData.end()) {
            std::cout << *md;
            rc = 1;
        }
        if (!label.empty()) std::cout << std::endl;
        return rc;
    } // Print::printTag

    int Print::printList()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path_
                      << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        Exiv2::ExifData::const_iterator md;
        bool const manyFiles = Params::instance().files_.size() > 1;
        for (md = exifData.begin(); md != exifData.end(); ++md) {
            if (   Params::instance().unknown_
                && md->tagName().substr(0, 2) == "0x") {
                continue;
            }
            if (manyFiles) {
                std::cout << std::setfill(' ') << std::left << std::setw(20)
                          << path_ << "  ";
            }
            bool first = true;
            if (Params::instance().printItems_ & Params::prTag) {
                if (!first) std::cout << " ";
                first = false;
                std::cout << "0x" << std::setw(4) << std::setfill('0')
                          << std::right << std::hex
                          << md->tag();
            }
            if (Params::instance().printItems_ & Params::prGroup) {
                if (!first) std::cout << " ";
                first = false;
                std::cout << std::setw(12) << std::setfill(' ') << std::left
                          << md->groupName();
            }
            if (Params::instance().printItems_ & Params::prKey) {
                if (!first) std::cout << " ";
                first = false;
                std::cout << std::setfill(' ') << std::left << std::setw(44)
                          << md->key();
            }
            if (Params::instance().printItems_ & Params::prName) {
                if (!first) std::cout << " ";
                first = false;
                std::cout << std::setw(27) << std::setfill(' ') << std::left
                          << md->tagName();
            }
            if (Params::instance().printItems_ & Params::prLabel) {
                if (!first) std::cout << " ";
                first = false;
                std::cout << std::setw(30) << std::setfill(' ') << std::left
                          << md->tagLabel();
            }
            if (Params::instance().printItems_ & Params::prType) {
                if (!first) std::cout << " ";
                first = false;
                std::cout << std::setw(9) << std::setfill(' ') << std::left
                          << md->typeName();
            }
            if (Params::instance().printItems_ & Params::prCount) {
                if (!first) std::cout << " ";
                first = false;
                std::cout << std::dec << std::setw(3)
                          << std::setfill(' ') << std::right
                          << md->count();
            }
            if (Params::instance().printItems_ & Params::prSize) {
                if (!first) std::cout << " ";
                first = false;
                std::cout << std::dec << std::setw(3)
                          << std::setfill(' ') << std::right
                          << md->size();
            }
            if (Params::instance().printItems_ & Params::prValue) {
                if (!first) std::cout << "  ";
                first = false;
                if (   Params::instance().binary_
                    && md->typeId() == Exiv2::undefined
                    && md->size() > 100) {
                    std::cout << _("(Binary value suppressed)") << std::endl;
                    continue;
                }
                std::cout << std::dec << md->value();
            }
            if (Params::instance().printItems_ & Params::prTrans) {
                if (!first) std::cout << "  ";
                first = false;
                if (   Params::instance().binary_
                    && md->typeId() == Exiv2::undefined
                    && md->size() > 100) {
                    std::cout << _("(Binary value suppressed)") << std::endl;
                    continue;
                }
                std::cout << std::dec << *md;
            }
            if (Params::instance().printItems_ & Params::prHex) {
                if (!first) std::cout << std::endl;
                first = false;
                if (   Params::instance().binary_
                    && md->typeId() == Exiv2::undefined
                    && md->size() > 100) {
                    std::cout << _("(Binary value suppressed)") << std::endl;
                    continue;
                }
                Exiv2::DataBuf buf(md->size());
                md->copy(buf.pData_, exifData.byteOrder());
                Exiv2::hexdump(std::cout, buf.pData_, buf.size_);
            }
            std::cout << std::endl;
        }

        return 0;
    } // Print::printList

    int Print::printIptc()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::IptcData& iptcData = image->iptcData();
        if (iptcData.empty()) {
            std::cerr << path_
                      << ": " << _("No IPTC data found in the file\n");
            return -3;
        }
        Exiv2::IptcData::const_iterator end = iptcData.end();
        Exiv2::IptcData::const_iterator md;
        bool manyFiles = Params::instance().files_.size() > 1;
        for (md = iptcData.begin(); md != end; ++md) {
            std::cout << std::setfill(' ') << std::left;
            if (manyFiles) {
                std::cout << std::setw(20) << path_ << " ";
            }
            std::cout << std::setw(44)
                      << md->key() << " "
                      << std::setw(9) << std::setfill(' ') << std::left
                      << md->typeName() << " "
                      << std::dec << std::setw(3)
                      << std::setfill(' ') << std::right
                      << md->count() << "  "
                      << std::dec << md->value()
                      << std::endl;
        }

        return 0;
    } // Print::printIptc

    int Print::printXmp()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::XmpData& xmpData = image->xmpData();
        if (xmpData.empty()) {
            std::cerr << path_
                      << ": " << _("No XMP data found in the file\n");
            return -3;
        }
        Exiv2::XmpData::const_iterator end = xmpData.end();
        Exiv2::XmpData::const_iterator md;
        bool manyFiles = Params::instance().files_.size() > 1;
        for (md = xmpData.begin(); md != end; ++md) {
            std::cout << std::setfill(' ') << std::left;
            if (manyFiles) {
                std::cout << std::setw(20) << path_ << " ";
            }
            std::cout << std::setw(44)
                      << md->key() << " "
                      << std::setw(9) << std::setfill(' ') << std::left
                      << md->typeName() << " "
                      << std::dec << std::setw(3)
                      << std::setfill(' ') << std::right
                      << md->count() << "  "
                      << std::dec << md->value()
                      << std::endl;
        }

        return 0;
    } // Print::printXmp

    int Print::printComment()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        if (Params::instance().verbose_) {
            std::cout << _("JPEG comment") << ": ";
        }
        std::cout << image->comment() << std::endl;
        return 0;
    } // Print::printComment

    Print::AutoPtr Print::clone() const
    {
        return AutoPtr(clone_());
    }

    Print* Print::clone_() const
    {
        return new Print(*this);
    }

    int Rename::run(const std::string& path)
    {
    try {
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path
                      << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        Exiv2::ExifKey key("Exif.Photo.DateTimeOriginal");
        Exiv2::ExifData::iterator md = exifData.findKey(key);
        if (md == exifData.end()) {
            key = Exiv2::ExifKey("Exif.Image.DateTime");
            md = exifData.findKey(key);
        }
        if (md == exifData.end()) {
            std::cerr << _("Neither tag") << " `Exif.Photo.DateTimeOriginal' "
                      << _("nor") << " `Exif.Image.DateTime' "
                      << _("found in the file") << " " << path << "\n";
            return 1;
        }
        std::string v = md->toString();
        if (v.length() == 0 || v[0] == ' ') {
            std::cerr << _("Image file creation timestamp not set in the file")
                      << " " << path << "\n";
            return 1;
        }
        struct tm tm;
        if (str2Tm(v, &tm) != 0) {
            std::cerr << _("Failed to parse timestamp") << " `" << v
                      << "' " << _("in the file") << " " << path << "\n";
            return 1;
        }
        if (   Params::instance().timestamp_
            || Params::instance().timestampOnly_) {
            ts.read(&tm);
        }
        int rc = 0;
        std::string newPath = path;
        if (Params::instance().timestampOnly_) {
            if (Params::instance().verbose_) {
                std::cout << _("Updating timestamp to") << " " << v << std::endl;
            }
        }
        else {
            rc = renameFile(newPath, &tm);
            if (rc == -1) return 0; // skip
        }
        if (   0 == rc
            && (   Params::instance().preserve_
                || Params::instance().timestamp_
                || Params::instance().timestampOnly_)) {
            ts.touch(newPath);
        }
        return rc;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in rename action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    }} // Rename::run

    Rename::AutoPtr Rename::clone() const
    {
        return AutoPtr(clone_());
    }

    Rename* Rename::clone_() const
    {
        return new Rename(*this);
    }

    int Erase::run(const std::string& path)
    try {
        path_ = path;

        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        // Thumbnail must be before Exif
        int rc = 0;
        if (Params::instance().target_ & Params::ctThumb) {
            rc = eraseThumbnail(image.get());
        }
        if (0 == rc && Params::instance().target_ & Params::ctExif) {
            rc = eraseExifData(image.get());
        }
        if (0 == rc && Params::instance().target_ & Params::ctIptc) {
            rc = eraseIptcData(image.get());
        }
        if (0 == rc && Params::instance().target_ & Params::ctComment) {
            rc = eraseComment(image.get());
        }
        if (0 == rc && Params::instance().target_ & Params::ctXmp) {
            rc = eraseXmpData(image.get());
        }
        if (0 == rc) {
            image->writeMetadata();
        }
        if (Params::instance().preserve_) {
            ts.touch(path);
        }

        return rc;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in erase action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Erase::run

    int Erase::eraseThumbnail(Exiv2::Image* image) const
    {
        Exiv2::ExifData& exifData = image->exifData();
        std::string thumbExt = exifData.thumbnailExtension();
        if (thumbExt.empty()) {
            return 0;
        }
        long delta = exifData.eraseThumbnail();
        if (Params::instance().verbose_) {
            std::cout << _("Erasing") << " " << delta
                        << " " << _("Bytes of thumbnail data") << std::endl;
        }
        return 0;
    }

    int Erase::eraseExifData(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->exifData().count() > 0) {
            std::cout << _("Erasing Exif data from the file") << std::endl;
        }
        image->clearExifData();
        return 0;
    }

    int Erase::eraseIptcData(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->iptcData().count() > 0) {
            std::cout << _("Erasing IPTC data from the file") << std::endl;
        }
        image->clearIptcData();
        return 0;
    }

    int Erase::eraseComment(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->comment().size() > 0) {
            std::cout << _("Erasing JPEG comment from the file") << std::endl;
        }
        image->clearComment();
        return 0;
    }

    int Erase::eraseXmpData(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->xmpData().count() > 0) {
            std::cout << _("Erasing XMP data from the file") << std::endl;
        }
        image->clearXmpPacket();
        return 0;
    }

    Erase::AutoPtr Erase::clone() const
    {
        return AutoPtr(clone_());
    }

    Erase* Erase::clone_() const
    {
        return new Erase(*this);
    }

    int Extract::run(const std::string& path)
    try {
        path_ = path;
        int rc = 0;
        if (Params::instance().target_ & Params::ctThumb) {
            rc = writeThumbnail();
        }
        if (Params::instance().target_ & Params::ctXmpPacket) {
            rc = writeXmp();
        }
        if (Params::instance().target_ & ~Params::ctThumb & ~Params::ctXmpPacket) {
            std::string exvPath = newFilePath(path_, ".exv");
            if (dontOverwrite(exvPath)) return 0;
            rc = metacopy(path_, exvPath, false);
        }
        return rc;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in extract action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Extract::run

    int Extract::writeXmp() const
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        const std::string& xmpPacket = image->xmpPacket();
        if (xmpPacket.empty()) {
            return -3;
        }
        std::string xmpPath = newFilePath(path_, ".xmp");
        if (dontOverwrite(xmpPath)) return 0;
        if (Params::instance().verbose_) {
            std::cout << _("Writing XMP packet from") << " " << path_
                      << " " << _("to") << " " << xmpPath << std::endl;
        }
        std::ofstream file(xmpPath.c_str());
        if (!file) {
            std::cerr << Params::instance().progname() << ": " 
                      << _("Failed to open file ") << " " << xmpPath << ": "
                      << Exiv2::strError() << "\n";
            return 1;
        }
        file << xmpPacket;
        return 0;
    } // Extract::writeXmp

    int Extract::writeThumbnail() const
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path_
                      << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        int rc = 0;
        std::string thumbExt = exifData.thumbnailExtension();
        if (thumbExt.empty()) {
            std::cerr << path_ << ": " << _("Image does not contain an Exif thumbnail\n");
        }
        else {
            std::string thumb = newFilePath(path_, "-thumb");
            std::string thumbPath = thumb + thumbExt;
            if (dontOverwrite(thumbPath)) return 0;
            if (Params::instance().verbose_) {
                Exiv2::DataBuf buf = exifData.copyThumbnail();
                std::cout << _("Writing") << " "
                          << exifData.thumbnailFormat() << " " << _("thumbnail") << " ("
                          << buf.size_ << " " << _("Bytes") << ") " << _("to file") << " "
                          << thumbPath << std::endl;
            }
            rc = exifData.writeThumbnail(thumb);
            if (rc) {
                std::cerr << path_ << ": " << _("Exif data doesn't contain a thumbnail\n");
            }
        }
        return rc;
    } // Extract::writeThumbnail

    Extract::AutoPtr Extract::clone() const
    {
        return AutoPtr(clone_());
    }

    Extract* Extract::clone_() const
    {
        return new Extract(*this);
    }

    int Insert::run(const std::string& path)
    try {
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        int rc = 0;
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        if (Params::instance().target_ & Params::ctThumb) {
            rc = insertThumbnail(path);
        }
        if (   rc == 0
            && Params::instance().target_ & Params::ctExif
            || Params::instance().target_ & Params::ctIptc
            || Params::instance().target_ & Params::ctComment
            || Params::instance().target_ & Params::ctXmp) {
            std::string suffix = Params::instance().suffix_;
            if (suffix.empty()) suffix = ".exv";
            std::string exvPath = newFilePath(path, suffix);
            rc = metacopy(exvPath, path, true);
        }
        if (0 == rc && Params::instance().target_ & Params::ctXmpPacket) {
            rc = insertXmpPacket(path);
        }
        if (Params::instance().preserve_) {
            ts.touch(path);
        }
        return rc;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in insert action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Insert::run

    int Insert::insertXmpPacket(const std::string& path) const
    {
        std::string xmpPath = newFilePath(path, ".xmp");
        if (!Exiv2::fileExists(xmpPath, true)) {
            std::cerr << xmpPath
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::DataBuf buf = Exiv2::readFile(xmpPath);
        std::string xmpPacket;
        xmpPacket.assign(reinterpret_cast<char*>(buf.pData_), buf.size_);
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();
        image->setXmpPacket(xmpPacket);
        image->writeMetadata();

        return 0;
    }

    int Insert::insertThumbnail(const std::string& path) const
    {
        std::string thumbPath = newFilePath(path, "-thumb.jpg");
        if (!Exiv2::fileExists(thumbPath, true)) {
            std::cerr << thumbPath
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        exifData.setJpegThumbnail(thumbPath);
        image->writeMetadata();

        return 0;
    } // Insert::insertThumbnail

    Insert::AutoPtr Insert::clone() const
    {
        return AutoPtr(clone_());
    }

    Insert* Insert::clone_() const
    {
        return new Insert(*this);
    }

    int Modify::run(const std::string& path)
    {
    try {
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();

        applyCommands(image.get());

        // Save both exif and iptc metadata
        image->writeMetadata();

        if (Params::instance().preserve_) {
            ts.touch(path);
        }
        return 0;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in modify action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    }
    } // Modify::run

    void Modify::applyCommands(Exiv2::Image* pImage)
    {
        if (!Params::instance().jpegComment_.empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Setting JPEG comment") << " '"
                          << Params::instance().jpegComment_
                          << "'"
                          << std::endl;
            }
            pImage->setComment(Params::instance().jpegComment_);
        }

        // loop through command table and apply each command
        ModifyCmds& modifyCmds = Params::instance().modifyCmds_;
        ModifyCmds::const_iterator i = modifyCmds.begin();
        ModifyCmds::const_iterator end = modifyCmds.end();
        for (; i != end; ++i) {
            switch (i->cmdId_) {
            case add:
                addMetadatum(pImage, *i);
                break;
            case set:
                setMetadatum(pImage, *i);
                break;
            case del:
                delMetadatum(pImage, *i);
                break;
            case reg:
                regNamespace(*i);
                break;
            case invalidCmdId:
                assert(invalidCmdId == i->cmdId_);
                break;
            }
        }
    } // Modify::applyCommands

    void Modify::addMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Add") << " " << modifyCmd.key_ << " \""
                      << modifyCmd.value_ << "\" ("
                      << Exiv2::TypeInfo::typeName(modifyCmd.typeId_)
                      << ")" << std::endl;
        }
        Exiv2::ExifData& exifData = pImage->exifData();
        Exiv2::IptcData& iptcData = pImage->iptcData();
        Exiv2::XmpData&  xmpData  = pImage->xmpData();
        Exiv2::Value::AutoPtr value = Exiv2::Value::create(modifyCmd.typeId_);
        if (0 == value->read(modifyCmd.value_)) {
            if (modifyCmd.metadataId_ == exif) {
                exifData.add(Exiv2::ExifKey(modifyCmd.key_), value.get());
            }
            if (modifyCmd.metadataId_ == iptc) {
                iptcData.add(Exiv2::IptcKey(modifyCmd.key_), value.get());
            }
            if (modifyCmd.metadataId_ == xmp) {
                xmpData.add(Exiv2::XmpKey(modifyCmd.key_), value.get());
            }
        }
    }

    // This function looks rather complex because we try to avoid adding an
    // empty metadatum if reading the value fails
    void Modify::setMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Set") << " " << modifyCmd.key_ << " \""
                      << modifyCmd.value_ << "\" ("
                      << Exiv2::TypeInfo::typeName(modifyCmd.typeId_)
                      << ")" << std::endl;
        }
        Exiv2::ExifData& exifData = pImage->exifData();
        Exiv2::IptcData& iptcData = pImage->iptcData();
        Exiv2::XmpData&  xmpData  = pImage->xmpData();
        Exiv2::Metadatum* metadatum = 0;
        if (modifyCmd.metadataId_ == exif) {
            Exiv2::ExifData::iterator pos =
                exifData.findKey(Exiv2::ExifKey(modifyCmd.key_));
            if (pos != exifData.end()) {
                metadatum = &(*pos);
            }
        }
        if (modifyCmd.metadataId_ == iptc) {
            Exiv2::IptcData::iterator pos =
                iptcData.findKey(Exiv2::IptcKey(modifyCmd.key_));
            if (pos != iptcData.end()) {
                metadatum = &(*pos);
            }
        }
        if (modifyCmd.metadataId_ == xmp) {
            Exiv2::XmpData::iterator pos =
                xmpData.findKey(Exiv2::XmpKey(modifyCmd.key_));
            if (pos != xmpData.end()) {
                metadatum = &(*pos);
            }
        }
        // If a type was explicitly requested, use it; else
        // use the current type of the metadatum, if any;
        // or the default type
        Exiv2::Value::AutoPtr value;
        if (metadatum) {
            value = metadatum->getValue();
        }
        if (   value.get() == 0
            || (   modifyCmd.explicitType_
                && modifyCmd.typeId_ != value->typeId())) {
            value = Exiv2::Value::create(modifyCmd.typeId_);
        }
        if (0 == value->read(modifyCmd.value_)) {
            if (metadatum) {
                metadatum->setValue(value.get());
            }
            else {
                if (modifyCmd.metadataId_ == exif) {
                    exifData.add(Exiv2::ExifKey(modifyCmd.key_), value.get());
                }
                if (modifyCmd.metadataId_ == iptc) {
                    iptcData.add(Exiv2::IptcKey(modifyCmd.key_), value.get());
                }
                if (modifyCmd.metadataId_ == xmp) {
                    xmpData.add(Exiv2::XmpKey(modifyCmd.key_), value.get());
                }
            }
        }
        else {
            std::cerr << _("Warning") << ": " << modifyCmd.key_ << ": "
                      << _("Failed to read") << " "
                      << Exiv2::TypeInfo::typeName(value->typeId()) 
                      << " " << _("value")
                      << " \"" << modifyCmd.value_ << "\"\n";
        }
    }

    void Modify::delMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Del") << " " << modifyCmd.key_ << std::endl;
        }

        Exiv2::ExifData& exifData = pImage->exifData();
        Exiv2::IptcData& iptcData = pImage->iptcData();
        Exiv2::XmpData&  xmpData  = pImage->xmpData();
        if (modifyCmd.metadataId_ == exif) {
            Exiv2::ExifData::iterator pos;
            Exiv2::ExifKey exifKey = Exiv2::ExifKey(modifyCmd.key_);
            while((pos = exifData.findKey(exifKey)) != exifData.end()) {
                exifData.erase(pos);
            }
        }
        if (modifyCmd.metadataId_ == iptc) {
            Exiv2::IptcData::iterator pos;
            Exiv2::IptcKey iptcKey = Exiv2::IptcKey(modifyCmd.key_);
            while((pos = iptcData.findKey(iptcKey)) != iptcData.end()) {
                iptcData.erase(pos);
            }
        }
        if (modifyCmd.metadataId_ == xmp) {
            Exiv2::XmpData::iterator pos;
            Exiv2::XmpKey xmpKey = Exiv2::XmpKey(modifyCmd.key_);
            while((pos = xmpData.findKey(xmpKey)) != xmpData.end()) {
                xmpData.erase(pos);
            }
        }
    }

    void Modify::regNamespace(const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Reg ") << modifyCmd.key_ << "=\"" 
                      << modifyCmd.value_ << "\"" << std::endl;
        }
        Exiv2::XmpProperties::registerNs(modifyCmd.value_, modifyCmd.key_);
    }

    Modify::AutoPtr Modify::clone() const
    {
        return AutoPtr(clone_());
    }

    Modify* Modify::clone_() const
    {
        return new Modify(*this);
    }

    int Adjust::run(const std::string& path)
    try {
        adjustment_ = Params::instance().adjustment_;

        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path
                      << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        int rc = adjustDateTime(exifData, "Exif.Image.DateTime", path);
        rc += adjustDateTime(exifData, "Exif.Photo.DateTimeOriginal", path);
        rc += adjustDateTime(exifData, "Exif.Photo.DateTimeDigitized", path);
        if (rc) return 1;
        image->writeMetadata();
        if (Params::instance().preserve_) {
            ts.touch(path);
        }
        return rc;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in adjust action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Adjust::run

    Adjust::AutoPtr Adjust::clone() const
    {
        return AutoPtr(clone_());
    }

    Adjust* Adjust::clone_() const
    {
        return new Adjust(*this);
    }

    int Adjust::adjustDateTime(Exiv2::ExifData& exifData,
                               const std::string& key,
                               const std::string& path) const
    {
        Exiv2::ExifKey ek(key);
        Exiv2::ExifData::iterator md = exifData.findKey(ek);
        if (md == exifData.end()) {
            // Key not found. That's ok, we do nothing.
            return 0;
        }
        std::string timeStr = md->toString();
        if (timeStr == "" || timeStr[0] == ' ') {
            std::cerr << path << ": " << _("Timestamp of metadatum with key") << " `"
                      << ek << "' " << _("not set\n");
            return 1;
        }
        time_t time = str2Time(timeStr);
        if (time == (time_t)-1) {
            std::cerr << path << ": " << _("Failed to parse or convert timestamp") << " `"
                      << timeStr << "'\n";
            return 1;
        }
        if (Params::instance().verbose_) {
            std::cout << _("Adjusting") << " `" << ek << "' " << _("by")
                      << (adjustment_ < 0 ? " " : " +")
                      << adjustment_ << _(" s to ");
        }
        time += adjustment_;
        timeStr = time2Str(time);
        if (Params::instance().verbose_) {
            std::cout << timeStr << std::endl;
        }
        md->setValue(timeStr);
        return 0;
    } // Adjust::adjustDateTime

    int FixIso::run(const std::string& path)
    {
    try {
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path
                      << ": " <<_("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_) {
            ts.read(path);
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
        assert(image.get() != 0);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path
                      << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        Exiv2::ExifKey key("Exif.Nikon3.ISOSpeed");
        Exiv2::ExifData::iterator md = exifData.findKey(key);
        if (md == exifData.end()) {
            key = Exiv2::ExifKey("Exif.Nikon2.ISOSpeed");
            md = exifData.findKey(key);
        }
        if (md == exifData.end()) {
            key = Exiv2::ExifKey("Exif.Nikon1.ISOSpeed");
            md = exifData.findKey(key);
        }
        if (md != exifData.end()) {
            std::ostringstream os;
            os << *md;
            if (Params::instance().verbose_) {
                std::cout << _("Setting Exif ISO value to") << " " << os.str() << "\n";
            }
            exifData["Exif.Photo.ISOSpeedRatings"] = os.str();
        }
        image->writeMetadata();
        if (Params::instance().preserve_) {
            ts.touch(path);
        }
        return 0;
    }
    catch(const Exiv2::AnyError& e)
    {
        std::cerr << "Exiv2 exception in fixiso action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    }
    } // FixIso::run

    FixIso::AutoPtr FixIso::clone() const
    {
        return AutoPtr(clone_());
    }

    FixIso* FixIso::clone_() const
    {
        return new FixIso(*this);
    }

}                                       // namespace Action

// *****************************************************************************
// local definitions
namespace {

    int Timestamp::read(const std::string& path)
    {
        struct stat buf;
        int rc = stat(path.c_str(), &buf);
        if (0 == rc) {
            actime_  = buf.st_atime;
            modtime_ = buf.st_mtime;
        }
        return rc;
    }

    int Timestamp::read(struct tm* tm)
    {
        int rc = 1;
        time_t t = mktime(tm);
        if (t != (time_t)-1) {
            rc = 0;
            actime_  = t;
            modtime_ = t;
        }
        return rc;
    }

    int Timestamp::touch(const std::string& path)
    {
        if (0 == actime_) return 1;
        struct utimbuf buf;
        buf.actime = actime_;
        buf.modtime = modtime_;
        return utime(path.c_str(), &buf);
    }

    int str2Tm(const std::string& timeStr, struct tm* tm)
    {
        if (timeStr.length() == 0 || timeStr[0] == ' ') return 1;
        if (timeStr.length() < 19) return 2;
        if (   timeStr[4]  != ':' || timeStr[7]  != ':' || timeStr[10] != ' '
            || timeStr[13] != ':' || timeStr[16] != ':') return 3;
        if (0 == tm) return 4;
        std::memset(tm, 0x0, sizeof(struct tm));
        tm->tm_isdst = -1;

        long tmp;
        if (!Util::strtol(timeStr.substr(0,4).c_str(), tmp)) return 5;
        tm->tm_year = tmp - 1900;
        if (!Util::strtol(timeStr.substr(5,2).c_str(), tmp)) return 6;
        tm->tm_mon = tmp - 1;
        if (!Util::strtol(timeStr.substr(8,2).c_str(), tmp)) return 7;
        tm->tm_mday = tmp;
        if (!Util::strtol(timeStr.substr(11,2).c_str(), tmp)) return 8;
        tm->tm_hour = tmp;
        if (!Util::strtol(timeStr.substr(14,2).c_str(), tmp)) return 9;
        tm->tm_min = tmp;
        if (!Util::strtol(timeStr.substr(17,2).c_str(), tmp)) return 10;
        tm->tm_sec = tmp;

        return 0;
    } // str2Tm

    time_t str2Time(const std::string& timeStr)
    {
        struct tm tm;
        if (str2Tm(timeStr, &tm) != 0) return (time_t)-1;
        time_t t = timegm(&tm);
        return t;
    }

    std::string time2Str(time_t time)
    {
        struct tm* tm = gmtime(&time);
        if (0 == tm) return "";

        std::ostringstream os;
        os << std::setfill('0')
           << tm->tm_year + 1900 << ":"
           << std::setw(2) << tm->tm_mon + 1 << ":"
           << std::setw(2) << tm->tm_mday << " "
           << std::setw(2) << tm->tm_hour << ":"
           << std::setw(2) << tm->tm_min << ":"
           << std::setw(2) << tm->tm_sec;

        return os.str();
    } // time2Str

    int metacopy(const std::string& source,
                 const std::string& target,
                 bool preserve)
    {
        if (!Exiv2::fileExists(source, true)) {
            std::cerr << source
                      << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::AutoPtr sourceImage = Exiv2::ImageFactory::open(source);
        assert(sourceImage.get() != 0);
        sourceImage->readMetadata();

        // Apply any modification commands to the source image on-the-fly
        Action::Modify::applyCommands(sourceImage.get());

        Exiv2::Image::AutoPtr targetImage;
        if (Exiv2::fileExists(target)) {
            targetImage = Exiv2::ImageFactory::open(target);
            assert(targetImage.get() != 0);
            if (preserve) targetImage->readMetadata();
        }
        else {
            targetImage
                = Exiv2::ImageFactory::create(Exiv2::ImageType::exv, target);
            assert(targetImage.get() != 0);
        }
        if (   Params::instance().target_ & Params::ctExif
            && !sourceImage->exifData().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing Exif data from") << " " << source
                          << " " << _("to") << " " << target << std::endl;
            }
            targetImage->setExifData(sourceImage->exifData());
        }
        if (   Params::instance().target_ & Params::ctIptc
            && !sourceImage->iptcData().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing IPTC data from") << " " << source
                          << " " << _("to") << " " << target << std::endl;
            }
            targetImage->setIptcData(sourceImage->iptcData());
        }
        if (   Params::instance().target_ & Params::ctXmp
            && !sourceImage->xmpData().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing XMP data from") << " " << source
                          << " " << _("to") << " " << target << std::endl;
            }
            // Todo: Should use XMP packet if there are no XMP modification commands
            targetImage->setXmpData(sourceImage->xmpData());
        }
        if (   Params::instance().target_ & Params::ctComment
            && !sourceImage->comment().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing JPEG comment from") << " " << source
                          << " " << _("to") << " " << target << std::endl;
            }
            targetImage->setComment(sourceImage->comment());
        }
        try {
            targetImage->writeMetadata();
        }
        catch (const Exiv2::AnyError& e) {
            std::cerr << target <<
                ": " << _("Could not write metadata to file") << ": " << e << "\n";
        }

        return 0;
    } // metacopy

    int renameFile(std::string& newPath, const struct tm* tm)
    {
        std::string path = newPath;
        std::string format = Params::instance().format_;
        Util::replace(format, ":basename:",   Util::basename(path, true));
        Util::replace(format, ":dirname:",    Util::basename(Util::dirname(path)));
        Util::replace(format, ":parentname:", Util::basename(Util::dirname(Util::dirname(path))));

        const size_t max = 1024;
        char basename[max];
        std::memset(basename, 0x0, max);
        if (strftime(basename, max, format.c_str(), tm) == 0) {
            std::cerr << _("Filename format yields empty filename for the file") << " "
                      << path << "\n";
            return 1;
        }
        newPath =   Util::dirname(path) + EXV_SEPERATOR_STR
                  + basename + Util::suffix(path);
        if (   Util::dirname(newPath)  == Util::dirname(path)
            && Util::basename(newPath) == Util::basename(path)) {
            if (Params::instance().verbose_) {
                std::cout << _("This file already has the correct name") << std::endl;
            }
            return -1;
        }

        bool go = true;
        int seq = 1;
        std::string s;
        Params::FileExistsPolicy fileExistsPolicy
            = Params::instance().fileExistsPolicy_;
        while (go) {
            if (Exiv2::fileExists(newPath)) {
                switch (fileExistsPolicy) {
                case Params::overwritePolicy:
                    go = false;
                    break;
                case Params::renamePolicy:
                    newPath = Util::dirname(path)
                        + EXV_SEPERATOR_STR + basename
                        + "_" + Exiv2::toString(seq++)
                        + Util::suffix(path);
                    break;
                case Params::askPolicy:
                    std::cout << Params::instance().progname()
                              << ": " << _("File") << " `" << newPath
                              << "' " << _("exists. [O]verwrite, [r]ename or [s]kip?")
                              << " ";
                    std::cin >> s;
                    switch (s[0]) {
                    case 'o':
                    case 'O':
                        go = false;
                        break;
                    case 'r':
                    case 'R':
                        fileExistsPolicy = Params::renamePolicy;
                        newPath = Util::dirname(path)
                            + EXV_SEPERATOR_STR + basename
                            + "_" + Exiv2::toString(seq++)
                            + Util::suffix(path);
                        break;
                    default: // skip
                        return -1;
                        break;
                    }
                }
            }
            else {
                go = false;
            }
        }

        if (Params::instance().verbose_) {
            std::cout << _("Renaming file to") << " " << newPath;
            if (Params::instance().timestamp_) {
                std::cout << ", " << _("updating timestamp");
            }
            std::cout << std::endl;
        }

        // Workaround for MinGW rename which does not overwrite existing files
        remove(newPath.c_str());
        if (std::rename(path.c_str(), newPath.c_str()) == -1) {
            std::cerr << Params::instance().progname()
                      << ": " << _("Failed to rename") << " "
                      << path << " " << _("to") << " " << newPath << ": "
                      << Exiv2::strError() << "\n";
            return 1;
        }

        return 0;
    } // renameFile

    std::string newFilePath(const std::string& path, const std::string& ext) 
    {
        std::string directory = Params::instance().directory_;
        if (directory.empty()) directory = Util::dirname(path);
        std::string newPath =   directory + EXV_SEPERATOR_STR 
                              + Util::basename(path, true) + ext;
        return newPath;
    }

    int dontOverwrite(const std::string& path)
    {
        if (!Params::instance().force_ && Exiv2::fileExists(path)) {
            std::cout << Params::instance().progname()
                      << ": " << _("Overwrite") << " `" << path << "'? ";
            std::string s;
            std::cin >> s;
            if (s[0] != 'y' && s[0] != 'Y') return 1;
        }
        return 0;
    }

}
