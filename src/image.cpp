// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
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
  File:      image.cpp
  Version:   $Rev$
 */
// *****************************************************************************
#include "rcsid_int.hpp"
EXIV2_RCSID("@(#) $Id$")

// included header files
#include "config.h"

#include "image.hpp"
#include "image_int.hpp"
#include "error.hpp"
#include "futils.hpp"

#include "cr2image.hpp"
#include "crwimage.hpp"
#include "epsimage.hpp"
#include "jpgimage.hpp"
#include "mrwimage.hpp"
#ifdef EXV_HAVE_LIBZ
# include "pngimage.hpp"
#endif // EXV_HAVE_LIBZ
#include "rafimage.hpp"
#include "tiffimage.hpp"
#include "webpimage.hpp"
#include "orfimage.hpp"
#include "gifimage.hpp"
#include "psdimage.hpp"
#include "tgaimage.hpp"
#include "bmpimage.hpp"
#include "jp2image.hpp"
#ifdef EXV_ENABLE_VIDEO
#include "matroskavideo.hpp"
#include "quicktimevideo.hpp"
#include "riffvideo.hpp"
#include "asfvideo.hpp"
#endif // EXV_ENABLE_VIDEO
#include "rw2image.hpp"
#include "pgfimage.hpp"
#include "xmpsidecar.hpp"

// + standard includes
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cassert>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#ifdef _MSC_VER
# define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#endif
#ifdef EXV_HAVE_UNISTD_H
# include <unistd.h>                            // stat
#endif

// *****************************************************************************
namespace {

    using namespace Exiv2;

    //! Struct for storing image types and function pointers.
    struct Registry {
        //! Comparison operator to compare a Registry structure with an image type
        bool operator==(const int& imageType) const { return imageType == imageType_; }

        // DATA
        int            imageType_;
        NewInstanceFct newInstance_;
        IsThisTypeFct  isThisType_;
        AccessMode     exifSupport_;
        AccessMode     iptcSupport_;
        AccessMode     xmpSupport_;
        AccessMode     commentSupport_;
    };

