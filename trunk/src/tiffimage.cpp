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
  File:      tiffimage.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   15-Mar-06, ahu: created

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

#include "tiffimage.hpp"
#include "tiffimage_int.hpp"
#include "tiffcomposite_int.hpp"
#include "tiffvisitor_int.hpp"
#include "makernote_int.hpp"
#include "image.hpp"
#include "error.hpp"
#include "futils.hpp"
#include "types.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <memory>

/* --------------------------------------------------------------------------

   Todo:

   + CR2 Makernotes don't seem to have a next pointer but Canon Jpeg Makernotes
     do. What a mess. (That'll become an issue when it comes to writing to CR2)
   + Sony makernotes in RAW files do not seem to have header like those in Jpegs.
     And maybe no next pointer either.

   in crwimage.* :

   + Fix CiffHeader according to TiffHeader
   + Combine Error(15) and Error(33), add format argument %1
   + Search crwimage for todos, fix writeMetadata comment
   + rename loadStack to getPath for consistency

   -------------------------------------------------------------------------- */

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    using namespace Internal;

    TiffImage::TiffImage(BasicIo::AutoPtr io, bool /*create*/)
        : Image(ImageType::tiff, mdExif | mdIptc, io)
    {
    } // TiffImage::TiffImage

    int TiffImage::pixelWidth() const
    {
        ExifData::const_iterator imageWidth = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageWidth"));
        if (imageWidth != exifData_.end() && imageWidth->count() > 0) {
            return imageWidth->toLong();
        }
        return 0;
    }

    int TiffImage::pixelHeight() const
    {
        ExifData::const_iterator imageHeight = exifData_.findKey(Exiv2::ExifKey("Exif.Image.ImageLength"));
        if (imageHeight != exifData_.end() && imageHeight->count() > 0) {
            return imageHeight->toLong();
        }
        return 0;
    }

    void TiffImage::setComment(const std::string& /*comment*/)
    {
        // not supported
        throw(Error(32, "Image comment", "TIFF"));
    }

    void TiffImage::readMetadata()
    {
#ifdef DEBUG
        std::cerr << "Reading TIFF file " << io_->path() << "\n";
#endif
        if (io_->open() != 0) throw Error(9, io_->path(), strError());
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isTiffType(*io_, false)) {
            if (io_->error() || io_->eof()) throw Error(14);
            throw Error(3, "TIFF");
        }
        clearMetadata();
        ByteOrder bo = TiffParser::decode(exifData_,
                                          iptcData_,
                                          xmpData_,
                                          io_->mmap(),
                                          io_->size());
        setByteOrder(bo);
    } // TiffImage::readMetadata

    void TiffImage::writeMetadata()
    {
#ifdef DEBUG
        std::cerr << "Writing TIFF file " << io_->path() << "\n";
#endif
        // Read existing image
        ByteOrder bo = byteOrder();
        DataBuf buf;
        if (io_->open() == 0) {
            IoCloser closer(*io_);
            // Ensure that this is the correct image type
            if (isTiffType(*io_, false)) {
                // Read the image into a memory buffer
                buf.alloc(io_->size());
                io_->read(buf.pData_, buf.size_);
                if (io_->error() || io_->eof()) {
                    buf.reset();
                }
                TiffHeader tiffHeader;
                if (0 == tiffHeader.read(buf.pData_, 8)) {
                    bo = tiffHeader.byteOrder();
                }
            }
        }
        if (bo == invalidByteOrder) {
            bo = littleEndian;
        }
        setByteOrder(bo);
        Blob blob;
        WriteMethod wm = TiffParser::encode(blob,
                                            buf.pData_,
                                            buf.size_,
                                            bo,
                                            exifData_,
                                            iptcData_,
                                            xmpData_);
        // Write updated or new buffer to file
        BasicIo::AutoPtr tempIo(io_->temporary()); // may throw
        assert(tempIo.get() != 0);
        if (wm == wmNonIntrusive) {
            // Buffer may be modified but size is unchanged, write buffer back
            tempIo->write(buf.pData_, buf.size_);
        }
        else {
            // Size of the buffer changed, write from blob
            tempIo->write((blob.size() > 0 ? &blob[0] : 0), static_cast<long>(blob.size()));
        }
        io_->close();
        io_->transfer(*tempIo); // may throw

    } // TiffImage::writeMetadata

    ByteOrder TiffParser::decode(
              ExifData& exifData,
              IptcData& iptcData,
              XmpData&  xmpData,
        const byte*     pData,
              uint32_t  size
    )
    {
        return TiffParserWorker::decode(exifData,
                                        iptcData,
                                        xmpData,
                                        pData,
                                        size,
                                        Tag::root,
                                        TiffMapping::findDecoder);
    } // TiffParser::decode

    WriteMethod TiffParser::encode(
              Blob&     blob,
        const byte*     pData,
              uint32_t  size,
              ByteOrder byteOrder,
        const ExifData& exifData,
        const IptcData& iptcData,
        const XmpData&  xmpData
    )
    {
        std::auto_ptr<TiffHeaderBase> header(new TiffHeader(byteOrder));
        return TiffParserWorker::encode(blob,
                                        pData,
                                        size,
                                        exifData,
                                        iptcData,
                                        xmpData,
                                        Tag::root,
                                        TiffMapping::findEncoder,
                                        header.get());
    } // TiffParser::encode

    // *************************************************************************
    // free functions
    Image::AutoPtr newTiffInstance(BasicIo::AutoPtr io, bool create)
    {
        Image::AutoPtr image(new TiffImage(io, create));
        if (!image->good()) {
            image.reset();
        }
        return image;
    }

    bool isTiffType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 8;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof()) {
            return false;
        }
        TiffHeader tiffHeader;
        bool rc = tiffHeader.read(buf, len);
        if (!advance || !rc) {
            iIo.seek(-len, BasicIo::cur);
        }
        return rc;
    }

}                                       // namespace Exiv2

