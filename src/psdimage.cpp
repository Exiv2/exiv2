// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "psdimage.hpp"

#include "basicio.hpp"
#include "config.h"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "jpgimage.hpp"
#include "photoshop.hpp"

#include <iostream>

// Todo: Consolidate with existing code in struct Photoshop (jpgimage.hpp):
//       Extend this helper to a proper class with all required functionality,
//       then move it here or into a separate file?

//! @cond IGNORE
struct PhotoshopResourceBlock {
  uint32_t resourceType;  // one of the markers in Photoshop::irbId_[]
  uint16_t resourceId;
  unsigned char resourceName[2];  // Pascal string (length byte + characters), padded to an even size -- this assumes
                                  // the empty string
  uint32_t resourceDataSize;
};
//! @endcond

// Photoshop resource IDs (Cf.
// <http://search.cpan.org/~bettelli/Image-MetaData-JPEG-0.15/lib/Image/MetaData/JPEG/TagLists.pod>)
enum {
  kPhotoshopResourceID_Photoshop2Info =
      0x03e8,  // [obsolete -- Photoshop 2.0 only] General information -- contains five 2-byte values: number of
               // channels, rows, columns, depth and mode
  kPhotoshopResourceID_MacintoshClassicPrintInfo = 0x03e9,  // [optional] Macintosh classic print record (120 bytes)
  kPhotoshopResourceID_MacintoshCarbonPrintInfo =
      0x03ea,  // [optional] Macintosh carbon print info (variable-length XML format)
  kPhotoshopResourceID_Photoshop2ColorTable = 0x03eb,   // [obsolete -- Photoshop 2.0 only] Indexed color table
  kPhotoshopResourceID_ResolutionInfo = 0x03ed,         // PhotoshopResolutionInfo structure (see below)
  kPhotoshopResourceID_AlphaChannelsNames = 0x03ee,     // as a series of Pstrings
  kPhotoshopResourceID_DisplayInfo = 0x03ef,            // see appendix A in Photoshop SDK
  kPhotoshopResourceID_PStringCaption = 0x03f0,         // [optional] the caption, as a Pstring
  kPhotoshopResourceID_BorderInformation = 0x03f1,      // border width and units
  kPhotoshopResourceID_BackgroundColor = 0x03f2,        // see additional Adobe information
  kPhotoshopResourceID_PrintFlags = 0x03f3,             // labels, crop marks, colour bars, ecc...
  kPhotoshopResourceID_BWHalftoningInfo = 0x03f4,       // Gray-scale and multich. half-toning info
  kPhotoshopResourceID_ColorHalftoningInfo = 0x03f5,    // Colour half-toning information
  kPhotoshopResourceID_DuotoneHalftoningInfo = 0x03f6,  // Duo-tone half-toning information
  kPhotoshopResourceID_BWTransferFunc = 0x03f7,         // Gray-scale and multich. transfer function
  kPhotoshopResourceID_ColorTransferFuncs = 0x03f8,     // Colour transfer function
  kPhotoshopResourceID_DuotoneTransferFuncs = 0x03f9,   // Duo-tone transfer function
  kPhotoshopResourceID_DuotoneImageInfo = 0x03fa,       // Duo-tone image information
  kPhotoshopResourceID_EffectiveBW = 0x03fb,            // two bytes for the effective black and white values
  kPhotoshopResourceID_ObsoletePhotoshopTag1 = 0x03fc,  // [obsolete]
  kPhotoshopResourceID_EPSOptions = 0x03fd,             // Encapsulated Postscript options
  kPhotoshopResourceID_QuickMaskInfo = 0x03fe,  // Quick Mask information. 2 bytes containing Quick Mask channel ID,
                                                // 1 byte boolean indicating whether the mask was initially empty.
  kPhotoshopResourceID_ObsoletePhotoshopTag2 = 0x03ff,    // [obsolete]
  kPhotoshopResourceID_LayerStateInfo = 0x0400,           // index of target layer (0 means bottom)
  kPhotoshopResourceID_WorkingPathInfo = 0x0401,          // should not be saved to the file
  kPhotoshopResourceID_LayersGroupInfo = 0x0402,          // for grouping layers together
  kPhotoshopResourceID_ObsoletePhotoshopTag3 = 0x0403,    // [obsolete] ??
  kPhotoshopResourceID_IPTC_NAA = 0x0404,                 // IPTC/NAA data
  kPhotoshopResourceID_RawImageMode = 0x0405,             // image mode for raw format files
  kPhotoshopResourceID_JPEGQuality = 0x0406,              // [private]
  kPhotoshopResourceID_GridGuidesInfo = 0x0408,           // see additional Adobe information
  kPhotoshopResourceID_ThumbnailResource = 0x0409,        // see additional Adobe information
  kPhotoshopResourceID_CopyrightFlag = 0x040a,            // true if image is copyrighted
  kPhotoshopResourceID_URL = 0x040b,                      // text string with a resource locator
  kPhotoshopResourceID_ThumbnailResource2 = 0x040c,       // see additional Adobe information
  kPhotoshopResourceID_GlobalAngle = 0x040d,              // global lighting angle for effects layer
  kPhotoshopResourceID_ColorSamplersResource = 0x040e,    // see additional Adobe information
  kPhotoshopResourceID_ICCProfile = 0x040f,               // see notes from Internat. Color Consortium
  kPhotoshopResourceID_Watermark = 0x0410,                // one byte
  kPhotoshopResourceID_ICCUntagged = 0x0411,              // 1 means intentionally untagged
  kPhotoshopResourceID_EffectsVisible = 0x0412,           // 1 byte to show/hide all effects layers
  kPhotoshopResourceID_SpotHalftone = 0x0413,             // version, length and data
  kPhotoshopResourceID_IDsBaseValue = 0x0414,             // base value for new layers ID's
  kPhotoshopResourceID_UnicodeAlphaNames = 0x0415,        // length plus Unicode string
  kPhotoshopResourceID_IndexedColourTableCount = 0x0416,  // [Photoshop 6.0 and later] 2 bytes
  kPhotoshopResourceID_TransparentIndex = 0x0417,         // [Photoshop 6.0 and later] 2 bytes
  kPhotoshopResourceID_GlobalAltitude = 0x0419,           // [Photoshop 6.0 and later] 4 bytes
  kPhotoshopResourceID_Slices = 0x041a,                   // [Photoshop 6.0 and later] see additional Adobe info
  kPhotoshopResourceID_WorkflowURL = 0x041b,              // [Photoshop 6.0 and later] 4 bytes length + Unicode string
  kPhotoshopResourceID_JumpToXPEP = 0x041c,               // [Photoshop 6.0 and later] see additional Adobe info
  kPhotoshopResourceID_AlphaIdentifiers = 0x041d,         // [Photoshop 6.0 and later] 4*(n+1) bytes
  kPhotoshopResourceID_URLList = 0x041e,                  // [Photoshop 6.0 and later] structured Unicode URL's
  kPhotoshopResourceID_VersionInfo = 0x0421,              // [Photoshop 6.0 and later] see additional Adobe info
  kPhotoshopResourceID_ExifInfo = 0x0422,                 // [Photoshop 7.0?] Exif metadata
  kPhotoshopResourceID_XMPPacket =
      0x0424,  // [Photoshop 7.0?] XMP packet -- see  http://www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf
  kPhotoshopResourceID_ClippingPathName = 0x0bb7,  // [Photoshop 6.0 and later] name of clipping path
  kPhotoshopResourceID_MorePrintFlags =
      0x2710  // [Photoshop 6.0 and later] Print flags information. 2 bytes version (=1), 1 byte center crop  marks, 1
              // byte (=0), 4 bytes bleed width value, 2 bytes bleed width  scale.
};

