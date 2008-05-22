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
  File:      xmpsidecar.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   07-Mar-08, ahu: created
  Credits:   See header file
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

#include "xmpsidecar.hpp"
#include "image.hpp"
#include "basicio.hpp"
#include "error.hpp"
#include "xmp.hpp"
#include "futils.hpp"
#include "convert.hpp"

// + standard includes
#include <string>
#include <iostream>
#include <cassert>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const char* XmpSidecar::xmlHeader_ = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    const long XmpSidecar::xmlHdrCnt_ = 39;

    XmpSidecar::XmpSidecar(BasicIo::AutoPtr io, bool create)
        : Image(ImageType::xmp, mdXmp, io)
    {
        if (create) {
            if (io_->open() == 0) {
                IoCloser closer(*io_);
                io_->write(reinterpret_cast<const byte*>(xmlHeader_), xmlHdrCnt_);
            }
        }
    } // XmpSidecar::XmpSidecar

    void XmpSidecar::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "XMP"));
    }

    void XmpSidecar::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading XMP file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isXmpType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "XMP");
        }
        // Read the XMP packet from the IO stream
        std::string xmpPacket;
        const long len = 64 * 1024;
        byte buf[len];
        long l;
        while ((l = io_->read(buf, len)) > 0) {
            xmpPacket.append(reinterpret_cast<char*>(buf), l);
        }
        if (io_->error()) throw Error(14);
        clearMetadata();
        xmpPacket_ = xmpPacket;
        if (xmpPacket_.size() > 0 && XmpParser::decode(xmpData_, xmpPacket_)) {
#ifndef SUPPRESS_WARNINGS
            std::cerr << "Warning: Failed to decode XMP metadata.\n";
#endif
        }
        copyXmpToIptc(xmpData_, iptcData_);
        copyXmpToExif(xmpData_, exifData_);
    } // XmpSidecar::readMetadata

    void XmpSidecar::writeMetadata()
    {
        if (io_->open() != 0) {
            throw Error(9, io_->path(), strError());
        }
        IoCloser closer(*io_);

        if (writeXmpFromPacket() == false) {
            copyExifToXmp(exifData_, xmpData_);
            copyIptcToXmp(iptcData_, xmpData_);
            if (XmpParser::encode(xmpPacket_, xmpData_,
                                  XmpParser::omitPacketWrapper|XmpParser::useCompactFormat)) {
#ifndef SUPPRESS_WARNINGS
                std::cerr << "Error: Failed to encode XMP metadata.\n";
#endif
            }
        }
        if (xmpPacket_.size() > 0) {
            if (xmpPacket_.substr(0, 5)  != "<?xml") {
                xmpPacket_ = xmlHeader_ + xmpPacket_;
            }
            BasicIo::AutoPtr tempIo(io_->temporary()); // may throw
            assert(tempIo.get() != 0);
            // Write XMP packet
            if (   tempIo->write(reinterpret_cast<const byte*>(xmpPacket_.data()),
                                 static_cast<long>(xmpPacket_.size()))
                != static_cast<long>(xmpPacket_.size())) throw Error(21);
            if (tempIo->error()) throw Error(21);
            io_->close();
            io_->transfer(*tempIo); // may throw
        }
    } // XmpSidecar::writeMetadata

    // *************************************************************************
    // free functions
    Image::AutoPtr newXmpInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new XmpSidecar(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isXmpType(BasicIo& iIo, bool advance)
    {
        /*
          Make sure the file starts with and (optional) XML declaration,
          followed by an XMP header (<?xpacket ... ?>) or an <x:xmpmeta>
          element. That doesn't cover all cases, since also x:xmpmeta is
          optional, but let's wait and see.
         */

        // Todo: Proper implementation

        const int32_t len = 10;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        bool rc = false;
        const std::string head(reinterpret_cast<const char*>(buf), len);
        if (   head.substr(0, 5)  == "<?xml"
            || head.substr(0, 9)  == "<?xpacket"
            || head.substr(0, 10) == "<x:xmpmeta") {
            rc = true;
        }
        if (!advance || !rc) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc;

    }

}                                       // namespace Exiv2
