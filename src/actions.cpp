// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
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
 */
// *****************************************************************************
// included header files
#include "actions.hpp"

#include "config.h"
#include "i18n.h"  // NLS support.

#include <exiv2/easyaccess.hpp>
#include <exiv2/exif.hpp>
#include <exiv2/futils.hpp>
#include <exiv2/types.hpp>
#include <exiv2/error.hpp>

// + standard includes
#include <sys/stat.h>   // for stat()
#include <sys/types.h>  // for stat()
#include <fstream>
#ifdef EXV_HAVE_UNISTD_H
#include <unistd.h>  // for stat()
#endif
#ifdef _MSC_VER
#include <Windows.h>
#include <sys/utime.h>
#else
#include <utime.h>
#endif

#if defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER)
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif

#if !defined(__MINGW__) && !defined(_MSC_VER)
#define _fileno(a) a
#define _setmode(a, b)
#endif

// *****************************************************************************
// local declarations
namespace
{
    //! Helper class to set the timestamp of a file to that of another file
    class Timestamp
    {
    public:
        //! C'tor
      Timestamp() = default;
      //! Read the timestamp of a file
      int read(const std::string &path);
      //! Read the timestamp from a broken-down time in buffer \em tm.
      int read(struct tm *tm);
      //! Set the timestamp of a file
      int touch(const std::string &path);

    private:
      time_t actime_{0};
      time_t modtime_{0};
    };

    /*!
      @brief Convert a string "YYYY:MM:DD HH:MI:SS" to a struct tm type,
             returns 0 if successful
     */
    int str2Tm(const std::string& timeStr, struct tm* tm);

    //! Convert a localtime to a string "YYYY:MM:DD HH:MI:SS", "" on error
    std::string time2Str(time_t time);

    //! Convert a tm structure to a string "YYYY:MM:DD HH:MI:SS", "" on error
    std::string tm2Str(const struct tm* tm);

    /*!
      @brief Copy metadata from source to target according to Params::copyXyz

      @param source Source file path
      @param target Target file path. An *.exv file is created if target doesn't
                    exist.
      @param targetType Image type for the target image in case it needs to be
                    created.
      @param preserve Indicates if existing metadata in the target file should
                    be kept.
      @return 0 if successful, else an error code
    */
    int metacopy(const std::string& source, const std::string& target, Exiv2::ImageType targetType, bool preserve);

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
      @param ext  New filename extension (incl. the dot '.' if required)
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

    /*!
      @brief Output a text with a given minimum number of chars, honoring
             multi-byte characters correctly. Replace code in the form
             os << setw(width) << myString
             with
             os << make_pair( myString, width)
     */
    std::ostream& operator<<(std::ostream& os, std::pair<std::string, int> strAndWidth);

    //! Print image Structure information
    int printStructure(std::ostream& out, Exiv2::PrintStructureOption option, const std::string& path);
}  // namespace

// *****************************************************************************
// class member definitions
namespace Action
{
Task::~Task() = default;

std::unique_ptr<Task> Task::clone() const {
  return std::unique_ptr<Task>(clone_());
}

    TaskFactory& TaskFactory::instance()
    {
        static TaskFactory ins;
        return ins;
    }

    TaskFactory::TaskFactory()
        : registry_{
              {adjust, new Adjust}, {print, new Print},     {rename, new Rename},
              {erase, new Erase},   {extract, new Extract}, {insert, new Insert},
              {modify, new Modify}, {fixiso, new FixIso},   {fixcom, new FixCom},
          }
    {
    }

    TaskFactory::~TaskFactory()
    {
        for (auto it = registry_.begin(); it != registry_.end(); ++it) {
            delete it->second;
        }
    }

    std::unique_ptr<Task> TaskFactory::create(TaskType type)
    {
        auto i = registry_.find(type);
        if (i != registry_.end() && i->second != 0) {
            Task* t = i->second;
            return t->clone();
        }
        return nullptr;
    }

    Print::~Print() = default;

    int setModeAndPrintStructure(Exiv2::PrintStructureOption option, const std::string& path)
    {
        _setmode(_fileno(stdout), O_BINARY);
        return printStructure(std::cout, option, path);
    }

    int Print::run(const std::string& path)
    {
        try {
            path_ = path;
            int rc = 0;
            Exiv2::PrintStructureOption option = Exiv2::kpsNone;
            switch (Params::instance().printMode_) {
                case Params::pmSummary:
                    rc = Params::instance().greps_.empty() ? printSummary() : printList();
                    break;
                case Params::pmList:
                    rc = printList();
                    break;
                case Params::pmComment:
                    rc = printComment();
                    break;
                case Params::pmPreview:
                    rc = printPreviewList();
                    break;
                case Params::pmStructure:
                    rc = printStructure(std::cout, Exiv2::kpsBasic, path_);
                    break;
                case Params::pmRecursive:
                    rc = printStructure(std::cout, Exiv2::kpsRecursive, path_);
                    break;
                case Params::pmXMP:
                    if (option == Exiv2::kpsNone)
                        option = Exiv2::kpsXMP;
                    rc = setModeAndPrintStructure(option, path_);
                    break;
                case Params::pmIccProfile:
                    if (option == Exiv2::kpsNone)
                        option = Exiv2::kpsIccProfile;
                    rc = setModeAndPrintStructure(option, path_);
                    break;
            }
            return rc;
        } catch (const Exiv2::AnyError& e) {
            std::cerr << "Exiv2 exception in print action for file " << path << ":\n" << e << "\n";
            return 1;
        } catch (const std::overflow_error& e) {
            std::cerr << "std::overflow_error exception in print action for file " << path << ":\n" << e.what() << "\n";
            return 1;
        }
    }