namespace Exiv2 {
    namespace Internal {

    /* 
      This table lists for each group in a tree, its parent group and tag.
      Root identifies the root of a TIFF tree, as there is a need for multiple
      trees. Groups are the nodes of a TIFF tree. A group is an IFD or any
      other composite component.

      With this table, it is possible, for a given group (and tag) to find a
      path, i.e., a list of groups and tags, from the root to that group (tag).
    */
    const TiffTreeStruct TiffCreator::tiffTreeStruct_[] = {
        // root      group             parent group      parent tag 
        //---------  ----------------- ----------------- ---------- 
        { Tag::root, Group::none,      Group::none,      Tag::root },
        { Tag::root, Group::ifd0,      Group::none,      Tag::root },
        { Tag::root, Group::subimg1,   Group::ifd0,      0x014a    },
        { Tag::root, Group::subimg2,   Group::ifd0,      0x014a    },
        { Tag::root, Group::subimg3,   Group::ifd0,      0x014a    },
        { Tag::root, Group::subimg4,   Group::ifd0,      0x014a    },
        { Tag::root, Group::exif,      Group::ifd0,      0x8769    },
        { Tag::root, Group::gps,       Group::ifd0,      0x8825    },
        { Tag::root, Group::iop,       Group::exif,      0xa005    },
        { Tag::root, Group::ifd1,      Group::ifd0,      Tag::next },
        { Tag::root, Group::ifd2,      Group::ifd1,      Tag::next },
        { Tag::root, Group::olymp1mn,  Group::exif,      0x927c    },
        { Tag::root, Group::olymp2mn,  Group::exif,      0x927c    },
        { Tag::root, Group::olympeq,   Group::olymp2mn,  0x2010    },
        { Tag::root, Group::olympcs,   Group::olymp2mn,  0x2020    },
        { Tag::root, Group::olymprd,   Group::olymp2mn,  0x2030    },
        { Tag::root, Group::olymprd2,  Group::olymp2mn,  0x2031    },
        { Tag::root, Group::olympip,   Group::olymp2mn,  0x2040    },
        { Tag::root, Group::olympfi,   Group::olymp2mn,  0x2050    },
        { Tag::root, Group::olympfe1,  Group::olymp2mn,  0x2100    },
        { Tag::root, Group::olympfe2,  Group::olymp2mn,  0x2200    },
        { Tag::root, Group::olympfe3,  Group::olymp2mn,  0x2300    },
        { Tag::root, Group::olympfe4,  Group::olymp2mn,  0x2400    },
        { Tag::root, Group::olympfe5,  Group::olymp2mn,  0x2500    },
        { Tag::root, Group::olympfe6,  Group::olymp2mn,  0x2600    },
        { Tag::root, Group::olympfe7,  Group::olymp2mn,  0x2700    },
        { Tag::root, Group::olympfe8,  Group::olymp2mn,  0x2800    },
        { Tag::root, Group::olympfe9,  Group::olymp2mn,  0x2900    },
        { Tag::root, Group::olympri,   Group::olymp2mn,  0x3000    },
        { Tag::root, Group::fujimn,    Group::exif,      0x927c    },
        { Tag::root, Group::canonmn,   Group::exif,      0x927c    },
        { Tag::root, Group::canoncs,   Group::canonmn,   0x0001    },
        { Tag::root, Group::canonsi,   Group::canonmn,   0x0004    },
        { Tag::root, Group::canonpa,   Group::canonmn,   0x0005    },
        { Tag::root, Group::canoncf,   Group::canonmn,   0x000f    },
        { Tag::root, Group::canonpi,   Group::canonmn,   0x0012    },
        { Tag::root, Group::nikon1mn,  Group::exif,      0x927c    },
        { Tag::root, Group::nikon2mn,  Group::exif,      0x927c    },
        { Tag::root, Group::nikon3mn,  Group::exif,      0x927c    },
        { Tag::root, Group::nikonpv,   Group::nikon3mn,  0x0011    },
        { Tag::root, Group::panamn,    Group::exif,      0x927c    },
        { Tag::root, Group::pentaxmn,  Group::exif,      0x927c    },
        { Tag::root, Group::sigmamn,   Group::exif,      0x927c    },
        { Tag::root, Group::sony1mn,   Group::exif,      0x927c    },
        { Tag::root, Group::sony2mn,   Group::exif,      0x927c    },
        { Tag::root, Group::minoltamn, Group::exif,      0x927c    },
        { Tag::root, Group::minocso,   Group::minoltamn, 0x0001    },
        { Tag::root, Group::minocsn,   Group::minoltamn, 0x0003    },
        { Tag::root, Group::minocs7,   Group::minoltamn, 0x0004    },
        { Tag::root, Group::minocs5,   Group::minoltamn, 0x0114    }
    };

