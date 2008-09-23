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
  File:      preview.cpp
  Version:   $Rev$
  Author(s): Vladimir Nadvornik (vn) <nadvornik@suse.cz>
  History:   18-Sep-08, vn: created
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

#include <string>

#include "preview.hpp"
#include "futils.hpp"

#include "image.hpp"
#include "cr2image.hpp"
#include "tiffimage.hpp"

// *****************************************************************************
namespace {
    /*!
      @brief Compare two preview images by length. Return true if the 
             lhs is smaller than rhs.
     */
    bool cmpPreviewProperties(
        const Exiv2::PreviewProperties& lhs,
        const Exiv2::PreviewProperties& rhs
    )
    {
        return lhs.length_ < rhs.length_;
    }
}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    /*!
      Base class for image loaders. Provides virtual methods for reading properties
      and DataBuf.
     */
    class Loader {
    protected:
        Loader(PreviewId id, const Image &image);

    public:
        typedef std::auto_ptr<Loader> AutoPtr;

        //! Create a Loader subclass for requested id
        static Loader::AutoPtr create(PreviewId id, const Image &image);

        //! Check if a preview image with given params exists in the image
        virtual bool valid() const = 0;

        //! Get properties of a preview image with given params
        virtual PreviewProperties getProperties() const;

        //! Get properties of a preview image with given params
        virtual DataBuf getData() const = 0;

        //! A number of image loaders configured in the loaderList_ table
        static PreviewId getNumLoaders();
        
    protected:
        typedef AutoPtr (*CreateFunc)(PreviewId id, const Image &image, int parIdx);

        struct LoaderList {
            const char *imageMimeType_; //!< Image type for which is the loader valid, NULL matches all images
            CreateFunc create_;         //!< Function that creates particular loader instance
            int parIdx_;                //!< Parameter that is passed into CreateFunc

        };

        static const LoaderList loaderList_[]; // PreviewId is an index to this table

        PreviewId id_;
        const Image &image_;
    };


    //! Loader for Jpeg previews that are not read into ExifData directly
    class LoaderExifJpeg : public Loader {
    public:
        LoaderExifJpeg(PreviewId id, const Image &image, int parIdx);

        virtual bool valid() const;
        virtual PreviewProperties getProperties() const;
        virtual DataBuf getData() const;

        long getOffset() const;
        long getLength() const;

    protected:
    
        // this table lists possible offset/length key pairs
        // parIdx is an index to this table
        
        struct Param {
            const char* offsetKey_;  
            const char* lengthKey_;
        };
        static const Param param_[];
        
        ExifKey offsetKey_;
        ExifKey lengthKey_;
    };

    Loader::AutoPtr createLoaderExifJpeg(PreviewId id, const Image &image, int parIdx);


    //! Loader for standard Exif thumbnail - just a wrapper around ExifThumbC
    class LoaderExifThumbC : public Loader {
    public:
        LoaderExifThumbC(PreviewId id, const Image &image);

        virtual bool valid() const;
        virtual PreviewProperties getProperties() const;
        virtual DataBuf getData() const;

    protected:
	ExifThumbC thumb_;
    };

    Loader::AutoPtr createLoaderExifThumbC(PreviewId id, const Image &image, int parIdx);

    //! Loader for Tiff previews
    class LoaderTiff : public Loader {
    public:
        LoaderTiff(PreviewId id, const Image &image, int parIdx);

        virtual bool valid() const;
        virtual PreviewProperties getProperties() const;
        virtual DataBuf getData() const;

    protected:
        const char *group_;
        long length_;
        bool valid_;
        std::string offsetTag_;
        std::string sizeTag_;

        // this table lists possible groups
        // parIdx is an index to this table
        
        struct Param {
            const char* group_;  
        };
        static const Param param_[];


    };

    Loader::AutoPtr createLoaderTiff(PreviewId id, const Image &image, int parIdx);