    int Print::printSummary()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_ << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path_);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
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

        // MIME type
        printLabel(_("MIME type"));
        std::cout << image->mimeType() << std::endl;

        // Image size
        printLabel(_("Image size"));
        std::cout << image->pixelWidth() << " x " << image->pixelHeight() << std::endl;

        if (exifData.empty()) {
            std::cerr << path_ << ": " << _("No Exif data found in the file\n");
            return -3;
        }

        // Camera make
        printTag(exifData, "Exif.Image.Make", _("Camera make"));

        // Camera model
        printTag(exifData, "Exif.Image.Model", _("Camera model"));

        // Image Timestamp
        printTag(exifData, "Exif.Photo.DateTimeOriginal", _("Image timestamp"));

        // Image number
        // Todo: Image number for cameras other than Canon
        printTag(exifData, "Exif.Canon.FileNumber", _("Image number"));

        // Exposure time
        // From ExposureTime, failing that, try ShutterSpeedValue
        printLabel(_("Exposure time"));
        {
            bool done = 0 != printTag(exifData, "Exif.Photo.ExposureTime");
            if (!done) {
                printTag(exifData, "Exif.Photo.ShutterSpeedValue");
            }
        }
        std::cout << std::endl;

        // Aperture
        // Get if from FNumber and, failing that, try ApertureValue
        {
            printLabel(_("Aperture"));
            {
                bool done = 0 != printTag(exifData, "Exif.Photo.FNumber");
                if (!done) {
                    printTag(exifData, "Exif.Photo.ApertureValue");
                }
            }
            std::cout << std::endl;

            // Exposure bias
            printTag(exifData, "Exif.Photo.ExposureBiasValue", _("Exposure bias"));

            // Flash
            printTag(exifData, "Exif.Photo.Flash", _("Flash"));

            // Flash bias
            printTag(exifData, Exiv2::flashBias, _("Flash bias"));

            // Actual focal length and 35 mm equivalent
            // Todo: Calculate 35 mm equivalent a la jhead
            printLabel(_("Focal length"));
            if (1 == printTag(exifData, "Exif.Photo.FocalLength")) {
                auto md = exifData.findKey(Exiv2::ExifKey("Exif.Photo.FocalLengthIn35mmFilm"));
                if (md != exifData.end()) {
                    std::cout << " (" << _("35 mm equivalent") << ": " << md->print(&exifData) << ")";
                }
            } else {
                printTag(exifData, "Exif.Canon.FocalLength");
            }
            std::cout << std::endl;
        }

        // Subject distance
        {
            printLabel(_("Subject distance"));
            bool done = 0 != printTag(exifData, "Exif.Photo.SubjectDistance");
            if (!done) {
                printTag(exifData, "Exif.CanonSi.SubjectDistance");
                printTag(exifData, "Exif.CanonFi.FocusDistanceLower");
                printTag(exifData, "Exif.CanonFi.FocusDistanceUpper");
            }
            std::cout << std::endl;
        }

        // ISO speed
        printTag(exifData, Exiv2::isoSpeed, _("ISO speed"));

        // Exposure mode
        printTag(exifData, Exiv2::exposureMode, _("Exposure mode"));

        // Metering mode
        printTag(exifData, "Exif.Photo.MeteringMode", _("Metering mode"));

        // Macro mode
        printTag(exifData, Exiv2::macroMode, _("Macro mode"));

        // Image quality setting (compression)
        printTag(exifData, Exiv2::imageQuality, _("Image quality"));

        // Exif Resolution
        {
            printLabel(_("Exif Resolution"));
            long xdim = 0;
            long ydim = 0;
            if (image->mimeType() == "image/tiff") {
                xdim = image->pixelWidth();
                ydim = image->pixelHeight();
            } else {
                auto md = exifData.findKey(Exiv2::ExifKey("Exif.Image.ImageWidth"));
                if (md == exifData.end()) {
                    md = exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelXDimension"));
                }
                if (md != exifData.end() && md->count() > 0) {
                    xdim = md->toLong();
                }
                md = exifData.findKey(Exiv2::ExifKey("Exif.Image.ImageLength"));
                if (md == exifData.end()) {
                    md = exifData.findKey(Exiv2::ExifKey("Exif.Photo.PixelYDimension"));
                }
                if (md != exifData.end() && md->count() > 0) {
                    ydim = md->toLong();
                }
            }
            if (xdim != 0 && ydim != 0) {
                std::cout << xdim << " x " << ydim;
            }
            std::cout << std::endl;
        }

        // White balance
        printTag(exifData, Exiv2::whiteBalance, _("White balance"));

        // Thumbnail
        printLabel(_("Thumbnail"));
        Exiv2::ExifThumbC exifThumb(exifData);
        std::string thumbExt = exifThumb.extension();
        if (thumbExt.empty()) {
            std::cout << _("None");
        } else {
            Exiv2::DataBuf dataBuf = exifThumb.copy();
            if (dataBuf.size_ == 0) {
                std::cout << _("None");
            } else {
                std::cout << exifThumb.mimeType() << ", " << dataBuf.size_ << " " << _("Bytes");
            }
        }
        std::cout << std::endl;

        // Copyright
        printTag(exifData, "Exif.Image.Copyright", _("Copyright"));

        // Exif Comment
        printTag(exifData, "Exif.Photo.UserComment", _("Exif comment"));
        std::cout << std::endl;

        return 0;
    }

    void Print::printLabel(const std::string& label) const
    {
        std::cout << std::setfill(' ') << std::left;
        if (Params::instance().files_.size() > 1) {
            std::cout << std::setw(20) << path_ << " ";
        }
        std::cout << std::make_pair(label, align_) << ": ";
    }

    int Print::printTag(const Exiv2::ExifData& exifData, const std::string& key, const std::string& label) const
    {
        int rc = 0;
        if (!label.empty()) {
            printLabel(label);
        }
        Exiv2::ExifKey ek(key);
        auto md = exifData.findKey(ek);
        if (md != exifData.end()) {
            md->write(std::cout, &exifData);
            rc = 1;
        }
        if (!label.empty())
            std::cout << std::endl;
        return rc;
    }

    int Print::printTag(const Exiv2::ExifData& exifData, EasyAccessFct easyAccessFct, const std::string& label) const
    {
        int rc = 0;
        if (!label.empty()) {
            printLabel(label);
        }
        auto md = easyAccessFct(exifData);
        if (md != exifData.end()) {
            md->write(std::cout, &exifData);
            rc = 1;
        }
        if (!label.empty())
            std::cout << std::endl;
        return rc;
    }

    int Print::printList()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_ << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != nullptr);
        image->readMetadata();
        // Set defaults for metadata types and data columns
        if (Params::instance().printTags_ == Exiv2::mdNone) {
            Params::instance().printTags_ = Exiv2::mdExif | Exiv2::mdIptc | Exiv2::mdXmp;
        }
        if (Params::instance().printItems_ == 0) {
            Params::instance().printItems_ = Params::prKey | Params::prType | Params::prCount | Params::prTrans;
        }
        return printMetadata(image.get());
    }

    int Print::printMetadata(const Exiv2::Image* image)
    {
        bool ret = false;
        bool noExif = false;
        if (Params::instance().printTags_ & Exiv2::mdExif) {
            const Exiv2::ExifData& exifData = image->exifData();
            for (auto md = exifData.begin(); md != exifData.end(); ++md) {
                ret |= printMetadatum(*md, image);
            }
            if (exifData.empty())
                noExif = true;
        }

        bool noIptc = false;
        if (Params::instance().printTags_ & Exiv2::mdIptc) {
            const Exiv2::IptcData& iptcData = image->iptcData();
            for (auto md = iptcData.begin(); md != iptcData.end(); ++md) {
                ret |= printMetadatum(*md, image);
            }
            if (iptcData.empty())
                noIptc = true;
        }

        bool noXmp = false;
        if (Params::instance().printTags_ & Exiv2::mdXmp) {
            const Exiv2::XmpData& xmpData = image->xmpData();
            for (auto md = xmpData.begin(); md != xmpData.end(); ++md) {
                ret |= printMetadatum(*md, image);
            }
            if (xmpData.empty())
                noXmp = true;
        }

        // With -v, inform about the absence of any (requested) type of metadata
        if (Params::instance().verbose_) {
            if (noExif)
                std::cerr << path_ << ": " << _("No Exif data found in the file\n");
            if (noIptc)
                std::cerr << path_ << ": " << _("No IPTC data found in the file\n");
            if (noXmp)
                std::cerr << path_ << ": " << _("No XMP data found in the file\n");
        }

        // With -g or -K, return -3 if no matching tags were found
        int rc = 0;
        if ((!Params::instance().greps_.empty() || !Params::instance().keys_.empty()) && !ret)
            rc = 1;

        return rc;
    }

    bool Print::grepTag(const std::string& key)
    {
        bool result = Params::instance().greps_.empty();
        for (auto g = Params::instance().greps_.begin(); !result && g != Params::instance().greps_.end(); ++g)
        {
            re::smatch m;
            result = re::regex_search(key, m, *g);
        }
        return result;
    }

    bool Print::keyTag(const std::string& key)
    {
        bool result = Params::instance().keys_.empty();
        for (auto k = Params::instance().keys_.begin(); !result && k != Params::instance().keys_.end(); ++k) {
            result = key.compare(*k) == 0;
        }
        return result;
    }

    bool Print::printMetadatum(const Exiv2::Metadatum& md, const Exiv2::Image* pImage)
    {
        if (!grepTag(md.key()))
            return false;
        if (!keyTag(md.key()))
            return false;

        if (Params::instance().unknown_ && md.tagName().substr(0, 2) == "0x") {
            return false;
        }

        bool const manyFiles = Params::instance().files_.size() > 1;
        if (manyFiles) {
            std::cout << std::setfill(' ') << std::left << std::setw(20) << path_ << "  ";
        }

        bool first = true;
        if (Params::instance().printItems_ & Params::prTag) {
            if (!first)
                std::cout << " ";
            first = false;
            std::cout << "0x" << std::setw(4) << std::setfill('0') << std::right << std::hex << md.tag();
        }
        if (Params::instance().printItems_ & Params::prSet) {
            if (!first)
                std::cout << " ";
            first = false;
            std::cout << "set";
        }
        if (Params::instance().printItems_ & Params::prGroup) {
            if (!first)
                std::cout << " ";
            first = false;
            std::cout << std::setw(12) << std::setfill(' ') << std::left << md.groupName();
        }
        if (Params::instance().printItems_ & Params::prKey) {
            if (!first)
                std::cout << " ";
            first = false;
            std::cout << std::setfill(' ') << std::left << std::setw(44) << md.key();
        }
        if (Params::instance().printItems_ & Params::prName) {
            if (!first)
                std::cout << " ";
            first = false;
            std::cout << std::setw(27) << std::setfill(' ') << std::left << md.tagName();
        }
        if (Params::instance().printItems_ & Params::prLabel) {
            if (!first)
                std::cout << " ";
            first = false;
            std::cout << std::setw(30) << std::setfill(' ') << std::left << md.tagLabel();
        }
        if (Params::instance().printItems_ & Params::prType) {
            if (!first)
                std::cout << " ";
            first = false;
            std::cout << std::setw(9) << std::setfill(' ') << std::left;
            const char* tn = md.typeName();
            if (tn) {
                std::cout << tn;
            } else {
                std::ostringstream os;
                os << "0x" << std::setw(4) << std::setfill('0') << std::hex << md.typeId();
                std::cout << os.str();
            }
        }
        if (Params::instance().printItems_ & Params::prCount) {
            if (!first)
                std::cout << " ";
            first = false;
            std::cout << std::dec << std::setw(3) << std::setfill(' ') << std::right << md.count();
        }
        if (Params::instance().printItems_ & Params::prSize) {
            if (!first)
                std::cout << " ";
            first = false;
            std::cout << std::dec << std::setw(3) << std::setfill(' ') << std::right << md.size();
        }
        if (Params::instance().printItems_ & Params::prValue && md.size() > 0) {
            if (!first)
                std::cout << "  ";
            first = false;
            if (md.size() > 128 && Params::instance().binary_ &&
                (md.typeId() == Exiv2::undefined || md.typeId() == Exiv2::unsignedByte ||
                 md.typeId() == Exiv2::signedByte)) {
                std::cout << _("(Binary value suppressed)") << std::endl;
                return true;
            }
            bool done = false;
            if (0 == strcmp(md.key().c_str(), "Exif.Photo.UserComment")) {
                const Exiv2::CommentValue* pcv = dynamic_cast<const Exiv2::CommentValue*>(&md.value());
                if (pcv) {
                    Exiv2::CommentValue::CharsetId csId = pcv->charsetId();
                    if (csId != Exiv2::CommentValue::undefined) {
                        std::cout << "charset=\"" << Exiv2::CommentValue::CharsetInfo::name(csId) << "\" ";
                    }
                    std::cout << pcv->comment(Params::instance().charset_.c_str());
                    done = true;
                }
            }
            if (!done) {
                // #1114 - show negative values for SByte
                if (md.typeId() != Exiv2::signedByte) {
                    std::cout << std::dec << md.value();
                } else {
                    long value = md.value().toLong();
                    std::cout << std::dec << (value < 128 ? value : value - 256);
                }
            }
        }
        if (Params::instance().printItems_ & Params::prTrans) {
            if (!first)
                std::cout << "  ";
            first = false;
            if (Params::instance().binary_ &&
                (md.typeId() == Exiv2::undefined || md.typeId() == Exiv2::unsignedByte ||
                 md.typeId() == Exiv2::signedByte) &&
                md.size() > 128) {
                std::cout << _("(Binary value suppressed)") << std::endl;
                return true;
            }
            bool done = false;
            if (0 == strcmp(md.key().c_str(), "Exif.Photo.UserComment")) {
                const Exiv2::CommentValue* pcv = dynamic_cast<const Exiv2::CommentValue*>(&md.value());
                if (pcv) {
                    std::cout << pcv->comment(Params::instance().charset_.c_str());
                    done = true;
                }
            }
            if (!done)
                std::cout << std::dec << md.print(&pImage->exifData());
        }
        if (Params::instance().printItems_ & Params::prHex) {
            if (!first)
                std::cout << std::endl;
            first = false;
            if (Params::instance().binary_ &&
                (md.typeId() == Exiv2::undefined || md.typeId() == Exiv2::unsignedByte ||
                 md.typeId() == Exiv2::signedByte) &&
                md.size() > 128) {
                std::cout << _("(Binary value suppressed)") << std::endl;
                return true;
            }
            Exiv2::DataBuf buf(md.size());
            md.copy(buf.pData_, pImage->byteOrder());
            Exiv2::hexdump(std::cout, buf.pData_, (long)buf.size_);
        }
        std::cout << std::endl;
        return true;
    }

    int Print::printComment()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_ << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != nullptr);
        image->readMetadata();
        if (Params::instance().verbose_) {
            std::cout << _("JPEG comment") << ": ";
        }
        std::cout << image->comment() << std::endl;
        return 0;
    }

    int Print::printPreviewList()
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_ << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path_);
        assert(image.get() != nullptr);
        image->readMetadata();
        bool const manyFiles = Params::instance().files_.size() > 1;
        int cnt = 0;
        Exiv2::PreviewManager pm(*image);
        Exiv2::PreviewPropertiesList list = pm.getPreviewProperties();
        for (auto pos = list.begin(); pos != list.end(); ++pos) {
            if (manyFiles) {
                std::cout << std::setfill(' ') << std::left << std::setw(20) << path_ << "  ";
            }
            std::cout << _("Preview") << " " << ++cnt << ": " << pos->mimeType_ << ", ";
            if (pos->width_ != 0 && pos->height_ != 0) {
                std::cout << pos->width_ << "x" << pos->height_ << " " << _("pixels") << ", ";
            }
            std::cout << pos->size_ << " " << _("bytes") << "\n";
        }
        return 0;
    }

    std::unique_ptr<Print> Print::clone() const
    {
        return std::unique_ptr<Print>(clone_());
    }

    Print* Print::clone_() const
    {
        return new Print(*this);
    }

    int Rename::run(const std::string& path)
    {
        try {
            if (!Exiv2::fileExists(path, true)) {
                std::cerr << path << ": " << _("Failed to open the file\n");
                return -1;
            }
            Timestamp ts;
            if (Params::instance().preserve_)
                ts.read(path);

            Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path);
            image->readMetadata();
            Exiv2::ExifData& exifData = image->exifData();
            if (exifData.empty()) {
                std::cerr << path << ": " << _("No Exif data found in the file\n");
                return -3;
            }

            auto md = exifData.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal"));
            if (md == exifData.end()) {
                md = exifData.findKey(Exiv2::ExifKey("Exif.Image.DateTime"));
            }
            if (md == exifData.end()) {
                std::cerr << _("Neither tag") << " `Exif.Photo.DateTimeOriginal' " << _("nor")
                          << " `Exif.Image.DateTime' " << _("found in the file") << " " << path << "\n";
                return 1;
            }

            std::string dateTime = md->toString();
            if (dateTime.length() == 0 || dateTime[0] == ' ') {
                std::cerr << _("Image file creation timestamp not set in the file") << " " << path << "\n";
                return 1;
            }

            struct tm tm;
            if (str2Tm(dateTime, &tm) != 0) {
                std::cerr << _("Failed to parse timestamp") << " `" << dateTime << "' " << _("in the file") << " "
                          << path << "\n";
                return 1;
            }

            if (Params::instance().timestamp_ || Params::instance().timestampOnly_) {
                ts.read(&tm);
            }
            int rc = 0;
            std::string newPath = path;
            if (Params::instance().timestampOnly_) {
                if (Params::instance().verbose_) {
                    std::cout << _("Updating timestamp to") << " " << dateTime << std::endl;
                }
            } else {
                rc = renameFile(newPath, &tm);
                if (rc == -1)
                    return 0;  // skip
            }
            if (0 == rc &&
                (Params::instance().preserve_ || Params::instance().timestamp_ || Params::instance().timestampOnly_)) {
                ts.touch(newPath);
            }
            return rc;
        } catch (const Exiv2::AnyError& e) {
            std::cerr << "Exiv2 exception in rename action for file " << path << ":\n" << e << "\n";
            return 1;
        }
    }

    std::unique_ptr<Rename> Rename::clone() const
    {
        return std::unique_ptr<Rename>(clone_());
    }

    Rename* Rename::clone_() const
    {
        return new Rename(*this);
    }

    int Erase::run(const std::string& path) try {
        path_ = path;

        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_ << ": " << _("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_)
            ts.read(path);

        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path_);
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
        if (0 == rc && Params::instance().target_ & Params::ctIccProfile) {
            rc = eraseIccProfile(image.get());
        }
        if (0 == rc && Params::instance().target_ & Params::ctIptcRaw) {
            rc = printStructure(std::cout, Exiv2::kpsIptcErase, path_);
        }

        if (0 == rc) {
            image->writeMetadata();
            if (Params::instance().preserve_)
                ts.touch(path);
        }

        return rc;
    } catch (const Exiv2::AnyError& e) {
        std::cerr << "Exiv2 exception in erase action for file " << path << ":\n" << e << "\n";
        return 1;
    }

    int Erase::eraseThumbnail(Exiv2::Image* image) const
    {
        Exiv2::ExifThumb exifThumb(image->exifData());
        std::string thumbExt = exifThumb.extension();
        if (thumbExt.empty()) {
            return 0;
        }
        exifThumb.erase();
        if (Params::instance().verbose_) {
            std::cout << _("Erasing thumbnail data") << std::endl;
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
        image->clearXmpData();  // Quick fix for bug #612
        image->clearXmpPacket();
        return 0;
    }
    int Erase::eraseIccProfile(Exiv2::Image* image) const
    {
        if (Params::instance().verbose_ && image->iccProfileDefined()) {
            std::cout << _("Erasing ICC Profile data from the file") << std::endl;
        }
        image->clearIccProfile();
        return 0;
    }

    std::unique_ptr<Erase> Erase::clone() const
    {
        return std::unique_ptr<Erase>(clone_());
    }

    Erase* Erase::clone_() const
    {
        return new Erase(*this);
    }

    int Extract::run(const std::string& path)
    {
        try {
            path_ = path;
            int rc = 0;

            bool bStdout = Params::instance().target_ & Params::ctStdInOut ? true : false;
            if (bStdout) {
                _setmode(_fileno(stdout), _O_BINARY);
            }

            if (Params::instance().target_ & Params::ctThumb) {
                rc = writeThumbnail();
            }
            if (!rc && Params::instance().target_ & Params::ctPreview) {
                rc = writePreviews();
            }
            if (!rc && Params::instance().target_ & Params::ctXmpSidecar) {
                std::string xmpPath = bStdout ? "-" : newFilePath(path_, ".xmp");
                if (dontOverwrite(xmpPath))
                    return 0;
                rc = metacopy(path_, xmpPath, Exiv2::ImageType::xmp, false);
            }
            if (!rc && Params::instance().target_ & Params::ctIccProfile) {
                std::string iccPath = bStdout ? "-" : newFilePath(path_, ".icc");
                rc = writeIccProfile(iccPath);
            }
            if (!rc && !(Params::instance().target_ & Params::ctXmpSidecar) &&
                !(Params::instance().target_ & Params::ctThumb) && !(Params::instance().target_ & Params::ctPreview) &&
                !(Params::instance().target_ & Params::ctIccProfile)) {
                std::string exvPath = bStdout ? "-" : newFilePath(path_, ".exv");
                if (dontOverwrite(exvPath))
                    return 0;
                rc = metacopy(path_, exvPath, Exiv2::ImageType::exv, false);
            }
            return rc;
        } catch (const Exiv2::AnyError& e) {
            std::cerr << "Exiv2 exception in extract action for file " << path << ":\n" << e << "\n";
            return 1;
        }
    }

    int Extract::writeThumbnail() const
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_ << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path_);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path_ << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        int rc = 0;
        Exiv2::ExifThumb exifThumb(exifData);
        std::string thumbExt = exifThumb.extension();
        if (thumbExt.empty()) {
            std::cerr << path_ << ": " << _("Image does not contain an Exif thumbnail\n");
        } else {
            std::string thumb = newFilePath(path_, "-thumb");
            std::string thumbPath = thumb + thumbExt;
            if (dontOverwrite(thumbPath))
                return 0;
            if (Params::instance().verbose_) {
                Exiv2::DataBuf buf = exifThumb.copy();
                if (buf.size_ != 0) {
                    std::cout << _("Writing thumbnail") << " (" << exifThumb.mimeType() << ", " << buf.size_ << " "
                              << _("Bytes") << ") " << _("to file") << " " << thumbPath << std::endl;
                }
            }
            rc = static_cast<int>(exifThumb.writeFile(thumb));
            if (rc == 0) {
                std::cerr << path_ << ": " << _("Exif data doesn't contain a thumbnail\n");
            }
        }
        return rc;
    }

    int Extract::writePreviews() const
    {
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_ << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path_);
        image->readMetadata();

        Exiv2::PreviewManager pvMgr(*image);
        Exiv2::PreviewPropertiesList pvList = pvMgr.getPreviewProperties();

        const Params::PreviewNumbers& numbers = Params::instance().previewNumbers_;
        for (auto n = numbers.cbegin(); n != numbers.cend(); ++n) {
            if (*n == 0) {
                // Write all previews
                for (int num = 0; num < static_cast<int>(pvList.size()); ++num) {
                    writePreviewFile(pvMgr.getPreviewImage(pvList[num]), num + 1);
                }
                break;
            }
            if (*n > static_cast<int>(pvList.size())) {
                std::cerr << path_ << ": " << _("Image does not have preview") << " " << *n << "\n";
                continue;
            }
            writePreviewFile(pvMgr.getPreviewImage(pvList[*n - 1]), *n);
        }
        return 0;
    }

    int Extract::writeIccProfile(const std::string& target) const
    {
        int rc = 0;
        if (!Exiv2::fileExists(path_, true)) {
            std::cerr << path_ << ": " << _("Failed to open the file\n");
            rc = -1;
        }

        bool bStdout = target == "-";

        if (rc == 0) {
            Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path_);
            image->readMetadata();
            if (!image->iccProfileDefined()) {
                std::cerr << _("No embedded iccProfile: ") << path_ << std::endl;
                rc = -2;
            } else {
                if (bStdout) {  // -eC-
                    std::cout.write((const char*)image->iccProfile()->pData_, image->iccProfile()->size_);
                } else {
                    if (Params::instance().verbose_) {
                        std::cout << _("Writing iccProfile: ") << target << std::endl;
                    }
                    Exiv2::FileIo iccFile(target);
                    iccFile.open("wb");
                    iccFile.write(image->iccProfile()->pData_, image->iccProfile()->size_);
                    iccFile.close();
                }
            }
        }
        return rc;
    }

    void Extract::writePreviewFile(const Exiv2::PreviewImage& pvImg, int num) const
    {
        std::string pvFile = newFilePath(path_, "-preview") + Exiv2::toString(num);
        std::string pvPath = pvFile + pvImg.extension();
        if (dontOverwrite(pvPath))
            return;
        if (Params::instance().verbose_) {
            std::cout << _("Writing preview") << " " << num << " (" << pvImg.mimeType() << ", ";
            if (pvImg.width() != 0 && pvImg.height() != 0) {
                std::cout << pvImg.width() << "x" << pvImg.height() << " " << _("pixels") << ", ";
            }
            std::cout << pvImg.size() << " " << _("bytes") << ") " << _("to file") << " " << pvPath << std::endl;
        }
        const size_t rc = pvImg.writeFile(pvFile);
        if (rc == 0) {
            std::cerr << path_ << ": " << _("Image does not have preview") << " " << num << "\n";
        }
    }

    std::unique_ptr<Extract> Extract::clone() const
    {
        return std::unique_ptr<Extract>(clone_());
    }

    Extract* Extract::clone_() const
    {
        return new Extract(*this);
    }

    int Insert::run(const std::string& path) try {
        // -i{tgt}-  reading from stdin?
        bool bStdin = (Params::instance().target_ & Params::ctStdInOut) ? true : false;

        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path << ": " << _("Failed to open the file\n");
            return -1;
        }

        int rc = 0;
        Timestamp ts;
        if (Params::instance().preserve_)
            ts.read(path);

        if (Params::instance().target_ & Params::ctThumb) {
            rc = insertThumbnail(path);
        }

        if (rc == 0 && !(Params::instance().target_ & Params::ctXmpRaw) &&
            (Params::instance().target_ & Params::ctExif || Params::instance().target_ & Params::ctIptc ||
             Params::instance().target_ & Params::ctComment || Params::instance().target_ & Params::ctXmp)) {
            std::string suffix = Params::instance().suffix_;
            if (suffix.empty())
                suffix = ".exv";
            if (Params::instance().target_ & Params::ctXmpSidecar)
                suffix = ".xmp";
            std::string exvPath = bStdin ? "-" : newFilePath(path, suffix);
            rc = metacopy(exvPath, path, Exiv2::ImageType::none, true);
        }

        if (0 == rc && (Params::instance().target_ & (Params::ctXmpSidecar | Params::ctXmpRaw))) {
            std::string xmpPath = bStdin ? "-" : newFilePath(path, ".xmp");
            rc = insertXmpPacket(path, xmpPath);
        }

        if (0 == rc && Params::instance().target_ & Params::ctIccProfile) {
            std::string iccPath = bStdin ? "-" : newFilePath(path, ".icc");
            rc = insertIccProfile(path, iccPath);
        }

        if (Params::instance().preserve_)
            ts.touch(path);
        return rc;
    } catch (const Exiv2::AnyError& e) {
        std::cerr << "Exiv2 exception in insert action for file " << path << ":\n" << e << "\n";
        return 1;
    }

    int Insert::insertXmpPacket(const std::string& path, const std::string& xmpPath) const
    {
        int rc = 0;
        bool bStdin = xmpPath == "-";
        if (bStdin) {
            Exiv2::DataBuf xmpBlob;
            Params::instance().getStdin(xmpBlob);
            rc = insertXmpPacket(path, xmpBlob, true);
        } else {
            if (!Exiv2::fileExists(xmpPath, true)) {
                std::cerr << xmpPath << ": " << _("Failed to open the file\n");
                rc = -1;
            }
            if (rc == 0 && !Exiv2::fileExists(path, true)) {
                std::cerr << path << ": " << _("Failed to open the file\n");
                rc = -1;
            }
            if (rc == 0) {
                Exiv2::DataBuf xmpBlob = Exiv2::readFile(xmpPath);
                rc = insertXmpPacket(path, xmpBlob);
            }
        }
        return rc;

    }

    int Insert::insertXmpPacket(const std::string& path, const Exiv2::DataBuf& xmpBlob, bool usePacket) const
    {
        std::string xmpPacket;
        for (size_t i = 0; i < xmpBlob.size_; i++) {
            xmpPacket += static_cast<char>(xmpBlob.pData_[i]);
        }
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path);
        image->readMetadata();
        image->clearXmpData();
        image->setXmpPacket(xmpPacket);
        image->writeXmpFromPacket(usePacket);
        image->writeMetadata();

        return 0;
    }

    int Insert::insertIccProfile(const std::string& path, const std::string& iccPath) const
    {
        int rc = 0;
        // for path "foo.XXX", do a binary copy of "foo.icc"
        std::string iccProfilePath = newFilePath(path, ".icc");
        if (iccPath == "-") {
            Exiv2::DataBuf iccProfile;
            Params::instance().getStdin(iccProfile);
            rc = insertIccProfile(path, iccProfile);
        } else {
            if (!Exiv2::fileExists(iccProfilePath, true)) {
                std::cerr << iccProfilePath << ": " << _("Failed to open the file\n");
                rc = -1;
            } else {
                Exiv2::DataBuf iccProfile = Exiv2::readFile(iccPath);
                rc = insertIccProfile(path, iccProfile);
            }
        }
        return rc;
    }

    int Insert::insertIccProfile(const std::string& path, Exiv2::DataBuf& iccProfileBlob) const
    {
        int rc = 0;
        // test path exists
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path << ": " << _("Failed to open the file\n");
            rc = -1;
        }

        // read in the metadata
        if (rc == 0) {
            Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path);
            image->readMetadata();
            // clear existing profile, assign the blob and rewrite image
            image->clearIccProfile();
            if (iccProfileBlob.size_) {
                image->setIccProfile(iccProfileBlob);
            }
            image->writeMetadata();
        }

        return rc;
    }

    int Insert::insertThumbnail(const std::string& path) const
    {
        std::string thumbPath = newFilePath(path, "-thumb.jpg");
        if (!Exiv2::fileExists(thumbPath, true)) {
            std::cerr << thumbPath << ": " << _("Failed to open the file\n");
            return -1;
        }
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path);
        image->readMetadata();
        Exiv2::ExifThumb exifThumb(image->exifData());
        exifThumb.setJpegThumbnail(thumbPath);
        image->writeMetadata();

        return 0;
    }

    std::unique_ptr<Insert> Insert::clone() const
    {
        return std::unique_ptr<Insert>(clone_());
    }

    Insert* Insert::clone_() const
    {
        return new Insert(*this);
    }

    int Modify::run(const std::string& path)
    {
        try {
            if (!Exiv2::fileExists(path, true)) {
                std::cerr << path << ": " << _("Failed to open the file\n");
                return -1;
            }
            Timestamp ts;
            if (Params::instance().preserve_)
                ts.read(path);

            Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path);
            image->readMetadata();

            int rc = applyCommands(image.get());

            // Save both exif and iptc metadata
            image->writeMetadata();

            if (Params::instance().preserve_)
                ts.touch(path);

            return rc;
        } catch (const Exiv2::AnyError& e) {
            std::cerr << "Exiv2 exception in modify action for file " << path << ":\n" << e << "\n";
            return 1;
        }
    }

    int Modify::applyCommands(Exiv2::Image* pImage)
    {
        if (!Params::instance().jpegComment_.empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Setting JPEG comment") << " '" << Params::instance().jpegComment_ << "'" << std::endl;
            }
            pImage->setComment(Params::instance().jpegComment_);
        }

        // loop through command table and apply each command
        ModifyCmds& modifyCmds = Params::instance().modifyCmds_;
        int rc = 0;
        int ret = 0;
        for (auto i = modifyCmds.cbegin(); i != modifyCmds.cend(); ++i) {
            switch (i->cmdId_) {
                case add:
                    ret = addMetadatum(pImage, *i);
                    if (rc == 0)
                        rc = ret;
                    break;
                case set:
                    ret = setMetadatum(pImage, *i);
                    if (rc == 0)
                        rc = ret;
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
        return rc;
    }

    int Modify::addMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Add") << " " << modifyCmd.key_ << " \"" << modifyCmd.value_ << "\" ("
                      << Exiv2::TypeInfo::typeName(modifyCmd.typeId_) << ")" << std::endl;
        }
        Exiv2::ExifData& exifData = pImage->exifData();
        Exiv2::IptcData& iptcData = pImage->iptcData();
        Exiv2::XmpData& xmpData = pImage->xmpData();
        Exiv2::Value::UniquePtr value = Exiv2::Value::create(modifyCmd.typeId_);
        int rc = value->read(modifyCmd.value_);
        if (0 == rc) {
            if (modifyCmd.metadataId_ == exif) {
                exifData.add(Exiv2::ExifKey(modifyCmd.key_), value.get());
            }
            if (modifyCmd.metadataId_ == iptc) {
                iptcData.add(Exiv2::IptcKey(modifyCmd.key_), value.get());
            }
            if (modifyCmd.metadataId_ == xmp) {
                xmpData.add(Exiv2::XmpKey(modifyCmd.key_), value.get());
            }
        } else {
            std::cerr << _("Warning") << ": " << modifyCmd.key_ << ": " << _("Failed to read") << " "
                      << Exiv2::TypeInfo::typeName(value->typeId()) << " " << _("value") << " \"" << modifyCmd.value_
                      << "\"\n";
        }
        return rc;
    }

    // This function looks rather complex because we try to avoid adding an
    // empty metadatum if reading the value fails
    int Modify::setMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Set") << " " << modifyCmd.key_ << " \"" << modifyCmd.value_ << "\" ("
                      << Exiv2::TypeInfo::typeName(modifyCmd.typeId_) << ")" << std::endl;
        }
        Exiv2::ExifData& exifData = pImage->exifData();
        Exiv2::IptcData& iptcData = pImage->iptcData();
        Exiv2::XmpData& xmpData = pImage->xmpData();
        Exiv2::Metadatum* metadatum = nullptr;
        if (modifyCmd.metadataId_ == exif) {
            auto pos = exifData.findKey(Exiv2::ExifKey(modifyCmd.key_));
            if (pos != exifData.end()) {
                metadatum = &(*pos);
            }
        }
        if (modifyCmd.metadataId_ == iptc) {
            auto pos = iptcData.findKey(Exiv2::IptcKey(modifyCmd.key_));
            if (pos != iptcData.end()) {
                metadatum = &(*pos);
            }
        }
        if (modifyCmd.metadataId_ == xmp) {
            auto pos = xmpData.findKey(Exiv2::XmpKey(modifyCmd.key_));
            if (pos != xmpData.end()) {
                metadatum = &(*pos);
            }
        }
        // If a type was explicitly requested, use it; else
        // use the current type of the metadatum, if any;
        // or the default type
        Exiv2::Value::UniquePtr value;
        if (metadatum) {
            value = metadatum->getValue();
        }
        if (value.get() == nullptr || (modifyCmd.explicitType_ && modifyCmd.typeId_ != value->typeId())) {
            value = Exiv2::Value::create(modifyCmd.typeId_);
        }
        int rc = value->read(modifyCmd.value_);
        if (0 == rc) {
            if (metadatum) {
                metadatum->setValue(value.get());
            } else {
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
        } else {
            std::cerr << _("Warning") << ": " << modifyCmd.key_ << ": " << _("Failed to read") << " "
                      << Exiv2::TypeInfo::typeName(value->typeId()) << " " << _("value") << " \"" << modifyCmd.value_
                      << "\"\n";
        }
        return rc;
    }

    void Modify::delMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Del") << " " << modifyCmd.key_ << std::endl;
        }

        Exiv2::ExifData& exifData = pImage->exifData();
        Exiv2::IptcData& iptcData = pImage->iptcData();
        Exiv2::XmpData& xmpData = pImage->xmpData();
        if (modifyCmd.metadataId_ == exif) {
            Exiv2::ExifData::iterator pos;
            Exiv2::ExifKey exifKey = Exiv2::ExifKey(modifyCmd.key_);
            while ((pos = exifData.findKey(exifKey)) != exifData.end()) {
                exifData.erase(pos);
            }
        }
        if (modifyCmd.metadataId_ == iptc) {
            Exiv2::IptcData::iterator pos;
            Exiv2::IptcKey iptcKey = Exiv2::IptcKey(modifyCmd.key_);
            while ((pos = iptcData.findKey(iptcKey)) != iptcData.end()) {
                iptcData.erase(pos);
            }
        }
        if (modifyCmd.metadataId_ == xmp) {
            Exiv2::XmpData::iterator pos;
            Exiv2::XmpKey xmpKey = Exiv2::XmpKey(modifyCmd.key_);
            if ((pos = xmpData.findKey(xmpKey)) != xmpData.end()) {
                xmpData.eraseFamily(pos);
            }
        }
    }

    void Modify::regNamespace(const ModifyCmd& modifyCmd)
    {
        if (Params::instance().verbose_) {
            std::cout << _("Reg ") << modifyCmd.key_ << "=\"" << modifyCmd.value_ << "\"" << std::endl;
        }
        Exiv2::XmpProperties::registerNs(modifyCmd.value_, modifyCmd.key_);
    }

    std::unique_ptr<Modify> Modify::clone() const
    {
        return std::unique_ptr<Modify>(clone_());
    }

    Modify* Modify::clone_() const
    {
        return new Modify(*this);
    }

    int Adjust::run(const std::string& path) try {
        adjustment_ = Params::instance().adjustment_;
        yearAdjustment_ = Params::instance().yodAdjust_[Params::yodYear].adjustment_;
        monthAdjustment_ = Params::instance().yodAdjust_[Params::yodMonth].adjustment_;
        dayAdjustment_ = Params::instance().yodAdjust_[Params::yodDay].adjustment_;

        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path << ": " << _("Failed to open the file\n");
            return -1;
        }
        Timestamp ts;
        if (Params::instance().preserve_)
            ts.read(path);

        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path);
        image->readMetadata();
        Exiv2::ExifData& exifData = image->exifData();
        if (exifData.empty()) {
            std::cerr << path << ": " << _("No Exif data found in the file\n");
            return -3;
        }
        int rc = adjustDateTime(exifData, "Exif.Image.DateTime", path);
        rc += adjustDateTime(exifData, "Exif.Photo.DateTimeOriginal", path);
        rc += adjustDateTime(exifData, "Exif.Image.DateTimeOriginal", path);
        rc += adjustDateTime(exifData, "Exif.Photo.DateTimeDigitized", path);

        if (rc == 0) {
            image->writeMetadata();
            if (Params::instance().preserve_)
                ts.touch(path);
        }
        return rc ? 1 : 0;
    } catch (const Exiv2::AnyError& e) {
        std::cerr << "Exiv2 exception in adjust action for file " << path << ":\n" << e << "\n";
        return 1;
    }

    std::unique_ptr<Adjust> Adjust::clone() const
    {
        return std::unique_ptr<Adjust>(clone_());
    }

    Adjust* Adjust::clone_() const
    {
        return new Adjust(*this);
    }

    int Adjust::adjustDateTime(Exiv2::ExifData& exifData, const std::string& key, const std::string& path) const
    {
        Exiv2::ExifKey ek(key);
        auto md = exifData.findKey(ek);
        if (md == exifData.end()) {
            // Key not found. That's ok, we do nothing.
            return 0;
        }
        std::string timeStr = md->toString();
        if (timeStr == "" || timeStr[0] == ' ') {
            std::cerr << path << ": " << _("Timestamp of metadatum with key") << " `" << ek << "' " << _("not set\n");
            return 1;
        }
        if (Params::instance().verbose_) {
            bool comma = false;
            std::cout << _("Adjusting") << " `" << ek << "' " << _("by");
            if (yearAdjustment_ != 0) {
                std::cout << (yearAdjustment_ < 0 ? " " : " +") << yearAdjustment_ << " ";
                if (yearAdjustment_ < -1 || yearAdjustment_ > 1) {
                    std::cout << _("years");
                } else {
                    std::cout << _("year");
                }
                comma = true;
            }
            if (monthAdjustment_ != 0) {
                if (comma)
                    std::cout << ",";
                std::cout << (monthAdjustment_ < 0 ? " " : " +") << monthAdjustment_ << " ";
                if (monthAdjustment_ < -1 || monthAdjustment_ > 1) {
                    std::cout << _("months");
                } else {
                    std::cout << _("month");
                }
                comma = true;
            }
            if (dayAdjustment_ != 0) {
                if (comma)
                    std::cout << ",";
                std::cout << (dayAdjustment_ < 0 ? " " : " +") << dayAdjustment_ << " ";
                if (dayAdjustment_ < -1 || dayAdjustment_ > 1) {
                    std::cout << _("days");
                } else {
                    std::cout << _("day");
                }
                comma = true;
            }
            if (adjustment_ != 0) {
                if (comma)
                    std::cout << ",";
                std::cout << " " << adjustment_ << _("s");
            }
        }
        struct tm tm;
        if (str2Tm(timeStr, &tm) != 0) {
            if (Params::instance().verbose_)
                std::cout << std::endl;
            std::cerr << path << ": " << _("Failed to parse timestamp") << " `" << timeStr << "'\n";
            return 1;
        }
        const long monOverflow = (tm.tm_mon + monthAdjustment_) / 12;
        tm.tm_mon = (tm.tm_mon + monthAdjustment_) % 12;
        tm.tm_year += yearAdjustment_ + monOverflow;
        // Let's not create files with non-4-digit years, we can't read them.
        if (tm.tm_year > 9999 - 1900 || tm.tm_year < 1000 - 1900) {
            if (Params::instance().verbose_)
                std::cout << std::endl;
            std::cerr << path << ": " << _("Can't adjust timestamp by") << " " << yearAdjustment_ + monOverflow << " "
                      << _("years") << "\n";
            return 1;
        }
        time_t time = mktime(&tm);
        time += adjustment_ + dayAdjustment_ * 86400;
        timeStr = time2Str(time);
        if (Params::instance().verbose_) {
            std::cout << " " << _("to") << " " << timeStr << std::endl;
        }
        md->setValue(timeStr);
        return 0;
    }

    int FixIso::run(const std::string& path)
    {
        try {
            if (!Exiv2::fileExists(path, true)) {
                std::cerr << path << ": " << _("Failed to open the file\n");
                return -1;
            }
            Timestamp ts;
            if (Params::instance().preserve_)
                ts.read(path);

            Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path);
            image->readMetadata();
            Exiv2::ExifData& exifData = image->exifData();
            if (exifData.empty()) {
                std::cerr << path << ": " << _("No Exif data found in the file\n");
                return -3;
            }
            auto md = Exiv2::isoSpeed(exifData);
            if (md != exifData.end()) {
                if (strcmp(md->key().c_str(), "Exif.Photo.ISOSpeedRatings") == 0) {
                    if (Params::instance().verbose_) {
                        std::cout << _("Standard Exif ISO tag exists; not modified\n");
                    }
                    return 0;
                }
                // Copy the proprietary tag to the standard place
                std::ostringstream os;
                md->write(os, &exifData);
                if (Params::instance().verbose_) {
                    std::cout << _("Setting Exif ISO value to") << " " << os.str() << "\n";
                }
                exifData["Exif.Photo.ISOSpeedRatings"] = os.str();
            }
            image->writeMetadata();
            if (Params::instance().preserve_)
                ts.touch(path);

            return 0;
        } catch (const Exiv2::AnyError& e) {
            std::cerr << "Exiv2 exception in fixiso action for file " << path << ":\n" << e << "\n";
            return 1;
        }
    }

    std::unique_ptr<FixIso> FixIso::clone() const
    {
        return std::unique_ptr<FixIso>(clone_());
    }

    FixIso* FixIso::clone_() const
    {
        return new FixIso(*this);
    }

    int FixCom::run(const std::string& path)
    {
        try {
            if (!Exiv2::fileExists(path, true)) {
                std::cerr << path << ": " << _("Failed to open the file\n");
                return -1;
            }
            Timestamp ts;
            if (Params::instance().preserve_)
                ts.read(path);

            Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path);
            image->readMetadata();
            Exiv2::ExifData& exifData = image->exifData();
            if (exifData.empty()) {
                std::cerr << path << ": " << _("No Exif data found in the file\n");
                return -3;
            }
            auto pos = exifData.findKey(Exiv2::ExifKey("Exif.Photo.UserComment"));
            if (pos == exifData.end()) {
                if (Params::instance().verbose_) {
                    std::cout << _("No Exif user comment found") << "\n";
                }
                return 0;
            }
            Exiv2::Value::UniquePtr v = pos->getValue();
            const Exiv2::CommentValue* pcv = dynamic_cast<const Exiv2::CommentValue*>(v.get());
            if (!pcv) {
                if (Params::instance().verbose_) {
                    std::cout << _("Found Exif user comment with unexpected value type") << "\n";
                }
                return 0;
            }
            Exiv2::CommentValue::CharsetId csId = pcv->charsetId();
            if (csId != Exiv2::CommentValue::unicode) {
                if (Params::instance().verbose_) {
                    std::cout << _("No Exif UNICODE user comment found") << "\n";
                }
                return 0;
            }
            std::string comment = pcv->comment(Params::instance().charset_.c_str());
            if (Params::instance().verbose_) {
                std::cout << _("Setting Exif UNICODE user comment to") << " \"" << comment << "\"\n";
            }
            comment = std::string("charset=\"") + Exiv2::CommentValue::CharsetInfo::name(csId) + "\" " + comment;
            // Remove BOM and convert value from source charset to UCS-2, but keep byte order
            pos->setValue(comment);
            image->writeMetadata();
            if (Params::instance().preserve_)
                ts.touch(path);

            return 0;
        } catch (const Exiv2::AnyError& e) {
            std::cerr << "Exiv2 exception in fixcom action for file " << path << ":\n" << e << "\n";
            return 1;
        }
    }

    std::unique_ptr<FixCom> FixCom::clone() const
    {
        return std::unique_ptr<FixCom>(clone_());
    }

    FixCom* FixCom::clone_() const
    {
        return new FixCom(*this);
    }

}  // namespace Action

