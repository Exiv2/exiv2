// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
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
  File:    pgfimage.cpp
  Author(s): Gilles Caulier (cgilles) <caulier dot gilles at gmail dot com>
  History: 16-Jun-09, gc: submitted
  Credits: See header file
 */
// *****************************************************************************
// included header files
#include "config.h"

#include "pgfimage.hpp"
#include "image.hpp"
#include "pngimage.hpp"
#include "basicio.hpp"
#include "enforce.hpp"
#include "error.hpp"
#include "futils.hpp"

// + standard includes
#include <cstdio>                               // for EOF
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>

// Signature from front of PGF file
const unsigned char pgfSignature[3] = { 0x50, 0x47, 0x46 };

const unsigned char pgfBlank[] = { 0x50,0x47,0x46,0x36,0x10,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x00,0x00,
                                   0x00,0x00,0x18,0x03,0x03,0x00,0x00,0x00,0x14,0x00,0x67,0x08,0x20,0x00,0xc0,0x01,
                                   0x00,0x00,0x37,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x37,0x00,
                                   0x00,0x78,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x37,0x00,0x00,0x78,0x00,0x00,
                                   0x00,0x00,0x01,0x00,0x00,0x00,0x37,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x01,0x00,
                                   0x00,0x00,0x37,0x00,0x00,0x78,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00,0x37,0x00,
                                   0x00,0x78,0x00,0x00,0x00,0x00,0x01,0x00,0x00,0x00
                                 };


// *****************************************************************************
// class member definitions

namespace Exiv2 {

    static uint32_t byteSwap_(uint32_t value,bool bSwap)
    {
        uint32_t result = 0;
        result |= (value & 0x000000FF) << 24;
        result |= (value & 0x0000FF00) << 8;
        result |= (value & 0x00FF0000) >> 8;
        result |= (value & 0xFF000000) >> 24;
        return bSwap ? result : value;
    }

    static uint32_t byteSwap_(Exiv2::DataBuf& buf,size_t offset,bool bSwap)
    {
        uint32_t v;
        char*    p = (char*) &v;
        int      i;
        for ( i = 0 ; i < 4 ; i++ ) p[i] = buf.pData_[offset+i];
        uint32_t result = byteSwap_(v,bSwap);
        p               = (char*) &result;
        for ( i = 0 ; i < 4 ; i++ ) buf.pData_[offset+i] = p[i];
        return result;
    }

    PgfImage::PgfImage(BasicIo::UniquePtr io, bool create)
            : Image(ImageType::pgf, mdExif | mdIptc| mdXmp | mdComment, std::move(io))
            , bSwap_(isBigEndianPlatform())
    {
        if (create)
        {
            if (io_->open() == 0)
            {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Exiv2::PgfImage:: Creating PGF image to memory\n";
#endif
                IoCloser closer(*io_);
                if (io_->write(pgfBlank, sizeof(pgfBlank)) != sizeof(pgfBlank))
                {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cerr << "Exiv2::PgfImage:: Failed to create PGF image on memory\n";
#endif
                }
            }
        }
    } // PgfImage::PgfImage