    const Registry registry[] = {
        //image type       creation fct     type check  Exif mode    IPTC mode    XMP mode     Comment mode
        //---------------  ---------------  ----------  -----------  -----------  -----------  ------------
        { ImageType::jpeg, newJpegInstance, isJpegType, amReadWrite, amReadWrite, amReadWrite, amReadWrite },
        { ImageType::exv,  newExvInstance,  isExvType,  amReadWrite, amReadWrite, amReadWrite, amReadWrite },
        { ImageType::cr2,  newCr2Instance,  isCr2Type,  amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::crw,  newCrwInstance,  isCrwType,  amReadWrite, amNone,      amNone,      amReadWrite },
        { ImageType::mrw,  newMrwInstance,  isMrwType,  amRead,      amRead,      amRead,      amNone      },
        { ImageType::tiff, newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::webp, newWebPInstance, isWebPType, amReadWrite, amNone,      amReadWrite, amNone      },
        { ImageType::dng,  newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::nef,  newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::pef,  newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::arw,  newTiffInstance, isTiffType, amRead,      amRead,      amRead,      amNone      },
        { ImageType::rw2,  newRw2Instance,  isRw2Type,  amRead,      amRead,      amRead,      amNone      },
        { ImageType::sr2,  newTiffInstance, isTiffType, amRead,      amRead,      amRead,      amNone      },
        { ImageType::srw,  newTiffInstance, isTiffType, amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::orf,  newOrfInstance,  isOrfType,  amReadWrite, amReadWrite, amReadWrite, amNone      },
#ifdef EXV_HAVE_LIBZ
        { ImageType::png,  newPngInstance,  isPngType,  amReadWrite, amReadWrite, amReadWrite, amReadWrite },
#endif // EXV_HAVE_LIBZ
        { ImageType::pgf,  newPgfInstance,  isPgfType,  amReadWrite, amReadWrite, amReadWrite, amReadWrite },
        { ImageType::raf,  newRafInstance,  isRafType,  amRead,      amRead,      amRead,      amNone      },
        { ImageType::eps,  newEpsInstance,  isEpsType,  amNone,      amNone,      amReadWrite, amNone      },
        { ImageType::xmp,  newXmpInstance,  isXmpType,  amReadWrite, amReadWrite, amReadWrite, amNone      },
        { ImageType::gif,  newGifInstance,  isGifType,  amNone,      amNone,      amNone,      amNone      },
        { ImageType::psd,  newPsdInstance,  isPsdType,  amRead,      amRead,      amRead,      amNone      },
        { ImageType::tga,  newTgaInstance,  isTgaType,  amNone,      amNone,      amNone,      amNone      },
        { ImageType::bmp,  newBmpInstance,  isBmpType,  amNone,      amNone,      amNone,      amNone      },
        { ImageType::jp2,  newJp2Instance,  isJp2Type,  amReadWrite, amReadWrite, amReadWrite, amNone      },
#ifdef EXV_ENABLE_VIDEO
        { ImageType::qtime,newQTimeInstance,isQTimeType,amRead,      amNone,      amRead,      amNone      },
        { ImageType::riff, newRiffInstance, isRiffType, amRead,      amNone,      amRead,      amNone      },
        { ImageType::asf,  newAsfInstance,  isAsfType,  amNone,      amNone,      amRead,      amNone      },
        { ImageType::mkv,  newMkvInstance,  isMkvType,  amNone,      amNone,      amRead,      amNone      },
#endif // EXV_ENABLE_VIDEO
        // End of list marker
        { ImageType::none, 0,               0,          amNone,      amNone,      amNone,      amNone      }
    };

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    Image::Image(int              imageType,
                 uint16_t         supportedMetadata,
                 BasicIo::AutoPtr io)
        : io_(io),
          pixelWidth_(0),
          pixelHeight_(0),
          imageType_(imageType),
          supportedMetadata_(supportedMetadata),
#ifdef EXV_HAVE_XMP_TOOLKIT
          writeXmpFromPacket_(false),
#else
          writeXmpFromPacket_(true),
#endif
          byteOrder_(invalidByteOrder)
    {
    }

    Image::~Image()
    {
    }

    void Image::printStructure(std::ostream&, PrintStructureOption,int /*depth*/)
    {
        throw Error(13, io_->path());
    }

    void Image::clearMetadata()
    {
        clearExifData();
        clearIptcData();
        clearXmpPacket();
        clearXmpData();
        clearComment();
        clearIccProfile();
    }

    ExifData& Image::exifData()
    {
        return exifData_;
    }

    IptcData& Image::iptcData()
    {
        return iptcData_;
    }

    XmpData& Image::xmpData()
    {
        return xmpData_;
    }

    std::string& Image::xmpPacket()
    {
        return xmpPacket_;
    }

    void Image::setMetadata(const Image& image)
    {
        if (checkMode(mdExif) & amWrite) {
            setExifData(image.exifData());
        }
        if (checkMode(mdIptc) & amWrite) {
            setIptcData(image.iptcData());
        }
        if (checkMode(mdIccProfile) & amWrite && iccProfile()) {
            setIccProfile(*iccProfile());
        }
        if (checkMode(mdXmp) & amWrite) {
            setXmpPacket(image.xmpPacket());
            setXmpData(image.xmpData());
        }
        if (checkMode(mdComment) & amWrite) {
            setComment(image.comment());
        }
    }

    void Image::clearExifData()
    {
        exifData_.clear();
    }

    void Image::setExifData(const ExifData& exifData)
    {
        exifData_ = exifData;
    }

    void Image::clearIptcData()
    {
        iptcData_.clear();
    }

    void Image::setIptcData(const IptcData& iptcData)
    {
        iptcData_ = iptcData;
    }

    void Image::clearXmpPacket()
    {
        xmpPacket_.clear();
        writeXmpFromPacket(true);
    }