// *****************************************************************************
// class member definitions
namespace Exiv2 {
PsdImage::PsdImage(BasicIo::UniquePtr io) : Image(ImageType::psd, mdExif | mdIptc | mdXmp, std::move(io)) {
}  // PsdImage::PsdImage

std::string PsdImage::mimeType() const {
  return "image/x-photoshop";
}

void PsdImage::setComment(const std::string&) {
  // not supported
  throw(Error(ErrorCode::kerInvalidSettingForImage, "Image comment", "Photoshop"));
}

void PsdImage::readMetadata() {
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "Exiv2::PsdImage::readMetadata: Reading Photoshop file " << io_->path() << "\n";
#endif
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  // Ensure that this is the correct image type
  if (!isPsdType(*io_, false)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerFailedToReadImageData);
    throw Error(ErrorCode::kerNotAnImage, "Photoshop");
  }
  clearMetadata();

  /*
    The Photoshop header goes as follows -- all numbers are in big-endian byte order:

    offset  length   name       description
    ======  =======  =========  =========
     0      4 bytes  signature  always '8BPS'
     4      2 bytes  version    always equal to 1
     6      6 bytes  reserved   must be zero
    12      2 bytes  channels   number of channels in the image, including alpha channels (1 to 24)
    14      4 bytes  rows       the height of the image in pixels
    18      4 bytes  columns    the width of the image in pixels
    22      2 bytes  depth      the number of bits per channel
    24      2 bytes  mode       the color mode of the file; Supported values are: Bitmap=0; Grayscale=1;
    Indexed=2; RGB=3; CMYK=4; Multichannel=7; Duotone=8; Lab=9
  */
  byte buf[26];
  if (io_->read(buf, 26) != 26) {
    throw Error(ErrorCode::kerNotAnImage, "Photoshop");
  }
  pixelWidth_ = getLong(buf + 18, bigEndian);
  pixelHeight_ = getLong(buf + 14, bigEndian);

