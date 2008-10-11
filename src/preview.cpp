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
      @brief Compare two preview images by number of pixels, if width and height
             of both lhs and rhs are available or else by size.
             Return true if lhs is smaller than rhs.
     */
    bool cmpPreviewProperties(
        const Exiv2::PreviewProperties& lhs,
        const Exiv2::PreviewProperties& rhs
    )
    {
        uint32_t l = lhs.width_ * lhs.height_;
        uint32_t r = rhs.width_ * rhs.height_;

        return l < r;
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
        //! Constructor. Sets all image properies to unknown.
        Loader(PreviewId id, const Image &image);

    public:
        //! Loader auto pointer
        typedef std::auto_ptr<Loader> AutoPtr;

        //! Create a Loader subclass for requested id
        static Loader::AutoPtr create(PreviewId id, const Image &image);

        //! Check if a preview image with given params exists in the image
        virtual bool valid() const { return valid_; }

        //! Get properties of a preview image with given params
        virtual PreviewProperties getProperties() const;

        //! Get a buffer that contains the preview image
        virtual DataBuf getData() const = 0;

        //! Read preview image dimensions when they are not available directly
        virtual bool readDimensions() { return true; }

        //! A number of image loaders configured in the loaderList_ table
        static PreviewId getNumLoaders();
        
    protected:
        //! Functions that creates a loader from given parameters
        typedef AutoPtr (*CreateFunc)(PreviewId id, const Image &image, int parIdx);

        //! Structure to list possible loaders
        struct LoaderList {
            const char *imageMimeType_; //!< Image type for which is the loader valid, NULL matches all images
            CreateFunc create_;         //!< Function that creates particular loader instance
            int parIdx_;                //!< Parameter that is passed into CreateFunc
        };

        //! Table that lists possible loaders.  PreviewId is an index to this table.
        static const LoaderList loaderList_[];

        //! Identifies preview image type
        PreviewId id_;
        
        //! Source image reference
        const Image &image_;

        //! Preview image width
        uint32_t width_;

        //! Preview image length
        uint32_t height_;
        
        //! Preview image size in bytes
        uint32_t size_;
        
        //! True if the source image contains a preview image of given type
        bool valid_;
    };


    //! Loader for Jpeg previews that are not read into ExifData directly
    class LoaderExifJpeg : public Loader {
    public:
    
        //! Constructor
        LoaderExifJpeg(PreviewId id, const Image &image, int parIdx);

        //! Get properties of a preview image with given params
        virtual PreviewProperties getProperties() const;

        //! Get a buffer that contains the preview image
        virtual DataBuf getData() const;

        //! Read preview image dimensions
        virtual bool readDimensions();

    protected:

        //! Returns value of offset key
        uint32_t getOffset() const;

        //! Returns value of size key
        uint32_t getSize() const;
    
        //! Structure that lists offset/size tag pairs
        struct Param {
            const char* offsetKey_; //!< Offset tag
            const char* sizeKey_; //!< Size tag
        };

        //! Table that holds all possible offset/size pairs. parIdx is an index to this table
        static const Param param_[];

        //! Key that contains the offset of the JPEG preview in image file
        ExifKey offsetKey_;

        //! Key that contains the JPEG preview size
        ExifKey sizeKey_;

        //! Offset value
        uint32_t offset_;
    };

    //! Function to create new LoaderExifJpeg
    Loader::AutoPtr createLoaderExifJpeg(PreviewId id, const Image &image, int parIdx);

    //! Loader for Jpeg previews that are read into ExifData
    class LoaderExifDataJpeg : public Loader {
    public:
        //! Constructor
        LoaderExifDataJpeg(PreviewId id, const Image &image, int parIdx);

        //! Get properties of a preview image with given params
        virtual PreviewProperties getProperties() const;

        //! Get a buffer that contains the preview image
        virtual DataBuf getData() const;

        //! Read preview image dimensions
        virtual bool readDimensions();

    protected:

        //! Structure that lists data/size tag pairs
        struct Param {
            const char* dataKey_; //!< Data tag
            const char* sizeKey_; //!< Size tag
        };

        //! Table that holds all possible data/size pairs. parIdx is an index to this table
        static const Param param_[];
        
        //! Key that points to the Value that contains the JPEG preview in data area
        ExifKey dataKey_;

        //! Key that contains the JPEG preview size
        ExifKey sizeKey_;
    };

    //! Function to create new LoaderExifDataJpeg
    Loader::AutoPtr createLoaderExifDataJpeg(PreviewId id, const Image &image, int parIdx);


    //! Loader for Tiff previews - it can get image data from ExifData or image_.io() as needed
    class LoaderTiff : public Loader {
    public:
        //! Constructor
        LoaderTiff(PreviewId id, const Image &image, int parIdx);

        //! Get properties of a preview image with given params
        virtual PreviewProperties getProperties() const;

        //! Get a buffer that contains the preview image
        virtual DataBuf getData() const;

    protected:
        //! Name of the group that contains the preview image
        const char *group_;
        
        //! Tag that contains image data. Possible values are "StripOffsets" or "TileOffsets"
        std::string offsetTag_;

        //! Tag that contains data sizes. Possible values are "StripByteCountss" or "TileByteCounts"
        std::string sizeTag_;

        //! Structure that lists preview groups
        struct Param {
            const char* group_; //!< Group name
        };

        //! Table that holds all possible groups. parIdx is an index to this table.
        static const Param param_[];

    };

    //! Function to create new LoaderTiff
    Loader::AutoPtr createLoaderTiff(PreviewId id, const Image &image, int parIdx);

