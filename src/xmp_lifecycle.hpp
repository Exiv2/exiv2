// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef EXIV2_XMP_LIFECYCLE_HPP
#define EXIV2_XMP_LIFECYCLE_HPP

#include "error.hpp"
#include "exiv2lib_export.h"
#include "properties.hpp"

#include <mutex>

// Adobe XMP Toolkit
#ifdef EXV_HAVE_XMP_TOOLKIT
#ifdef EXV_ADOBE_XMPSDK
#include <XMP.hpp>
#else
#include <XMPSDK.hpp>
#endif
#endif  // EXV_HAVE_XMP_TOOLKIT

namespace Exiv2 {

/*!
  @brief Ensure the XMP Toolkit is initialized.
         This function is thread-safe and idempotent.
         It throws Exiv2::Error if initialization fails.
 */
void xmpToolkitEnsureInitialized();

#ifdef EXV_HAVE_XMP_TOOLKIT
class XmpToolkitLifetimeManager {
 public:
  XmpToolkitLifetimeManager() {
    if (SXMPMeta::Initialize()) {
#ifdef EXV_ADOBE_XMPSDK
      SXMPMeta::RegisterNamespace("http://ns.adobe.com/lightroom/1.0/", "lr", nullptr);
      SXMPMeta::RegisterNamespace("http://rs.tdwg.org/dwc/index.htm", "dwc", nullptr);
      SXMPMeta::RegisterNamespace("http://purl.org/dc/terms/", "dcterms", nullptr);
      SXMPMeta::RegisterNamespace("http://www.digikam.org/ns/1.0/", "digiKam", nullptr);
      SXMPMeta::RegisterNamespace("http://www.digikam.org/ns/kipi/1.0/", "kipi", nullptr);
      SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.0/", "MicrosoftPhoto", nullptr);
      SXMPMeta::RegisterNamespace("http://ns.acdsee.com/iptc/1.0/", "acdsee", nullptr);
      SXMPMeta::RegisterNamespace("http://iptc.org/std/Iptc4xmpExt/2008-02-29/", "iptcExt", nullptr);
      SXMPMeta::RegisterNamespace("http://ns.useplus.org/ldf/xmp/1.0/", "plus", nullptr);
      SXMPMeta::RegisterNamespace("http://ns.iview-multimedia.com/mediapro/1.0/", "mediapro", nullptr);
      SXMPMeta::RegisterNamespace("http://ns.microsoft.com/expressionmedia/1.0/", "expressionmedia", nullptr);
      SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/", "MP", nullptr);
      SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/t/RegionInfo#", "MPRI", nullptr);
      SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/t/Region#", "MPReg", nullptr);
      SXMPMeta::RegisterNamespace("http://ns.google.com/photos/1.0/panorama/", "GPano", nullptr);
      SXMPMeta::RegisterNamespace("http://www.metadataworkinggroup.com/schemas/regions/", "mwg-rs", nullptr);
      SXMPMeta::RegisterNamespace("http://www.metadataworkinggroup.com/schemas/keywords/", "mwg-kw", nullptr);
      SXMPMeta::RegisterNamespace("http://ns.adobe.com/xmp/sType/Area#", "stArea", nullptr);
      SXMPMeta::RegisterNamespace("http://cipa.jp/exif/1.0/", "exifEX", nullptr);
      SXMPMeta::RegisterNamespace("http://ns.adobe.com/camera-raw-saved-settings/1.0/", "crss", nullptr);
      SXMPMeta::RegisterNamespace("http://www.audio/", "audio", nullptr);
      SXMPMeta::RegisterNamespace("http://www.video/", "video", nullptr);
#else
      SXMPMeta::RegisterNamespace("http://ns.adobe.com/lightroom/1.0/", "lr");
      SXMPMeta::RegisterNamespace("http://rs.tdwg.org/dwc/index.htm", "dwc");
      SXMPMeta::RegisterNamespace("http://purl.org/dc/terms/", "dcterms");
      SXMPMeta::RegisterNamespace("http://www.digikam.org/ns/1.0/", "digiKam");
      SXMPMeta::RegisterNamespace("http://www.digikam.org/ns/kipi/1.0/", "kipi");
      SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.0/", "MicrosoftPhoto");
      SXMPMeta::RegisterNamespace("http://ns.acdsee.com/iptc/1.0/", "acdsee");
      SXMPMeta::RegisterNamespace("http://iptc.org/std/Iptc4xmpExt/2008-02-29/", "iptcExt");
      SXMPMeta::RegisterNamespace("http://ns.useplus.org/ldf/xmp/1.0/", "plus");
      SXMPMeta::RegisterNamespace("http://ns.iview-multimedia.com/mediapro/1.0/", "mediapro");
      SXMPMeta::RegisterNamespace("http://ns.microsoft.com/expressionmedia/1.0/", "expressionmedia");
      SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/", "MP");
      SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/t/RegionInfo#", "MPRI");
      SXMPMeta::RegisterNamespace("http://ns.microsoft.com/photo/1.2/t/Region#", "MPReg");
      SXMPMeta::RegisterNamespace("http://ns.google.com/photos/1.0/panorama/", "GPano");
      SXMPMeta::RegisterNamespace("http://www.metadataworkinggroup.com/schemas/regions/", "mwg-rs");
      SXMPMeta::RegisterNamespace("http://www.metadataworkinggroup.com/schemas/keywords/", "mwg-kw");
      SXMPMeta::RegisterNamespace("http://ns.adobe.com/xmp/sType/Area#", "stArea");
      SXMPMeta::RegisterNamespace("http://cipa.jp/exif/1.0/", "exifEX");
      SXMPMeta::RegisterNamespace("http://ns.adobe.com/camera-raw-saved-settings/1.0/", "crss");
      SXMPMeta::RegisterNamespace("http://www.audio/", "audio");
      SXMPMeta::RegisterNamespace("http://www.video/", "video");
#endif
    } else {
      throw Error(ErrorCode::kerXMPToolkitError, 2, "Failed to initialize XMP Toolkit");
    }
  }

  ~XmpToolkitLifetimeManager() {
    // Use Unsafe version to avoid acquiring mutex during static destruction.
    // This is safe because static destruction is single-threaded per C++ standard.
    XmpProperties::unregisterAllNsNoLock(XmpProperties::LifetimeKey{});
    SXMPMeta::Terminate();
  }
};
#endif  // EXV_HAVE_XMP_TOOLKIT

}  // namespace Exiv2

#endif  // EXIV2_XMP_LIFECYCLE_HPP