  // immediately following the image header is the color mode data section,
  // the first four bytes of which specify the byte size of the whole section
  if (io_->read(buf, 4) != 4) {
    throw Error(ErrorCode::kerNotAnImage, "Photoshop");
  }

  // skip it
  if (io_->seek(getULong(buf, bigEndian), BasicIo::cur)) {
    throw Error(ErrorCode::kerNotAnImage, "Photoshop");
  }

  // after the color data section, comes a list of resource blocks, preceded by the total byte size
  if (io_->read(buf, 4) != 4) {
    throw Error(ErrorCode::kerNotAnImage, "Photoshop");
  }
  uint32_t resourcesLength = getULong(buf, bigEndian);
  Internal::enforce(resourcesLength < io_->size(), Exiv2::ErrorCode::kerCorruptedMetadata);

  while (resourcesLength > 0) {
    Internal::enforce(resourcesLength >= 8, Exiv2::ErrorCode::kerCorruptedMetadata);
    resourcesLength -= 8;
    if (io_->read(buf, 8) != 8) {
      throw Error(ErrorCode::kerNotAnImage, "Photoshop");
    }

    if (!Photoshop::isIrb(buf)) {
      break;  // bad resource type
    }
    uint16_t resourceId = getUShort(buf + 4, bigEndian);
    uint32_t resourceNameLength = buf[6] & ~1;

    // skip the resource name, plus any padding
    Internal::enforce(resourceNameLength <= resourcesLength, Exiv2::ErrorCode::kerCorruptedMetadata);
    resourcesLength -= resourceNameLength;
    io_->seek(resourceNameLength, BasicIo::cur);

    // read resource size
    Internal::enforce(resourcesLength >= 4, Exiv2::ErrorCode::kerCorruptedMetadata);
    resourcesLength -= 4;
    if (io_->read(buf, 4) != 4) {
      throw Error(ErrorCode::kerNotAnImage, "Photoshop");
    }
    uint32_t resourceSize = getULong(buf, bigEndian);
    const size_t curOffset = io_->tell();

#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << std::hex << "resourceId: " << resourceId << std::dec << " length: " << resourceSize << std::hex
              << "\n";
#endif

    Internal::enforce(resourceSize <= resourcesLength, Exiv2::ErrorCode::kerCorruptedMetadata);
    readResourceBlock(resourceId, resourceSize);
    resourceSize = (resourceSize + 1) & ~1;  // pad to even
    Internal::enforce(resourceSize <= resourcesLength, Exiv2::ErrorCode::kerCorruptedMetadata);
    resourcesLength -= resourceSize;
    io_->seek(curOffset + resourceSize, BasicIo::beg);
  }

}  // PsdImage::readMetadata