// *****************************************************************************
// local definitions
namespace
{
    //! @cond IGNORE
    int Timestamp::read(const std::string& path)
    {
        struct stat buf;
        int rc = stat(path.c_str(), &buf);
        if (0 == rc) {
            actime_ = buf.st_atime;
            modtime_ = buf.st_mtime;
        }
        return rc;
    }

    int Timestamp::read(struct tm* tm)
    {
        int rc = 1;
        time_t t = mktime(tm);  // interpret tm according to current timezone settings
        if (t != (time_t)-1) {
            rc = 0;
            actime_ = t;
            modtime_ = t;
        }
        return rc;
    }

    int Timestamp::touch(const std::string& path)
    {
        if (0 == actime_)
            return 1;
        struct utimbuf buf;
        buf.actime = actime_;
        buf.modtime = modtime_;
        return utime(path.c_str(), &buf);
    }
    //! @endcond

    int str2Tm(const std::string& timeStr, struct tm* tm)
    {
        if (timeStr.length() == 0 || timeStr[0] == ' ')
            return 1;
        if (timeStr.length() < 19)
            return 2;
        if (timeStr[4] != ':' || timeStr[7] != ':' || timeStr[10] != ' ' || timeStr[13] != ':' || timeStr[16] != ':')
            return 3;
        if (nullptr == tm)
            return 4;
        std::memset(tm, 0x0, sizeof(struct tm));
        tm->tm_isdst = -1;

        long tmp;
        if (!Util::strtol(timeStr.substr(0, 4).c_str(), tmp))
            return 5;
        tm->tm_year = tmp - 1900;
        if (!Util::strtol(timeStr.substr(5, 2).c_str(), tmp))
            return 6;
        tm->tm_mon = tmp - 1;
        if (!Util::strtol(timeStr.substr(8, 2).c_str(), tmp))
            return 7;
        tm->tm_mday = tmp;
        if (!Util::strtol(timeStr.substr(11, 2).c_str(), tmp))
            return 8;
        tm->tm_hour = tmp;
        if (!Util::strtol(timeStr.substr(14, 2).c_str(), tmp))
            return 9;
        tm->tm_min = tmp;
        if (!Util::strtol(timeStr.substr(17, 2).c_str(), tmp))
            return 10;
        tm->tm_sec = tmp;

        // Conversions to set remaining fields of the tm structure
        if (mktime(tm) == (time_t)-1)
            return 11;

        return 0;
    }

