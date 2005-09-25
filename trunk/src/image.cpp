// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
  File:      image.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
             Brad Schick (brad) <brad@robotbattle.com>
  History:   26-Jan-04, ahu: created
             11-Feb-04, ahu: isolated as a component
             19-Jul-04, brad: revamped to be more flexible and support Iptc
             15-Jan-05, brad: inside-out design changes
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "image.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef _MSC_VER
# define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#endif
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                            // stat
#endif

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    const ImageFactory::Registry* ImageFactory::find(int imageType)
    {
        for (unsigned int i = 0; registry_[i].imageType_ != ImageType::none; ++i) {
            if (registry_[i].imageType_ == imageType) return &registry_[i];
        }
        return 0;
    }

    void ImageFactory::registerImage(int            type, 
                                     NewInstanceFct newInst, 
                                     IsThisTypeFct  isType)
    {
        unsigned int i = 0;
        for (; i < MAX_IMAGE_FORMATS; ++i) {
            if (registry_[i].imageType_ == ImageType::none) {
                registry_[i] = Registry(type, newInst, isType);
                break;
            }
        }
        if (i == MAX_IMAGE_FORMATS) throw Error(35);
    }

    int ImageFactory::getType(const std::string& path)
    {
        FileIo fileIo(path);
        return getType(fileIo);
    }

    int ImageFactory::getType(const byte* data, long size)
    {
        MemIo memIo(data, size);
        return getType(memIo);
    }

    int ImageFactory::getType(BasicIo& io)
    {
        if (io.open() != 0) return ImageType::none; 
        IoCloser closer(io);
        for (unsigned int i = 0; registry_[i].imageType_ != ImageType::none; ++i) {
            if (registry_[i].isThisType_(io, false)) {
                return registry_[i].imageType_;
            }
        }
        return ImageType::none;
    } // ImageFactory::getType

    Image::AutoPtr ImageFactory::open(const std::string& path)
    {
        BasicIo::AutoPtr io(new FileIo(path));
        Image::AutoPtr image = open(io); // may throw
        if (image.get() == 0) throw Error(11, path);
        return image;
    }

    Image::AutoPtr ImageFactory::open(const byte* data, long size)
    {
        BasicIo::AutoPtr io(new MemIo(data, size));
        Image::AutoPtr image = open(io); // may throw
        if (image.get() == 0) throw Error(12);
        return image;
    }

    Image::AutoPtr ImageFactory::open(BasicIo::AutoPtr io)
    {
        if (io->open() != 0) {
            throw Error(9, io->path(), strError());
        }
        for (unsigned int i = 0; registry_[i].imageType_ != ImageType::none; ++i) {
            if (registry_[i].isThisType_(*io, false)) {
                return registry_[i].newInstance_(io, false);
            }
        }
        return Image::AutoPtr();
    } // ImageFactory::open

    Image::AutoPtr ImageFactory::create(int type, 
                                        const std::string& path)
    {
        std::auto_ptr<FileIo> fileIo(new FileIo(path));
        // Create or overwrite the file, then close it
        if (fileIo->open("w+b") != 0) {
            throw Error(10, path, "w+b", strError());
        }
        fileIo->close();
        BasicIo::AutoPtr io(fileIo);
        Image::AutoPtr image = create(type, io);
        if (image.get() == 0) throw Error(13, type);
        return image;
    }

    Image::AutoPtr ImageFactory::create(int type)
    {
        BasicIo::AutoPtr io(new MemIo);
        Image::AutoPtr image = create(type, io);
        if (image.get() == 0) throw Error(13, type);
        return image;
    }

    Image::AutoPtr ImageFactory::create(int type, 
                                        BasicIo::AutoPtr io)
    {
        // BasicIo instance does not need to be open
        const Registry* r = find(type);
        if (0 != r) {
            return r->newInstance_(io, true);
        }
        return Image::AutoPtr();
    } // ImageFactory::create

    TiffHeader::TiffHeader(ByteOrder byteOrder) 
        : byteOrder_(byteOrder), tag_(0x002a), offset_(0x00000008)
    {
    }

    int TiffHeader::read(const byte* buf)
    {
        if (buf[0] == 0x49 && buf[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (buf[0] == 0x4d && buf[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            return 1;
        }
        tag_ = getUShort(buf+2, byteOrder_);
        offset_ = getULong(buf+4, byteOrder_);
        return 0;
    }

    long TiffHeader::copy(byte* buf) const
    {
        switch (byteOrder_) {
        case littleEndian:
            buf[0] = 0x49;
            buf[1] = 0x49;
            break;
        case bigEndian:
            buf[0] = 0x4d;
            buf[1] = 0x4d;
            break;
        case invalidByteOrder:
            // do nothing
            break;
        }
        us2Data(buf+2, 0x002a, byteOrder_);
        ul2Data(buf+4, 0x00000008, byteOrder_);
        return size();
    } // TiffHeader::copy

}                                       // namespace Exiv2
