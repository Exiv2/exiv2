// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "actions.hpp"

#include "app_utils.hpp"
#include "config.h"
#include "easyaccess.hpp"
#include "enforce.hpp"
#include "exif.hpp"
#include "futils.hpp"
#include "i18n.h"  // NLS support.
#include "image.hpp"
#include "iptc.hpp"
#include "preview.hpp"
#include "safe_op.hpp"
#include "types.hpp"
#include "xmp_exiv2.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>

// + standard includes
#include <sys/stat.h>   // for stat()
#include <sys/types.h>  // for stat()
#if __has_include(<unistd.h>)
#include <unistd.h>  // for stat()
#endif

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#include <sys/utime.h>
#include <windows.h>
#else
#include <utime.h>
#endif

#ifndef _WIN32
#define _setmode(a, b) \
  do {                 \
  } while (false)
#endif

#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

// *****************************************************************************
// local declarations
namespace {
std::mutex cs;

//! Helper class to set the timestamp of a file to that of another file
class Timestamp {
 public:
  //! C'tor
  int read(const std::string& path);
  //! Read the timestamp from a broken-down time in buffer \em tm.
  int read(tm* tm);
  //! Set the timestamp of a file
  int touch(const std::string& path) const;

 private:
  time_t actime_{0};
  time_t modtime_{0};
};

/*!
  @brief Convert a string "YYYY:MM:DD HH:MI:SS" to a tm type,
         returns 0 if successful
 */
int str2Tm(const std::string& timeStr, tm* tm);

//! Convert a localtime to a string "YYYY:MM:DD HH:MI:SS", "" on error
std::string time2Str(time_t time);

//! Convert a tm structure to a string "YYYY:MM:DD HH:MI:SS", "" on error
std::string tm2Str(const tm* tm);

/*!
  @brief Copy metadata from source to target according to Params::copyXyz

  @param source Source file path
  @param tgt Target file path. An *.exv file is created if target doesn't
                exist.
  @param targetType Image type for the target image in case it needs to be
                created.
  @param preserve Indicates if existing metadata in the target file should
                be kept.
  @return 0 if successful, else an error code
*/
int metacopy(const std::string& source, const std::string& tgt, Exiv2::ImageType targetType, bool preserve);

/*!
  @brief Rename a file according to a timestamp value.

  @param path The original file path. Contains the new path on exit.
  @param tm   Pointer to a buffer with the broken-down time to rename
              the file to.
  @return 0 if successful, -1 if the file was skipped, 1 on error.
*/
int renameFile(std::string& path, const tm* tm);

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
         os << pair( myString, width)
 */
std::ostream& operator<<(std::ostream& os, const std::pair<std::string, int>& strAndWidth);

//! Print image Structure information
int printStructure(std::ostream& out, Exiv2::PrintStructureOption option, const std::string& path);
}  // namespace