// *****************************************************************************
// class member definitions

    const Loader::LoaderList Loader::loaderList_[] = {
        { NULL,                 createLoaderExifDataJpeg, 0},
        { NULL,                 createLoaderExifDataJpeg, 1},
        { NULL,                 createLoaderExifDataJpeg, 2},
        { NULL,                 createLoaderExifDataJpeg, 3},
        { NULL,                 createLoaderTiff, 0},
        { NULL,                 createLoaderTiff, 1},
        { NULL,                 createLoaderTiff, 2},
        { NULL,                 createLoaderTiff, 3},
        { NULL,                 createLoaderTiff, 4},
        { NULL,                 createLoaderTiff, 5},
        { NULL,                 createLoaderExifJpeg, 0},
        { NULL,                 createLoaderExifJpeg, 1},
        { NULL,                 createLoaderExifJpeg, 2},
        { NULL,                 createLoaderExifJpeg, 3},
        { NULL,                 createLoaderExifJpeg, 4},
        { NULL,                 createLoaderExifJpeg, 5},
        { "image/x-canon-cr2",  createLoaderExifJpeg, 6}
    };

    const LoaderExifJpeg::Param LoaderExifJpeg::param_[] = {
        { "Exif.Image.JPEGInterchangeFormat",           "Exif.Image.JPEGInterchangeFormatLength"        }, // 0
        { "Exif.SubImage1.JPEGInterchangeFormat",       "Exif.SubImage1.JPEGInterchangeFormatLength"    }, // 1
        { "Exif.SubImage2.JPEGInterchangeFormat",       "Exif.SubImage2.JPEGInterchangeFormatLength"    }, // 2
        { "Exif.SubImage3.JPEGInterchangeFormat",       "Exif.SubImage3.JPEGInterchangeFormatLength"    }, // 3
        { "Exif.SubImage4.JPEGInterchangeFormat",       "Exif.SubImage4.JPEGInterchangeFormatLength"    }, // 4
        { "Exif.Image2.JPEGInterchangeFormat",          "Exif.Image2.JPEGInterchangeFormatLength"       }, // 5
        { "Exif.Image.StripOffsets",                    "Exif.Image.StripByteCounts"                    }  // 6
    };

    const LoaderExifDataJpeg::Param LoaderExifDataJpeg::param_[] = {
        { "Exif.Thumbnail.JPEGInterchangeFormat",       "Exif.Thumbnail.JPEGInterchangeFormatLength"    }, // 0
        { "Exif.NikonPreview.JPEGInterchangeFormat",    "Exif.NikonPreview.JPEGInterchangeFormatLength" }, // 1
        { "Exif.Pentax.PreviewOffset",                  "Exif.Pentax.PreviewLength"                     }, // 2
        { "Exif.Minolta.ThumbnailOffset",               "Exif.Minolta.ThumbnailLength"                  }  // 3
    };

    const LoaderTiff::Param LoaderTiff::param_[] = {
        { "Image" },     // 0
        { "SubImage1" }, // 1
        { "SubImage2" }, // 2
        { "SubImage3" }, // 3
        { "SubImage4" }, // 4
        { "Thumbnail" }  // 5
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

    uint32_t PreviewImage::size() const
    {
        return properties_.size_;
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
        : id_(id), image_(image),
          width_(0), height_(0),
          size_(0),
          valid_(false)
    {
    }

    PreviewProperties Loader::getProperties() const
    {
        PreviewProperties prop;
        prop.id_ = id_;
        prop.size_ = size_;
        prop.width_ = width_;
        prop.height_ = height_;
        return prop;
    }

    PreviewId Loader::getNumLoaders()
    {
        return (PreviewId)EXV_COUNTOF(loaderList_);
    }

    LoaderExifJpeg::LoaderExifJpeg(PreviewId id, const Image &image, int parIdx)
        : Loader(id, image), 
          offsetKey_(param_[parIdx].offsetKey_),
          sizeKey_(param_[parIdx].sizeKey_)
    {
        offset_ = getOffset();
        size_ = getSize();

        if (offset_ == 0 || size_ == 0) return;
        if (offset_ + size_ > static_cast<uint32_t>(image_.io().size())) return;
        
        valid_ = true;
    }

    Loader::AutoPtr createLoaderExifJpeg(PreviewId id, const Image &image, int parIdx)
    {
        return Loader::AutoPtr(new LoaderExifJpeg(id, image, parIdx));
    }

    uint32_t LoaderExifJpeg::getSize() const
    {
        uint32_t size = 0;
        ExifData::const_iterator pos = image_.exifData().findKey(sizeKey_);
        if (pos != image_.exifData().end()) {
            size = pos->toLong();
        }
        return size;
    }

    uint32_t LoaderExifJpeg::getOffset() const
    {
        uint32_t offset = 0;
        ExifData::const_iterator pos = image_.exifData().findKey(offsetKey_);
        if (pos != image_.exifData().end()) {
            offset = pos->toLong();
        }
        return offset;
    }

    PreviewProperties LoaderExifJpeg::getProperties() const
    {
        PreviewProperties prop = Loader::getProperties();
        prop.mimeType_ = "image/jpeg";
        prop.extension_ = ".jpg";
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

        const byte *base = io.mmap();

        return DataBuf(base + offset_, size_);
    }

    bool LoaderExifJpeg::readDimensions()
    {
        if (!valid()) return false;
        if (width_ || height_) return true;

        BasicIo &io = image_.io();

        if (io.open() != 0) {
            throw Error(9, io.path(), strError());
        }
        IoCloser closer(io);
        const byte *base = io.mmap();

        try {
            Image::AutoPtr image = ImageFactory::open(base + offset_, size_);
            if (image.get() == 0) return false;
            image->readMetadata();

            width_ = image->pixelWidth();
            height_ = image->pixelHeight();
        }
        catch (const AnyError& error) {
            return false;
        }

        return true;
    }

    LoaderExifDataJpeg::LoaderExifDataJpeg(PreviewId id, const Image &image, int parIdx)
        : Loader(id, image), 
          dataKey_(param_[parIdx].dataKey_),
          sizeKey_(param_[parIdx].sizeKey_)
    {
        ExifData::const_iterator pos = image_.exifData().findKey(dataKey_);
        if (pos != image_.exifData().end()) {
            size_ = pos->sizeDataArea();
        }

        if (size_ == 0) return;

        valid_ = true;
    }

    Loader::AutoPtr createLoaderExifDataJpeg(PreviewId id, const Image &image, int parIdx)
    {
        return Loader::AutoPtr(new LoaderExifDataJpeg(id, image, parIdx));
    }

    PreviewProperties LoaderExifDataJpeg::getProperties() const
    {
        PreviewProperties prop = Loader::getProperties();
        prop.mimeType_ = "image/jpeg";
        prop.extension_ = ".jpg";
        return prop;
    }

    DataBuf LoaderExifDataJpeg::getData() const
    {
        if (!valid()) return DataBuf();

        ExifData::const_iterator pos = image_.exifData().findKey(dataKey_);
        if (pos != image_.exifData().end()) {
            DataBuf buf = pos->dataArea();

            buf.pData_[0] = 0xff; // fix Minolta thumbnails with invalid jpeg header
            return buf;
        }

        return DataBuf();
    }

    bool LoaderExifDataJpeg::readDimensions()
    {
        if (!valid()) return false;

        ExifData::const_iterator pos = image_.exifData().findKey(dataKey_);
        if (pos == image_.exifData().end()) return false;

        DataBuf buf = pos->dataArea();

        buf.pData_[0] = 0xff; // fix Minolta thumbnails with invalid jpeg header

        try {
            Image::AutoPtr image = ImageFactory::open(buf.pData_, buf.size_);
            if (image.get() == 0) return false;
            image->readMetadata();

            width_ = image->pixelWidth();
            height_ = image->pixelHeight();
        }
        catch (const AnyError& error) {
            return false;
        }

        return true;
    }

    LoaderTiff::LoaderTiff(PreviewId id, const Image &image, int parIdx)
        : Loader(id, image),
          group_(param_[parIdx].group_)
    {
        const ExifData &exifData = image_.exifData();

        int offsetCount = 0;

        // check if the group_ contains a preview image

        ExifData::const_iterator pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".NewSubfileType"));
        if (pos == exifData.end() || pos->value().toLong() != 1) {
            return;
        }

        pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".StripOffsets"));
        if (pos != exifData.end()) {
            offsetTag_ = "StripOffsets";
            sizeTag_ = "StripByteCounts";
            offsetCount = pos->value().count();
        } 
        else {
            pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".TileOffsets"));
            if (pos != exifData.end()) {
                offsetTag_ = "TileOffsets";
                sizeTag_ = "TileByteCounts";
                offsetCount = pos->value().count();
            }
            else {
                return;
            }
        }

        pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + '.' + sizeTag_));
        if (pos != exifData.end()) {
            if (offsetCount != pos->value().count()) return;
            for (int i = 0; i < offsetCount; i++)
                size_ += pos->value().toLong(i);
        }
        else return;

        if (size_ == 0) return;

        pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".ImageWidth"));
        if (pos != exifData.end()) {
            width_ = pos->value().toLong();
        }

        pos = exifData.findKey(ExifKey(std::string("Exif.") + group_ + ".ImageLength"));
        if (pos != exifData.end()) {
            height_ = pos->value().toLong();
        }
        
        if (width_ == 0 || height_ == 0) return;

        valid_ = true;
    }

    Loader::AutoPtr createLoaderTiff(PreviewId id, const Image &image, int parIdx)
    {
        return Loader::AutoPtr(new LoaderTiff(id, image, parIdx));
    }

    PreviewProperties LoaderTiff::getProperties() const
    {
        PreviewProperties prop = Loader::getProperties();
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

        Value &dataValue = const_cast<Value&>(preview["Exif.Image." + offsetTag_].value());

        if (dataValue.sizeDataArea() == 0) {
            // image data are not available via exifData, read them from image_.io()
            BasicIo &io = image_.io();

            if (io.open() != 0) {
                throw Error(9, io.path(), strError());
            }
            IoCloser closer(io);

            const byte *base = io.mmap();

            const Value &sizes = preview["Exif.Image." + sizeTag_].value();

            if (sizes.count() == 1) {
                // this saves one copying of the buffer
                uint32_t offset = dataValue.toLong(0);
                uint32_t size = sizes.toLong(0);
                if (offset + size <= static_cast<uint32_t>(io.size()))
                    dataValue.setDataArea(base + offset, size);
            }
            else {
                // FIXME: the buffer is probably copied twice, it should be optimized
                DataBuf buf(size_);
                byte *pos = buf.pData_;
                for (int i = 0; i < sizes.count(); i++) {
                    uint32_t offset = dataValue.toLong(i);
                    uint32_t size = sizes.toLong(i);
                    if (offset + size <= static_cast<uint32_t>(io.size()))
                        memcpy(pos, base + offset, size);
                    pos += size;
                }
                dataValue.setDataArea(buf.pData_, buf.size_);
            }
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
            if (loader.get() && loader->readDimensions()) {
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
