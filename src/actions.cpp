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
  Version:   $Name:  $ $Revision: 1.35 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   08-Dec-03, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.35 $ $RCSfile: actions.cpp,v $");

// *****************************************************************************
// included header files
#include <config.h>

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
#include <sys/types.h>                  // for stat()
#include <sys/stat.h>                   // for stat()
#if defined HAVE_UNISTD_H && !defined _MSC_VER
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
        case Params::summary:     rc = printSummary(); break;
        case Params::interpreted: rc = printInterpreted(); break;
        case Params::values:      rc = printValues(); break;
        case Params::hexdump:     rc = printHexdump(); break;
        case Params::iptc:        rc = printIptc(); break;
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
                  << "Filename" << ": " << path_ << "\n";

        // Filesize
        struct stat buf;
        if (0 == stat(path_.c_str(), &buf)) {
            std::cout << std::setw(align_) << std::setfill(' ') << std::left
                      << "Filesize" << ": " << buf.st_size << " Bytes\n";
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
        std::cout << "\n";

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
        std::cout << "\n";

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
        std::cout << "\n";

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
        std::cout << "\n";

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
        std::cout << "\n";

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
                done = printTag(exifData, "Exif.Nikon1.ISOSpeed");
            }
            if (!done) {
                done = printTag(exifData, "Exif.Nikon2.ISOSpeed");
            }
            if (!done) {
                done = printTag(exifData, "Exif.Nikon3.ISOSpeed");
            }
        }
        std::cout << "\n";

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
        std::cout << "\n";

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
            done = printTag(exifData, "Exif.Fujifilm.Macro");
        }            
        std::cout << "\n";

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
            done = printTag(exifData, "Exif.Fujifilm.Quality");
        }
        if (!done) {
            done = printTag(exifData, "Exif.Sigma.Quality");
        }
        if (!done) {
            done = printTag(exifData, "Exif.Nikon1.Quality");
        }
        if (!done) {
            done = printTag(exifData, "Exif.Nikon2.Quality");
        }
        if (!done) {
            done = printTag(exifData, "Exif.Nikon3.Quality");
        }
        std::cout << "\n";

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
        std::cout << "\n";

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
            done = printTag(exifData, "Exif.Fujifilm.WhiteBalance");
        }
        if (!done) {
            done = printTag(exifData, "Exif.Sigma.WhiteBalance");
        }
        if (!done) {
            done = printTag(exifData, "Exif.Nikon1.WhiteBalance");
        }
        if (!done) {
            done = printTag(exifData, "Exif.Nikon2.WhiteBalance");
        }
        if (!done) {
            done = printTag(exifData, "Exif.Nikon3.WhiteBalance");
        }
        std::cout << "\n";

        // Thumbnail
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Thumbnail" << ": ";
        std::string thumbExt = exifData.thumbnailExtension();
        if (thumbExt.empty()) {
            std::cout << "None";
        } 
        else {
            std::cout << exifData.thumbnailFormat() << ", " 
                      << exifData.thumbnailSize() << " Bytes";
        }
        std::cout << "\n";

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
        if (!label.empty()) std::cout << "\n";
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
                      << std::dec << *md << "\n";
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
                      << "\n";
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
                      << std::setw(9) << std::setfill(' ') << std::left
                      << md->recordName() << " "
                      << std::setw(9) << std::setfill(' ') << std::left
                      << md->typeName() << " "
                      << std::dec << std::setw(3) 
                      << std::setfill(' ') << std::right
                      << md->count() << " "
                      << std::setw(27) << std::setfill(' ') << std::left
                      << md->tagName() << " "
                      << std::dec << md->value() 
                      << "\n";
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
                      << md->tagName() << "\n";
            Exiv2::DataBuf buf(md->size());
            md->copy(buf.pData_, exifData.byteOrder());
            Exiv2::hexdump(std::cout, buf.pData_, buf.size_);
        }

        return 0;
    } // Print::printHexdump

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
                std::cout << "This file already has the correct name\n";
            }
            return 0;
        }
        if (Params::instance().verbose_) {
            std::cout << "Renaming file to " << newPath << "\n";
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
        Exiv2::ExifData exifData;
        int rc = exifData.read(path);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path) << "\n";
            return rc;
        }
        switch (Params::instance().delTarget_) {
        case Params::delExif:  rc = eraseExifData(exifData); break;
        case Params::delThumb: rc = eraseThumbnail(exifData); break;
        }
        return rc;
    }
    catch(const Exiv2::Error& e)
    {
        std::cerr << "Exif exception in erase action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Erase::run

    int Erase::eraseThumbnail(Exiv2::ExifData& exifData) const
    {
        int rc = 0;
        std::string thumbExt = exifData.thumbnailExtension();
        if (thumbExt.empty()) {
            std::cerr << path_ << ": Image does not contain an Exif thumbnail\n"; 
        }
        else {
            long delta = exifData.eraseThumbnail();
            if (Params::instance().verbose_) {
                std::cout << "Erasing " << delta << " Bytes of thumbnail data\n"; 
            }
            rc = exifData.write(path_);
            if (rc) {
                std::cerr << Exiv2::ExifData::strError(rc, path_) << "\n";
            }
        }
        return rc;
    }

    int Erase::eraseExifData(Exiv2::ExifData& exifData) const
    {
        if (Params::instance().verbose_) {
            std::cout << "Erasing Exif data from the file\n"; 
        }
        int rc = exifData.erase(path_);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path_) << "\n";
        }
        return rc;
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
        Exiv2::ExifData exifData;
        int rc = exifData.read(path);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path) << "\n";
            return rc;
        }
        switch (Params::instance().extractTarget_) {
        case Params::extExif:  rc = writeExifData(exifData); break;
        case Params::extThumb: rc = writeThumbnail(exifData); break;
        }
        return rc;
    }
    catch(const Exiv2::Error& e)
    {
        std::cerr << "Exif exception in extract action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Extract::run

    int Extract::writeExifData(Exiv2::ExifData& exifData) const
    {
        std::string exvPath =   Util::dirname(path_) + SEPERATOR_STR
                              + Util::basename(path_, true) + ".exv";
        if (Params::instance().verbose_) {
            std::cout << "Writing Exif data to " << exvPath << "\n";
        }
        if (!Params::instance().force_ && Util::fileExists(exvPath)) {
            std::cout << Params::instance().progname() 
                      << ": Overwrite `" << exvPath << "'? ";
            std::string s;
            std::cin >> s;
            if (s[0] != 'y' && s[0] != 'Y') return 0;
        }
        int rc = exifData.writeExifData(exvPath);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, exvPath) << "\n";
        }
        return rc;
    }

    int Extract::writeThumbnail(const Exiv2::ExifData& exifData) const
    {
        int rc = 0;
        std::string thumb =   Util::dirname(path_) + SEPERATOR_STR
                            + Util::basename(path_, true) + "-thumb";
        std::string thumbExt = exifData.thumbnailExtension();
        if (thumbExt.empty()) {
            std::cerr << path_ << ": Image does not contain an Exif thumbnail\n"; 
        }
        else {
            if (Params::instance().verbose_) {
                std::cout << "Writing "
                          << exifData.thumbnailFormat() << " thumbnail (" 
                          << exifData.thumbnailSize() << " Bytes) to file "
                          << thumb << thumbExt << "\n";
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
    }

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
        std::string exvPath =   Util::dirname(path) + SEPERATOR_STR
                              + Util::basename(path, true) + ".exv";
        Exiv2::ExifData exifData;
        int rc = exifData.read(exvPath);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, exvPath) << "\n";
            return rc;
        }
        if (Params::instance().verbose_) {
            std::cout << "Inserting metadata from " << exvPath << "\n";
        }
        rc = exifData.write(path);
        if (rc) {
            std::cerr << Exiv2::ExifData::strError(rc, path) << "\n";
        }
        return rc;
    }
    catch(const Exiv2::Error& e)
    {
        std::cerr << "Exif exception in insert action for file " << path
                  << ":\n" << e << "\n";
        return 1;
    } // Insert::run

    Insert::AutoPtr Insert::clone() const
    {
        return AutoPtr(clone_());
    }

    Insert* Insert::clone_() const
    {
        return new Insert(*this);
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
            std::cout << timeStr << "\n";
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
}