    /*
      This table describes the layout of each known TIFF group (including
      non-standard structures and IFDs only seen in RAW images).

      The key of the table consists of the first two attributes, (extended) tag
      and group. Tag is the TIFF tag or one of a few extended tags, group
      identifies the IFD or any other composite component.

      Each entry of the table defines for a particular tag and group combination
      the corresponding TIFF component create function.
     */
    const TiffGroupStruct TiffCreator::tiffGroupStruct_[] = {
        // ext. tag  group             create function
        //---------  ----------------- -----------------------------------------
        // Root directory
        { Tag::root, Group::none,      newTiffDirectory<Group::ifd0>             },

        // IFD0
        {    0x8769, Group::ifd0,      newTiffSubIfd<Group::exif>                },
        {    0x8825, Group::ifd0,      newTiffSubIfd<Group::gps>                 },
        {    0x0111, Group::ifd0,      newTiffImageData<0x0117, Group::ifd0>     },
        {    0x0117, Group::ifd0,      newTiffImageSize<0x0111, Group::ifd0>     },
        {    0x0144, Group::ifd0,      newTiffImageData<0x0145, Group::ifd0>     },
        {    0x0145, Group::ifd0,      newTiffImageSize<0x0144, Group::ifd0>     },
        {    0x0201, Group::ifd0,      newTiffImageData<0x0202, Group::ifd0>     },
        {    0x0202, Group::ifd0,      newTiffImageSize<0x0201, Group::ifd0>     },
        {    0x014a, Group::ifd0,      newTiffSubIfd<Group::subimg1>             },
        { Tag::next, Group::ifd0,      newTiffDirectory<Group::ifd1>             },
        {  Tag::all, Group::ifd0,      newTiffEntry                              },

        // Subdir subimg1
        {    0x0111, Group::subimg1,   newTiffImageData<0x0117, Group::subimg1>  },
        {    0x0117, Group::subimg1,   newTiffImageSize<0x0111, Group::subimg1>  },
        {    0x0144, Group::subimg1,   newTiffImageData<0x0145, Group::subimg1>  },
        {    0x0145, Group::subimg1,   newTiffImageSize<0x0144, Group::subimg1>  },
        {    0x0201, Group::subimg1,   newTiffImageData<0x0202, Group::subimg1>  },
        {    0x0202, Group::subimg1,   newTiffImageSize<0x0201, Group::subimg1>  },
        { Tag::next, Group::subimg1,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::subimg1,   newTiffEntry                              },

        // Subdir subimg2
        {    0x0111, Group::subimg2,   newTiffImageData<0x0117, Group::subimg2>  },
        {    0x0117, Group::subimg2,   newTiffImageSize<0x0111, Group::subimg2>  },
        {    0x0144, Group::subimg2,   newTiffImageData<0x0145, Group::subimg2>  },
        {    0x0145, Group::subimg2,   newTiffImageSize<0x0144, Group::subimg2>  },
        {    0x0201, Group::subimg2,   newTiffImageData<0x0202, Group::subimg2>  },
        {    0x0202, Group::subimg2,   newTiffImageSize<0x0201, Group::subimg2>  },
        { Tag::next, Group::subimg2,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::subimg2,   newTiffEntry                              },

        // Subdir subimg3
        {    0x0111, Group::subimg3,   newTiffImageData<0x0117, Group::subimg3>  },
        {    0x0117, Group::subimg3,   newTiffImageSize<0x0111, Group::subimg3>  },
        {    0x0144, Group::subimg3,   newTiffImageData<0x0145, Group::subimg3>  },
        {    0x0145, Group::subimg3,   newTiffImageSize<0x0144, Group::subimg3>  },
        {    0x0201, Group::subimg3,   newTiffImageData<0x0202, Group::subimg3>  },
        {    0x0202, Group::subimg3,   newTiffImageSize<0x0201, Group::subimg3>  },
        { Tag::next, Group::subimg3,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::subimg3,   newTiffEntry                              },

        // Subdir subimg4
        {    0x0111, Group::subimg4,   newTiffImageData<0x0117, Group::subimg4>  },
        {    0x0117, Group::subimg4,   newTiffImageSize<0x0111, Group::subimg4>  },
        {    0x0144, Group::subimg4,   newTiffImageData<0x0145, Group::subimg4>  },
        {    0x0145, Group::subimg4,   newTiffImageSize<0x0144, Group::subimg4>  },
        {    0x0201, Group::subimg4,   newTiffImageData<0x0202, Group::subimg4>  },
        {    0x0202, Group::subimg4,   newTiffImageSize<0x0201, Group::subimg4>  },
        { Tag::next, Group::subimg4,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::subimg4,   newTiffEntry                              },

        // Exif subdir
        {    0xa005, Group::exif,      newTiffSubIfd<Group::iop>                 },
        {    0x927c, Group::exif,      newTiffMnEntry                            },
        { Tag::next, Group::exif,      newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::exif,      newTiffEntry                              },

        // GPS subdir
        { Tag::next, Group::gps,       newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::gps,       newTiffEntry                              },

        // IOP subdir
        { Tag::next, Group::iop,       newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::iop,       newTiffEntry                              },

        // IFD1
        {    0x0111, Group::ifd1,      newTiffThumbData<0x0117, Group::ifd1>     },
        {    0x0117, Group::ifd1,      newTiffThumbSize<0x0111, Group::ifd1>     },
        {    0x0201, Group::ifd1,      newTiffThumbData<0x0202, Group::ifd1>     },
        {    0x0202, Group::ifd1,      newTiffThumbSize<0x0201, Group::ifd1>     },
        { Tag::next, Group::ifd1,      newTiffDirectory<Group::ifd2>             },
        {  Tag::all, Group::ifd1,      newTiffEntry                              },

        // IFD2 (eg, in Pentax PEF files)
        {    0x0111, Group::ifd2,      newTiffImageData<0x0117, Group::ifd2>     },
        {    0x0117, Group::ifd2,      newTiffImageSize<0x0111, Group::ifd2>     },
        {    0x0201, Group::ifd2,      newTiffImageData<0x0202, Group::ifd2>     },
        {    0x0202, Group::ifd2,      newTiffImageSize<0x0201, Group::ifd2>     },
        { Tag::next, Group::ifd2,      newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::ifd2,      newTiffEntry                              },

        // Olympus makernote - some Olympus cameras use Minolta structures
        // Todo: Adding such tags will not work (maybe result in a Minolta makernote), need separate groups
        {    0x0001, Group::olymp1mn,  newTiffArrayEntry<Group::minocso, ttUnsignedLong, false>  },
        {    0x0003, Group::olymp1mn,  newTiffArrayEntry<Group::minocsn, ttUnsignedLong, false>  },
        { Tag::next, Group::olymp1mn,  newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::olymp1mn,  newTiffEntry                              },

        // Olympus2 makernote
        {    0x0001, Group::olymp2mn,  newTiffArrayEntry<Group::minocso, ttUnsignedLong, false>  },
        {    0x0003, Group::olymp2mn,  newTiffArrayEntry<Group::minocsn, ttUnsignedLong, false>  },
        {    0x2010, Group::olymp2mn,  newTiffSubIfd<Group::olympeq>             },
        {    0x2020, Group::olymp2mn,  newTiffSubIfd<Group::olympcs>             },
        {    0x2030, Group::olymp2mn,  newTiffSubIfd<Group::olymprd>             },
        {    0x2031, Group::olymp2mn,  newTiffSubIfd<Group::olymprd2>            },
        {    0x2040, Group::olymp2mn,  newTiffSubIfd<Group::olympip>             },
        {    0x2050, Group::olymp2mn,  newTiffSubIfd<Group::olympfi>             },
        {    0x2100, Group::olymp2mn,  newTiffSubIfd<Group::olympfe1>            },
        {    0x2200, Group::olymp2mn,  newTiffSubIfd<Group::olympfe2>            },
        {    0x2300, Group::olymp2mn,  newTiffSubIfd<Group::olympfe3>            },
        {    0x2400, Group::olymp2mn,  newTiffSubIfd<Group::olympfe4>            },
        {    0x2500, Group::olymp2mn,  newTiffSubIfd<Group::olympfe5>            },
        {    0x2600, Group::olymp2mn,  newTiffSubIfd<Group::olympfe6>            },
        {    0x2700, Group::olymp2mn,  newTiffSubIfd<Group::olympfe7>            },
        {    0x2800, Group::olymp2mn,  newTiffSubIfd<Group::olympfe8>            },
        {    0x2900, Group::olymp2mn,  newTiffSubIfd<Group::olympfe9>            },
        {    0x3000, Group::olymp2mn,  newTiffSubIfd<Group::olympri>             },
        { Tag::next, Group::olymp2mn,  newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::olymp2mn,  newTiffEntry                              },

        // Olympus2 equipment subdir
        {  Tag::all, Group::olympeq,   newTiffEntry                              },

        // Olympus2 camera settings subdir
        {    0x0101, Group::olympcs,   newTiffImageData<0x0102, Group::olympcs>  },
        {    0x0102, Group::olympcs,   newTiffImageSize<0x0101, Group::olympcs>  },
        {  Tag::all, Group::olympcs,   newTiffEntry                              },

        // Olympus2 raw development subdir
        {  Tag::all, Group::olymprd,   newTiffEntry                              },

        // Olympus2 raw development 2 subdir
        {  Tag::all, Group::olymprd2,  newTiffEntry                              },

        // Olympus2 image processing subdir
        {  Tag::all, Group::olympip,   newTiffEntry                              },

        // Olympus2 focus info subdir
        {  Tag::all, Group::olympfi,   newTiffEntry                              },

        // Olympus2 FE 1 subdir
        {  Tag::all, Group::olympfe1,  newTiffEntry                              },

        // Olympus2 FE 2 subdir
        {  Tag::all, Group::olympfe2,  newTiffEntry                              },

        // Olympus2 FE 3 subdir
        {  Tag::all, Group::olympfe3,  newTiffEntry                              },

        // Olympus2 FE 4 subdir
        {  Tag::all, Group::olympfe4,  newTiffEntry                              },

        // Olympus2 FE 5 subdir
        {  Tag::all, Group::olympfe5,  newTiffEntry                              },

        // Olympus2 FE 6 subdir
        {  Tag::all, Group::olympfe6,  newTiffEntry                              },

        // Olympus2 FE 7 subdir
        {  Tag::all, Group::olympfe7,  newTiffEntry                              },

        // Olympus2 FE 8 subdir
        {  Tag::all, Group::olympfe8,  newTiffEntry                              },

        // Olympus2 FE 9 subdir
        {  Tag::all, Group::olympfe9,  newTiffEntry                              },

        // Olympus2 Raw Info subdir
        {  Tag::all, Group::olympri,   newTiffEntry                              },

        // Fujifilm makernote
        { Tag::next, Group::fujimn,    newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::fujimn,    newTiffEntry                              },

        // Canon makernote
        {    0x0001, Group::canonmn,   newTiffArrayEntry<Group::canoncs, ttUnsignedShort, true>  },
        {    0x0004, Group::canonmn,   newTiffArrayEntry<Group::canonsi, ttUnsignedShort, true>  },
        {    0x0005, Group::canonmn,   newTiffArrayEntry<Group::canonpa, ttUnsignedShort, false>  },
        {    0x000f, Group::canonmn,   newTiffArrayEntry<Group::canoncf, ttUnsignedShort, true>  },
        {    0x0012, Group::canonmn,   newTiffArrayEntry<Group::canonpi, ttUnsignedShort, false>  },
        { Tag::next, Group::canonmn,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::canonmn,   newTiffEntry  },

        // Canon makernote composite tags
        {  Tag::all, Group::canoncs,   newTiffArrayElement<ttUnsignedShort>      },
        {  Tag::all, Group::canonsi,   newTiffArrayElement<ttUnsignedShort>      },
        {  Tag::all, Group::canonpa,   newTiffArrayElement<ttUnsignedShort>      },
        {  Tag::all, Group::canoncf,   newTiffArrayElement<ttUnsignedShort>      },
        {  Tag::all, Group::canonpi,   newTiffArrayElement<ttUnsignedShort>      },

        // Nikon1 makernote
        { Tag::next, Group::nikon1mn,  newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::nikon1mn,  newTiffEntry                              },

        // Nikon2 makernote
        { Tag::next, Group::nikon2mn,  newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::nikon2mn,  newTiffEntry                              },

        // Nikon3 makernote
        { Tag::next, Group::nikon3mn,  newTiffDirectory<Group::ignr>             },
        {    0x0011, Group::nikon3mn,  newTiffSubIfd<Group::nikonpv>             },
        {  Tag::all, Group::nikon3mn,  newTiffEntry  },

        // Nikon3 makernote preview subdir
        {    0x0201, Group::nikonpv,   newTiffThumbData<0x0202, Group::nikonpv>  },
        {    0x0202, Group::nikonpv,   newTiffThumbSize<0x0201, Group::nikonpv>  },
        { Tag::next, Group::nikonpv,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::nikonpv,   newTiffEntry                              },

        // Panasonic makernote
        { Tag::next, Group::panamn,    newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::panamn,    newTiffEntry                              },

        // Pentax makernote
        {    0x0003, Group::pentaxmn,  newTiffThumbSize<0x0004, Group::pentaxmn> },
        {    0x0004, Group::pentaxmn,  newTiffThumbData<0x0003, Group::pentaxmn> },
        { Tag::next, Group::pentaxmn,  newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::pentaxmn,  newTiffEntry                              },

        // Sigma/Foveon makernote
        { Tag::next, Group::sigmamn,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::sigmamn,   newTiffEntry                              },

        // Sony1 makernote
        { Tag::next, Group::sony1mn,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::sony1mn,   newTiffEntry                              },

        // Sony2 makernote
        { Tag::next, Group::sony2mn,   newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::sony2mn,   newTiffEntry                              },

        // Minolta makernote
        {    0x0001, Group::minoltamn, newTiffArrayEntry<Group::minocso, ttUnsignedLong, false>   },
        {    0x0003, Group::minoltamn, newTiffArrayEntry<Group::minocsn, ttUnsignedLong, false>   },
        {    0x0004, Group::minoltamn, newTiffArrayEntry<Group::minocs7, ttUnsignedShort, false>  },
        {    0x0088, Group::minoltamn, newTiffThumbData<0x0089, Group::minoltamn>},
        {    0x0089, Group::minoltamn, newTiffThumbSize<0x0088, Group::minoltamn>},
        {    0x0114, Group::minoltamn, newTiffArrayEntry<Group::minocs5, ttUnsignedShort, false>  },
        { Tag::next, Group::minoltamn, newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::minoltamn, newTiffEntry                              },

        // Minolta makernote composite tags
        {  Tag::all, Group::minocso,   newTiffArrayElement<ttUnsignedLong,  bigEndian>  },
        {  Tag::all, Group::minocsn,   newTiffArrayElement<ttUnsignedLong,  bigEndian>  },
        {  Tag::all, Group::minocs7,   newTiffArrayElement<ttUnsignedShort, bigEndian>  },
        {  Tag::all, Group::minocs5,   newTiffArrayElement<ttUnsignedShort, bigEndian>  },

        // Tags which are not de/encoded
        { Tag::next, Group::ignr,      newTiffDirectory<Group::ignr>             },
        {  Tag::all, Group::ignr,      newTiffEntry                              }
    };

