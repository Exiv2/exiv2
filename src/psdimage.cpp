// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
  File:      psdimage.cpp
  Version:   $Rev$
  Author(s): Marco Piovanelli, Ovolab (marco)
  History:   05-Mar-2007, marco: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

//#define DEBUG 1

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif
#include "psdimage.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <string>
#include <cstring>
#include <iostream>

// Todo: Consolidate with existing code in struct Photoshop (jpgimage.hpp):
//       Extend this helper to a proper class with all required functionality,
//       then move it here or into a separate file?

const uint32_t kPhotoshopResourceType = 0x3842494d; // '8BIM'

//! @cond IGNORE
struct PhotoshopResourceBlock {
    uint32_t      resourceType;       // always kPhotoshopResourceType
    uint16_t      resourceId;
    unsigned char resourceName[2];    // Pascal string (length byte + characters), padded to an even size -- this assumes the empty string
    uint32_t      resourceDataSize;
};
//! @endcond

// Photoshop resource IDs (Cf. <http://search.cpan.org/~bettelli/Image-MetaData-JPEG-0.15/lib/Image/MetaData/JPEG/TagLists.pod>)
enum {
    kPhotoshopResourceID_Photoshop2Info            = 0x03e8, // [obsolete -- Photoshop 2.0 only] General information -- contains five 2-byte values: number of channels, rows, columns, depth and mode
    kPhotoshopResourceID_MacintoshClassicPrintInfo = 0x03e9, // [optional] Macintosh classic print record (120 bytes)
    kPhotoshopResourceID_MacintoshCarbonPrintInfo  = 0x03ea, // [optional] Macintosh carbon print info (variable-length XML format)
    kPhotoshopResourceID_Photoshop2ColorTable      = 0x03eb, // [obsolete -- Photoshop 2.0 only] Indexed color table
    kPhotoshopResourceID_ResolutionInfo            = 0x03ed, // PhotoshopResolutionInfo structure (see below)
    kPhotoshopResourceID_AlphaChannelsNames        = 0x03ee, // as a series of Pstrings
    kPhotoshopResourceID_DisplayInfo               = 0x03ef, // see appendix A in Photoshop SDK
    kPhotoshopResourceID_PStringCaption            = 0x03f0, // [optional] the caption, as a Pstring
    kPhotoshopResourceID_BorderInformation         = 0x03f1, // border width and units 
    kPhotoshopResourceID_BackgroundColor           = 0x03f2, // see additional Adobe information
    kPhotoshopResourceID_PrintFlags                = 0x03f3, // labels, crop marks, colour bars, ecc...
    kPhotoshopResourceID_BWHalftoningInfo          = 0x03f4, // Gray-scale and multich. half-toning info
    kPhotoshopResourceID_ColorHalftoningInfo       = 0x03f5, // Colour half-toning information
    kPhotoshopResourceID_DuotoneHalftoningInfo     = 0x03f6, // Duo-tone half-toning information
    kPhotoshopResourceID_BWTransferFunc            = 0x03f7, // Gray-scale and multich. transfer function
    kPhotoshopResourceID_ColorTransferFuncs        = 0x03f8, // Colour transfer function
    kPhotoshopResourceID_DuotoneTransferFuncs      = 0x03f9, // Duo-tone transfer function
    kPhotoshopResourceID_DuotoneImageInfo          = 0x03fa, // Duo-tone image information
    kPhotoshopResourceID_EffectiveBW               = 0x03fb, // two bytes for the effective black and white values
    kPhotoshopResourceID_ObsoletePhotoshopTag1     = 0x03fc, // [obsolete]
    kPhotoshopResourceID_EPSOptions                = 0x03fd, // Encapsulated Postscript options
    kPhotoshopResourceID_QuickMaskInfo             = 0x03fe, // Quick Mask information. 2 bytes containing Quick Mask channel ID,  1 byte boolean indicating whether the mask was initially empty.
    kPhotoshopResourceID_ObsoletePhotoshopTag2     = 0x03ff, // [obsolete]
    kPhotoshopResourceID_LayerStateInfo            = 0x0400, // index of target layer (0 means bottom)
    kPhotoshopResourceID_WorkingPathInfo           = 0x0401, // should not be saved to the file
    kPhotoshopResourceID_LayersGroupInfo           = 0x0402, // for grouping layers together
    kPhotoshopResourceID_ObsoletePhotoshopTag3     = 0x0403, // [obsolete] ??
    kPhotoshopResourceID_IPTC_NAA                  = 0x0404, // IPTC/NAA data
    kPhotoshopResourceID_RawImageMode              = 0x0405, // image mode for raw format files
    kPhotoshopResourceID_JPEGQuality               = 0x0406, // [private]
    kPhotoshopResourceID_GridGuidesInfo            = 0x0408, // see additional Adobe information
    kPhotoshopResourceID_ThumbnailResource         = 0x0409, // see additional Adobe information
    kPhotoshopResourceID_CopyrightFlag             = 0x040a, // true if image is copyrighted
    kPhotoshopResourceID_URL                       = 0x040b, // text string with a resource locator
    kPhotoshopResourceID_ThumbnailResource2        = 0x040c, // see additional Adobe information
    kPhotoshopResourceID_GlobalAngle               = 0x040d, // global lighting angle for effects layer
    kPhotoshopResourceID_ColorSamplersResource     = 0x040e, // see additional Adobe information
    kPhotoshopResourceID_ICCProfile                = 0x040f, // see notes from Internat. Color Consortium
    kPhotoshopResourceID_Watermark                 = 0x0410, // one byte
    kPhotoshopResourceID_ICCUntagged               = 0x0411, // 1 means intentionally untagged
    kPhotoshopResourceID_EffectsVisible            = 0x0412, // 1 byte to show/hide all effects layers
    kPhotoshopResourceID_SpotHalftone              = 0x0413, // version, length and data
    kPhotoshopResourceID_IDsBaseValue              = 0x0414, // base value for new layers ID's
    kPhotoshopResourceID_UnicodeAlphaNames         = 0x0415, // length plus Unicode string
    kPhotoshopResourceID_IndexedColourTableCount   = 0x0416, // [Photoshop 6.0 and later] 2 bytes
    kPhotoshopResourceID_TransparentIndex          = 0x0417, // [Photoshop 6.0 and later] 2 bytes
    kPhotoshopResourceID_GlobalAltitude            = 0x0419, // [Photoshop 6.0 and later] 4 bytes
    kPhotoshopResourceID_Slices                    = 0x041a, // [Photoshop 6.0 and later] see additional Adobe info
    kPhotoshopResourceID_WorkflowURL               = 0x041b, // [Photoshop 6.0 and later] 4 bytes length + Unicode string
    kPhotoshopResourceID_JumpToXPEP                = 0x041c, // [Photoshop 6.0 and later] see additional Adobe info
    kPhotoshopResourceID_AlphaIdentifiers          = 0x041d, // [Photoshop 6.0 and later] 4*(n+1) bytes
    kPhotoshopResourceID_URLList                   = 0x041e, // [Photoshop 6.0 and later] structured Unicode URL's
    kPhotoshopResourceID_VersionInfo               = 0x0421, // [Photoshop 6.0 and later] see additional Adobe info
    kPhotoshopResourceID_ExifInfo                  = 0x0422, // [Photoshop 7.0?] Exif metadata
    kPhotoshopResourceID_XMPPacket                 = 0x0424, // [Photoshop 7.0?] XMP packet -- see  http://www.adobe.com/devnet/xmp/pdfs/xmp_specification.pdf
    kPhotoshopResourceID_ClippingPathName          = 0x0bb7, // [Photoshop 6.0 and later] name of clipping path
    kPhotoshopResourceID_MorePrintFlags            = 0x2710  // [Photoshop 6.0 and later] Print flags information. 2 bytes version (=1), 1 byte center crop  marks, 1 byte (=0), 4 bytes bleed width value, 2 bytes bleed width  scale.
};

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    PsdImage::PsdImage(BasicIo::AutoPtr io)
        : Image(ImageType::psd, mdExif | mdIptc | mdXmp, io)
    {
    } // PsdImage::PsdImage

    void PsdImage::setExifData(const ExifData& /*exifData*/)
    {
        // Todo: implement me!
        throw(Error(32, "Exif metadata", "Photoshop"));
    }

    void PsdImage::setIptcData(const IptcData& /*iptcData*/)
    {
        // Todo: implement me!
        throw(Error(32, "IPTC metadata", "Photoshop"));
    }

    void PsdImage::setComment(const std::string& /*comment*/)
    {
        // Todo: implement me!
        throw(Error(32, "Image comment", "Photoshop"));
    }

    void PsdImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Exiv2::PsdImage::readMetadata: Reading Photoshop file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) 
        {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isPsdType(*io_, false)) 
        {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "Photoshop");
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
          24      2 bytes  mode       the color mode of the file; Supported values are: Bitmap=0; Grayscale=1; Indexed=2; RGB=3; CMYK=4; Multichannel=7; Duotone=8; Lab=9
        */
        byte buf[26];
        if (io_->read(buf, 26) != 26)
        {
            throw Error(3, "Photoshop");
        }
        pixelWidth_ = getLong(buf + 18, bigEndian);
        pixelHeight_ = getLong(buf + 14, bigEndian);

        // immediately following the image header is the color mode data section,
        // the first four bytes of which specify the byte size of the whole section
        if (io_->read(buf, 4) != 4)
        {
            throw Error(3, "Photoshop");
        }

        // skip it
        uint32_t colorDataLength = getLong(buf, bigEndian);
        if (io_->seek(colorDataLength, BasicIo::cur))
        {
            throw Error(3, "Photoshop");
        }

        // after the color data section, comes a list of resource blocks, preceeded by the total byte size
        if (io_->read(buf, 4) != 4)
        {
            throw Error(3, "Photoshop");
        }
        uint32_t resourcesLength = getLong(buf, bigEndian);
        while (resourcesLength > 0)
        {
            if (io_->read(buf, 8) != 8)
            {
                throw Error(3, "Photoshop");
            }

            // read resource type and ID
            uint32_t resourceType = getLong(buf, bigEndian);
            uint16_t resourceId = getShort(buf + 4, bigEndian);

            if (resourceType != kPhotoshopResourceType)
            {
                break; // bad resource type
            }
            uint32_t resourceNameLength = buf[6] & ~1;

            // skip the resource name, plus any padding
            io_->seek(resourceNameLength, BasicIo::cur);

            // read resource size
            if (io_->read(buf, 4) != 4)
            {
                throw Error(3, "Photoshop");
            }
            uint32_t resourceSize = getLong(buf, bigEndian);
            uint32_t curOffset = io_->tell();

            processResourceBlock(resourceId, resourceSize);
            resourceSize = (resourceSize + 1) & ~1;        // pad to even
            io_->seek(curOffset + resourceSize, BasicIo::beg);
            resourcesLength -= (12 + resourceNameLength + resourceSize);
        }

    } // PsdImage::readMetadata

    void PsdImage::processResourceBlock(uint16_t resourceId, uint32_t resourceSize)
    {
        switch(resourceId)
        {
            case kPhotoshopResourceID_IPTC_NAA:
            {
                DataBuf rawIPTC(resourceSize);
                io_->read(rawIPTC.pData_, rawIPTC.size_);
                if (io_->error() || io_->eof()) throw Error(14);
                if (iptcData_.load(rawIPTC.pData_, rawIPTC.size_)) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: Failed to decode IPTC metadata.\n";
#endif
                    iptcData_.clear();
                }
                break;
            }

            case kPhotoshopResourceID_ExifInfo:
            {
                DataBuf rawExif(resourceSize);
                io_->read(rawExif.pData_, rawExif.size_);
                if (io_->error() || io_->eof()) throw Error(14);
                if (exifData_.load(rawExif.pData_, rawExif.size_)) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: Failed to decode Exif metadata.\n";
#endif
                    exifData_.clear();
                }
                break;
            }

            case kPhotoshopResourceID_XMPPacket:
            {
                DataBuf xmpPacket(resourceSize);
                io_->read(xmpPacket.pData_, xmpPacket.size_);
                if (io_->error() || io_->eof()) throw Error(14);
                xmpPacket_.assign(reinterpret_cast<char *>(xmpPacket.pData_), xmpPacket.size_);
                if (xmpPacket_.size() > 0 && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
                    std::cerr << "Warning: Failed to decode XMP metadata.\n";
#endif
                }
                break;
            }

            default:
            {
                break;
            }
        }
    }

    void PsdImage::writeMetadata()
    {
        // Todo: implement me!
        throw(Error(31, "Photoshop"));
    } // PsdImage::writeMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newPsdInstance(BasicIo::AutoPtr io, bool /*create*/)
    {
        Image::AutoPtr image(new PsdImage(io));
        if (!image->good()) 
        {
            image.reset();
        }
        return image;
    }

    bool isPsdType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 6;
        const unsigned char PsdHeader[6] = { '8', 'B', 'P', 'S', 0, 1 };
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) 
        {
            return false;
        }
        bool matched = (memcmp(buf, PsdHeader, len) == 0);
        if (!advance || !matched) 
        {
            iIo.seek(-len, BasicIo::cur);
        }

        return matched;
    }
}                                       // namespace Exiv2