    void Image::setXmpPacket(const std::string& xmpPacket)
    {
        xmpPacket_ = xmpPacket;
        writeXmpFromPacket(true);
    }

    void Image::clearXmpData()
    {
        xmpData_.clear();
        writeXmpFromPacket(false);
    }

    void Image::setXmpData(const XmpData& xmpData)
    {
        xmpData_ = xmpData;
        writeXmpFromPacket(false);
    }

#ifdef EXV_HAVE_XMP_TOOLKIT
    void Image::writeXmpFromPacket(bool flag)
    {
        writeXmpFromPacket_ = flag;
    }
#else
    void Image::writeXmpFromPacket(bool) {}
#endif

    void Image::clearComment()
    {
        comment_.erase();
    }

    void Image::setComment(const std::string& comment)
    {
        comment_ = comment;
    }

    void Image::setIccProfile(Exiv2::DataBuf& iccProfile,bool bTestValid)
    {
        if ( bTestValid ) {
            long size = iccProfile.pData_ ? getULong(iccProfile.pData_, bigEndian): -1;
            if ( size!= iccProfile.size_ ) throw Error(53);
        }
        iccProfile_ = iccProfile;
    }

    void Image::clearIccProfile()
    {
        iccProfile_.release();
    }

    void Image::setByteOrder(ByteOrder byteOrder)
    {
        byteOrder_ = byteOrder;
    }

    ByteOrder Image::byteOrder() const
    {
        return byteOrder_;
    }

    int Image::pixelWidth() const
    {
        return pixelWidth_;
    }

    int Image::pixelHeight() const
    {
        return pixelHeight_;
    }

    const ExifData& Image::exifData() const
    {
        return exifData_;
    }

    const IptcData& Image::iptcData() const
    {
        return iptcData_;
    }

    const XmpData& Image::xmpData() const
    {
        return xmpData_;
    }

    std::string Image::comment() const
    {
        return comment_;
    }

    const std::string& Image::xmpPacket() const
    {
        return xmpPacket_;
    }

    BasicIo& Image::io() const
    {
        return *io_;
    }

    bool Image::writeXmpFromPacket() const
    {
        return writeXmpFromPacket_;
    }

    const NativePreviewList& Image::nativePreviews() const
    {
        return nativePreviews_;
    }

    bool Image::good() const
    {
        if (io_->open() != 0) return false;
        IoCloser closer(*io_);
        return ImageFactory::checkType(imageType_, *io_, false);
    }

    bool Image::supportsMetadata(MetadataId metadataId) const
    {
        return (supportedMetadata_ & metadataId) != 0;
    }

    AccessMode Image::checkMode(MetadataId metadataId) const
    {
        return ImageFactory::checkMode(imageType_, metadataId);
    }

    AccessMode ImageFactory::checkMode(int type, MetadataId metadataId)
    {
        const Registry* r = find(registry, type);
        if (!r) throw Error(13, type);
        AccessMode am = amNone;
        switch (metadataId) {
        case mdNone:
            break;
        case mdExif:
            am = r->exifSupport_;
            break;
        case mdIptc:
            am = r->iptcSupport_;
            break;
        case mdXmp:
            am = r->xmpSupport_;
            break;
        case mdComment:
            am = r->commentSupport_;
            break;
        case mdIccProfile: break;

        // no default: let the compiler complain
        }
        return am;
    }

    bool ImageFactory::checkType(int type, BasicIo& io, bool advance)
    {
        const Registry* r = find(registry, type);
        if (0 != r) {
            return r->isThisType_(io, advance);
        }
        return false;
    } // ImageFactory::checkType

    int ImageFactory::getType(const std::string& path)
    {
        FileIo fileIo(path);
        return getType(fileIo);
    }

#ifdef EXV_UNICODE_PATH
    int ImageFactory::getType(const std::wstring& wpath)
    {
        FileIo fileIo(wpath);
        return getType(fileIo);
    }

#endif
    int ImageFactory::getType(const byte* data, long size)
    {
        MemIo memIo(data, size);
        return getType(memIo);
    }