    // TIFF mapping table for special decoding and encoding requirements
    const TiffMappingInfo TiffMapping::tiffMappingInfo_[] = {
        { "*",       Tag::all, Group::ignr,    0, 0 }, // Do not decode tags with group == Group::ignr
        { "*",         0x02bc, Group::ifd0,    &TiffDecoder::decodeXmp,          0 /*done before the tree is traversed*/ },
        { "*",         0x83bb, Group::ifd0,    &TiffDecoder::decodeIptc,         0 /*done before the tree is traversed*/ },
        { "*",         0x8649, Group::ifd0,    &TiffDecoder::decodeIptc,         0 /*done before the tree is traversed*/ },
        // Minolta makernote entries which need to be encoded in big endian byte order
        { "*",       Tag::all, Group::minocso, &TiffDecoder::decodeStdTiffEntry, &TiffEncoder::encodeBigEndianEntry },
        { "*",       Tag::all, Group::minocsn, &TiffDecoder::decodeStdTiffEntry, &TiffEncoder::encodeBigEndianEntry },
        { "*",       Tag::all, Group::minocs7, &TiffDecoder::decodeStdTiffEntry, &TiffEncoder::encodeBigEndianEntry },
        { "*",       Tag::all, Group::minocs5, &TiffDecoder::decodeStdTiffEntry, &TiffEncoder::encodeBigEndianEntry }
    };

