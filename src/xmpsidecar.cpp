// SPDX-License-Identifier: GPL-2.0-or-later
#include "xmpsidecar.hpp"

#include "basicio.hpp"
#include "config.h"
#include "convert.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "utils.hpp"
#include "xmp_exiv2.hpp"

#include <iostream>

namespace {
constexpr char xmlHeader[] = "<?xpacket begin=\"\xef\xbb\xbf\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n";
constexpr auto xmlHdrCnt = std::size(xmlHeader) - 1;  // without the trailing 0-character
constexpr auto xmlFooter = "<?xpacket end=\"w\"?>";
}  // namespace

// class member definitions
namespace Exiv2 {
XmpSidecar::XmpSidecar(BasicIo::UniquePtr io, bool create) : Image(ImageType::xmp, mdXmp, std::move(io)) {
  if (create && io_->open() == 0) {
    IoCloser closer(*io_);
    io_->write(reinterpret_cast<const byte*>(xmlHeader), xmlHdrCnt);
  }
}  // XmpSidecar::XmpSidecar

std::string XmpSidecar::mimeType() const {
  return "application/rdf+xml";
}

void XmpSidecar::setComment(const std::string&) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "XMP"));
}

void XmpSidecar::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Reading XMP file " << io_->path() << "\n";
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isXmpType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "XMP");
  }
  // Read the XMP packet from the IO stream
  std::string xmpPacket;
  const long len = 64 * 1024;
  byte buf[len];
  size_t l;
  while ((l = io_->read(buf, len)) > 0) {
    xmpPacket.append(reinterpret_cast<char*>(buf), l);
  }
  if (io_->error())
    throw Error(ErrorCode::kerFailedToReadImageData);
  clearMetadata();
  xmpPacket_ = xmpPacket;
  if (!xmpPacket_.empty() && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
    EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
  }

  // #1112 - store dates to deal with loss of TZ information during conversions
  for (const auto& xmp : xmpData_) {
    std::string key(xmp.key());
    if (Internal::contains(key, "Date")) {
      std::string value(xmp.value().toString());
      dates_[key] = value;
    }
  }

  copyXmpToIptc(xmpData_, iptcData_);
  copyXmpToExif(xmpData_, exifData_);
}  // XmpSidecar::readMetadata

static bool matchi(const std::string& key, const char* substr) {
  return Internal::contains(Internal::lower(key), substr);
}

void XmpSidecar::writeMetadata() {
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);

  if (!writeXmpFromPacket()) {
    // #589 copy XMP tags
    Exiv2::XmpData copy;
    for (const auto& xmp : xmpData_) {
      if (!matchi(xmp.key(), "exif") && !matchi(xmp.key(), "iptc")) {
        copy[xmp.key()] = xmp.value();
      }
    }

    // run the converters
    copyExifToXmp(exifData_, xmpData_);
    copyIptcToXmp(iptcData_, xmpData_);

    // #1112 - restore dates if they lost their TZ info
    for (const auto& [sKey, value_orig] : dates_) {
      Exiv2::XmpKey key(sKey);
      if (xmpData_.findKey(key) != xmpData_.end()) {
        std::string value_now(xmpData_[sKey].value().toString());
        // std::cout << key << " -> " << value_now << " => " << value_orig << '\n';
        if (Internal::contains(value_orig, value_now.substr(0, 10))) {
          xmpData_[sKey] = value_orig;
        }
      }
    }

    // #589 - restore tags which were modified by the converters
    for (const auto& xmp : copy) {
      xmpData_[xmp.key()] = xmp.value();
    }

    if (XmpParser::encode(xmpPacket_, xmpData_, XmpParser::omitPacketWrapper | XmpParser::useCompactFormat) > 1) {
#ifndef SUPPRESS_WARNINGS
      EXV_ERROR << "Failed to encode XMP metadata.\n";
#endif
    }
  }
  if (!xmpPacket_.empty()) {
    if (xmpPacket_.substr(0, 5) != "<?xml") {
      xmpPacket_ = xmlHeader + xmpPacket_ + xmlFooter;
    }
    MemIo tempIo;

    // Write XMP packet
    if (tempIo.write(reinterpret_cast<const byte*>(xmpPacket_.data()), xmpPacket_.size()) != xmpPacket_.size())
      throw Error(ErrorCode::kerImageWriteFailed);
    if (tempIo.error())
      throw Error(ErrorCode::kerImageWriteFailed);
    io_->close();
    io_->transfer(tempIo);  // may throw
  }
}  // XmpSidecar::writeMetadata

// *************************************************************************
// free functions
Image::UniquePtr newXmpInstance(BasicIo::UniquePtr io, bool create) {
  auto image = std::make_unique<XmpSidecar>(std::move(io), create);
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isXmpType(BasicIo& iIo, bool advance) {
  /*
    Check if the file starts with an optional XML declaration followed by
    either an XMP header (<?xpacket ... ?>) or an <x:xmpmeta> element.

    In addition, in order for empty XmpSidecar objects as created by
    Exiv2 to pass the test, just an XML header is also considered ok.
   */
  const int32_t len = 80;
  byte buf[len];
  iIo.read(buf, xmlHdrCnt + 1);
  if (iIo.eof() && 0 == strncmp(reinterpret_cast<const char*>(buf), xmlHeader, xmlHdrCnt)) {
    return true;
  }
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  iIo.read(buf + xmlHdrCnt + 1, len - xmlHdrCnt - 1);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  // Skip leading BOM
  int32_t start = 0;
  if (0 == strncmp(reinterpret_cast<const char*>(buf), "\xef\xbb\xbf", 3)) {
    start = 3;
  }
  bool rc = false;
  std::string head(reinterpret_cast<const char*>(buf + start), len - start);
  if (head.substr(0, 5) == "<?xml") {
    // Forward to the next tag
    auto it = std::find(head.begin() + 5, head.end(), '<');
    if (it != head.end())
      head = head.substr(std::distance(head.begin(), it));
  }
  if (head.size() > 9 && (head.substr(0, 9) == "<?xpacket" || head.substr(0, 10) == "<x:xmpmeta")) {
    rc = true;
  }
  if (!advance || !rc) {
    iIo.seek(-(len - start), BasicIo::cur);  // Swallow the BOM
  }
  return rc;
}

}  // namespace Exiv2