// *****************************************************************************
// class member definitions
namespace Action {
TaskFactory& TaskFactory::instance() {
  static TaskFactory instance_;
  return instance_;
}

void TaskFactory::cleanup() {
  registry_.clear();
}

TaskFactory::TaskFactory() {
  registry_.emplace(adjust, std::make_unique<Adjust>());
  registry_.emplace(print, std::make_unique<Print>());
  registry_.emplace(rename, std::make_unique<Rename>());
  registry_.emplace(erase, std::make_unique<Erase>());
  registry_.emplace(extract, std::make_unique<Extract>());
  registry_.emplace(insert, std::make_unique<Insert>());
  registry_.emplace(modify, std::make_unique<Modify>());
  registry_.emplace(fixiso, std::make_unique<FixIso>());
  registry_.emplace(fixcom, std::make_unique<FixCom>());
}

Task::UniquePtr TaskFactory::create(TaskType type) {
  auto i = registry_.find(type);
  if (i != registry_.end() && i->second) {
    return i->second->clone();
  }
  return nullptr;
}

static int setModeAndPrintStructure(Exiv2::PrintStructureOption option, const std::string& path, bool binary) {
  int result = 0;
  if (binary && option == Exiv2::kpsIccProfile) {
    std::stringstream output(std::stringstream::out | std::stringstream::binary);
    result = printStructure(output, option, path);
    std::string str = output.str();
    if (result == 0 && !str.empty()) {
      Exiv2::DataBuf iccProfile(str.size());
      Exiv2::DataBuf ascii((str.size() * 3) + 1);
      ascii.write_uint8(str.size() * 3, 0);
      std::copy(str.begin(), str.end(), iccProfile.begin());
      if (Exiv2::base64encode(iccProfile.c_data(), str.size(), reinterpret_cast<char*>(ascii.data()), str.size() * 3)) {
        const size_t chunk = 60;
        std::string code = std::string("data:") + ascii.c_str();
        size_t length = code.size();
        for (size_t start = 0; start < length; start += chunk) {
          size_t count = (start + chunk) < length ? chunk : length - start;
          std::cout << code.substr(start, count) << '\n';
        }
      }
    }
  } else {
    _setmode(fileno(stdout), O_BINARY);
    result = printStructure(std::cout, option, path);
  }

  return result;
}

int Print::run(const std::string& path) {
  try {
    path_ = path;
    switch (Params::instance().printMode_) {
      case Params::pmSummary:
        return Params::instance().greps_.empty() ? printSummary() : printList();
      case Params::pmList:
        return printList();
      case Params::pmComment:
        return printComment();
      case Params::pmPreview:
        return printPreviewList();
      case Params::pmStructure:
        return printStructure(std::cout, Exiv2::kpsBasic, path_);
      case Params::pmRecursive:
        return printStructure(std::cout, Exiv2::kpsRecursive, path_);
      case Params::pmXMP:
        return setModeAndPrintStructure(Exiv2::kpsXMP, path_, binary());
      case Params::pmIccProfile:
        return setModeAndPrintStructure(Exiv2::kpsIccProfile, path_, binary());
    }
    return 0;
  } catch (const Exiv2::Error& e) {
    std::cerr << "Exiv2 exception in print action for file " << path << ":\n" << e << "\n";
    return 1;
  } catch (const std::overflow_error& e) {
    std::cerr << "std::overflow_error exception in print action for file " << path << ":\n" << e.what() << "\n";
    return 1;
  }
}

int Print::printSummary() {
  if (!Exiv2::fileExists(path_)) {
    std::cerr << path_ << ": " << _("Failed to open the file") << "\n";
    return -1;
  }

  auto image = Exiv2::ImageFactory::open(path_);
  image->readMetadata();
  const Exiv2::ExifData& exifData = image->exifData();
  align_ = 16;

  // Filename
  printLabel(_("File name"));
  std::cout << path_ << '\n';

  // Filesize
  printLabel(_("File size"));
  std::cout << fs::file_size(path_) << " " << _("Bytes") << '\n';

  // MIME type
  printLabel(_("MIME type"));
  std::cout << image->mimeType() << '\n';

  // Image size
  printLabel(_("Image size"));
  std::cout << image->pixelWidth() << " x " << image->pixelHeight() << '\n';

  if (exifData.empty()) {
    std::cerr << path_ << ": " << _("No Exif data found in the file") << "\n";
    return -3;
  }

  // Thumbnail
  printLabel(_("Thumbnail"));
  Exiv2::ExifThumbC exifThumb(exifData);
  std::string thumbExt = exifThumb.extension();
  if (thumbExt.empty()) {
    std::cout << _("None");
  } else {
    auto dataBuf = exifThumb.copy();
    if (dataBuf.empty()) {
      std::cout << _("None");
    } else {
      std::cout << exifThumb.mimeType() << ", " << dataBuf.size() << " " << _("Bytes");
    }
  }
  std::cout << '\n';

  printTag(exifData, Exiv2::make, _("Camera make"));
  printTag(exifData, Exiv2::model, _("Camera model"));
  printTag(exifData, Exiv2::dateTimeOriginal, _("Image timestamp"));
  printTag(exifData, "Exif.Canon.FileNumber", _("File number"));
  printTag(exifData, Exiv2::exposureTime, _("Exposure time"), Exiv2::shutterSpeedValue);
  printTag(exifData, Exiv2::fNumber, _("Aperture"), Exiv2::apertureValue);
  printTag(exifData, Exiv2::exposureBiasValue, _("Exposure bias"));
  printTag(exifData, Exiv2::flash, _("Flash"));
  printTag(exifData, Exiv2::flashBias, _("Flash bias"));
  printTag(exifData, Exiv2::focalLength, _("Focal length"));
  printTag(exifData, Exiv2::subjectDistance, _("Subject distance"));
  printTag(exifData, Exiv2::isoSpeed, _("ISO speed"));
  printTag(exifData, Exiv2::exposureMode, _("Exposure mode"));
  printTag(exifData, Exiv2::meteringMode, _("Metering mode"));
  printTag(exifData, Exiv2::macroMode, _("Macro mode"));
  printTag(exifData, Exiv2::imageQuality, _("Image quality"));
  printTag(exifData, Exiv2::whiteBalance, _("White balance"));
  printTag(exifData, "Exif.Image.Copyright", _("Copyright"));
  printTag(exifData, "Exif.Photo.UserComment", _("Exif comment"));

  std::cout << '\n';

  return 0;
}  // Print::printSummary

void Print::printLabel(const std::string& label) const {
  std::cout << std::setfill(' ') << std::left;
  if (Params::instance().files_.size() > 1) {
    std::cout << std::setw(20) << path_ << " ";
  }
  std::cout << std::pair(label, align_) << ": ";
}

int Print::printTag(const Exiv2::ExifData& exifData, const std::string& key, const std::string& label) const {
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
    std::cout << '\n';
  return rc;
}  // Print::printTag

int Print::printTag(const Exiv2::ExifData& exifData, EasyAccessFct easyAccessFct, const std::string& label,
                    EasyAccessFct easyAccessFctFallback) const {
  int rc = 0;
  if (!label.empty()) {
    printLabel(label);
  }
  auto md = easyAccessFct(exifData);
  if (md != exifData.end()) {
    md->write(std::cout, &exifData);
    rc = 1;
  } else if (easyAccessFctFallback) {
    md = easyAccessFctFallback(exifData);
    if (md != exifData.end()) {
      md->write(std::cout, &exifData);
      rc = 1;
    }
  }
  if (!label.empty())
    std::cout << '\n';
  return rc;
}  // Print::printTag

int Print::printList() {
  if (!Exiv2::fileExists(path_)) {
    std::cerr << path_ << ": " << _("Failed to open the file") << "\n";
    return -1;
  }

  auto image = Exiv2::ImageFactory::open(path_);
  image->readMetadata();
  // Set defaults for metadata types and data columns
  if (Params::instance().printTags_ == MetadataId::invalid) {
    Params::instance().printTags_ = MetadataId::exif | MetadataId::iptc | MetadataId::xmp;
  }
  if (Params::instance().printItems_ == 0) {
    Params::instance().printItems_ = Params::prKey | Params::prType | Params::prCount | Params::prTrans;
  }
  return printMetadata(image.get());
}  // Print::printList

int Print::printMetadata(const Exiv2::Image* image) {
  bool ret = false;
  bool noExif = false;
  if ((Params::instance().printTags_ & MetadataId::exif) == MetadataId::exif) {
    const Exiv2::ExifData& exifData = image->exifData();
    for (auto&& md : exifData) {
      ret |= printMetadatum(md, image);
    }
    if (exifData.empty())
      noExif = true;
  }

  bool noIptc = false;
  if ((Params::instance().printTags_ & MetadataId::iptc) == MetadataId::iptc) {
    const Exiv2::IptcData& iptcData = image->iptcData();
    for (auto&& md : iptcData) {
      ret |= printMetadatum(md, image);
    }
    if (iptcData.empty())
      noIptc = true;
  }

  bool noXmp = false;
  if ((Params::instance().printTags_ & MetadataId::xmp) == MetadataId::xmp) {
    const Exiv2::XmpData& xmpData = image->xmpData();
    for (auto&& md : xmpData) {
      ret |= printMetadatum(md, image);
    }
    if (xmpData.empty())
      noXmp = true;
  }

  // With -v, inform about the absence of any (requested) type of metadata
  if (Params::instance().verbose_) {
    if (noExif)
      std::cerr << path_ << ": " << _("No Exif data found in the file") << "\n";
    if (noIptc)
      std::cerr << path_ << ": " << _("No IPTC data found in the file") << "\n";
    if (noXmp)
      std::cerr << path_ << ": " << _("No XMP data found in the file") << "\n";
  }

  // With -g or -K, return -3 if no matching tags were found
  int rc = 0;
  if ((!Params::instance().greps_.empty() || !Params::instance().keys_.empty()) && !ret)
    rc = 1;

  return rc;
}  // Print::printMetadata

bool Print::grepTag(const std::string& key) {
  bool result = Params::instance().greps_.empty();
  for (auto const& g : Params::instance().greps_) {
    result = std::regex_search(key, g);
    if (result) {
      break;
    }
  }
  return result;
}

bool Print::keyTag(const std::string& key) {
  bool result = Params::instance().keys_.empty();
  for (const auto& k : Params::instance().keys_) {
    if (result)
      break;
    result = key == k;
  }
  return result;
}

static void binaryOutput(const std::ostringstream& os) {
  std::cout << os.str();
}

bool Print::printMetadatum(const Exiv2::Metadatum& md, const Exiv2::Image* pImage) {
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
  if (Params::instance().printItems_ & Params::prDesc) {
    if (!first)
      std::cout << " ";
    first = false;
    std::cout << std::setw(30) << std::setfill(' ') << std::left << md.tagDesc();
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
    std::ostringstream os;
    // #1114 - show negative values for SByte
    if (md.typeId() == Exiv2::signedByte) {
      for (size_t c = 0; c < md.value().count(); c++) {
        const auto value = md.value().toInt64(c);
        os << (c ? " " : "") << std::dec << (value < 128 ? value : value - 256);
      }
    } else {
      os << std::dec << md.value();
    }
    binaryOutput(os);
  }
  if (Params::instance().printItems_ & Params::prTrans) {
    if (!first)
      std::cout << "  ";
    first = false;
    std::ostringstream os;
    os << std::dec << md.print(&pImage->exifData());
    binaryOutput(os);
  }
  if (Params::instance().printItems_ & Params::prHex) {
    if (!first)
      std::cout << '\n';
    if (md.size() > 0) {
      Exiv2::DataBuf buf(md.size());
      md.copy(buf.data(), pImage->byteOrder());
      Exiv2::hexdump(std::cout, buf.c_data(), buf.size());
    }
  }
  std::cout << '\n';
  return true;
}  // Print::printMetadatum

int Print::printComment() {
  if (!Exiv2::fileExists(path_)) {
    std::cerr << path_ << ": " << _("Failed to open the file") << "\n";
    return -1;
  }

  auto image = Exiv2::ImageFactory::open(path_);
  image->readMetadata();
  if (Params::instance().verbose_) {
    std::cout << _("JPEG comment") << ": ";
  }
  std::cout << image->comment() << '\n';
  return 0;
}  // Print::printComment

int Print::printPreviewList() {
  if (!Exiv2::fileExists(path_)) {
    std::cerr << path_ << ": " << _("Failed to open the file") << "\n";
    return -1;
  }

  auto image = Exiv2::ImageFactory::open(path_);
  image->readMetadata();
  bool const manyFiles = Params::instance().files_.size() > 1;
  int cnt = 0;
  Exiv2::PreviewManager pm(*image);
  Exiv2::PreviewPropertiesList list = pm.getPreviewProperties();
  for (const auto& pos : list) {
    if (manyFiles) {
      std::cout << std::setfill(' ') << std::left << std::setw(20) << path_ << "  ";
    }
    std::cout << _("Preview") << " " << ++cnt << ": " << pos.mimeType_ << ", ";
    if (pos.width_ != 0 && pos.height_ != 0) {
      std::cout << pos.width_ << "x" << pos.height_ << " " << _("pixels") << ", ";
    }
    std::cout << pos.size_ << " " << _("bytes") << "\n";
  }
  return 0;
}  // Print::printPreviewList

Task::UniquePtr Print::clone() const {
  return std::make_unique<Print>(*this);
}

int Rename::run(const std::string& path) {
  try {
    if (!Exiv2::fileExists(path)) {
      std::cerr << path << ": " << _("Failed to open the file") << "\n";
      return -1;
    }
    Timestamp ts;
    if (Params::instance().preserve_)
      ts.read(path);

    auto image = Exiv2::ImageFactory::open(path);
    image->readMetadata();
    Exiv2::ExifData& exifData = image->exifData();
    if (exifData.empty()) {
      std::cerr << path << ": " << _("No Exif data found in the file") << "\n";
      return -3;
    }
    auto md = exifData.findKey(Exiv2::ExifKey("Exif.Photo.DateTimeOriginal"));
    if (md == exifData.end())
      md = exifData.findKey(Exiv2::ExifKey("Exif.Image.DateTime"));
    if (md == exifData.end()) {
      std::cerr << _("Neither tag") << " `Exif.Photo.DateTimeOriginal' " << _("nor") << " `Exif.Image.DateTime' "
                << _("found in the file") << " " << path << "\n";
      return 1;
    }
    std::string v = md->toString();
    if (v.empty() || v.front() == ' ') {
      std::cerr << _("Image file creation timestamp not set in the file") << " " << path << "\n";
      return 1;
    }
    tm tm;
    if (str2Tm(v, &tm) != 0) {
      std::cerr << _("Failed to parse timestamp") << " `" << v << "' " << _("in the file") << " " << path << "\n";
      return 1;
    }
    if (Params::instance().timestamp_ || Params::instance().timestampOnly_) {
      ts.read(&tm);
    }
    int rc = 0;
    std::string newPath = path;
    if (Params::instance().timestampOnly_) {
      if (Params::instance().verbose_) {
        std::cout << _("Updating timestamp to") << " " << v << '\n';
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
  } catch (const Exiv2::Error& e) {
    std::cerr << "Exiv2 exception in rename action for file " << path << ":\n" << e << "\n";
    return 1;
  }
}

Task::UniquePtr Rename::clone() const {
  return std::make_unique<Rename>(*this);
}

int Erase::run(const std::string& path) {
  try {
    path_ = path;

    if (!Exiv2::fileExists(path_)) {
      std::cerr << path_ << ": " << _("Failed to open the file") << "\n";
      return -1;
    }
    Timestamp ts;
    if (Params::instance().preserve_)
      ts.read(path);

    auto image = Exiv2::ImageFactory::open(path_);
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
  } catch (const Exiv2::Error& e) {
    std::cerr << "Exiv2 exception in erase action for file " << path << ":\n" << e << "\n";
    return 1;
  }
}

int Erase::eraseThumbnail(Exiv2::Image* image) {
  Exiv2::ExifThumb exifThumb(image->exifData());
  std::string thumbExt = exifThumb.extension();
  if (thumbExt.empty()) {
    return 0;
  }
  exifThumb.erase();
  if (Params::instance().verbose_) {
    std::cout << _("Erasing thumbnail data") << '\n';
  }
  return 0;
}

int Erase::eraseExifData(Exiv2::Image* image) {
  if (Params::instance().verbose_ && !image->exifData().empty()) {
    std::cout << _("Erasing Exif data from the file") << '\n';
  }
  image->clearExifData();
  return 0;
}

int Erase::eraseIptcData(Exiv2::Image* image) {
  if (Params::instance().verbose_ && !image->iptcData().empty()) {
    std::cout << _("Erasing IPTC data from the file") << '\n';
  }
  image->clearIptcData();
  return 0;
}

int Erase::eraseComment(Exiv2::Image* image) {
  if (Params::instance().verbose_ && !image->comment().empty()) {
    std::cout << _("Erasing JPEG comment from the file") << '\n';
  }
  image->clearComment();
  return 0;
}

int Erase::eraseXmpData(Exiv2::Image* image) {
  if (Params::instance().verbose_ && !image->xmpData().empty()) {
    std::cout << _("Erasing XMP data from the file") << '\n';
  }
  image->clearXmpData();  // Quick fix for bug #612
  image->clearXmpPacket();
  return 0;
}
int Erase::eraseIccProfile(Exiv2::Image* image) {
  if (Params::instance().verbose_ && image->iccProfileDefined()) {
    std::cout << _("Erasing ICC Profile data from the file") << '\n';
  }
  image->clearIccProfile();
  return 0;
}

Task::UniquePtr Erase::clone() const {
  return std::make_unique<Erase>(*this);
}

int Extract::run(const std::string& path) {
  try {
    path_ = path;
    int rc = 0;

    bool bStdout = (Params::instance().target_ & Params::ctStdInOut) != 0;
    if (bStdout) {
      _setmode(fileno(stdout), _O_BINARY);
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
  } catch (const Exiv2::Error& e) {
    std::cerr << "Exiv2 exception in extract action for file " << path << ":\n" << e << "\n";
    return 1;
  }
}

int Extract::writeThumbnail() const {
  if (!Exiv2::fileExists(path_)) {
    std::cerr << path_ << ": " << _("Failed to open the file") << "\n";
    return -1;
  }
  auto image = Exiv2::ImageFactory::open(path_);
  image->readMetadata();
  Exiv2::ExifData& exifData = image->exifData();
  if (exifData.empty()) {
    std::cerr << path_ << ": " << _("No Exif data found in the file") << "\n";
    return -3;
  }
  int rc = 0;
  Exiv2::ExifThumb exifThumb(exifData);
  std::string thumbExt = exifThumb.extension();
  if (thumbExt.empty()) {
    std::cerr << path_ << ": " << _("Image does not contain an Exif thumbnail") << "\n";
  } else {
    if ((Params::instance().target_ & Params::ctStdInOut) != 0) {
      Exiv2::DataBuf buf = exifThumb.copy();
      std::cout.write(buf.c_str(), buf.size());
      return 0;
    }

    std::string thumb = newFilePath(path_, "-thumb");
    std::string thumbPath = thumb + thumbExt;
    if (dontOverwrite(thumbPath))
      return 0;
    if (Params::instance().verbose_) {
      Exiv2::DataBuf buf = exifThumb.copy();
      if (!buf.empty()) {
        std::cout << _("Writing thumbnail") << " (" << exifThumb.mimeType() << ", " << buf.size() << " " << _("Bytes")
                  << ") " << _("to file") << " " << thumbPath << '\n';
      }
    }
    rc = static_cast<int>(exifThumb.writeFile(thumb));
    if (rc == 0) {
      std::cerr << path_ << ": " << _("Exif data doesn't contain a thumbnail") << "\n";
    }
  }
  return rc;
}  // Extract::writeThumbnail

int Extract::writePreviews() const {
  if (!Exiv2::fileExists(path_)) {
    std::cerr << path_ << ": " << _("Failed to open the file") << "\n";
    return -1;
  }

  auto image = Exiv2::ImageFactory::open(path_);
  image->readMetadata();

  Exiv2::PreviewManager pvMgr(*image);
  Exiv2::PreviewPropertiesList pvList = pvMgr.getPreviewProperties();

  const Params::PreviewNumbers& numbers = Params::instance().previewNumbers_;
  for (auto number : numbers) {
    auto num = static_cast<size_t>(number);
    if (num == 0) {
      // Write all previews
      for (num = 0; num < pvList.size(); ++num) {
        writePreviewFile(pvMgr.getPreviewImage(pvList[num]), num + 1);
      }
      break;
    }
    num--;
    if (num >= pvList.size()) {
      std::cerr << path_ << ": " << _("Image does not have preview") << " " << num + 1 << "\n";
      continue;
    }
    writePreviewFile(pvMgr.getPreviewImage(pvList[num]), num + 1);
  }
  return 0;
}  // Extract::writePreviews

int Extract::writeIccProfile(const std::string& target) const {
  int rc = 0;
  if (!Exiv2::fileExists(path_)) {
    std::cerr << path_ << ": " << _("Failed to open the file") << "\n";
    rc = -1;
  }

  bool bStdout = target == "-";

  if (rc == 0) {
    auto image = Exiv2::ImageFactory::open(path_);
    image->readMetadata();
    if (!image->iccProfileDefined()) {
      std::cerr << _("No embedded iccProfile: ") << path_ << '\n';
      rc = -2;
    } else {
      if (bStdout) {  // -eC-
        std::cout.write(image->iccProfile().c_str(), image->iccProfile().size());
      } else {
        if (Params::instance().verbose_) {
          std::cout << _("Writing iccProfile: ") << target << '\n';
        }
        Exiv2::FileIo iccFile(target);
        iccFile.open("wb");
        iccFile.write(image->iccProfile().c_data(), image->iccProfile().size());
        iccFile.close();
      }
    }
  }
  return rc;
}  // Extract::writeIccProfile

void Extract::writePreviewFile(const Exiv2::PreviewImage& pvImg, size_t num) const {
  std::string pvFile = newFilePath(path_, "-preview") + std::to_string(num);
  std::string pvPath = pvFile + pvImg.extension();
  if (dontOverwrite(pvPath))
    return;
  if (Params::instance().verbose_) {
    std::cout << _("Writing preview") << " " << num << " (" << pvImg.mimeType() << ", ";
    if (pvImg.width() != 0 && pvImg.height() != 0) {
      std::cout << pvImg.width() << "x" << pvImg.height() << " " << _("pixels") << ", ";
    }
    std::cout << pvImg.size() << " " << _("bytes") << ") " << _("to file") << " " << pvPath << '\n';
  }
  auto rc = pvImg.writeFile(pvFile);
  if (rc == 0) {
    std::cerr << path_ << ": " << _("Image does not have preview") << " " << num << "\n";
  }
}

Task::UniquePtr Extract::clone() const {
  return std::make_unique<Extract>(*this);
}

int Insert::run(const std::string& path) try {
  // -i{tgt}-  reading from stdin?
  bool bStdin = (Params::instance().target_ & Params::ctStdInOut) != 0;

  if (!Exiv2::fileExists(path)) {
    std::cerr << path << ": " << _("Failed to open the file") << "\n";
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
} catch (const Exiv2::Error& e) {
  std::cerr << "Exiv2 exception in insert action for file " << path << ":\n" << e << "\n";
  return 1;
}  // Insert::run

int Insert::insertXmpPacket(const std::string& path, const std::string& xmpPath) {
  int rc = 0;
  bool bStdin = xmpPath == "-";
  if (bStdin) {
    Exiv2::DataBuf xmpBlob;
    Params::instance().getStdin(xmpBlob);
    rc = insertXmpPacket(path, xmpBlob, true);
  } else {
    if (!Exiv2::fileExists(xmpPath)) {
      std::cerr << xmpPath << ": " << _("Failed to open the file") << "\n";
      rc = -1;
    }
    if (rc == 0 && !Exiv2::fileExists(path)) {
      std::cerr << path << ": " << _("Failed to open the file") << "\n";
      rc = -1;
    }
    if (rc == 0) {
      Exiv2::DataBuf xmpBlob = Exiv2::readFile(xmpPath);
      rc = insertXmpPacket(path, xmpBlob);
    }
  }
  return rc;

}  // Insert::insertXmpPacket

int Insert::insertXmpPacket(const std::string& path, const Exiv2::DataBuf& xmpBlob, bool usePacket) {
  std::string xmpPacket;
  for (size_t i = 0; i < xmpBlob.size(); i++) {
    xmpPacket += static_cast<char>(xmpBlob.read_uint8(i));
  }
  auto image = Exiv2::ImageFactory::open(path);
  image->readMetadata();
  image->clearXmpData();
  image->setXmpPacket(xmpPacket);
  image->writeXmpFromPacket(usePacket);
  image->writeMetadata();

  return 0;
}

int Insert::insertIccProfile(const std::string& path, const std::string& iccPath) {
  int rc = 0;
  // for path "foo.XXX", do a binary copy of "foo.icc"
  std::string iccProfilePath = newFilePath(path, ".icc");
  if (iccPath == "-") {
    Exiv2::DataBuf iccProfile;
    Params::instance().getStdin(iccProfile);
    rc = insertIccProfile(path, std::move(iccProfile));
  } else {
    if (!Exiv2::fileExists(iccProfilePath)) {
      std::cerr << iccProfilePath << ": " << _("Failed to open the file") << "\n";
      rc = -1;
    } else {
      Exiv2::DataBuf iccProfile = Exiv2::readFile(iccPath);
      rc = insertIccProfile(path, std::move(iccProfile));
    }
  }
  return rc;
}  // Insert::insertIccProfile

int Insert::insertIccProfile(const std::string& path, Exiv2::DataBuf&& iccProfileBlob) {
  int rc = 0;
  // test path exists
  if (!Exiv2::fileExists(path)) {
    std::cerr << path << ": " << _("Failed to open the file") << "\n";
    rc = -1;
  }

  // read in the metadata
  if (rc == 0) {
    auto image = Exiv2::ImageFactory::open(path);
    image->readMetadata();
    // clear existing profile, assign the blob and rewrite image
    image->clearIccProfile();
    if (!iccProfileBlob.empty()) {
      image->setIccProfile(std::move(iccProfileBlob));
    }
    image->writeMetadata();
  }

  return rc;
}  // Insert::insertIccProfile

int Insert::insertThumbnail(const std::string& path) {
  std::string thumbPath = newFilePath(path, "-thumb.jpg");
  if (!Exiv2::fileExists(thumbPath)) {
    std::cerr << thumbPath << ": " << _("Failed to open the file") << "\n";
    return -1;
  }
  if (!Exiv2::fileExists(path)) {
    std::cerr << path << ": " << _("Failed to open the file") << "\n";
    return -1;
  }
  auto image = Exiv2::ImageFactory::open(path);
  image->readMetadata();
  Exiv2::ExifThumb exifThumb(image->exifData());
  exifThumb.setJpegThumbnail(thumbPath);
  image->writeMetadata();

  return 0;
}  // Insert::insertThumbnail

Task::UniquePtr Insert::clone() const {
  return std::make_unique<Insert>(*this);
}

int Modify::run(const std::string& path) {
  try {
    if (!Exiv2::fileExists(path)) {
      std::cerr << path << ": " << _("Failed to open the file") << "\n";
      return -1;
    }
    Timestamp ts;
    if (Params::instance().preserve_)
      ts.read(path);

    auto image = Exiv2::ImageFactory::open(path);
    image->readMetadata();

    int rc = applyCommands(image.get());

    // Save both exif and iptc metadata
    image->writeMetadata();

    if (Params::instance().preserve_)
      ts.touch(path);

    return rc;
  } catch (const Exiv2::Error& e) {
    std::cerr << "Exiv2 exception in modify action for file " << path << ":\n" << e << "\n";
    return 1;
  }
}  // Modify::run

int Modify::applyCommands(Exiv2::Image* pImage) {
  if (!Params::instance().jpegComment_.empty()) {
    // If modify is used when extracting to stdout then ignore verbose
    if (Params::instance().verbose_ &&
        !(Params::instance().action_ & Action::extract && Params::instance().target_ & Params::ctStdInOut)) {
      std::cout << _("Setting JPEG comment") << " '" << Params::instance().jpegComment_ << "'" << '\n';
    }
    pImage->setComment(Params::instance().jpegComment_);
  }

  // loop through command table and apply each command
  const ModifyCmds& modifyCmds = Params::instance().modifyCmds_;
  int rc = 0;
  int ret = 0;
  for (const auto& cmd : modifyCmds) {
    switch (cmd.cmdId_) {
      case CmdId::add:
        ret = addMetadatum(pImage, cmd);
        if (rc == 0)
          rc = ret;
        break;
      case CmdId::set:
        ret = setMetadatum(pImage, cmd);
        if (rc == 0)
          rc = ret;
        break;
      case CmdId::del:
        delMetadatum(pImage, cmd);
        break;
      case CmdId::reg:
        regNamespace(cmd);
        break;
      case CmdId::invalid:
        break;
    }
  }
  return rc;
}  // Modify::applyCommands

int Modify::addMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd) {
  // If modify is used when extracting to stdout then ignore verbose
  if (Params::instance().verbose_ &&
      !(Params::instance().action_ & Action::extract && Params::instance().target_ & Params::ctStdInOut)) {
    std::cout << _("Add") << " " << modifyCmd.key_ << " \"" << modifyCmd.value_ << "\" ("
              << Exiv2::TypeInfo::typeName(modifyCmd.typeId_) << ")" << '\n';
  }
  Exiv2::ExifData& exifData = pImage->exifData();
  Exiv2::IptcData& iptcData = pImage->iptcData();
  Exiv2::XmpData& xmpData = pImage->xmpData();
  auto value = Exiv2::Value::create(modifyCmd.typeId_);
  int rc = value->read(modifyCmd.value_);
  if (0 == rc) {
    if (modifyCmd.metadataId_ == MetadataId::exif) {
      exifData.add(Exiv2::ExifKey(modifyCmd.key_), value.get());
    }
    if (modifyCmd.metadataId_ == MetadataId::iptc) {
      iptcData.add(Exiv2::IptcKey(modifyCmd.key_), value.get());
    }
    if (modifyCmd.metadataId_ == MetadataId::xmp) {
      xmpData.add(Exiv2::XmpKey(modifyCmd.key_), value.get());
    }
  } else {
    std::cerr << _("Warning") << ": " << modifyCmd.key_ << ": " << _("Failed to read") << " "
              << Exiv2::TypeInfo::typeName(value->typeId()) << " " << _("value") << " \"" << modifyCmd.value_ << "\"\n";
  }
  return rc;
}

// This function looks rather complex because we try to avoid adding an
// empty metadatum if reading the value fails
int Modify::setMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd) {
  // If modify is used when extracting to stdout then ignore verbose
  if (Params::instance().verbose_ &&
      !(Params::instance().action_ & Action::extract && Params::instance().target_ & Params::ctStdInOut)) {
    std::cout << _("Set") << " " << modifyCmd.key_ << " \"" << modifyCmd.value_ << "\" ("
              << Exiv2::TypeInfo::typeName(modifyCmd.typeId_) << ")" << '\n';
  }
  Exiv2::ExifData& exifData = pImage->exifData();
  Exiv2::IptcData& iptcData = pImage->iptcData();
  Exiv2::XmpData& xmpData = pImage->xmpData();
  Exiv2::Metadatum* metadatum = nullptr;
  if (modifyCmd.metadataId_ == MetadataId::exif) {
    auto pos = exifData.findKey(Exiv2::ExifKey(modifyCmd.key_));
    if (pos != exifData.end()) {
      metadatum = &(*pos);
    }
  }
  if (modifyCmd.metadataId_ == MetadataId::iptc) {
    auto pos = iptcData.findKey(Exiv2::IptcKey(modifyCmd.key_));
    if (pos != iptcData.end()) {
      metadatum = &(*pos);
    }
  }
  if (modifyCmd.metadataId_ == MetadataId::xmp) {
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
  if (!value || (modifyCmd.explicitType_ && modifyCmd.typeId_ != value->typeId())) {
    value = Exiv2::Value::create(modifyCmd.typeId_);
  }
  int rc = value->read(modifyCmd.value_);
  if (0 == rc) {
    if (metadatum) {
      metadatum->setValue(value.get());
    } else {
      if (modifyCmd.metadataId_ == MetadataId::exif) {
        exifData.add(Exiv2::ExifKey(modifyCmd.key_), value.get());
      }
      if (modifyCmd.metadataId_ == MetadataId::iptc) {
        iptcData.add(Exiv2::IptcKey(modifyCmd.key_), value.get());
      }
      if (modifyCmd.metadataId_ == MetadataId::xmp) {
        xmpData.add(Exiv2::XmpKey(modifyCmd.key_), value.get());
      }
    }
  } else {
    std::cerr << _("Warning") << ": " << modifyCmd.key_ << ": " << _("Failed to read") << " "
              << Exiv2::TypeInfo::typeName(value->typeId()) << " " << _("value") << " \"" << modifyCmd.value_ << "\"\n";
  }
  return rc;
}

void Modify::delMetadatum(Exiv2::Image* pImage, const ModifyCmd& modifyCmd) {
  // If modify is used when extracting to stdout then ignore verbose
  if (Params::instance().verbose_ &&
      !(Params::instance().action_ & Action::extract && Params::instance().target_ & Params::ctStdInOut)) {
    std::cout << _("Del") << " " << modifyCmd.key_ << '\n';
  }

  Exiv2::ExifData& exifData = pImage->exifData();
  Exiv2::IptcData& iptcData = pImage->iptcData();
  Exiv2::XmpData& xmpData = pImage->xmpData();
  if (modifyCmd.metadataId_ == MetadataId::exif) {
    Exiv2::ExifData::iterator pos;
    const Exiv2::ExifKey exifKey(modifyCmd.key_);
    while ((pos = exifData.findKey(exifKey)) != exifData.end()) {
      exifData.erase(pos);
    }
  }
  if (modifyCmd.metadataId_ == MetadataId::iptc) {
    Exiv2::IptcData::iterator pos;
    const Exiv2::IptcKey iptcKey(modifyCmd.key_);
    while ((pos = iptcData.findKey(iptcKey)) != iptcData.end()) {
      iptcData.erase(pos);
    }
  }
  if (modifyCmd.metadataId_ == MetadataId::xmp) {
    const Exiv2::XmpKey xmpKey(modifyCmd.key_);
    auto pos = xmpData.findKey(xmpKey);
    if (pos != xmpData.end()) {
      xmpData.eraseFamily(pos);
    }
  }
}

void Modify::regNamespace(const ModifyCmd& modifyCmd) {
  // If modify is used when extracting to stdout then ignore verbose
  if (Params::instance().verbose_ &&
      !(Params::instance().action_ & Action::extract && Params::instance().target_ & Params::ctStdInOut)) {
    std::cout << _("Reg ") << modifyCmd.key_ << "=\"" << modifyCmd.value_ << "\"" << '\n';
  }
  Exiv2::XmpProperties::registerNs(modifyCmd.value_, modifyCmd.key_);
}

Task::UniquePtr Modify::clone() const {
  return std::make_unique<Modify>(*this);
}

int Adjust::run(const std::string& path) try {
  adjustment_ = Params::instance().adjustment_;
  yearAdjustment_ = Params::instance().yodAdjust_[Params::yodYear].adjustment_;
  monthAdjustment_ = Params::instance().yodAdjust_[Params::yodMonth].adjustment_;
  dayAdjustment_ = Params::instance().yodAdjust_[Params::yodDay].adjustment_;

  if (!Exiv2::fileExists(path)) {
    std::cerr << path << ": " << _("Failed to open the file") << "\n";
    return -1;
  }
  Timestamp ts;
  if (Params::instance().preserve_)
    ts.read(path);

  auto image = Exiv2::ImageFactory::open(path);
  image->readMetadata();
  Exiv2::ExifData& exifData = image->exifData();
  if (exifData.empty()) {
    std::cerr << path << ": " << _("No Exif data found in the file") << "\n";
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
} catch (const Exiv2::Error& e) {
  std::cerr << "Exiv2 exception in adjust action for file " << path << ":\n" << e << "\n";
  return 1;
}  // Adjust::run

Task::UniquePtr Adjust::clone() const {
  return std::make_unique<Adjust>(*this);
}

int Adjust::adjustDateTime(Exiv2::ExifData& exifData, const std::string& key, const std::string& path) const {
  Exiv2::ExifKey ek(key);
  auto md = exifData.findKey(ek);
  if (md == exifData.end()) {
    // Key not found. That's ok, we do nothing.
    return 0;
  }
  std::string timeStr = md->toString();
  if (timeStr.empty() || timeStr[0] == ' ') {
    std::cerr << path << ": " << _("Timestamp of metadatum with key") << " `" << ek << "' " << _("not set") << "\n";
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
  tm tm;
  if (str2Tm(timeStr, &tm) != 0) {
    if (Params::instance().verbose_)
      std::cout << '\n';
    std::cerr << path << ": " << _("Failed to parse timestamp") << " `" << timeStr << "'\n";
    return 1;
  }

  // bounds checking for yearAdjustment_
  Exiv2::Internal::enforce<std::overflow_error>(yearAdjustment_ >= std::numeric_limits<decltype(tm.tm_year)>::min(),
                                                "year adjustment too low");
  Exiv2::Internal::enforce<std::overflow_error>(yearAdjustment_ <= std::numeric_limits<decltype(tm.tm_year)>::max(),
                                                "year adjustment too high");
  const auto yearAdjustment = static_cast<decltype(tm.tm_year)>(yearAdjustment_);

  // bounds checking for monthAdjustment_
  Exiv2::Internal::enforce<std::overflow_error>(monthAdjustment_ >= std::numeric_limits<decltype(tm.tm_mon)>::min(),
                                                "month adjustment too low");
  Exiv2::Internal::enforce<std::overflow_error>(monthAdjustment_ <= std::numeric_limits<decltype(tm.tm_mon)>::max(),
                                                "month adjustment too high");
  const auto monthAdjustment = static_cast<decltype(tm.tm_mon)>(monthAdjustment_);

  // bounds checking for dayAdjustment_
  static constexpr time_t secondsInDay = 24 * 60 * 60;
  Exiv2::Internal::enforce<std::overflow_error>(dayAdjustment_ >= std::numeric_limits<time_t>::min() / secondsInDay,
                                                "day adjustment too low");
  Exiv2::Internal::enforce<std::overflow_error>(dayAdjustment_ <= std::numeric_limits<time_t>::max() / secondsInDay,
                                                "day adjustment too high");
  const auto dayAdjustment = static_cast<time_t>(dayAdjustment_);

  // bounds checking for adjustment_
  Exiv2::Internal::enforce<std::overflow_error>(adjustment_ >= std::numeric_limits<time_t>::min(),
                                                "seconds adjustment too low");
  Exiv2::Internal::enforce<std::overflow_error>(adjustment_ <= std::numeric_limits<time_t>::max(),
                                                "seconds adjustment too high");
  const auto adjustment = static_cast<time_t>(adjustment_);

  const auto monOverflow = Safe::add(tm.tm_mon, monthAdjustment) / 12;
  tm.tm_mon = Safe::add(tm.tm_mon, monthAdjustment) % 12;
  tm.tm_year = Safe::add(tm.tm_year, Safe::add(yearAdjustment, monOverflow));
  // Let's not create files with non-4-digit years, we can't read them.
  if (tm.tm_year > 9999 - 1900 || tm.tm_year < 1000 - 1900) {
    if (Params::instance().verbose_)
      std::cout << '\n';
    std::cerr << path << ": " << _("Can't adjust timestamp by") << " " << yearAdjustment + monOverflow << " "
              << _("years") << "\n";
    return 1;
  }
  time_t time = mktime(&tm);
  time = Safe::add(time, Safe::add(adjustment, dayAdjustment * secondsInDay));
  timeStr = time2Str(time);
  if (Params::instance().verbose_) {
    std::cout << " " << _("to") << " " << timeStr << '\n';
  }
  md->setValue(timeStr);
  return 0;
}  // Adjust::adjustDateTime

int FixIso::run(const std::string& path) {
  try {
    if (!Exiv2::fileExists(path)) {
      std::cerr << path << ": " << _("Failed to open the file") << "\n";
      return -1;
    }
    Timestamp ts;
    if (Params::instance().preserve_)
      ts.read(path);

    auto image = Exiv2::ImageFactory::open(path);
    image->readMetadata();
    Exiv2::ExifData& exifData = image->exifData();
    if (exifData.empty()) {
      std::cerr << path << ": " << _("No Exif data found in the file") << "\n";
      return -3;
    }
    auto md = Exiv2::isoSpeed(exifData);
    if (md != exifData.end()) {
      if (md->key() == "Exif.Photo.ISOSpeedRatings") {
        if (Params::instance().verbose_) {
          std::cout << _("Standard Exif ISO tag exists; not modified") << "\n";
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
  } catch (const Exiv2::Error& e) {
    std::cerr << "Exiv2 exception in fixiso action for file " << path << ":\n" << e << "\n";
    return 1;
  }
}  // FixIso::run

Task::UniquePtr FixIso::clone() const {
  return std::make_unique<FixIso>(*this);
}

int FixCom::run(const std::string& path) {
  try {
    if (!Exiv2::fileExists(path)) {
      std::cerr << path << ": " << _("Failed to open the file") << "\n";
      return -1;
    }
    Timestamp ts;
    if (Params::instance().preserve_)
      ts.read(path);

    auto image = Exiv2::ImageFactory::open(path);
    image->readMetadata();
    Exiv2::ExifData& exifData = image->exifData();
    if (exifData.empty()) {
      std::cerr << path << ": " << _("No Exif data found in the file") << "\n";
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
    const auto pcv = dynamic_cast<const Exiv2::CommentValue*>(v.get());
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
  } catch (const Exiv2::Error& e) {
    std::cerr << "Exiv2 exception in fixcom action for file " << path << ":\n" << e << "\n";
    return 1;
  }
}  // FixCom::run

Task::UniquePtr FixCom::clone() const {
  return std::make_unique<FixCom>(*this);
}

}  // namespace Action

// *****************************************************************************
// local definitions
namespace {
//! @cond IGNORE
int Timestamp::read(const std::string& path) {
  struct stat buf;
  int rc = stat(path.c_str(), &buf);
  if (0 == rc) {
    actime_ = buf.st_atime;
    modtime_ = buf.st_mtime;
  }
  return rc;
}

int Timestamp::read(tm* tm) {
  int rc = 1;
  time_t t = mktime(tm);  // interpret tm according to current timezone settings
  if (t != static_cast<time_t>(-1)) {
    rc = 0;
    actime_ = t;
    modtime_ = t;
  }
  return rc;
}

int Timestamp::touch(const std::string& path) const {
  if (0 == actime_)
    return 1;
  utimbuf buf;
  buf.actime = actime_;
  buf.modtime = modtime_;
  return utime(path.c_str(), &buf);
}
//! @endcond

int str2Tm(const std::string& timeStr, tm* tm) {
  if (timeStr.empty() || timeStr.front() == ' ')
    return 1;
  if (timeStr.length() < 19)
    return 2;
  if ((timeStr[4] != ':' && timeStr[4] != '-') || (timeStr[7] != ':' && timeStr[7] != '-') || timeStr[10] != ' ' ||
      timeStr[13] != ':' || timeStr[16] != ':')
    return 3;
  if (!tm)
    return 4;
  std::memset(tm, 0x0, sizeof(*tm));
  tm->tm_isdst = -1;

  int64_t tmp = 0;
  if (!Util::strtol(timeStr.substr(0, 4).c_str(), tmp))
    return 5;
  // tmp is a 4-digit number so this cast cannot overflow
  tm->tm_year = static_cast<decltype(tm->tm_year)>(tmp - 1900);
  if (!Util::strtol(timeStr.substr(5, 2).c_str(), tmp))
    return 6;
  // tmp is a 2-digit number so this cast cannot overflow
  tm->tm_mon = static_cast<decltype(tm->tm_mon)>(tmp - 1);
  if (!Util::strtol(timeStr.substr(8, 2).c_str(), tmp))
    return 7;
  // tmp is a 2-digit number so this cast cannot overflow
  tm->tm_mday = static_cast<decltype(tm->tm_mday)>(tmp);
  if (!Util::strtol(timeStr.substr(11, 2).c_str(), tmp))
    return 8;
  // tmp is a 2-digit number so this cast cannot overflow
  tm->tm_hour = static_cast<decltype(tm->tm_hour)>(tmp);
  if (!Util::strtol(timeStr.substr(14, 2).c_str(), tmp))
    return 9;
  // tmp is a 2-digit number so this cast cannot overflow
  tm->tm_min = static_cast<decltype(tm->tm_min)>(tmp);
  if (!Util::strtol(timeStr.substr(17, 2).c_str(), tmp))
    return 10;
  // tmp is a 2-digit number so this cast cannot overflow
  tm->tm_sec = static_cast<decltype(tm->tm_sec)>(tmp);

  // Conversions to set remaining fields of the tm structure
  if (mktime(tm) == static_cast<time_t>(-1))
    return 11;

  return 0;
}  // str2Tm

std::string time2Str(time_t time) {
  auto tm = localtime(&time);
  return tm2Str(tm);
}  // time2Str

std::string tm2Str(const tm* tm) {
  if (!tm)
    return "";

  std::ostringstream os;
  os << std::setfill('0') << tm->tm_year + 1900 << ":" << std::setw(2) << tm->tm_mon + 1 << ":" << std::setw(2)
     << tm->tm_mday << " " << std::setw(2) << tm->tm_hour << ":" << std::setw(2) << tm->tm_min << ":" << std::setw(2)
     << tm->tm_sec;

  return os.str();
}  // tm2Str

std::string temporaryPath() {
  static int count = 0;
  auto guard = std::scoped_lock(cs);

#if defined(_WIN32)
  DWORD pid = ::GetCurrentProcessId();
#else
  pid_t pid = ::getpid();
#endif
  /// \todo check if we can use std::tmpnam
  auto p = fs::temp_directory_path() / (Exiv2::toString(pid) + "_" + std::to_string(count));
  if (fs::exists(p)) {
    fs::remove(p);
  }

  return p.string();
}

int metacopy(const std::string& source, const std::string& tgt, Exiv2::ImageType targetType, bool preserve) {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "actions.cpp::metacopy"
            << " source = " << source << " target = " << tgt << '\n';
#endif

  // read the source metadata
  int rc = -1;
  if (!Exiv2::fileExists(source)) {
    std::cerr << source << ": " << _("Failed to open the file") << "\n";
    return rc;
  }

  bool bStdin = source == "-";
  bool bStdout = tgt == "-";

  Exiv2::DataBuf stdIn;
  Exiv2::Image::UniquePtr sourceImage;
  if (bStdin) {
    Params::instance().getStdin(stdIn);
    auto ioStdin = std::make_unique<Exiv2::MemIo>(stdIn.c_data(), stdIn.size());
    sourceImage = Exiv2::ImageFactory::open(std::move(ioStdin));
  } else {
    sourceImage = Exiv2::ImageFactory::open(source);
  }

  sourceImage->readMetadata();

  // Apply any modification commands to the source image on-the-fly
  Action::Modify::applyCommands(sourceImage.get());

  // Open or create the target file
  std::string target(bStdout ? temporaryPath() : tgt);

  std::unique_ptr<Exiv2::Image> targetImage;
  if (Exiv2::fileExists(target)) {
    targetImage = Exiv2::ImageFactory::open(target);
    targetImage->readMetadata();
  } else {
    targetImage = Exiv2::ImageFactory::create(targetType, target);
  }

  // Copy each type of metadata
  if (Params::instance().target_ & Params::ctExif && !sourceImage->exifData().empty()) {
    if (Params::instance().verbose_ && !bStdout) {
      std::cout << _("Writing Exif data from") << " " << source << " " << _("to") << " " << target << '\n';
    }
    if (preserve) {
      for (const auto& exif : sourceImage->exifData()) {
        targetImage->exifData()[exif.key()] = exif.value();
      }
    } else {
      targetImage->setExifData(sourceImage->exifData());
    }
  }
  if (Params::instance().target_ & Params::ctIptc && !sourceImage->iptcData().empty()) {
    if (Params::instance().verbose_ && !bStdout) {
      std::cout << _("Writing IPTC data from") << " " << source << " " << _("to") << " " << target << '\n';
    }
    if (preserve) {
      for (const auto& iptc : sourceImage->iptcData()) {
        targetImage->iptcData()[iptc.key()] = iptc.value();
      }
    } else {
      targetImage->setIptcData(sourceImage->iptcData());
    }
  }
  if (Params::instance().target_ & (Params::ctXmp | Params::ctXmpRaw) && !sourceImage->xmpData().empty()) {
    if (Params::instance().verbose_ && !bStdout) {
      std::cout << _("Writing XMP data from") << " " << source << " " << _("to") << " " << target << '\n';
    }

    // #1148 use Raw XMP packet if there are no XMP modification commands
    Params::CommonTarget tRawSidecar = Params::ctXmpSidecar | Params::ctXmpRaw;  // option -eXX
    if (Params::instance().modifyCmds_.empty() && (Params::instance().target_ & tRawSidecar) == tRawSidecar) {
      // std::cout << "short cut" << '\n';
      // http://www.cplusplus.com/doc/tutorial/files/
      std::ofstream os;
      os.open(target.c_str());
      sourceImage->printStructure(os, Exiv2::kpsXMP);
      os.close();
      rc = 0;
    } else if (preserve) {
      for (const auto& xmp : sourceImage->xmpData()) {
        targetImage->xmpData()[xmp.key()] = xmp.value();
      }
    } else {
      // std::cout << "long cut" << '\n';
      targetImage->setXmpData(sourceImage->xmpData());
    }
  }
  if (Params::instance().target_ & Params::ctComment && !sourceImage->comment().empty()) {
    if (Params::instance().verbose_ && !bStdout) {
      std::cout << _("Writing JPEG comment from") << " " << source << " " << _("to") << " " << tgt << '\n';
    }
    targetImage->setComment(sourceImage->comment());
  }
  if (rc < 0)
    try {
      targetImage->writeMetadata();
      rc = 0;
    } catch (const Exiv2::Error& e) {
      std::cerr << tgt << ": " << _("Could not write metadata to file") << ": " << e << "\n";
      rc = 1;
    }

  // if we used a temporary target, copy it to stdout
  if (rc == 0 && bStdout) {
    _setmode(fileno(stdout), O_BINARY);
    if (auto f = std::ifstream(target, std::ios::binary)) {
      std::vector<char> buffer(8 * 1024);

      while (f.read(buffer.data(), buffer.size()) || f.gcount() > 0) {
        std::fwrite(buffer.data(), 1, f.gcount(), stdout);
      }
    }
  }

  // delete temporary target
  if (bStdout)
    fs::remove(target.c_str());

  return rc;
}  // metacopy

void replace(std::string& text, const std::string& searchText, const std::string& replaceText) {
  std::string::size_type index = 0;
  while ((index = text.find(searchText, index)) != std::string::npos) {
    text.replace(index, searchText.length(), replaceText.c_str(), replaceText.length());
    index++;
  }
}

int renameFile(std::string& newPath, const tm* tm) {
  auto p = fs::path(newPath);
  std::string path = newPath;
  auto oldFsPath = fs::path(path);
  std::string format = Params::instance().format_;
  std::string filename = p.stem().string();
  std::string basesuffix;
  int pos = filename.find('.');
  if (pos > 0)
    basesuffix = filename.substr(filename.find('.'));
  replace(format, ":basename:", p.stem().string());
  replace(format, ":basesuffix:", basesuffix);
  replace(format, ":dirname:", p.parent_path().filename().string());
  replace(format, ":parentname:", p.parent_path().parent_path().filename().string());

  const size_t max = 1024;
  char basename[max] = {};
  if (strftime(basename, max, format.c_str(), tm) == 0) {
    std::cerr << _("Filename format yields empty filename for the file") << " " << path << "\n";
    return 1;
  }

  newPath = (p.parent_path() / (basename + p.extension().string())).string();
  p = fs::path(newPath);

  if (p.parent_path() == oldFsPath.parent_path() && p.filename() == oldFsPath.filename()) {
    if (Params::instance().verbose_) {
      std::cout << _("This file already has the correct name") << '\n';
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
          newPath = (p.parent_path() / (std::string(basename) + "_" + Exiv2::toString(seq++) + p.extension().string()))
                        .string();
          break;
        case Params::askPolicy:
          std::cout << Params::instance().progname() << ": " << _("File") << " `" << newPath << "' "
                    << _("exists. [O]verwrite, [r]ename or [s]kip?") << " ";
          std::cin >> s;
          switch (s.at(0)) {
            case 'o':
            case 'O':
              go = false;
              break;
            case 'r':
            case 'R':
              fileExistsPolicy = Params::renamePolicy;
              newPath =
                  (p.parent_path() / (std::string(basename) + "_" + Exiv2::toString(seq++) + p.extension().string()))
                      .string();
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
    std::cout << '\n';
  }

  fs::rename(path, newPath);
  return 0;
}

std::string newFilePath(const std::string& path, const std::string& ext) {
  auto p = fs::path(path);
  auto directory = fs::path(Params::instance().directory_);
  if (directory.empty())
    directory = p.parent_path();
  if (Exiv2::fileProtocol(path) != Exiv2::pFile)
    directory.clear();  // use current directory for remote files
  return (directory / (p.stem().string() + ext)).string();
}

int dontOverwrite(const std::string& path) {
  if (path == "-")
    return 0;

  if (!Params::instance().force_ && Exiv2::fileExists(path)) {
    std::cout << Params::instance().progname() << ": " << _("Overwrite") << " `" << path << "'? ";
    std::string s;
    std::cin >> s;
    if (s.at(0) != 'y' && s.at(0) != 'Y')
      return 1;
  }
  return 0;
}

std::ostream& operator<<(std::ostream& os, const std::pair<std::string, int>& strAndWidth) {
  const std::string& str(strAndWidth.first);
  size_t minChCount(strAndWidth.second);
  size_t count = mbstowcs(nullptr, str.c_str(), 0);  // returns 0xFFFFFFFF on error
  if (count < minChCount) {
    minChCount += str.size() - count;
  }
  return os << std::setw(minChCount) << str;
}

int printStructure(std::ostream& out, Exiv2::PrintStructureOption option, const std::string& path) {
  if (!Exiv2::fileExists(path)) {
    std::cerr << path << ": " << _("Failed to open the file") << "\n";
    return -1;
  }
  Exiv2::Image::UniquePtr image = Exiv2::ImageFactory::open(path);
  image->printStructure(out, option);
  return 0;
}
}  // namespace