    DecoderFct TiffMapping::findDecoder(const std::string& make,
                                              uint32_t     extendedTag,
                                              uint16_t     group)
    {
        DecoderFct decoderFct = &TiffDecoder::decodeStdTiffEntry;
        const TiffMappingInfo* td = find(tiffMappingInfo_,
                                         TiffMappingInfo::Key(make, extendedTag, group));
        if (td) {
            // This may set decoderFct to 0, meaning that the tag should not be decoded
            decoderFct = td->decoderFct_;
        }
        return decoderFct;
    }

    EncoderFct TiffMapping::findEncoder(
        const std::string& make,
              uint32_t     extendedTag,
              uint16_t     group
    )
    {
        EncoderFct encoderFct = 0;
        const TiffMappingInfo* td = find(tiffMappingInfo_,
                                         TiffMappingInfo::Key(make, extendedTag, group));
        if (td) {
            // Returns 0 if no special encoder function is found
            encoderFct = td->encoderFct_;
        }
        return encoderFct;
    }

    bool TiffGroupStruct::operator==(const TiffGroupStruct::Key& key) const
    {
        return    (Tag::all == extendedTag_ || key.e_ == extendedTag_)
               && key.g_ == group_;
    }

    bool TiffTreeStruct::operator==(const TiffTreeStruct::Key& key) const
    {
        return key.r_ == root_ && key.g_ == group_;
    }