void PsdImage::readResourceBlock(uint16_t resourceId, uint32_t resourceSize) {
  switch (resourceId) {
    case kPhotoshopResourceID_IPTC_NAA: {
      DataBuf rawIPTC(resourceSize);
      io_->read(rawIPTC.data(), rawIPTC.size());
      if (io_->error() || io_->eof())
        throw Error(ErrorCode::kerFailedToReadImageData);
      if (IptcParser::decode(iptcData_, rawIPTC.c_data(), rawIPTC.size())) {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Failed to decode IPTC metadata.\n";
#endif
        iptcData_.clear();
      }
      break;
    }

    case kPhotoshopResourceID_ExifInfo: {
      DataBuf rawExif(resourceSize);
      io_->read(rawExif.data(), rawExif.size());
      if (io_->error() || io_->eof())
        throw Error(ErrorCode::kerFailedToReadImageData);
      ByteOrder bo = ExifParser::decode(exifData_, rawExif.c_data(), rawExif.size());
      setByteOrder(bo);
      if (!rawExif.empty() && byteOrder() == invalidByteOrder) {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Failed to decode Exif metadata.\n";
#endif
        exifData_.clear();
      }
      break;
    }

    case kPhotoshopResourceID_XMPPacket: {
      DataBuf xmpPacket(resourceSize);
      io_->read(xmpPacket.data(), xmpPacket.size());
      if (io_->error() || io_->eof())
        throw Error(ErrorCode::kerFailedToReadImageData);
      xmpPacket_.assign(xmpPacket.c_str(), xmpPacket.size());
      if (!xmpPacket_.empty() && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
        EXV_WARNING << "Failed to decode XMP metadata.\n";
#endif
      }
      break;
    }

    // - PS 4.0 preview data is fetched from ThumbnailResource
    // - PS >= 5.0 preview data is fetched from ThumbnailResource2
    case kPhotoshopResourceID_ThumbnailResource:
    case kPhotoshopResourceID_ThumbnailResource2: {
      /*
        Photoshop thumbnail resource header

        offset  length    name            description
        ======  ========  ====            ===========
         0      4 bytes   format          = 1 (kJpegRGB). Also supports kRawRGB (0).
         4      4 bytes   width           Width of thumbnail in pixels.
         8      4 bytes   height          Height of thumbnail in pixels.
        12      4 bytes   widthbytes      Padded row bytes as (width * bitspixel + 31) / 32 * 4.
        16      4 bytes   size            Total size as widthbytes * height * planes
        20      4 bytes   compressedsize  Size after compression. Used for consistency check.
        24      2 bytes   bitspixel       = 24. Bits per pixel.
        26      2 bytes   planes          = 1. Number of planes.
        28      variable  data            JFIF data in RGB format.
                                          Note: For resource ID 1033 the data is in BGR format.
      */
      byte buf[28];
      if (io_->read(buf, 28) != 28) {
        throw Error(ErrorCode::kerNotAnImage, "Photoshop");
      }
      NativePreview nativePreview;
      nativePreview.position_ = io_->tell();
      nativePreview.size_ = getLong(buf + 20, bigEndian);  // compressedsize
      nativePreview.width_ = getLong(buf + 4, bigEndian);
      nativePreview.height_ = getLong(buf + 8, bigEndian);
      const uint32_t format = getLong(buf + 0, bigEndian);

      if (nativePreview.size_ > 0 && nativePreview.position_ > 0) {
        io_->seek(static_cast<long>(nativePreview.size_), BasicIo::cur);
        if (io_->error() || io_->eof())
          throw Error(ErrorCode::kerFailedToReadImageData);

        // unsupported format of native preview
        if (format != 1)
          break;
        nativePreview.filter_ = "";
        nativePreview.mimeType_ = "image/jpeg";
        nativePreviews_.push_back(std::move(nativePreview));
      }
      break;
    }

    default:
      break;
  }
}  // PsdImage::readResourceBlock

