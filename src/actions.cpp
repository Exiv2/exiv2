// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
/*
  File:      actions.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   08-Dec-03, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#ifdef HAVE_CONFIG_H
# include <config.h>
#else
# ifdef _MSC_VER
#  include <config_win32.h>
# endif
#endif

#include "actions.hpp"
#include "exiv2.hpp"
#include "utils.hpp"
#include "types.hpp"
#include "exif.hpp"
#include "canonmn.hpp"
#include "iptc.hpp"
#ifndef HAVE_TIMEGM
# include "timegm.h"
#endif

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
#ifdef HAVE_UNISTD_H
# include <unistd.h>                    // for stat()
#endif

// *****************************************************************************
// local declarations
namespace {

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
        case Params::pmInterpreted: rc = printInterpreted(); break;
        case Params::pmValues:      rc = printValues(); break;
        case Params::pmHexdump:     rc = printHexdump(); break;
        case Params::pmIptc:        rc = printIptc(); break;
        case Params::pmComment:     rc = printComment(); break;
        }
        return rc;
    }
    catch(const Exiv2::Error& e) {
        std::cerr << "Exif exception in print action for file " 
                  << path << ":\n" << e << "\n";
        return 1;
    } // Print::run

    int Print::printSummary()
    {
        Exiv2::ExifData exifData;
        int rc = exifData.read(path_);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path_) << "\n";
            return rc;
        }

        align_ = 16;

        // Filename
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Filename" << ": " << path_ << std::endl;

        // Filesize
        struct stat buf;
        if (0 == stat(path_.c_str(), &buf)) {
            std::cout << std::setw(align_) << std::setfill(' ') << std::left
                      << "Filesize" << ": " << buf.st_size << " Bytes"
                      << std::endl;
        }

        // Camera make
        printTag(exifData, "Exif.Image.Make", "Camera make");

        // Camera model
        printTag(exifData, "Exif.Image.Model", "Camera model");

        // Image Timestamp
        printTag(exifData, "Exif.Photo.DateTimeOriginal", "Image timestamp");

        // Image number
        // Todo: Image number for cameras other than Canon
        printTag(exifData, "Exif.Canon.ImageNumber", "Image number");

        // Exposure time
        // From ExposureTime, failing that, try ShutterSpeedValue
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Exposure time" << ": ";
        Exiv2::ExifData::const_iterator md;
        if (0 == printTag(exifData, "Exif.Photo.ExposureTime")) {
            md = exifData.findKey(
                Exiv2::ExifKey("Exif.Photo.ShutterSpeedValue"));
            if (md != exifData.end()) {
                double tmp = exp(log(2.0) * md->toFloat()) + 0.5;
                if (tmp > 1) {
                    std::cout << "1/" << static_cast<long>(tmp) << " s";
                }
                else {
                    std::cout << static_cast<long>(1/tmp) << " s";
                }
            }
        }
        std::cout << std::endl;

        // Aperture
        // Get if from FNumber and, failing that, try ApertureValue
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Aperture" << ": ";
        if (0 == printTag(exifData, "Exif.Photo.FNumber")) {
            md = exifData.findKey(
                Exiv2::ExifKey("Exif.Photo.ApertureValue"));
            if (md != exifData.end()) {
                std::cout << std::fixed << std::setprecision(1)
                          << "F" << exp(log(2.0) * md->toFloat() / 2);
            }
        }
        std::cout << std::endl;

        // Exposure bias
        printTag(exifData, "Exif.Photo.ExposureBiasValue", "Exposure bias");

        // Flash
        printTag(exifData, "Exif.Photo.Flash", "Flash");

        // Todo: Flash bias, flash energy
        // Todo: Implement this for other cameras
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Flash bias" << ": ";
        md = exifData.findKey(Exiv2::ExifKey("Exif.Canon.CameraSettings2"));
        if (md != exifData.end() && md->count() >= 15) {
            Exiv2::CanonMakerNote::print0x0004_15(std::cout, md->toLong(15));
        }
        std::cout << std::endl;

        // Actual focal length and 35 mm equivalent
        // Todo: Calculate 35 mm equivalent a la jhead
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Focal length" << ": ";
        if (1 == printTag(exifData, "Exif.Photo.FocalLength")) {
            md = exifData.findKey(
                Exiv2::ExifKey("Exif.Photo.FocalLengthIn35mmFilm"));
            if (md != exifData.end()) {
                std::cout << " (35 mm equivalent: " << *md << ")";
            }
        }
        std::cout << std::endl;

        // Subject distance
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Subject distance" << ": ";
	if (0 == printTag(exifData, "Exif.Photo.SubjectDistance")) {
            md = exifData.findKey(
                Exiv2::ExifKey("Exif.Canon.CameraSettings2"));
            if (md != exifData.end() && md->count() >= 19) {
                Exiv2::CanonMakerNote::print0x0004_19(std::cout, md->toLong(19));
            }
        }
        std::cout << std::endl;

        // ISO speed
        // from ISOSpeedRatings or the Makernote
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "ISO speed" << ": ";
        bool done = false;
        if (0 == printTag(exifData, "Exif.Photo.ISOSpeedRatings")) {
            md = exifData.findKey(
                Exiv2::ExifKey("Exif.Canon.CameraSettings1"));
            if (md != exifData.end() && md->count() >= 16) {
                Exiv2::CanonMakerNote::print0x0001_16(std::cout, md->toLong(16));
                done = true;
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
        }
        std::cout << std::endl;

        // Exposure mode 
        // From ExposureProgram or Canon Makernote
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Exposure mode" << ": ";
        if (0 == printTag(exifData, "Exif.Photo.ExposureProgram")) {
            md = exifData.findKey(
                Exiv2::ExifKey("Exif.Canon.CameraSettings1"));
            if (md != exifData.end() && md->count() >= 20) {
                Exiv2::CanonMakerNote::print0x0001_20(std::cout, md->toLong(20));
            }
        }
        std::cout << std::endl;

        // Metering mode
        printTag(exifData, "Exif.Photo.MeteringMode", "Metering mode");

        // Macro mode
        // Todo: Implement this for other cameras
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Macro mode" << ": ";
        done = false;
        md = exifData.findKey(
            Exiv2::ExifKey("Exif.Canon.CameraSettings1"));
        if (md != exifData.end() && md->count() >= 1) {
            Exiv2::CanonMakerNote::print0x0001_01(std::cout, md->toLong(1));
            done = true;
        }
        if (!done) {
            done = 0 != printTag(exifData, "Exif.Fujifilm.Macro");
        }            
        std::cout << std::endl;

        // Image quality setting (compression)
        // Todo: Implement this for other cameras
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Image quality" << ": ";
        done = false;
        md = exifData.findKey(Exiv2::ExifKey("Exif.Canon.CameraSettings1"));
        if (md != exifData.end() && md->count() >= 3) {
            Exiv2::CanonMakerNote::print0x0001_03(std::cout, md->toLong(3));
            done = true;
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
        std::cout << std::endl;

        // Exif Resolution
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Exif Resolution" << ": ";
        long xdim = 0;
        long ydim = 0;
        md = exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
        if (md != exifData.end()) xdim = md->toLong();
        md = exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
        if (md != exifData.end()) ydim = md->toLong();
        if (xdim != 0 && ydim != 0) {
            std::cout << xdim << " x " << ydim;
        }
        std::cout << std::endl;

        // White balance
        // Todo: Implement this for other cameras
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "White balance" << ": ";
        done = false;
        md = exifData.findKey(Exiv2::ExifKey("Exif.Canon.CameraSettings2"));
        if (md != exifData.end() && md->count() >= 7) {
            Exiv2::CanonMakerNote::print0x0004_07(std::cout, md->toLong(7));
            done = true;
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
        std::cout << std::endl;

        // Thumbnail
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Thumbnail" << ": ";
        std::string thumbExt = exifData.thumbnailExtension();
        if (thumbExt.empty()) {
            std::cout << "None";
        } 
        else {
            Exiv2::DataBuf buf = exifData.copyThumbnail();
            std::cout << exifData.thumbnailFormat() << ", " 
                      << buf.size_ << " Bytes";
        }
        std::cout << std::endl;

        // Copyright
        printTag(exifData, "Exif.Image.Copyright", "Copyright");

        // Exif Comment
        printTag(exifData, "Exif.Photo.UserComment", "Exif comment");
        std::cout << std::endl;

        return 0;
    } // Print::printSummary

    int Print::printTag(const Exiv2::ExifData& exifData,
                        const std::string& key,
                        const std::string& label) const
    {
        int rc = 0;
        if (!label.empty()) {
            // Print the label in any case for the moment (to see what's missing)
            std::cout << std::setw(align_) << std::setfill(' ') << std::left
                      << label << ": ";
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

    int Print::printInterpreted()
    {
        Exiv2::ExifData exifData;
        int rc = exifData.read(path_);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path_) << "\n";
            return rc;
        }

        Exiv2::ExifData::const_iterator md;
        for (md = exifData.begin(); md != exifData.end(); ++md) {
            std::cout << "0x" << std::setw(4) << std::setfill('0') << std::right
                      << std::hex << md->tag() << " " 
                      << std::setw(9) << std::setfill(' ') << std::left
                      << md->ifdName() << " "
                      << std::setw(27) << std::setfill(' ') << std::left
                      << md->tagName() << " "
                      << std::dec << *md << std::endl;
        }

        return 0;
    } // Print::printInterpreted

    int Print::printValues()
    {
        Exiv2::ExifData exifData;
        int rc = exifData.read(path_);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path_) << "\n";
            return rc;
        }

        Exiv2::ExifData::const_iterator end = exifData.end();
        Exiv2::ExifData::const_iterator md;
        for (md = exifData.begin(); md != end; ++md) {
            std::cout << "0x" << std::setw(4) << std::setfill('0') << std::right
                      << std::hex << md->tag() << " " 
                      << std::setw(9) << std::setfill(' ') << std::left
                      << md->ifdName() << " "
                      << std::setw(9) << std::setfill(' ') << std::left
                      << md->typeName() << " "
                      << std::dec << std::setw(3) 
                      << std::setfill(' ') << std::right
                      << md->count() << " "
                      << std::setw(27) << std::setfill(' ') << std::left
                      << md->tagName() << " "
                      << std::dec << md->value() 
                      << std::endl;
        }

        return 0;
    } // Print::printValues

    int Print::printIptc()
    {
        Exiv2::IptcData iptcData;
        int rc = iptcData.read(path_);
        if (rc) {
            std::cerr << Exiv2::IptcData::strError(rc, path_) << "\n";
            return rc;
        }

        Exiv2::IptcData::const_iterator end = iptcData.end();
        Exiv2::IptcData::const_iterator md;
        for (md = iptcData.begin(); md != end; ++md) {
            std::cout << "0x" << std::setw(4) << std::setfill('0') << std::right
                      << std::hex << md->tag() << " " 
                      << std::setw(12) << std::setfill(' ') << std::left
                      << md->recordName() << " "
                      << std::setw(9) << std::setfill(' ') << std::left
                      << md->typeName() << " "
                      << std::dec << std::setw(3) 
                      << std::setfill(' ') << std::right
                      << md->count() << " "
                      << std::setw(27) << std::setfill(' ') << std::left
                      << md->tagName() << " "
                      << std::dec << md->value() 
                      << std::endl;
        } 

        return 0;
    } // Print::printIptc

    int Print::printHexdump()
    {
        Exiv2::ExifData exifData;
        int rc = exifData.read(path_);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path_) << "\n";
            return rc;
        }

        Exiv2::ExifData::const_iterator md;
        for (md = exifData.begin(); md != exifData.end(); ++md) {
            std::cout << std::setw(4) << std::setfill(' ') << std::left
                      << md->ifdName() << " "
                      << "0x" << std::setw(4) << std::setfill('0') << std::right
                      << std::hex << md->tag() << " " 
                      << std::setw(9) << std::setfill(' ') << std::left
                      << md->typeName() << " "
                      << std::dec << std::setw(3) 
                      << std::setfill(' ') << std::right
                      << md->count() << " "
                      << std::dec << std::setw(3) 
                      << std::setfill(' ') << std::right
                      << md->size() << " "
                      << std::setw(27) << std::setfill(' ') << std::left
                      << md->tagName() << std::endl;
            Exiv2::DataBuf buf(md->size());
            md->copy(buf.pData_, exifData.byteOrder());
            Exiv2::hexdump(std::cout, buf.pData_, buf.size_);
        }

        return 0;
    } // Print::printHexdump

    int Print::printComment()
    {
        if (!Util::fileExists(path_, true)) {
            std::cerr << path_ 
                      << ": Failed to open the file\n";
            return -1;
        }
        Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::instance().open(path_);
        if (image.get() == 0) {
            std::cerr << path_
                      << ": The file contains data of an unknown image type\n";
            return -2;
        }
        int rc = image->readMetadata();
        if (rc) {
            std::cerr << path_
                      << ": Could not read metadata\n";
            return 1;
        }
        if (Params::instance().verbose_) {
            std::cout << "Jpeg comment: ";
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
    try {
        Exiv2::ExifData exifData;
        int rc = exifData.read(path);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path) << "\n";
            return rc;
        }
        Exiv2::ExifKey key("Exif.Photo.DateTimeOriginal");
        Exiv2::ExifData::iterator md = exifData.findKey(key);
        if (md == exifData.end()) {
            std::cerr << "Metadatum with key `" << key << "' "
                      << "not found in the file " << path << "\n";
            return 1;
        }
        std::string v = md->toString();
        if (v.length() == 0 || v[0] == ' ') {
            std::cerr << "Image file creation timestamp not set in the file " 
                      << path << "\n";
            return 1;
        }
        // Assemble the new filename from the timestamp
        struct tm tm;
        if (str2Tm(v, &tm) != 0) {
            std::cerr << "Failed to parse timestamp `" << v
                      << "' in the file " << path << "\n";
            return 1;
        }
        const size_t max = 1024;
        char basename[max];
        memset(basename, 0x0, max);
        if (strftime(basename, max, Params::instance().format_.c_str(), &tm) == 0) {
            std::cerr << "Filename format yields empty filename for the file "
                      << path << "\n";
            return 1;
        }
        std::string newPath 
            = Util::dirname(path) + SEPERATOR_STR + basename + Util::suffix(path);
        if (   Util::dirname(newPath)  == Util::dirname(path)
            && Util::basename(newPath) == Util::basename(path)) {
            if (Params::instance().verbose_) {
                std::cout << "This file already has the correct name" << std::endl;
            }
            return 0;
        }
        if (Params::instance().verbose_) {
            std::cout << "Renaming file to " << newPath << std::endl;
        }
        if (!Params::instance().force_ && Util::fileExists(newPath)) {
            std::cout << Params::instance().progname() 
                      << ": Overwrite `" << newPath << "'? ";
            std::string s;
            std::cin >> s;
            if (s[0] != 'y' && s[0] != 'Y') return 0;
        }
        // Workaround for MinGW rename which does not overwrite existing files
        remove(newPath.c_str());
        if (::rename(path.c_str(), newPath.c_str()) == -1) {
            std::cerr << Params::instance().progname()
                      << ": Failed to rename "
                      << path << " to " << newPath << ": "
                      << Util::strError() << "\n";
            return 1;
        }
        return 0;
    }
    catch(const Exiv2::Error& e)
    {
        std::cerr << "Exif exception in rename action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Rename::run

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

        Exiv2::Image::AutoPtr image 
            = Exiv2::ImageFactory::instance().open(path_);
        if (image.get() == 0) {
            std::cerr << path_
                      << ": The file contains data of an unknown image type\n";
            return -2;
        }
        int rc = image->readMetadata();
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path_) << "\n";
        }
        // Thumbnail must be before Exif
        if (0 == rc && Params::instance().target_ & Params::ctThumb) {
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
        if (0 == rc) {
            rc = image->writeMetadata();
            if (rc) {
                std::cerr << Exiv2::ExifData::strError(rc, path_) << "\n";
            }
        }

        return rc;
    }
    catch(const Exiv2::Error& e)
    {
        std::cerr << "Exif exception in erase action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Erase::run

    int Erase::eraseThumbnail(Exiv2::Image* image) const
    {
        if (image->sizeExifData() == 0) {
            return 0;
        }
        int rc = 0;
        Exiv2::ExifData exifData;
        rc = exifData.read(image->exifData(), image->sizeExifData());
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path_) << "\n";
        }
        if (0 == rc) {
            std::string thumbExt = exifData.thumbnailExtension();
            if (!thumbExt.empty()) {
                long delta = exifData.eraseThumbnail();
                if (Params::instance().verbose_) {
                    std::cout << "Erasing " << delta 
                              << " Bytes of thumbnail data" << std::endl;
                }
                Exiv2::DataBuf buf(exifData.copy());
                image->setExifData(buf.pData_, buf.size_);
            }
        }
        return rc;
    }

    int Erase::eraseExifData(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->sizeExifData() > 0) {
            std::cout << "Erasing Exif data from the file" << std::endl; 
        }
        image->clearExifData();
        return 0;
    }

    int Erase::eraseIptcData(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->sizeIptcData() > 0) {
            std::cout << "Erasing Iptc data from the file" << std::endl; 
        }
        image->clearIptcData();
        return 0;
    }

    int Erase::eraseComment(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->comment().size() > 0) {
            std::cout << "Erasing Jpeg comment from the file" << std::endl; 
        }
        image->clearComment();
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
        if (Params::instance().target_ & ~Params::ctThumb) {
            std::string exvPath =   Util::dirname(path_) + SEPERATOR_STR
                                  + Util::basename(path_, true) + ".exv";
            if (!Params::instance().force_ && Util::fileExists(exvPath)) {
                std::cout << Params::instance().progname() 
                          << ": Overwrite `" << exvPath << "'? ";
                std::string s;
                std::cin >> s;
                if (s[0] != 'y' && s[0] != 'Y') return 0;
            }
            rc = metacopy(path_, exvPath, false);
        }
        return rc;
    }
    catch(const Exiv2::Error& e)
    {
        std::cerr << "Exif exception in extract action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Extract::run

    int Extract::writeThumbnail() const
    {
        Exiv2::ExifData exifData;
        int rc = exifData.read(path_);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path_) << "\n";
            return rc;
        }
        std::string thumb =   Util::dirname(path_) + SEPERATOR_STR
                            + Util::basename(path_, true) + "-thumb";
        std::string thumbExt = exifData.thumbnailExtension();
        if (thumbExt.empty()) {
            std::cerr << path_ << ": Image does not contain an Exif thumbnail\n"; 
        }
        else {
            if (Params::instance().verbose_) {
                Exiv2::DataBuf buf = exifData.copyThumbnail();
                std::cout << "Writing "
                          << exifData.thumbnailFormat() << " thumbnail (" 
                          << buf.size_ << " Bytes) to file "
                          << thumb << thumbExt << std::endl;
            }
            if (!Params::instance().force_ && Util::fileExists(thumb + thumbExt)) {
                std::cout << Params::instance().progname() 
                          << ": Overwrite `" << thumb + thumbExt << "'? ";
                std::string s;
                std::cin >> s;
                if (s[0] != 'y' && s[0] != 'Y') return 0;
            }
            rc = exifData.writeThumbnail(thumb);
            if (rc) {
                std::cerr << Exiv2::ExifData::strError(rc, thumb) << "\n";
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
        if (!Util::fileExists(path, true)) {
            std::cerr << path
                      << ": Failed to open the file\n";
            return -1;
        }
        int rc = 0;
        if (Params::instance().target_ & Params::ctThumb) {
            rc = insertThumbnail(path);
        }
        if (   rc == 0
            && Params::instance().target_ & Params::ctExif
            || Params::instance().target_ & Params::ctIptc
            || Params::instance().target_ & Params::ctComment) {
            std::string exvPath =   Util::dirname(path) + SEPERATOR_STR
                                  + Util::basename(path, true) + ".exv";
            rc = metacopy(exvPath, path, true);
        }
        return rc;
    }
    catch(const Exiv2::Error& e)
    {
        std::cerr << "Exif exception in insert action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Insert::run

    int Insert::insertThumbnail(const std::string& path) const
    {
        std::string thumbPath =   Util::dirname(path) + SEPERATOR_STR
                                + Util::basename(path, true) + "-thumb.jpg";
        if (!Util::fileExists(thumbPath, true)) {
            std::cerr << thumbPath
                      << ": Failed to open the file\n";
            return -1;
        }
        Exiv2::ExifData exifData;
        int rc = exifData.read(path);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path) << "\n";
            return rc;
        }
        exifData.setJpegThumbnail(thumbPath);
        return exifData.write(path);

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
    try {
        if (!Util::fileExists(path, true)) {
            std::cerr << path
                      << ": Failed to open the file\n";
            return -1;
        }

        // Read both exif and iptc metadata (ignore return code)
        exifData_.read(path);
        iptcData_.read(path);

        // loop through command table and apply each command
        ModifyCmds& modifyCmds = Params::instance().modifyCmds_;
        ModifyCmds::const_iterator i = modifyCmds.begin();
        ModifyCmds::const_iterator end = modifyCmds.end();
        for (; i != end; ++i) {
            switch (i->cmdId_) {
            case add:
                addMetadatum(*i);
                break;
            case set:
                setMetadatum(*i);
                break;
            case del:
                delMetadatum(*i);
                break;
            default:
                // Todo: complain
                break;
            }
        }

        // Save both exif and iptc metadata
        int rc = exifData_.write(path);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path) << "\n";
        }
        rc = iptcData_.write(path);
        if (rc) {
            std::cerr << Exiv2::IptcData::strError(rc, path) << "\n";
        }
        return rc;
    }
    catch(const Exiv2::Error& e)
    {
        std::cerr << "Exif exception in modify action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Modify::run

    void Modify::addMetadatum(const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << "Add " << modifyCmd.key_ << " \""
                      << modifyCmd.value_ << "\" (" 
                      << Exiv2::TypeInfo::typeName(modifyCmd.typeId_) 
                      << ")" << std::endl; 
        }
        Exiv2::Value::AutoPtr value = Exiv2::Value::create(modifyCmd.typeId_);
        value->read(modifyCmd.value_);
        if (modifyCmd.metadataId_ == exif) {
            exifData_.add(Exiv2::ExifKey(modifyCmd.key_), value.get());
        }
        if (modifyCmd.metadataId_ == iptc) {
            iptcData_.add(Exiv2::IptcKey(modifyCmd.key_), value.get());
        }
    }

    void Modify::setMetadatum(const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << "Set " << modifyCmd.key_ << " \""
                      << modifyCmd.value_ << "\" (" 
                      << Exiv2::TypeInfo::typeName(modifyCmd.typeId_) 
                      << ")" << std::endl; 
        }
        Exiv2::Metadatum* metadatum = 0;
        if (modifyCmd.metadataId_ == exif) {
            metadatum = &exifData_[modifyCmd.key_];
        }
        if (modifyCmd.metadataId_ == iptc) {
            metadatum = &iptcData_[modifyCmd.key_];
        }
        assert(metadatum);
        Exiv2::Value::AutoPtr value = metadatum->getValue();
        // If a type was explicitly requested, use it; else
        // use the current type of the metadatum, if any;
        // or the default type
        if (modifyCmd.explicitType_ || value.get() == 0) {
            value = Exiv2::Value::create(modifyCmd.typeId_);
        }
        value->read(modifyCmd.value_);
        metadatum->setValue(value.get());
    }

    void Modify::delMetadatum(const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << "Del " << modifyCmd.key_ << std::endl; 
        }
        if (modifyCmd.metadataId_ == exif) {
            Exiv2::ExifData::iterator pos =
                exifData_.findKey(Exiv2::ExifKey(modifyCmd.key_));
            if (pos != exifData_.end()) exifData_.erase(pos);
        }
        if (modifyCmd.metadataId_ == iptc) {
            Exiv2::IptcData::iterator pos =
                iptcData_.findKey(Exiv2::IptcKey(modifyCmd.key_));
            if (pos != iptcData_.end()) iptcData_.erase(pos);
        }
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

        Exiv2::ExifData exifData;
        int rc = exifData.read(path);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path) << "\n";
            return rc;
        }
        rc  = adjustDateTime(exifData, "Exif.Image.DateTime", path);
        rc += adjustDateTime(exifData, "Exif.Photo.DateTimeOriginal", path);
        rc += adjustDateTime(exifData, "Exif.Photo.DateTimeDigitized", path);
        if (rc) return 1;
        rc = exifData.write(path);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path) << "\n";
        }
        return rc;
    }
    catch(const Exiv2::Error& e)
    {
        std::cerr << "Exif exception in adjust action for file " << path
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
            std::cerr << path << ": Timestamp of metadatum with key `" 
                      << ek << "' not set\n";
            return 1;
        }
        time_t time = str2Time(timeStr);
        if (time == (time_t)-1) {
            std::cerr << path << ": Failed to parse or convert timestamp `" 
                      << timeStr << "'\n";
            return 1;
        }
        if (Params::instance().verbose_) {
            std::cout << "Adjusting `" << ek << "' by" 
                      << (adjustment_ < 0 ? " " : " +")
                      << adjustment_ << " s to ";
        }
        time += adjustment_;
        timeStr = time2Str(time);
        if (Params::instance().verbose_) {
            std::cout << timeStr << std::endl;
        }
        md->setValue(timeStr);
        return 0;
    } // Adjust::adjustDateTime

}                                       // namespace Action

// *****************************************************************************
// local definitions
namespace {

    int str2Tm(const std::string& timeStr, struct tm* tm)
    {
        if (timeStr.length() == 0 || timeStr[0] == ' ') return 1;
        if (timeStr.length() < 19) return 2;
        if (   timeStr[4]  != ':' || timeStr[7]  != ':' || timeStr[10] != ' '
            || timeStr[13] != ':' || timeStr[16] != ':') return 3;
        if (0 == tm) return 4;
        memset(tm, 0x0, sizeof(struct tm));

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
        if (!Util::fileExists(source, true)) {
            std::cerr << source
                      << ": Failed to open the file\n";
            return -1;
        }
        Exiv2::Image::AutoPtr sourceImage 
            = Exiv2::ImageFactory::instance().open(source);
        if (sourceImage.get() == 0) {
            std::cerr << source
                      << ": The file contains data of an unknown image type\n";
            return -2;
        }
        int rc = sourceImage->readMetadata();
        if (rc) {
            std::cerr << source
                      << ": Could not read metadata\n";
            return 1;
        }
        Exiv2::Image::AutoPtr targetImage 
            = Exiv2::ImageFactory::instance().open(target);
        if (preserve && targetImage.get() != 0) {
            if (targetImage->readMetadata()) {
                std::cerr << target
                          << ": Could not read metadata\n";
                return 1;
            }
        }
        if (targetImage.get() == 0) {
            targetImage 
                = Exiv2::ImageFactory::instance().create(Exiv2::Image::exv, target);
        }
        if (targetImage.get() == 0) {
            std::cerr << target 
                      << ": Could not open nor create the file\n";
            return 2;
        }
        if (   Params::instance().target_ & Params::ctExif
            && sourceImage->sizeExifData() > 0) {
            if (Params::instance().verbose_) {
                std::cout << "Writing Exif data from " << source 
                          << " to " << target << std::endl;
            }
            targetImage->setExifData(sourceImage->exifData(), 
                                     sourceImage->sizeExifData());
        }
        if (   Params::instance().target_ & Params::ctIptc
            && sourceImage->sizeIptcData() > 0) {
            if (Params::instance().verbose_) {
                std::cout << "Writing Iptc data from " << source 
                          << " to " << target << std::endl;
            }
            targetImage->setIptcData(sourceImage->iptcData(), 
                                     sourceImage->sizeIptcData());
        }
        if (   Params::instance().target_ & Params::ctComment
            && !sourceImage->comment().empty()) {
            if (Params::instance().verbose_) {
                std::cout << "Writing Jpeg comment from " << source 
                          << " to " << target << std::endl;
            }
            targetImage->setComment(sourceImage->comment());
        }
        rc = targetImage->writeMetadata();
        if (rc) {
            std::cerr << target <<
                ": Could not write metadata to file, rc = " << rc << "\n";
        }
        return rc;
    } // metacopy
}