    TiffComponent::AutoPtr TiffCreator::create(uint32_t extendedTag,
                                               uint16_t group)
    {
        TiffComponent::AutoPtr tc(0);
        uint16_t tag = static_cast<uint16_t>(extendedTag & 0xffff);
        const TiffGroupStruct* ts = find(tiffGroupStruct_,
                                         TiffGroupStruct::Key(extendedTag, group));
        if (ts && ts->newTiffCompFct_) {
            tc = ts->newTiffCompFct_(tag, group);
        }
#ifdef DEBUG
        else {
            if (!ts) {
                std::cerr << "Warning: No TIFF structure entry found for ";
            }
            else {
                std::cerr << "Warning: No TIFF component creator found for ";
            }
            std::cerr << "extended tag 0x" << std::setw(4) << std::setfill('0')
                      << std::hex << std::right << extendedTag
                      << ", group " << tiffGroupName(group) << "\n";
        }
#endif
        return tc;
    } // TiffCreator::create

    void TiffCreator::getPath(TiffPath& tiffPath,
                              uint32_t  extendedTag,
                              uint16_t  group,
                              uint32_t  root)
    {
        const TiffTreeStruct* ts = 0;
        do {
            tiffPath.push(TiffPathItem(extendedTag, group));
            ts = find(tiffTreeStruct_, TiffTreeStruct::Key(root, group));
            assert(ts != 0);
            extendedTag = ts->parentExtTag_;
            group = ts->parentGroup_;
        } while (!(ts->root_ == root && ts->group_ == Group::none));

    } // TiffCreator::getPath

