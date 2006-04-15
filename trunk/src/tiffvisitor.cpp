// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2006 Andreas Huggel <ahuggel@gmx.net>
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
  File:      tiffvisitor.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   11-Apr-06, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$");

// Define DEBUG to output debug information to std::cerr, e.g, by calling make
// like this: make DEFS=-DDEBUG tiffvisitor.o
//#define DEBUG

// *****************************************************************************
// included header files
#ifdef _MSC_VER
# include "exv_msvc.h"
#else
# include "exv_conf.h"
#endif

#include "tiffvisitor.hpp"
#include "tiffcomposite.hpp"
#include "makernote2.hpp"
#include "exif.hpp"
#include "image.hpp"

// + standard includes
#include <string>

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    void TiffFinder::init(uint16_t tag, uint16_t group)
    {
        tag_ = tag;
        group_ = group;
        tiffComponent_ = 0;
    }

    void TiffFinder::findObject(TiffComponent* object)
    {
        if (object->tag() == tag_ && object->group() == group_) {
            tiffComponent_ = object;
            setGo(false);
        }
    }

    void TiffFinder::visitEntry(TiffEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitDirectory(TiffDirectory* object)
    {
        findObject(object);
    }

    void TiffFinder::visitSubIfd(TiffSubIfd* object)
    {
        findObject(object);
    }

    void TiffFinder::visitMnEntry(TiffMnEntry* object)
    {
        findObject(object);
    }

    void TiffFinder::visitIfdMakernote(TiffIfdMakernote* object)
    {
        findObject(object);
    }

    void TiffMetadataDecoder::visitEntry(TiffEntry* object)
    {
        decodeTiffEntry(object);
    }

    void TiffMetadataDecoder::visitDirectory(TiffDirectory* object)
    {
        // Nothing to do
    }

    void TiffMetadataDecoder::visitSubIfd(TiffSubIfd* object)
    {
        decodeTiffEntry(object);
    }

    void TiffMetadataDecoder::visitMnEntry(TiffMnEntry* object)
    {
        if (!object->mn_) decodeTiffEntry(object);
    }

    void TiffMetadataDecoder::visitIfdMakernote(TiffIfdMakernote* object)
    {
        // Nothing to do
    }

    void TiffMetadataDecoder::decodeTiffEntry(const TiffEntryBase* object)
    {
        assert(object != 0);

        // Todo: ExifKey should have an appropriate c'tor, it should not be 
        //       necessary to use groupName here
        ExifKey k(object->tag(), object->groupName());
        assert(pImage_ != 0);
        pImage_->exifData().add(k, object->pValue());
    } // TiffMetadataDecoder::decodeTiffEntry

    const std::string TiffPrinter::indent_("   ");

    void TiffPrinter::incIndent()
    {
        prefix_ += indent_;
    } // TiffPrinter::incIndent

    void TiffPrinter::decIndent()
    {
        if (prefix_.length() >= indent_.length()) {
            prefix_.erase(prefix_.length() - indent_.length(), indent_.length());
        }
    } // TiffPrinter::decIndent

    void TiffPrinter::visitEntry(TiffEntry* object)
    {
        printTiffEntry(object, prefix());
    } // TiffPrinter::visitEntry

    void TiffPrinter::visitDirectory(TiffDirectory* object)
    {
        assert(object != 0);

        os_ << prefix() << object->groupName() << " directory with " 
        // cast to make MSVC happy
           << std::dec << static_cast<unsigned int>(object->components_.size());
        if (object->components_.size() == 1) os_ << " entry:\n";
        else os_ << " entries:\n";
        incIndent();

    } // TiffPrinter::visitDirectory

    void TiffPrinter::visitDirectoryNext(TiffDirectory* object)
    {
        decIndent();
        if (object->pNext_) os_ << prefix() << "Next directory:\n";
        else os_ << prefix() << "No next directory\n";        
    } // TiffPrinter::visitDirectoryNext

    void TiffPrinter::visitDirectoryEnd(TiffDirectory* object)
    {
        // Nothing to do
    } // TiffPrinter::visitDirectoryEnd

    void TiffPrinter::visitSubIfd(TiffSubIfd* object)
    {
        os_ << prefix() << "Sub-IFD ";
        printTiffEntry(object);
    } // TiffPrinter::visitSubIfd

    void TiffPrinter::visitMnEntry(TiffMnEntry* object)
    {
        if (!object->mn_) printTiffEntry(object, prefix());
        else os_ << prefix() << "Makernote ";
    } // TiffPrinter::visitMnEntry

    void TiffPrinter::visitIfdMakernote(TiffIfdMakernote* object)
    {
        os_ << prefix() << "Todo: Print IFD makernote header\n";
    } // TiffPrinter::visitIfdMakernote

    void TiffPrinter::printTiffEntry(TiffEntryBase* object,
                                     const std::string& px) const
    {
        assert(object != 0);

        os_ << px << object->groupName()
            << " tag 0x" << std::setw(4) << std::setfill('0')
            << std::hex << std::right << object->tag()
            << ", type " << TypeInfo::typeName(object->typeId())
            << ", " << std::dec << object->count() << " component";
        if (object->count() > 1) os_ << "s";
        os_ << " in " << object->size() << " bytes";
        if (object->size() > 4) os_ << ", offset " << object->offset();
        os_ << "\n";
        const Value* vp = object->pValue();
        if (vp && vp->count() < 100) os_ << prefix() << *vp;
        else os_ << prefix() << "...";
        os_ << "\n";

    } // TiffPrinter::printTiffEntry

    // *************************************************************************
    // free functions

}                                       // namespace Exiv2