    int ImageFactory::getType(BasicIo& io)
    {
        if (io.open() != 0) return ImageType::none;
        IoCloser closer(io);
        for (unsigned int i = 0; registry[i].imageType_ != ImageType::none; ++i) {
            if (registry[i].isThisType_(io, false)) {
                return registry[i].imageType_;
            }
        }
        return ImageType::none;
    } // ImageFactory::getType

    BasicIo::AutoPtr ImageFactory::createIo(const std::string& path, bool useCurl)
    {
        Protocol fProt = fileProtocol(path);
#if EXV_USE_SSH == 1
        if (fProt == pSsh || fProt == pSftp) {
            return BasicIo::AutoPtr(new SshIo(path)); // may throw
        }
#endif
#if EXV_USE_CURL == 1
        if (useCurl && (fProt == pHttp || fProt == pHttps || fProt == pFtp)) {
            return BasicIo::AutoPtr(new CurlIo(path)); // may throw
        }
#endif
        if (fProt == pHttp)
            return BasicIo::AutoPtr(new HttpIo(path)); // may throw
        if (fProt == pFileUri)
            return BasicIo::AutoPtr(new FileIo(pathOfFileUrl(path)));
        if (fProt == pStdin || fProt == pDataUri)
            return BasicIo::AutoPtr(new XPathIo(path)); // may throw

        return BasicIo::AutoPtr(new FileIo(path));

        (void)(useCurl);
    } // ImageFactory::createIo

#ifdef EXV_UNICODE_PATH
    BasicIo::AutoPtr ImageFactory::createIo(const std::wstring& wpath, bool useCurl)
    {
        Protocol fProt = fileProtocol(wpath);
#if EXV_USE_SSH == 1
        if (fProt == pSsh || fProt == pSftp) {
            return BasicIo::AutoPtr(new SshIo(wpath));
        }
#endif
#if EXV_USE_CURL == 1
        if (useCurl && (fProt == pHttp || fProt == pHttps || fProt == pFtp)) {
            return BasicIo::AutoPtr(new CurlIo(wpath));
        }
#endif
        if (fProt == pHttp)
            return BasicIo::AutoPtr(new HttpIo(wpath));
        if (fProt == pFileUri)
            return BasicIo::AutoPtr(new FileIo(pathOfFileUrl(wpath)));
        if (fProt == pStdin || fProt == pDataUri)
            return BasicIo::AutoPtr(new XPathIo(wpath)); // may throw
        return BasicIo::AutoPtr(new FileIo(wpath));
    } // ImageFactory::createIo
#endif
    Image::AutoPtr ImageFactory::open(const std::string& path, bool useCurl)
    {
        Image::AutoPtr image = open(ImageFactory::createIo(path, useCurl)); // may throw
        if (image.get() == 0) throw Error(11, path);
        return image;
    }

#ifdef EXV_UNICODE_PATH
    Image::AutoPtr ImageFactory::open(const std::wstring& wpath, bool useCurl)
    {
        Image::AutoPtr image = open(ImageFactory::createIo(wpath, useCurl)); // may throw
        if (image.get() == 0) throw WError(11, wpath);
        return image;
    }

#endif
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
        for (unsigned int i = 0; registry[i].imageType_ != ImageType::none; ++i) {
            if (registry[i].isThisType_(*io, false)) {
                return registry[i].newInstance_(io, false);
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

#ifdef EXV_UNICODE_PATH
    Image::AutoPtr ImageFactory::create(int type,
                                        const std::wstring& wpath)
    {
        std::auto_ptr<FileIo> fileIo(new FileIo(wpath));
        // Create or overwrite the file, then close it
        if (fileIo->open("w+b") != 0) {
            throw WError(10, wpath, "w+b", strError().c_str());
        }
        fileIo->close();
        BasicIo::AutoPtr io(fileIo);
        Image::AutoPtr image = create(type, io);
        if (image.get() == 0) throw Error(13, type);
        return image;
    }

#endif
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
        const Registry* r = find(registry, type);
        if (0 != r) {
            return r->newInstance_(io, true);
        }
        return Image::AutoPtr();
    } // ImageFactory::create

// *****************************************************************************
// template, inline and free functions

    void append(Blob& blob, const byte* buf, uint32_t len)
    {
        if (len != 0) {
            assert(buf != 0);
            Blob::size_type size = blob.size();
            if (blob.capacity() - size < len) {
                blob.reserve(size + 65536);
            }
            blob.resize(size + len);
            std::memcpy(&blob[size], buf, len);
        }
    } // append

}                                       // namespace Exiv2

namespace Exiv2 {
    namespace Internal {

#ifdef  MSDEV_2003
#undef  vsnprintf
#define vsnprintf _vsnprintf
#endif