    ByteOrder TiffParserWorker::decode(
              ExifData&          exifData,
              IptcData&          iptcData,
              XmpData&           xmpData,
        const byte*              pData,
              uint32_t           size,
              uint32_t           root,
              FindDecoderFct     findDecoderFct,
              TiffHeaderBase*    pHeader
    )
    {
        // Create standard TIFF header if necessary
        std::auto_ptr<TiffHeaderBase> ph;
        if (!pHeader) {
            ph = std::auto_ptr<TiffHeaderBase>(new TiffHeader);
            pHeader = ph.get();
        }
        TiffComponent::AutoPtr rootDir = parse(pData, size, root, pHeader);
        if (0 != rootDir.get()) {
            TiffDecoder decoder(exifData,
                                iptcData,
                                xmpData,
                                rootDir.get(),
                                findDecoderFct);
            rootDir->accept(decoder);
        }
        return pHeader->byteOrder();

    } // TiffParserWorker::decode

    WriteMethod TiffParserWorker::encode(
              Blob&              blob,
        const byte*              pData,
              uint32_t           size,
        const ExifData&          exifData,
        const IptcData&          iptcData,
        const XmpData&           xmpData,
              uint32_t           root,
              FindEncoderFct     findEncoderFct,
              TiffHeaderBase*    pHeader
    )
    {
        /*
           1) parse the binary image, if one is provided, and
           2) attempt updating the parsed tree in-place ("non-intrusive writing")
           3) else, create a new tree and write a new TIFF structure ("intrusive
              writing"). If there is a parsed tree, it is only used to access the
              image data in this case.
         */
        assert(pHeader);
        assert(pHeader->byteOrder() != invalidByteOrder);
        blob.clear();
        WriteMethod writeMethod = wmIntrusive;
        TiffComponent::AutoPtr createdTree;
        TiffComponent::AutoPtr parsedTree = parse(pData, size, root, pHeader);
        if (0 != parsedTree.get()) {
            // Attempt to update existing TIFF components based on metadata entries
            TiffEncoder encoder(exifData,
                                iptcData,
                                xmpData,
                                parsedTree.get(),
                                pHeader->byteOrder(),
                                findEncoderFct);
            parsedTree->accept(encoder);
            if (!encoder.dirty()) writeMethod = wmNonIntrusive;
        }
        if (writeMethod == wmIntrusive) {
            createdTree = TiffCreator::create(root, Group::none);
            TiffEncoder encoder(exifData,
                                iptcData,
                                xmpData,
                                createdTree.get(),
                                pHeader->byteOrder(),
                                findEncoderFct);
            // Add entries from metadata to composite
            encoder.add(createdTree.get(), parsedTree.get(), root);
            // Write binary representation from the composite tree
            uint32_t offset = pHeader->write(blob);
            uint32_t imageIdx(uint32_t(-1));
            uint32_t len = createdTree->write(blob, pHeader->byteOrder(), offset, uint32_t(-1), uint32_t(-1), imageIdx);
            // Avoid writing just the header if there is no IFD data
            if (len == 0) blob.clear();
#ifdef DEBUG
            std::cerr << "Intrusive writing\n";
#endif
        }
#ifdef DEBUG
        else {
            std::cerr << "Non-intrusive writing\n";
        }
#endif
        return writeMethod;
    } // TiffParserWorker::encode

