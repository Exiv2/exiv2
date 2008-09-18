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


namespace Exiv2 {

    /*!
      Base class for image loaders. Provides virtual methods for reading properties
      and DataBuf.
     */
    class Loader {
    protected:
        Loader(PreviewProperties::PreviewId id, const Image &image);

    public:
        typedef std::auto_ptr<Loader> AutoPtr;

        //! Create a Loader subclass for requested id
        static Loader::AutoPtr create(PreviewProperties::PreviewId id, const Image &image);

        //! Check if a preview image with given params exists in the image
        virtual bool valid() const = 0;

        //! Get properties of a preview image with given params
        virtual PreviewProperties getProperties() const;

        //! Get properties of a preview image with given params
        virtual DataBuf getData() const = 0;

        //! A number of image loaders configured in the loaderList_ table
        static PreviewProperties::PreviewId getNumLoaders();
        
    protected:
        typedef AutoPtr (*CreateFunc)(PreviewProperties::PreviewId id, const Image &image, int parIdx);

        struct LoaderList {
            const char *imageMimeType_; //!< Image type for which is the loader valid, NULL matches all images
            CreateFunc create_;         //!< Function that creates particular loader instance
            int parIdx_;                //!< Parameter that is passed into CreateFunc

        };

        static const LoaderList loaderList_[]; // PreviewId is an index to this table

        PreviewProperties::PreviewId id_;
        const Image &image_;
    };


    //! Loader for Jpeg previews that are not read into ExifData directly
    class LoaderExifJpeg : public Loader {
    public:
        LoaderExifJpeg(PreviewProperties::PreviewId id, const Image &image, int parIdx);

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

    Loader::AutoPtr createLoaderExifJpeg(PreviewProperties::PreviewId id, const Image &image, int parIdx);


    //! Loader for standard Exif thumbnail - just a wrapper around ExifThumbC
    class LoaderExifThumbC : public Loader {
    public:
        LoaderExifThumbC(PreviewProperties::PreviewId id, const Image &image);

        virtual bool valid() const;
        virtual PreviewProperties getProperties() const;
        virtual DataBuf getData() const;

    protected:
	ExifThumbC thumb_;
    };

    Loader::AutoPtr createLoaderExifThumbC(PreviewProperties::PreviewId id, const Image &image, int parIdx);

// *****************************************************************************
// class member definitions

    const Loader::LoaderList Loader::loaderList_[] = {
        { NULL,                 createLoaderExifThumbC, 0},
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

    Loader::AutoPtr Loader::create(PreviewProperties::PreviewId id, const Image &image)
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

    Loader::Loader(PreviewProperties::PreviewId id, const Image &image)
            : id_(id), image_(image)
    {
    }

    PreviewProperties Loader::getProperties() const
    {
        PreviewProperties prop;
        prop.id_ = id_;
        return prop;
    }

    PreviewProperties::PreviewId Loader::getNumLoaders()
    {
        return (PreviewProperties::PreviewId)EXV_COUNTOF(loaderList_);
    }

    LoaderExifJpeg::LoaderExifJpeg(PreviewProperties::PreviewId id, const Image &image, int parIdx)
            : Loader(id, image), 
              offsetKey_(param_[parIdx].offsetKey_),
              lengthKey_(param_[parIdx].lengthKey_)
    {
    }

    Loader::AutoPtr createLoaderExifJpeg(PreviewProperties::PreviewId id, const Image &image, int parIdx)
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


    LoaderExifThumbC::LoaderExifThumbC(PreviewProperties::PreviewId id, const Image &image)
            : Loader(id, image), 
              thumb_(image_.exifData())
    {
    }

    Loader::AutoPtr createLoaderExifThumbC(PreviewProperties::PreviewId id, const Image &image, int /* parIdx */)
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

    bool cmpPreviewProperties(const PreviewProperties& lhs, const PreviewProperties& rhs)
    {
        return lhs.length_ < rhs.length_;
    }

    PreviewImageLoader::PreviewImageLoader(const Image& image)
            : image_(image)
    {
    }

    PreviewPropertiesList PreviewImageLoader::getPreviewPropertiesList() const
    {
        PreviewPropertiesList list;
        // go through the loader table and store all successfuly created loaders in the list
        for (PreviewProperties::PreviewId id = 0; id < Loader::getNumLoaders(); id++) {
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
