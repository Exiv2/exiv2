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
  Version:   $Name:  $ $Revision: 1.21 $
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   08-Dec-03, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Name:  $ $Revision: 1.21 $ $RCSfile: actions.cpp,v $")

// *****************************************************************************
// included header files
#include "actions.hpp"
#include "exiv2.hpp"
#include "utils.hpp"
#include "types.hpp"
#include "exif.hpp"
#include "canonmn.hpp"

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
#include <unistd.h>                     // for stat()

// *****************************************************************************
// local declarations
namespace {

    // Convert a string "YYYY:MM:DD HH:MI:SS" to a struct tm type, 
    // returns 0 if successful
    int str2Tm(const std::string& timeStr, struct tm* tm);

    // Convert a string "YYYY:MM:DD HH:MI:SS" to a time type, -1 on error
    time_t str2Time(const std::string& timeStr);

    // Convert a time type to a string "YYYY:MM:DD HH:MI:SS", "" on error
    std::string time2Str(time_t time);

    // Return an error message for the return code of Exiv2::ExifData::read
    std::string exifReadError(int rc, const std::string& path);

    // Return an error message for the return code of Exiv2::ExifData::write
    std::string exifWriteError(int rc, const std::string& path);

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
        Exiv2::ExifData exifData;
        int rc = exifData.read(path);
        if (rc) {
            std::cerr << exifReadError(rc, path) << "\n";
            return rc;
        }
        switch (Params::instance().printMode_) {
        case Params::summary:     printSummary(exifData); break;
        case Params::interpreted: printInterpreted(exifData); break;
        case Params::values:      printValues(exifData); break;
        case Params::hexdump:     printHexdump(exifData); break;
        }
        return 0;
    }
    catch(const Exiv2::Error& e)
    {
        std::cerr << "Exif exception in print action for file " 
                  << path << ":\n" << e << "\n";
        return 1;
    } // Print::run

    void Print::printSummary(const Exiv2::ExifData& exifData)
    {
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
        printTag(exifData, "Image.OtherTags.Make", "Camera make");

        // Camera model
        printTag(exifData, "Image.OtherTags.Model", "Camera model");

        // Image Timestamp
        printTag(exifData, "Image.DateTime.DateTimeOriginal", "Image timestamp");

        // Image number
        // Todo: Image number for cameras other than Canon
        printTag(exifData, "Makernote.Canon.ImageNumber", "Image number");

        // Exposure time
        // From ExposureTime, failing that, try ShutterSpeedValue
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Exposure time" << ": ";
        Exiv2::ExifData::const_iterator md;
        if (0 == printTag(exifData, "Image.CaptureConditions.ExposureTime")) {
            md = exifData.findKey("Image.CaptureConditions.ShutterSpeedValue");
            if (md != exifData.end()) {
                float f = exp2f(md->toFloat()) + 0.5;
                if (f > 1) {
                    std::cout << "1/" << static_cast<long>(f) << " s";
                }
                else {
                    std::cout << static_cast<long>(1/f) << " s";
                }
            }
        }
        std::cout << "\n";

        // Aperture
        // Get if from FNumber and, failing that, try ApertureValue
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Aperture" << ": ";
        if (0 == printTag(exifData, "Image.CaptureConditions.FNumber")) {
            md = exifData.findKey("Image.CaptureConditions.ApertureValue");
            if (md != exifData.end()) {
                std::cout << std::fixed << std::setprecision(1)
                          << "F" << exp2f(md->toFloat()/2);
            }
        }
        std::cout << "\n";

        // Exposure bias
        printTag(exifData, "Image.CaptureConditions.ExposureBiasValue", "Exposure bias");

        // Flash
        printTag(exifData, "Image.CaptureConditions.Flash", "Flash");

        // Todo: Flash bias, flash energy
        // Todo: Implement this for other cameras
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Flash bias" << ": ";
        md = exifData.findKey("Makernote.Canon.CameraSettings2");
        if (md != exifData.end() && md->count() >= 15) {
            Exiv2::CanonMakerNote::print0x0004_15(std::cout, md->toLong(15));
        }
        std::cout << "\n";

        // Actual focal length and 35 mm equivalent
        // Todo: Calculate 35 mm equivalent a la jhead
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Focal length" << ": ";
        if (1 == printTag(exifData, "Image.CaptureConditions.FocalLength")) {
            md = exifData.findKey("Image.CaptureConditions.FocalLengthIn35mmFilm");
            if (md != exifData.end()) {
                std::cout << " (35 mm equivalent: " << *md << ")";
            }
        }
        std::cout << "\n";

        // Subject distance
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Subject distance" << ": ";
	if (0 == printTag(exifData, "Image.CaptureConditions.SubjectDistance")) {
            md = exifData.findKey("Makernote.Canon.CameraSettings2");
            if (md != exifData.end() && md->count() >= 19) {
                Exiv2::CanonMakerNote::print0x0004_19(std::cout, md->toLong(19));
            }
        }
        std::cout << "\n";

        // ISO speed
        // from ISOSpeedRatings or Canon Makernote
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "ISO speed" << ": ";
        if (0 == printTag(exifData, "Image.CaptureConditions.ISOSpeedRatings")) {
            md = exifData.findKey("Makernote.Canon.CameraSettings1");
            if (md != exifData.end() && md->count() >= 16) {
                Exiv2::CanonMakerNote::print0x0001_16(std::cout, md->toLong(16));
            }
        }
        std::cout << "\n";

        // Exposure mode 
        // From ExposureProgram or Canon Makernote
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Exposure mode" << ": ";
        if (0 == printTag(exifData, "Image.CaptureConditions.ExposureProgram")) {
            md = exifData.findKey("Makernote.Canon.CameraSettings1");
            if (md != exifData.end() && md->count() >= 20) {
                Exiv2::CanonMakerNote::print0x0001_20(std::cout, md->toLong(20));
            }
        }
        std::cout << "\n";

        // Metering mode
        printTag(exifData, "Image.CaptureConditions.MeteringMode", "Metering mode");

        // Macro mode
        // Todo: Implement this for other cameras
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Macro mode" << ": ";
        bool done = false;
        md = exifData.findKey("Makernote.Canon.CameraSettings1");
        if (md != exifData.end() && md->count() >= 1) {
            Exiv2::CanonMakerNote::print0x0001_01(std::cout, md->toLong(1));
            done = true;
        }
        if (!done) {
            done = printTag(exifData, "Makernote.Fujifilm.Macro");
        }            
        std::cout << "\n";

        // Image quality setting (compression)
        // Todo: Implement this for other cameras
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Image quality" << ": ";
        done = false;
        md = exifData.findKey("Makernote.Canon.CameraSettings1");
        if (md != exifData.end() && md->count() >= 3) {
            Exiv2::CanonMakerNote::print0x0001_03(std::cout, md->toLong(3));
            done = true;
        }
        if (!done) {
            done = printTag(exifData, "Makernote.Fujifilm.Quality");
        }            
        if (!done) {
            done = printTag(exifData, "Makernote.Sigma.Quality");
        }            
        std::cout << "\n";

        // Exif Resolution
        std::cout << std::setw(align_) << std::setfill(' ') << std::left
                  << "Exif Resolution" << ": ";
        long xdim = 0;
        long ydim = 0;
        md = exifData.findKey("Image.ImageConfig.PixelXDimension");
        if (md != exifData.end()) xdim = md->toLong();
        md = exifData.findKey("Image.ImageConfig.PixelYDimension");
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
        md = exifData.findKey("Makernote.Canon.CameraSettings2");
        if (md != exifData.end() && md->count() >= 7) {
            Exiv2::CanonMakerNote::print0x0004_07(std::cout, md->toLong(7));
            done = true;
        }
        if (!done) {
            done = printTag(exifData, "Makernote.Fujifilm.WhiteBalance");
        }            
        if (!done) {
            done = printTag(exifData, "Makernote.Sigma.WhiteBalance");
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
        printTag(exifData, "Image.OtherTags.Copyright", "Copyright");

        // Exif Comment
        printTag(exifData, "Image.UserInfo.UserComment", "Exif comment");
        std::cout << std::endl;

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
        Exiv2::ExifData::const_iterator md = exifData.findKey(key);
        if (md != exifData.end()) {
            std::cout << *md;
            rc = 1;
        }
        if (!label.empty()) std::cout << "\n";
        return rc;
    } // Print::printTag

    void Print::printInterpreted(const Exiv2::ExifData& exifData)
    {
        Exiv2::ExifData::const_iterator md;
        for (md = exifData.begin(); md != exifData.end(); ++md) {
            std::cout << "0x" << std::setw(4) << std::setfill('0') << std::right
                      << std::hex << md->tag() << " " 
                      << std::setw(9) << std::setfill(' ') << std::left
                      << md->ifdItem() << " "
                      << std::setw(27) << std::setfill(' ') << std::left
                      << md->tagName() << " "
                      << std::dec << *md << "\n";
        }
    } // Print::printInterpreted

    void Print::printValues(const Exiv2::ExifData& exifData)
    {
        Exiv2::ExifData::const_iterator md;
        for (md = exifData.begin(); md != exifData.end(); ++md) {
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
    } // Print::printValues

    void Print::printHexdump(const Exiv2::ExifData& exifData)
    {
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
    } // Print::printHexdump

    Print::AutoPtr Print::clone() const
    {
        return AutoPtr(dynamic_cast<Print*>(clone_()));
    }

    Task* Print::clone_() const
    {
        return new Print(*this);
    }

    int Rename::run(const std::string& path)
    try {
        Exiv2::ExifData exifData;
        int rc = exifData.read(path);
        if (rc) {
            std::cerr << exifReadError(rc, path) << "\n";
            return rc;
        }
        std::string key = "Image.DateTime.DateTimeOriginal";
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
            = Util::dirname(path) + "/" + basename + Util::suffix(path);
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
        ::remove(newPath.c_str());
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
        return AutoPtr(dynamic_cast<Rename*>(clone_()));
    }

    Task* Rename::clone_() const
    {
        return new Rename(*this);
    }

    int Erase::run(const std::string& path)
    try {
        path_ = path;
        Exiv2::ExifData exifData;
        int rc = exifData.read(path);
        if (rc) {
            std::cerr << exifReadError(rc, path) << "\n";
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
            if (Params::instance().verbose_) {
                std::cout << "Image does not contain an Exif thumbnail\n"; 
            }
        }
        else {
            long delta = exifData.eraseThumbnail();
            if (Params::instance().verbose_) {
                std::cout << "Erasing " << delta << " Bytes of thumbnail data\n"; 
            }
            rc = exifData.write(path_);
            if (rc) {
                std::cerr << exifWriteError(rc, path_) << "\n";
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
            std::cerr << exifWriteError(rc, path_) << "\n";
        }
        return rc;
    }

    Erase::AutoPtr Erase::clone() const
    {
        return AutoPtr(dynamic_cast<Erase*>(clone_()));
    }

    Task* Erase::clone_() const
    {
        return new Erase(*this);
    }

    int Extract::run(const std::string& path)
    try {
        path_ = path;
        Exiv2::ExifData exifData;
        int rc = exifData.read(path);
        if (rc) {
            std::cerr << exifReadError(rc, path) << "\n";
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
        std::string exvPath =   Util::dirname(path_) + "/"
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
            std::cerr << exifWriteError(rc, exvPath) << "\n";
        }
        return rc;
    }

    int Extract::writeThumbnail(const Exiv2::ExifData& exifData) const
    {
        std::string thumb =   Util::dirname(path_) + "/"
                            + Util::basename(path_, true) + "-thumb";
        std::string thumbExt = exifData.thumbnailExtension();
        if (Params::instance().verbose_) {
            if (thumbExt.empty()) {
                std::cout << "Image does not contain an Exif thumbnail\n";
            }
            else {
                std::cout << "Writing "
                          << exifData.thumbnailFormat() << " thumbnail (" 
                          << exifData.thumbnailSize() << " Bytes) to file "
                          << thumb << thumbExt << "\n";
            }
        }
        int rc = 0;
        if (!thumbExt.empty()) {
            if (!Params::instance().force_ && Util::fileExists(thumb + thumbExt)) {
                std::cout << Params::instance().progname() 
                          << ": Overwrite `" << thumb + thumbExt << "'? ";
                std::string s;
                std::cin >> s;
                if (s[0] != 'y' && s[0] != 'Y') return 0;
            }
            rc = exifData.writeThumbnail(thumb);
            if (rc) {
                std::cerr << exifWriteError(rc, thumb) << "\n";
            }
        }
        return rc;
    }

    Extract::AutoPtr Extract::clone() const
    {
        return AutoPtr(dynamic_cast<Extract*>(clone_()));
    }

    Task* Extract::clone_() const
    {
        return new Extract(*this);
    }

    int Insert::run(const std::string& path)
    try {
        std::string exvPath =   Util::dirname(path) + "/"
                              + Util::basename(path, true) + ".exv";
        Exiv2::ExifData exifData;
        int rc = exifData.read(exvPath);
        if (rc) {
            std::cerr << exifReadError(rc, exvPath) << "\n";
            return rc;
        }
        if (Params::instance().verbose_) {
            std::cout << "Inserting metadata from " << exvPath << "\n";
        }
        rc = exifData.write(path);
        if (rc) {
            std::cerr << exifWriteError(rc, path) << "\n";
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
        return AutoPtr(dynamic_cast<Insert*>(clone_()));
    }

    Task* Insert::clone_() const
    {
        return new Insert(*this);
    }

    int Adjust::run(const std::string& path)
    try {
        adjustment_ = Params::instance().adjustment_;

        Exiv2::ExifData exifData;
        int rc = exifData.read(path);
        if (rc) {
            std::cerr << exifReadError(rc, path) << "\n";
            return rc;
        }
        rc  = adjustDateTime(exifData, "Image.OtherTags.DateTime", path);
        rc += adjustDateTime(exifData, "Image.DateTime.DateTimeOriginal", path);
        rc += adjustDateTime(exifData, "Image.DateTime.DateTimeDigitized", path);
        if (rc) return 1;
        rc = exifData.write(path);
        if (rc) {
            std::cerr << exifWriteError(rc, path) << "\n";
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
        return AutoPtr(dynamic_cast<Adjust*>(clone_()));
    }

    Task* Adjust::clone_() const
    {
        return new Adjust(*this);
    }

    int Adjust::adjustDateTime(Exiv2::ExifData& exifData,
                               const std::string& key, 
                               const std::string& path) const
    {
        Exiv2::ExifData::iterator md = exifData.findKey(key);
        if (md == exifData.end()) {
            // Key not found. That's ok, we do nothing.
            return 0;
        }
        std::string timeStr = md->toString();
        if (timeStr == "" || timeStr[0] == ' ') {
            std::cerr << path << ": Timestamp of metadatum with key `" 
                      << key << "' not set\n";
            return 1;
        }
        time_t time = str2Time(timeStr);
        if (time == (time_t)-1) {
            std::cerr << path << ": Failed to parse or convert timestamp `" 
                      << timeStr << "'\n";
            return 1;
        }
        if (Params::instance().verbose_) {
            std::cout << "Adjusting `" << key << "' by" 
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
        ::memset(tm, 0x0, sizeof(struct tm));

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
        return ::mktime(&tm);
    }

    std::string time2Str(time_t time)
    {
        struct tm* tm = localtime(&time);
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

    std::string exifReadError(int rc, const std::string& path)
    {
        std::string error;
        switch (rc) {
        case -1:
            error = path + ": Failed to open the file";
            break;
        case -2:
            error = path + ": The file contains data of an unknown image type";
            break;
        case 1:
            error = path + ": Couldn't read from the input stream";
            break;
        case 2:
            error = path + ": This does not look like a JPEG image";
            break;
        case 3:
            error = path + ": No Exif data found in the file";
            break;
        case -99:
            error = path + ": Unsupported Exif or GPS data found in IFD 1";
            break;
        default:
            error = path + ": Reading Exif data failed, rc = " + Exiv2::toString(rc);
            break;
        }
        return error;
    } // exifReadError

    std::string exifWriteError(int rc, const std::string& path)
    {
        std::string error;
        switch (rc) {
        case -1:
            error = path + ": Failed to open the file";
            break;
        case -2:
            error = path + ": The file contains data of an unknown image type";
            break;
        case -3:
            error = path + ": Couldn't open temporary file";
            break;
        case -4:
            error = path + ": Renaming temporary file failed: " + Util::strError();
            break;
        case 1:
            error = path + ": Couldn't read from the input stream";
            break;
        case 2:
            error = path + ": This does not look like a JPEG image";
            break;
        case 3:
            error = path + ": No JFIF APP0 or Exif APP1 segment found in the file";
            break;
        case 4:
            error = path + ": Writing to the output stream failed";
            break;
        default:
            error = path + ": Reading Exif data failed, rc = " + Exiv2::toString(rc);
            break;
        }
        return error;
    } // exifWriteError
}