    TiffComponent::AutoPtr TiffParserWorker::parse(
        const byte*              pData,
              uint32_t           size,
              uint32_t           root,
              TiffHeaderBase*    pHeader
    )
    {
        if (pData == 0 || size == 0) return TiffComponent::AutoPtr(0);
        if (!pHeader->read(pData, size) || pHeader->offset() >= size) {
            throw Error(3, "TIFF");
        }
        TiffComponent::AutoPtr rootDir = TiffCreator::create(root, Group::none);
        if (0 != rootDir.get()) {
            rootDir->setStart(pData + pHeader->offset());
            TiffRwState::AutoPtr state(
                new TiffRwState(pHeader->byteOrder(), 0));
            TiffReader reader(pData, size, rootDir.get(), state);
            rootDir->accept(reader);
        }
        return rootDir;

    } // TiffParserWorker::parse

    TiffHeaderBase::TiffHeaderBase(uint16_t  tag,
                                   uint32_t  size,
                                   ByteOrder byteOrder,
                                   uint32_t  offset)
        : tag_(tag),
          size_(size),
          byteOrder_(byteOrder),
          offset_(offset)
    {
    }

    TiffHeaderBase::~TiffHeaderBase()
    {
    }

    bool TiffHeaderBase::read(const byte* pData, uint32_t size)
    {
        if (!pData || size < 8) return false;

        if (pData[0] == 0x49 && pData[1] == 0x49) {
            byteOrder_ = littleEndian;
        }
        else if (pData[0] == 0x4d && pData[1] == 0x4d) {
            byteOrder_ = bigEndian;
        }
        else {
            return false;
        }
        if (tag_ != getUShort(pData + 2, byteOrder_)) return false;
        offset_ = getULong(pData + 4, byteOrder_);

        return true;
    } // TiffHeaderBase::read

    uint32_t TiffHeaderBase::write(Blob& blob) const
    {
        byte buf[8];
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
            assert(false);
            break;
        }
        us2Data(buf + 2, tag_, byteOrder_);
        ul2Data(buf + 4, 0x00000008, byteOrder_);
        append(blob, buf, 8);
        return 8;
    }

    void TiffHeaderBase::print(std::ostream& os, const std::string& prefix) const
    {
        os << prefix
           << _("TIFF header, offset") << " = 0x"
           << std::setw(8) << std::setfill('0') << std::hex << std::right
           << offset_;

        switch (byteOrder_) {
        case littleEndian:     os << ", " << _("little endian encoded"); break;
        case bigEndian:        os << ", " << _("big endian encoded");    break;
        case invalidByteOrder: break;
        }
        os << "\n";
    } // TiffHeaderBase::print

    ByteOrder TiffHeaderBase::byteOrder() const
    {
        return byteOrder_;
    }

    void TiffHeaderBase::setByteOrder(ByteOrder byteOrder)
    {
        byteOrder_ = byteOrder;
    }

    uint32_t TiffHeaderBase::offset() const
    {
        return offset_;
    }

    void TiffHeaderBase::setOffset(uint32_t offset)
    {
        offset_ = offset;
    }

    uint32_t TiffHeaderBase::size() const
    {
        return size_;
    }

    uint16_t TiffHeaderBase::tag() const
    {
        return tag_;
    }

    TiffHeader::TiffHeader(ByteOrder byteOrder)
        : TiffHeaderBase(42, 8, byteOrder, 0x00000008)
    {
    }

    TiffHeader::~TiffHeader()
    {
    }

}}                                       // namespace Internal, Exiv2