    void PgfImage::readMetadata()
    {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Exiv2::PgfImage::readMetadata: Reading PGF file " << io_->path() << "\n";
#endif
        if (io_->open() != 0)
        {
            throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isPgfType(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(ErrorCode::kerFailedToReadImageData);
            throw Error(ErrorCode::kerNotAnImage, "PGF");
        }
        clearMetadata();

        readPgfMagicNumber(*io_);

        uint32_t headerSize = readPgfHeaderSize(*io_);
        readPgfHeaderStructure(*io_, pixelWidth_, pixelHeight_);

        // And now, the most interresting, the user data byte array where metadata are stored as small image.

        enforce(headerSize <= std::numeric_limits<uint32_t>::max() - 8, ErrorCode::kerCorruptedMetadata);
#if LONG_MAX < UINT_MAX
        enforce(headerSize + 8 <= static_cast<uint32_t>(std::numeric_limits<long>::max()),
                ErrorCode::kerCorruptedMetadata);
#endif
        int64 size = static_cast<int64>(headerSize) + 8 - io_->tell();

#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PgfImage::readMetadata: Found Image data (" << size << " bytes)\n";
#endif

        if (size < 0 || static_cast<size_t>(size) > io_->size()) throw Error(ErrorCode::kerInputDataReadFailed);
        if (size == 0) return;

        DataBuf imgData(size);
        std::memset(imgData.pData_, 0x0, imgData.size_);
        size_t bufRead = io_->read(imgData.pData_, imgData.size_);
        if (io_->error()) throw Error(ErrorCode::kerFailedToReadImageData);
        if (bufRead != imgData.size_) throw Error(ErrorCode::kerInputDataReadFailed);

        Image::UniquePtr image = Exiv2::ImageFactory::open(imgData.pData_, imgData.size_);
        image->readMetadata();
        exifData() = image->exifData();
        iptcData() = image->iptcData();
        xmpData()  = image->xmpData();

    } // PgfImage::readMetadata

    void PgfImage::writeMetadata()
    {
        if (io_->open() != 0)
        {
            throw Error(ErrorCode::kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        BasicIo::UniquePtr tempIo(new MemIo);
        assert (tempIo.get() != 0);

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw

    } // PgfImage::writeMetadata

    void PgfImage::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(ErrorCode::kerInputDataReadFailed);
        if (!outIo.isopen()) throw Error(ErrorCode::kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PgfImage::doWriteMetadata: Writing PGF file " << io_->path() << "\n";
        std::cout << "Exiv2::PgfImage::doWriteMetadata: tmp file created " << outIo.path() << "\n";
#endif

        // Ensure that this is the correct image type
        if (!isPgfType(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(ErrorCode::kerInputDataReadFailed);
            throw Error(ErrorCode::kerNoImageInInputData);
        }

        // Ensure PGF version.
        byte mnb            = readPgfMagicNumber(*io_);

        readPgfHeaderSize(*io_);

        int w, h;
        DataBuf header      = readPgfHeaderStructure(*io_, w, h);

        Image::UniquePtr img  = ImageFactory::create(ImageType::png);

        img->setExifData(exifData_);
        img->setIptcData(iptcData_);
        img->setXmpData(xmpData_);
        img->writeMetadata();
        size_t imgSize = img->io().size();
        DataBuf imgBuf = img->io().read((long)imgSize);

#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PgfImage::doWriteMetadata: Creating image to host metadata (" << imgSize << " bytes)\n";
#endif

        //---------------------------------------------------------------

        // Write PGF Signature.
        if (outIo.write(pgfSignature, 3) != 3) throw Error(ErrorCode::kerImageWriteFailed);

        // Write Magic number.
        if (outIo.putb(mnb) == EOF) throw Error(ErrorCode::kerImageWriteFailed);

        // Write new Header size.
        size_t newHeaderSize = header.size_ + imgSize;
        DataBuf buffer(4);
        memcpy (buffer.pData_, &newHeaderSize, 4);
        byteSwap_(buffer,0,bSwap_);
        if (outIo.write(buffer.pData_, 4) != 4) throw Error(ErrorCode::kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PgfImage: new PGF header size : " << newHeaderSize << " bytes\n";

        printf("%x\n", buffer.pData_[0]);
        printf("%x\n", buffer.pData_[1]);
        printf("%x\n", buffer.pData_[2]);
        printf("%x\n", buffer.pData_[3]);
#endif

        // Write Header data.
        if (outIo.write(header.pData_, header.size_) != header.size_) throw Error(ErrorCode::kerImageWriteFailed);

        // Write new metadata byte array.
        if (outIo.write(imgBuf.pData_, imgBuf.size_) != imgBuf.size_) throw Error(ErrorCode::kerImageWriteFailed);

        // Copy the rest of PGF image data.

        DataBuf buf(4096);
        size_t readSize = 0;
        while ((readSize=io_->read(buf.pData_, buf.size_)))
        {
            if (outIo.write(buf.pData_, readSize) != readSize) throw Error(ErrorCode::kerImageWriteFailed);
        }
        if (outIo.error()) throw Error(ErrorCode::kerImageWriteFailed);

    } // PgfImage::doWriteMetadata

    byte PgfImage::readPgfMagicNumber(BasicIo& iIo)
    {
        byte b = iIo.getb();
        if (iIo.error()) throw Error(ErrorCode::kerFailedToReadImageData);

        if (b < 0x36)   // 0x36 = '6'.
        {
            // Not right Magick version.
#ifdef EXIV2_DEBUG_MESSAGES
            std::cout << "Exiv2::PgfImage::readMetadata: wrong Magick number\n";
#endif
        }

        return b;
    } // PgfImage::readPgfMagicNumber

    uint32_t PgfImage::readPgfHeaderSize(BasicIo& iIo)
    {
        DataBuf buffer(4);
        size_t bufRead = iIo.read(buffer.pData_, buffer.size_);
        if (iIo.error()) throw Error(ErrorCode::kerFailedToReadImageData);
        if (bufRead != buffer.size_) throw Error(ErrorCode::kerInputDataReadFailed);

        int headerSize = (int) byteSwap_(buffer,0,bSwap_);
        if (headerSize <= 0 ) throw Error(ErrorCode::kerNoImageInInputData);

#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::PgfImage: PGF header size : " << headerSize << " bytes\n";
#endif

        return headerSize;
    } // PgfImage::readPgfHeaderSize

    DataBuf PgfImage::readPgfHeaderStructure(BasicIo& iIo, int& width, int& height)
    {
        DataBuf header(16);
        size_t bufRead = iIo.read(header.pData_, header.size_);
        if (iIo.error()) throw Error(ErrorCode::kerFailedToReadImageData);
        if (bufRead != header.size_) throw Error(ErrorCode::kerInputDataReadFailed);

        DataBuf work(8);  // don't disturb the binary data - doWriteMetadata reuses it
        memcpy (work.pData_,header.pData_,8);
        width   = byteSwap_(work,0,bSwap_);
        height  = byteSwap_(work,4,bSwap_);

        /* NOTE: properties not yet used
        byte nLevels  = buffer.pData_[8];
        byte quality  = buffer.pData_[9];
        byte bpp      = buffer.pData_[10];
        byte channels = buffer.pData_[11];
        */
        byte mode     = header.pData_[12];

        if (mode == 2)  // Indexed color image. We pass color table (256 * 3 bytes).
        {
            header.alloc(16 + 256*3);

            bufRead = iIo.read(&header.pData_[16], 256*3);
            if (iIo.error()) throw Error(ErrorCode::kerFailedToReadImageData);
            if (bufRead != 256*3) throw Error(ErrorCode::kerInputDataReadFailed);
        }

        return header;
    } // PgfImage::readPgfHeaderStructure

    // *************************************************************************
    // free functions
    Image::UniquePtr newPgfInstance(BasicIo::UniquePtr io, bool create)
    {
        Image::UniquePtr image(new PgfImage(std::move(io), create));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isPgfType(BasicIo& iIo, bool advance)
    {
        const int32_t len = 3;
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        int rc = memcmp(buf, pgfSignature, 3);
        if (!advance || rc != 0)
        {
            iIo.seek(-len, BasicIo::cur);
        }

        return rc == 0;
    }
}                                       // namespace Exiv2