    std::string time2Str(time_t time)
    {
        struct tm* tm = localtime(&time);
        return tm2Str(tm);
    }

    std::string tm2Str(const struct tm* tm)
    {
        if (nullptr == tm)
            return "";

        std::ostringstream os;
        os << std::setfill('0') << tm->tm_year + 1900 << ":" << std::setw(2) << tm->tm_mon + 1 << ":" << std::setw(2)
           << tm->tm_mday << " " << std::setw(2) << tm->tm_hour << ":" << std::setw(2) << tm->tm_min << ":"
           << std::setw(2) << tm->tm_sec;

        return os.str();
    }

// use static CS/MUTEX to make temporaryPath() thread safe
#if defined(_MSC_VER) || defined(__MINGW__)
    static CRITICAL_SECTION cs;
#else
/* Unix/Linux/Cygwin/macOS */
#include <pthread.h>
/* This is the critical section object (statically allocated). */
#if defined(__APPLE__)
#if defined(PTHREAD_RECURSIVE_MUTEX_INITIALIZER)
    static pthread_mutex_t cs = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;
#else
    static pthread_mutex_t cs = PTHREAD_MUTEX_INITIALIZER;
#endif
#else
#if defined(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)
    static pthread_mutex_t cs = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#else
    static pthread_mutex_t cs = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif
#endif