void PsdImage::writeMetadata() {
  if (io_->open() != 0) {
    throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
  }
  IoCloser closer(*io_);
  MemIo tempIo;

  doWriteMetadata(tempIo);  // may throw
  io_->close();
  io_->transfer(tempIo);  // may throw

}  // PsdImage::writeMetadata

void PsdImage::doWriteMetadata(BasicIo& outIo) {
  if (!io_->isopen())
    throw Error(ErrorCode::kerInputDataReadFailed);
  if (!outIo.isopen())
    throw Error(ErrorCode::kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cout << "Exiv2::PsdImage::doWriteMetadata: Writing PSD file " << io_->path() << "\n";
  std::cout << "Exiv2::PsdImage::doWriteMetadata: tmp file created " << outIo.path() << "\n";
#endif

  // Ensure that this is the correct image type
  if (!isPsdType(*io_, true)) {
    if (io_->error() || io_->eof())
      throw Error(ErrorCode::kerInputDataReadFailed);
    throw Error(ErrorCode::kerNoImageInInputData);
  }

  io_->seek(0, BasicIo::beg);  // rewind

  DataBuf lbuf(4096);
  byte buf[8];

  // Get Photoshop header from original file
  byte psd_head[26];
  if (io_->read(psd_head, 26) != 26)
    throw Error(ErrorCode::kerNotAnImage, "Photoshop");

  // Write Photoshop header data out to new PSD file
  if (outIo.write(psd_head, 26) != 26)
    throw Error(ErrorCode::kerImageWriteFailed);

  // Read colorDataLength from original PSD
  if (io_->read(buf, 4) != 4)
    throw Error(ErrorCode::kerNotAnImage, "Photoshop");

  uint32_t colorDataLength = getULong(buf, bigEndian);

  // Write colorDataLength
  ul2Data(buf, colorDataLength, bigEndian);
  if (outIo.write(buf, 4) != 4)
    throw Error(ErrorCode::kerImageWriteFailed);
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << std::dec << "colorDataLength: " << colorDataLength << "\n";
#endif
  // Copy colorData
  size_t readTotal = 0;
  while (readTotal < colorDataLength) {
    size_t toRead =
        (colorDataLength - readTotal) < lbuf.size() ? static_cast<size_t>(colorDataLength) - readTotal : lbuf.size();
    if (io_->read(lbuf.data(), toRead) != toRead)
      throw Error(ErrorCode::kerNotAnImage, "Photoshop");
    readTotal += toRead;
    if (outIo.write(lbuf.c_data(), toRead) != toRead)
      throw Error(ErrorCode::kerImageWriteFailed);
  }
  if (outIo.error())
    throw Error(ErrorCode::kerImageWriteFailed);

  const size_t resLenOffset = io_->tell();  // remember for later update

  // Read length of all resource blocks from original PSD
  if (io_->read(buf, 4) != 4)
    throw Error(ErrorCode::kerNotAnImage, "Photoshop");

  uint32_t oldResLength = getULong(buf, bigEndian);
  uint32_t newResLength = 0;

  // Write oldResLength (will be updated later)
  ul2Data(buf, oldResLength, bigEndian);
  if (outIo.write(buf, 4) != 4)
    throw Error(ErrorCode::kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << std::dec << "oldResLength: " << oldResLength << "\n";
#endif

  // Iterate over original resource blocks.
  // Replace or insert IPTC, EXIF and XMP
  // Original resource blocks assumed to be sorted ASC

  bool iptcDone = false;
  bool exifDone = false;
  bool xmpDone = false;
  while (oldResLength > 0) {
    if (io_->read(buf, 8) != 8)
      throw Error(ErrorCode::kerNotAnImage, "Photoshop");

    // read resource type and ID
    uint32_t resourceType = getULong(buf, bigEndian);

    if (!Photoshop::isIrb(buf)) {
      throw Error(ErrorCode::kerNotAnImage, "Photoshop");  // bad resource type
    }
    uint16_t resourceId = getUShort(buf + 4, bigEndian);
    uint32_t resourceNameLength = buf[6];
    uint32_t adjResourceNameLen = resourceNameLength & ~1;
    unsigned char resourceNameFirstChar = buf[7];

    // read rest of resource name, plus any padding
    DataBuf resName(256);
    if (io_->read(resName.data(), adjResourceNameLen) != adjResourceNameLen)
      throw Error(ErrorCode::kerNotAnImage, "Photoshop");

    // read resource size (actual length w/o padding!)
    if (io_->read(buf, 4) != 4)
      throw Error(ErrorCode::kerNotAnImage, "Photoshop");

    uint32_t resourceSize = getULong(buf, bigEndian);
    uint32_t pResourceSize = (resourceSize + 1) & ~1;  // padded resource size
    const size_t curOffset = io_->tell();

    // Write IPTC_NAA resource block
    if ((resourceId == kPhotoshopResourceID_IPTC_NAA || resourceId > kPhotoshopResourceID_IPTC_NAA) && !iptcDone) {
      newResLength += writeIptcData(iptcData_, outIo);
      iptcDone = true;
    }

    // Write ExifInfo resource block
    else if ((resourceId == kPhotoshopResourceID_ExifInfo || resourceId > kPhotoshopResourceID_ExifInfo) && !exifDone) {
      newResLength += writeExifData(exifData_, outIo);
      exifDone = true;
    }

    // Write XMPpacket resource block
    else if ((resourceId == kPhotoshopResourceID_XMPPacket || resourceId > kPhotoshopResourceID_XMPPacket) &&
             !xmpDone) {
      newResLength += writeXmpData(xmpData_, outIo);
      xmpDone = true;
    }

    // Copy all other resource blocks
    if (resourceId != kPhotoshopResourceID_IPTC_NAA && resourceId != kPhotoshopResourceID_ExifInfo &&
        resourceId != kPhotoshopResourceID_XMPPacket) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << std::hex << "copy : resourceType: " << resourceType << "\n";
      std::cerr << std::hex << "copy : resourceId: " << resourceId << "\n";
      std::cerr << std::dec;
#endif
      // Copy resource block to new PSD file
      ul2Data(buf, resourceType, bigEndian);
      if (outIo.write(buf, 4) != 4)
        throw Error(ErrorCode::kerImageWriteFailed);
      us2Data(buf, resourceId, bigEndian);
      if (outIo.write(buf, 2) != 2)
        throw Error(ErrorCode::kerImageWriteFailed);
      // Write resource name as Pascal string
      buf[0] = resourceNameLength & 0x00ff;
      if (outIo.write(buf, 1) != 1)
        throw Error(ErrorCode::kerImageWriteFailed);
      buf[0] = resourceNameFirstChar;
      if (outIo.write(buf, 1) != 1)
        throw Error(ErrorCode::kerImageWriteFailed);
      if (outIo.write(resName.c_data(), adjResourceNameLen) != static_cast<size_t>(adjResourceNameLen))
        throw Error(ErrorCode::kerImageWriteFailed);
      ul2Data(buf, resourceSize, bigEndian);
      if (outIo.write(buf, 4) != 4)
        throw Error(ErrorCode::kerImageWriteFailed);

      readTotal = 0;
      while (readTotal < pResourceSize) {
        /// \todo almost same code as in lines 403-410. Factor out & reuse!
        size_t toRead = (pResourceSize - readTotal) < lbuf.size() ? pResourceSize - readTotal : lbuf.size();
        if (io_->read(lbuf.data(), toRead) != toRead) {
          throw Error(ErrorCode::kerNotAnImage, "Photoshop");
        }
        readTotal += toRead;
        if (outIo.write(lbuf.c_data(), toRead) != toRead)
          throw Error(ErrorCode::kerImageWriteFailed);
      }
      if (outIo.error())
        throw Error(ErrorCode::kerImageWriteFailed);
      newResLength += pResourceSize + adjResourceNameLen + 12;
    }

    io_->seek(curOffset + pResourceSize, BasicIo::beg);
    oldResLength -= (12 + adjResourceNameLen + pResourceSize);
  }

  // Append IPTC_NAA resource block, if not yet written
  if (!iptcDone) {
    newResLength += writeIptcData(iptcData_, outIo);
    iptcDone = true;
  }

  // Append ExifInfo resource block, if not yet written
  if (!exifDone) {
    newResLength += writeExifData(exifData_, outIo);
  }

  // Append XmpPacket resource block, if not yet written
  if (!xmpDone) {
    newResLength += writeXmpData(xmpData_, outIo);
  }

  // Populate the fake data, only make sense for remoteio, httpio and sshio.
  // it avoids allocating memory for parts of the file that contain image-date.
  io_->populateFakeData();

  // Copy remaining data
  size_t readSize = io_->read(lbuf.data(), lbuf.size());
  while (readSize != 0) {
    if (outIo.write(lbuf.c_data(), readSize) != readSize)
      throw Error(ErrorCode::kerImageWriteFailed);
    readSize = io_->read(lbuf.data(), lbuf.size());
  }
  if (outIo.error())
    throw Error(ErrorCode::kerImageWriteFailed);

    // Update length of resources
#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "newResLength: " << newResLength << "\n";
#endif
  outIo.seek(resLenOffset, BasicIo::beg);
  ul2Data(buf, newResLength, bigEndian);
  if (outIo.write(buf, 4) != 4)
    throw Error(ErrorCode::kerImageWriteFailed);

}  // PsdImage::doWriteMetadata

uint32_t PsdImage::writeIptcData(const IptcData& iptcData, BasicIo& out) {
  uint32_t resLength = 0;
  byte buf[8];

  if (!iptcData.empty()) {
    DataBuf rawIptc = IptcParser::encode(iptcData);
    if (!rawIptc.empty()) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << std::hex << "write: resourceId: " << kPhotoshopResourceID_IPTC_NAA << "\n";
      std::cerr << std::dec << "Writing IPTC_NAA: size: " << rawIptc.size() << "\n";
#endif
      if (out.write(reinterpret_cast<const byte*>(Photoshop::irbId_.front()), 4) != 4)
        throw Error(ErrorCode::kerImageWriteFailed);
      us2Data(buf, kPhotoshopResourceID_IPTC_NAA, bigEndian);
      if (out.write(buf, 2) != 2)
        throw Error(ErrorCode::kerImageWriteFailed);
      us2Data(buf, 0, bigEndian);  // NULL resource name
      if (out.write(buf, 2) != 2)
        throw Error(ErrorCode::kerImageWriteFailed);
      ul2Data(buf, static_cast<uint32_t>(rawIptc.size()), bigEndian);
      if (out.write(buf, 4) != 4)
        throw Error(ErrorCode::kerImageWriteFailed);
      // Write encoded Iptc data
      if (out.write(rawIptc.c_data(), rawIptc.size()) != rawIptc.size())
        throw Error(ErrorCode::kerImageWriteFailed);
      resLength += static_cast<uint32_t>(rawIptc.size()) + 12;
      if (rawIptc.size() & 1)  // even padding
      {
        buf[0] = 0;
        if (out.write(buf, 1) != 1)
          throw Error(ErrorCode::kerImageWriteFailed);
        resLength++;
      }
    }
  }
  return resLength;
}  // PsdImage::writeIptcData