// *****************************************************************************
// class member definitions

    const Loader::LoaderList Loader::loaderList_[] = {
        { NULL,                 createLoaderExifThumbC, 0},
        { NULL,                 createLoaderTiff, 0},
        { NULL,                 createLoaderTiff, 1},
        { NULL,                 createLoaderTiff, 2},
        { NULL,                 createLoaderExifJpeg, 0},
        { NULL,                 createLoaderExifJpeg, 1},
        { NULL,                 createLoaderExifJpeg, 2},
        { NULL,                 createLoaderExifJpeg, 3},
        { "image/x-canon-cr2",  createLoaderExifJpeg, 4} // FIXME: this needs to be fixed (enabled) in cr2image.cpp
        };

    const LoaderExifJpeg::Param LoaderExifJpeg::param_[] = {
        { "Exif.Image.JPEGInterchangeFormat",           "Exif.Image.JPEGInterchangeFormatLength"        }, // 0
        { "Exif.SubImage1.JPEGInterchangeFormat",       "Exif.SubImage1.JPEGInterchangeFormatLength",   }, // 1
        { "Exif.SubImage2.JPEGInterchangeFormat",       "Exif.SubImage2.JPEGInterchangeFormatLength",   }, // 2
        { "Exif.Image2.JPEGInterchangeFormat",          "Exif.Image2.JPEGInterchangeFormatLength",      }, // 3
        { "Exif.Image.StripOffsets",                    "Exif.Image.StripByteCounts",                   }, // 4
        };

    const LoaderTiff::Param LoaderTiff::param_[] = {
        { "Image" }, // 0
        { "SubImage1" }, // 1
        { "SubImage2" }, // 2
        };

    PreviewImage::PreviewImage(const PreviewProperties &properties, DataBuf &data)
            : properties_(properties), data_(data)
    {
    }

    PreviewImage::PreviewImage(const PreviewImage &src)
            : properties_(src.properties_), data_(const_cast<DataBuf&>(src.data_))
    {
    }


    long PreviewImage::writeFile(const std::string& path) const
    {
        std::string name = path + extension();
        return Exiv2::writeFile(data_, name);
    }

    DataBuf &PreviewImage::data()
    {
        return data_;
    }

    const char* PreviewImage::mimeType() const
    {
        return properties_.mimeType_;
    }

    const char* PreviewImage::extension() const
    {
        return properties_.extension_;
    }

    long PreviewImage::length() const
    {
        return properties_.length_;
    }

    Loader::AutoPtr Loader::create(PreviewId id, const Image &image)
    {
            if (id < 0 || id >= Loader::getNumLoaders())
                return AutoPtr();

            if (loaderList_[id].imageMimeType_ &&
                std::string(loaderList_[id].imageMimeType_) != std::string(image.mimeType()))
                return AutoPtr();

            AutoPtr loader = loaderList_[id].create_(id, image, loaderList_[id].parIdx_);

            if (loader.get() && !loader->valid()) loader.reset();
            return loader;
    }

    Loader::Loader(PreviewId id, const Image &image)
            : id_(id), image_(image)
    {
    }

    PreviewProperties Loader::getProperties() const
    {
        PreviewProperties prop;
        prop.id_ = id_;
        return prop;
    }

    PreviewId Loader::getNumLoaders()
    {
        return (PreviewId)EXV_COUNTOF(loaderList_);
    }

    LoaderExifJpeg::LoaderExifJpeg(PreviewId id, const Image &image, int parIdx)
            : Loader(id, image), 
              offsetKey_(param_[parIdx].offsetKey_),
              lengthKey_(param_[parIdx].lengthKey_)
    {
    }

    Loader::AutoPtr createLoaderExifJpeg(PreviewId id, const Image &image, int parIdx)
    {
            return Loader::AutoPtr(new LoaderExifJpeg(id, image, parIdx));
    }

    long LoaderExifJpeg::getLength() const
    {
        long length = 0;
        ExifData::const_iterator pos = image_.exifData().findKey(lengthKey_);
        if (pos != image_.exifData().end()) {
            length = pos->toLong();
        }
        return length;
    }

    long LoaderExifJpeg::getOffset() const
    {
        long offset = 0;
        ExifData::const_iterator pos = image_.exifData().findKey(offsetKey_);
        if (pos != image_.exifData().end()) {
            offset = pos->toLong();
        }
        return offset;
    }

    bool LoaderExifJpeg::valid() const
    {
        long offset = getOffset();
        long length = getLength();

        if (offset == 0 || length == 0) return false;
        if (offset + length > image_.io().size()) return false;
        return true;
    }

    PreviewProperties LoaderExifJpeg::getProperties() const
    {
        PreviewProperties prop = Loader::getProperties();
        prop.mimeType_ = "image/jpeg";
        prop.extension_ = ".jpg";
        prop.length_ = getLength(); 
        return prop;
    }

    DataBuf LoaderExifJpeg::getData() const
    {
        if (!valid()) return DataBuf();
        BasicIo &io = image_.io();

        if (io.open() != 0) {
            throw Error(9, io.path(), strError());
        }
        IoCloser closer(io);

        long offset = getOffset();
        long length = getLength();

        const byte *base = io.mmap();

        return DataBuf(base + offset, length);
    }


    LoaderExifThumbC::LoaderExifThumbC(PreviewId id, const Image &image)
            : Loader(id, image), 
              thumb_(image_.exifData())
    {
    }

    Loader::AutoPtr createLoaderExifThumbC(PreviewId id, const Image &image, int /* parIdx */)
    {
        return Loader::AutoPtr(new LoaderExifThumbC(id, image));
    }

    bool LoaderExifThumbC::valid() const
    {
        return thumb_.copy().size_ > 0; // FIXME: this is inefficient
    }

    PreviewProperties LoaderExifThumbC::getProperties() const
    {
        PreviewProperties prop = Loader::getProperties();
        prop.length_ = thumb_.copy().size_; // FIXME: this is inefficient
        prop.mimeType_ = thumb_.mimeType();
        prop.extension_ = thumb_.extension();
        return prop;
    }

    DataBuf LoaderExifThumbC::getData() const
    {
        return thumb_.copy();
    }

    LoaderTiff::LoaderTiff(PreviewId id, const Image &image, int parIdx)
            : Loader(id, image),
              group_(param_[parIdx].group_),
              length_(0), valid_(false)
    {
        const ExifData &exifData = image_.exifData();

        int offsetCount = 0;

        // check if the group_ contains a preview image

        ExifData::const_iterator pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".NewSubfileType"));
        if (pos == image_.exifData().end() || pos->value().toLong() != 1) {
            return;
        }

        pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".StripOffsets"));
        if (pos != image_.exifData().end()) {
            offsetTag_ = "StripOffsets";
            sizeTag_ = "StripByteCounts";
            offsetCount = pos->value().count();
        } 
        else {
            pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".TileOffsets"));
            if (pos != image_.exifData().end()) {
                offsetTag_ = "TileOffsets";
                sizeTag_ = "TileByteCounts";
                offsetCount = pos->value().count();
            }
            else {
                return;
            }
        }

        pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + '.' + sizeTag_));
        if (pos != image_.exifData().end()) {
            if (offsetCount != pos->value().count()) return;
            for (int i = 0; i < offsetCount; i++)
                length_ += pos->value().toLong(i);
        }
        else return;

        if (length_ == 0) return;

        valid_ = true;
    }

    Loader::AutoPtr createLoaderTiff(PreviewId id, const Image &image, int parIdx)
    {
        return Loader::AutoPtr(new LoaderTiff(id, image, parIdx));
    }

    bool LoaderTiff::valid() const
    {
        return valid_;
    }

    PreviewProperties LoaderTiff::getProperties() const
    {
        PreviewProperties prop = Loader::getProperties();
        prop.length_ = length_;
        prop.mimeType_ = "image/tiff";
        prop.extension_ = ".tif";
        return prop;
    }

    DataBuf LoaderTiff::getData() const
    {
        const ExifData &exifData = image_.exifData();

        ExifData preview;

        // copy tags
        for (ExifData::const_iterator pos = exifData.begin(); pos != exifData.end(); ++pos) {
            if (pos->groupName() == group_) {
                if (pos->tagName() == "NewSubfileType")
                    continue;

                preview.add(ExifKey("Exif.Image." + pos->tagName()), &pos->value());
            }
        }

        // read image data
        BasicIo &io = image_.io();

        if (io.open() != 0) {
            throw Error(9, io.path(), strError());
        }
        IoCloser closer(io);

        const byte *base = io.mmap();

        Value &dataValue = const_cast<Value&>(preview["Exif.Image." + offsetTag_].value());
        const Value &sizes = preview["Exif.Image." + sizeTag_].value();

        if (sizes.count() == 1) {
            // this saves one copying of the buffer
            uint32_t offset = dataValue.toLong(0);
            uint32_t length = sizes.toLong(0);
            if (offset + length <= static_cast<uint32_t>(io.size()))
                dataValue.setDataArea(base + offset, length);
        }
        else {
            // FIXME: the buffer is probably copied twice, it should be optimized
            DataBuf buf(length_);
            byte *pos = buf.pData_;
            for (int i = 0; i < sizes.count(); i++) {
                uint32_t offset = dataValue.toLong(i);
                uint32_t length = sizes.toLong(i);
                if (offset + length <= static_cast<uint32_t>(io.size()))
                    memcpy(pos, base + offset, length);
                pos += length;
            }
            dataValue.setDataArea(buf.pData_, buf.size_);
        }

        // write new image
        Blob blob;
        const IptcData emptyIptc;
        const XmpData  emptyXmp;
        TiffParser::encode(blob, 0, 0, Exiv2::littleEndian, preview, emptyIptc, emptyXmp);
        return DataBuf(&blob[0], static_cast<long>(blob.size()));
    }


    PreviewImageLoader::PreviewImageLoader(const Image& image)
            : image_(image)
    {
    }

    PreviewPropertiesList PreviewImageLoader::getPreviewPropertiesList() const
    {
        PreviewPropertiesList list;
        // go through the loader table and store all successfuly created loaders in the list
        for (PreviewId id = 0; id < Loader::getNumLoaders(); id++) {
            Loader::AutoPtr loader = Loader::create(id, image_);
            if (loader.get()) {
                list.push_back(loader->getProperties());
            }
        }
        std::sort(list.begin(), list.end(), cmpPreviewProperties);
        return list;
    }

    PreviewImage PreviewImageLoader::getPreviewImage(const PreviewProperties &properties) const
    {
        Loader::AutoPtr loader = Loader::create(properties.id_, image_);
        DataBuf buf;
        if (loader.get()) {
            buf = loader->getData();
        }
        return PreviewImage(properties, buf);
    }

}                                       // namespace Exiv2