    std::string stringFormat(const char* format, ...)
    {
        std::string result;

        int     need   = (int) std::strlen(format)*2;          // initial guess
        char*   buffer = NULL;
        int     again  =    4;
        int     rc     =   -1;

        while (rc < 0 && again--) {
            if ( buffer ) delete[] buffer;
            need  *= 2 ;
            buffer = new char[need];
            if ( buffer ) {
                va_list  args;                                 // variable arg list
                va_start(args, format);                        // args start after format
                rc=vsnprintf(buffer,(unsigned int)need, format, args);
                va_end(args);                                  // free the args
            }
        }

        if ( rc > 0 ) result = std::string(buffer) ;
        if ( buffer ) delete[] buffer;                         // free buffer
        return result;
    }

    std::string binaryToString(const byte* buff, size_t size, size_t start /*=0*/)
    {
        std::string result = "";
        size += start;

        while (start < size) {
            int   c             = (int) buff[start++] ;
            bool  bTrailingNull = c == 0 && start == size;
            if ( !bTrailingNull ) {
                if (c < ' ' || c >= 127) c = '.' ;
                result +=  (char) c ;
            }
        }
        return result;
    }

    std::string binaryToString(DataBuf& buf, size_t size, size_t start /*=0*/)
    {
        return binaryToString(buf.pData_,size,start);
    }

    std::string binaryToHex(const byte *data, size_t size)
    {
        std::stringstream hexOutput;

        unsigned long tl = (unsigned long)((size / 16) * 16);
        unsigned long tl_offset = (unsigned long)(size - tl);

        for (unsigned long loop = 0; loop < (unsigned long)size; loop++) {
            if (data[loop] < 16) {
                hexOutput << "0";
            }
            hexOutput << std::hex << (int)data[loop];
            if ((loop % 8) == 7) {
                hexOutput << "  ";
            }
            if ((loop % 16) == 15 || loop == (tl + tl_offset - 1)) {
                int max = 15;
                if (loop >= tl) {
                    max = tl_offset - 1;
                    for (int offset = 0; offset < (int)(16 - tl_offset); offset++) {
                        if ((offset % 8) == 7) {
                            hexOutput << "  ";
                        }
                        hexOutput << "   ";
                    }
                }
                hexOutput << " ";
                for (int  offset = max; offset >= 0; offset--) {
                    if (offset == (max - 8)) {
                        hexOutput << "  ";
                    }
                    byte c = '.';
                    if (data[loop - offset] >= 0x20 && data[loop - offset] <= 0x7E) {
                        c = data[loop - offset] ;
                    }
                    hexOutput << (char) c ;
                }
                hexOutput << std::endl;
            }
        }

        hexOutput << std::endl << std::endl << std::endl;

        return hexOutput.str();
    }

    std::string indent(int32_t d)
    {
        std::string result ;
        if ( d > 0 )
            while ( d--)
                result += "  ";
        return result;
    }

}}                                      // namespace Internal, Exiv2