uint32_t PsdImage::writeExifData(ExifData& exifData, BasicIo& out) {
  uint32_t resLength = 0;
  byte buf[8];

  if (!exifData.empty()) {
    Blob blob;
    ByteOrder bo = byteOrder();
    if (bo == invalidByteOrder) {
      bo = littleEndian;
      setByteOrder(bo);
    }
    ExifParser::encode(blob, bo, exifData);

    if (!blob.empty()) {
#ifdef EXIV2_DEBUG_MESSAGES
      std::cerr << std::hex << "write: resourceId: " << kPhotoshopResourceID_ExifInfo << "\n";
      std::cerr << std::dec << "Writing ExifInfo: size: " << blob.size() << "\n";
#endif
      if (out.write(reinterpret_cast<const byte*>(Photoshop::irbId_.front()), 4) != 4)
        throw Error(ErrorCode::kerImageWriteFailed);
      us2Data(buf, kPhotoshopResourceID_ExifInfo, bigEndian);
      if (out.write(buf, 2) != 2)
        throw Error(ErrorCode::kerImageWriteFailed);
      us2Data(buf, 0, bigEndian);  // NULL resource name
      if (out.write(buf, 2) != 2)
        throw Error(ErrorCode::kerImageWriteFailed);
      ul2Data(buf, static_cast<uint32_t>(blob.size()), bigEndian);
      if (out.write(buf, 4) != 4)
        throw Error(ErrorCode::kerImageWriteFailed);
      // Write encoded Exif data
      if (out.write(blob.data(), blob.size()) != blob.size())
        throw Error(ErrorCode::kerImageWriteFailed);
      resLength += static_cast<long>(blob.size()) + 12;
      if (blob.size() & 1)  // even padding
      {
        buf[0] = 0;
        if (out.write(buf, 1) != 1)
          throw Error(ErrorCode::kerImageWriteFailed);
        resLength++;
      }
    }
  }
  return resLength;
}  // PsdImage::writeExifData