    static std::string temporaryPath()
    {
        static int count = 0;

#if defined(_MSC_VER) || defined(__MINGW__)
        EnterCriticalSection(&cs);
        char lpTempPathBuffer[MAX_PATH];
        GetTempPath(MAX_PATH, lpTempPathBuffer);
        std::string tmp(lpTempPathBuffer);
        tmp += "\\";
        HANDLE process = nullptr;
        DWORD pid = ::GetProcessId(process);
#else
        pid_t pid = ::getpid();
        pthread_mutex_lock(&cs);
        std::string tmp = "/tmp/";
#endif
        char sCount[12];
        std::snprintf(sCount, 12, "_%d", ++count);

        std::string result = tmp + Exiv2::toString(pid) + sCount;
        if (Exiv2::fileExists(result))
            std::remove(result.c_str());

#if defined(_MSC_VER) || defined(__MINGW__)
        LeaveCriticalSection(&cs);
#else
        pthread_mutex_unlock(&cs);
#endif

        return result;
    }

    int metacopy(const std::string& source, const std::string& tgt, Exiv2::ImageType targetType, bool preserve)
    {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "actions.cpp::metacopy"
                  << " source = " << source << " target = " << tgt << std::endl;
#endif

        // read the source metadata
        int rc = -1;
        if (!Exiv2::fileExists(source, true)) {
            std::cerr << source << ": " << _("Failed to open the file\n");
            return rc;
        }

        bool bStdin = source == "-";
        bool bStdout = tgt == "-";

        Exiv2::DataBuf stdIn;
        if (bStdin)
            Params::instance().getStdin(stdIn);
        Exiv2::BasicIo::UniquePtr ioStdin = Exiv2::BasicIo::UniquePtr(new Exiv2::MemIo(stdIn.pData_, stdIn.size_));

        Exiv2::Image::UniquePtr sourceImage =
            bStdin ? Exiv2::ImageFactory::open(std::move(ioStdin)) : Exiv2::ImageFactory::open(source);
        sourceImage->readMetadata();

        // Apply any modification commands to the source image on-the-fly
        Action::Modify::applyCommands(sourceImage.get());

        // Open or create the target file
        std::string target(bStdout ? temporaryPath() : tgt);

        Exiv2::Image::UniquePtr targetImage;
        if (Exiv2::fileExists(target)) {
            targetImage = Exiv2::ImageFactory::open(target);
            targetImage->readMetadata();
        } else {
            targetImage = Exiv2::ImageFactory::create(targetType, target);
            assert(targetImage.get() != nullptr);
        }

        // Copy each type of metadata
        if (Params::instance().target_ & Params::ctExif && !sourceImage->exifData().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing Exif data from") << " " << source << " " << _("to") << " " << target
                          << std::endl;
            }
            if (preserve) {
                for (auto i = sourceImage->exifData().begin(); i != sourceImage->exifData().end(); ++i) {
                    targetImage->exifData()[i->key()] = i->value();
                }
            } else {
                targetImage->setExifData(sourceImage->exifData());
            }
        }
        if (Params::instance().target_ & Params::ctIptc && !sourceImage->iptcData().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing IPTC data from") << " " << source << " " << _("to") << " " << target
                          << std::endl;
            }
            if (preserve) {
                for (auto i = sourceImage->iptcData().begin(); i != sourceImage->iptcData().end(); ++i) {
                    targetImage->iptcData()[i->key()] = i->value();
                }
            } else {
                targetImage->setIptcData(sourceImage->iptcData());
            }
        }
        if (Params::instance().target_ & (Params::ctXmp | Params::ctXmpRaw) && !sourceImage->xmpData().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing XMP data from") << " " << source << " " << _("to") << " " << target
                          << std::endl;
            }

            // #1148 use Raw XMP packet if there are no XMP modification commands
            int tRawSidecar = Params::ctXmpSidecar | Params::ctXmpRaw;  // option -eXX
            // printTarget("in metacopy",Params::instance().target_,true);
            if (Params::instance().modifyCmds_.size() == 0 &&
                (Params::instance().target_ & tRawSidecar) == tRawSidecar) {
                // std::cout << "short cut" << std::endl;
                // http://www.cplusplus.com/doc/tutorial/files/
                std::ofstream os;
                os.open(target.c_str());
                sourceImage->printStructure(os, Exiv2::kpsXMP);
                os.close();
                rc = 0;
            } else if (preserve) {
                for (auto i = sourceImage->xmpData().begin(); i != sourceImage->xmpData().end(); ++i) {
                    targetImage->xmpData()[i->key()] = i->value();
                }
            } else {
                // std::cout << "long cut" << std::endl;
                targetImage->setXmpData(sourceImage->xmpData());
            }
        }
        if (Params::instance().target_ & Params::ctComment && !sourceImage->comment().empty()) {
            if (Params::instance().verbose_) {
                std::cout << _("Writing JPEG comment from") << " " << source << " " << _("to") << " " << tgt
                          << std::endl;
            }
            targetImage->setComment(sourceImage->comment());
        }
        if (rc < 0)
            try {
                targetImage->writeMetadata();
                rc = 0;
            } catch (const Exiv2::AnyError& e) {
                std::cerr << tgt << ": " << _("Could not write metadata to file") << ": " << e << "\n";
                rc = 1;
            }

        // if we used a temporary target, copy it to stdout
        if (rc == 0 && bStdout) {
            FILE* f = ::fopen(target.c_str(), "rb");
            _setmode(_fileno(stdout), O_BINARY);

            if (f) {
                char buffer[8 * 1024];
                size_t n = 1;
                while (!feof(f) && n > 0) {
                    n = fread(buffer, 1, sizeof buffer, f);
                    fwrite(buffer, 1, n, stdout);
                }
                fclose(f);
            }
        }

        // delete temporary target
        if (bStdout)
            std::remove(target.c_str());

        return rc;
    }

    int renameFile(std::string& newPath, const struct tm* tm)
    {
        std::string path = newPath;
        std::string format = Params::instance().format_;
        Util::replace(format, ":basename:", Util::basename(path, true));
        Util::replace(format, ":dirname:", Util::basename(Util::dirname(path)));
        Util::replace(format, ":parentname:", Util::basename(Util::dirname(Util::dirname(path))));

        const size_t max = 1024;
        char basename[max];
        std::memset(basename, 0x0, max);
        if (strftime(basename, max, format.c_str(), tm) == 0) {
            std::cerr << _("Filename format yields empty filename for the file") << " " << path << "\n";
            return 1;
        }
        newPath = Util::dirname(path) + EXV_SEPARATOR_STR + basename + Util::suffix(path);
        if (Util::dirname(newPath) == Util::dirname(path) && Util::basename(newPath) == Util::basename(path)) {
            if (Params::instance().verbose_) {
                std::cout << _("This file already has the correct name") << std::endl;
            }
            return -1;
        }

        bool go = true;
        int seq = 1;
        std::string s;
        Params::FileExistsPolicy fileExistsPolicy = Params::instance().fileExistsPolicy_;
        while (go) {
            if (Exiv2::fileExists(newPath)) {
                switch (fileExistsPolicy) {
                    case Params::overwritePolicy:
                        go = false;
                        break;
                    case Params::renamePolicy:
                        newPath = Util::dirname(path) + EXV_SEPARATOR_STR + basename + "_" + Exiv2::toString(seq++) +
                                  Util::suffix(path);
                        break;
                    case Params::askPolicy:
                        std::cout << Params::instance().progname() << ": " << _("File") << " `" << newPath << "' "
                                  << _("exists. [O]verwrite, [r]ename or [s]kip?") << " ";
                        std::cin >> s;
                        switch (s[0]) {
                            case 'o':
                            case 'O':
                                go = false;
                                break;
                            case 'r':
                            case 'R':
                                fileExistsPolicy = Params::renamePolicy;
                                newPath = Util::dirname(path) + EXV_SEPARATOR_STR + basename + "_" +
                                          Exiv2::toString(seq++) + Util::suffix(path);
                                break;
                            default:  // skip
                                return -1;
                        }
                }
            } else {
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
            std::cerr << Params::instance().progname() << ": " << _("Failed to rename") << " " << path << " " << _("to")
                      << " " << newPath << ": " << Exiv2::strError() << "\n";
            return 1;
        }

        return 0;
    }

    std::string newFilePath(const std::string& path, const std::string& ext)
    {
        std::string directory = Params::instance().directory_;
        if (directory.empty())
            directory = Util::dirname(path);
        directory = Exiv2::fileProtocol(path) == Exiv2::pFile ? directory + EXV_SEPARATOR_STR
                                                              : ""  // use current directory for remote files
            ;
        return directory + Util::basename(path, true) + ext;
    }

    int dontOverwrite(const std::string& path)
    {
        if (path == "-")
            return 0;

        if (!Params::instance().force_ && Exiv2::fileExists(path)) {
            std::cout << Params::instance().progname() << ": " << _("Overwrite") << " `" << path << "'? ";
            std::string s;
            std::cin >> s;
            if (s[0] != 'y' && s[0] != 'Y')
                return 1;
        }
        return 0;
    }

    std::ostream& operator<<(std::ostream& os, std::pair<std::string, int> strAndWidth)
    {
        const std::string& str(strAndWidth.first);
        size_t minChCount = static_cast<size_t>(strAndWidth.second);
        size_t count = mbstowcs(nullptr, str.c_str(), 0);  // returns 0xFFFFFFFF on error
        if (count < minChCount) {
            minChCount += str.size() - count;
        }
        return os << std::setw(static_cast<int>(minChCount)) << str;
    }

    int printStructure(std::ostream& out, Exiv2::PrintStructureOption option, const std::string& path)
    {
        if (!Exiv2::fileExists(path, true)) {
            std::cerr << path << ": " << _("Failed to open the file\n");
            return -1;
        }
        Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path);
        image->printStructure(out, option);
        return 0;
    }
}  // namespace