uint32_t PsdImage::writeXmpData(const XmpData& xmpData, BasicIo& out) const {
  std::string xmpPacket;
  uint32_t resLength = 0;
  byte buf[8];

#ifdef EXIV2_DEBUG_MESSAGES
  std::cerr << "writeXmpFromPacket(): " << writeXmpFromPacket() << "\n";
#endif
  //        writeXmpFromPacket(true);
  if (!writeXmpFromPacket() && XmpParser::encode(xmpPacket, xmpData) > 1) {
#ifndef SUPPRESS_WARNINGS
    EXV_ERROR << "Failed to encode XMP metadata.\n";
#endif
  }

  if (!xmpPacket.empty()) {
#ifdef EXIV2_DEBUG_MESSAGES
    std::cerr << std::hex << "write: resourceId: " << kPhotoshopResourceID_XMPPacket << "\n";
    std::cerr << std::dec << "Writing XMPPacket: size: " << xmpPacket.size() << "\n";
#endif
    if (out.write(reinterpret_cast<const byte*>(Photoshop::irbId_.front()), 4) != 4)
      throw Error(ErrorCode::kerImageWriteFailed);
    us2Data(buf, kPhotoshopResourceID_XMPPacket, bigEndian);
    if (out.write(buf, 2) != 2)
      throw Error(ErrorCode::kerImageWriteFailed);
    us2Data(buf, 0, bigEndian);  // NULL resource name
    if (out.write(buf, 2) != 2)
      throw Error(ErrorCode::kerImageWriteFailed);
    ul2Data(buf, static_cast<uint32_t>(xmpPacket.size()), bigEndian);
    if (out.write(buf, 4) != 4)
      throw Error(ErrorCode::kerImageWriteFailed);
    // Write XMPPacket
    if (out.write(reinterpret_cast<const byte*>(xmpPacket.data()), xmpPacket.size()) != xmpPacket.size())
      throw Error(ErrorCode::kerImageWriteFailed);
    if (out.error())
      throw Error(ErrorCode::kerImageWriteFailed);
    resLength += static_cast<uint32_t>(xmpPacket.size()) + 12;
    if (xmpPacket.size() & 1)  // even padding
    {
      buf[0] = 0;
      if (out.write(buf, 1) != 1)
        throw Error(ErrorCode::kerImageWriteFailed);
      resLength++;
    }
  }
  return resLength;
}  // PsdImage::writeXmpData

// *************************************************************************
// free functions
Image::UniquePtr newPsdInstance(BasicIo::UniquePtr io, bool /*create*/) {
  auto image = std::make_unique<PsdImage>(std::move(io));
  if (!image->good()) {
    return nullptr;
  }
  return image;
}

bool isPsdType(BasicIo& iIo, bool advance) {
  const int32_t len = 6;
  const unsigned char PsdHeader[6] = {'8', 'B', 'P', 'S', 0, 1};
  byte buf[len];
  iIo.read(buf, len);
  if (iIo.error() || iIo.eof()) {
    return false;
  }
  bool matched = (memcmp(buf, PsdHeader, len) == 0);
  if (!advance || !matched) {
    iIo.seek(-len, BasicIo::cur);
  }

  return matched;
}
}  // namespace Exiv2
