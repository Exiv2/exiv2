// *****************************************************************************
// included header files
#include "config.h"

#ifdef EXIV2_ENABLE_HEIF

#include "basicio.hpp"
#include "error.hpp"
#include "enforce.hpp"
#include "futils.hpp"
#include "image.hpp"
#include "image_int.hpp"
#include "heifimage_int.hpp"
#include "types.hpp"

// + standard includes
#include <cassert>
#include <cstring>
#include <iostream>
#include <iterator>
#include <string>

const unsigned char HeifSignature[] = { 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70, 0x68, 0x65, 0x69, 0x63 };
const unsigned char HeifBlank[] = { 0x00, 0x00, 0x00, 0x18, 0x66, 0x74, 0x79, 0x70, 0x68, 0x65, 0x69, 0x63 };

// *****************************************************************************
// class member definitions
namespace Exiv2 {
    using namespace Exiv2::Internal;

    HeifImage::HeifImage(BasicIo::UniquePtr io, bool create)
            : Image(ImageType::heif, mdExif | mdXmp, std::move(io))
    {
        if (create)
        {
            if (io_->open() == 0)
            {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Exiv2::HeifImage:: Creating HEIF image to memory" << std::endl;
#endif
                IoCloser closer(*io_);
                if (io_->write(HeifBlank, sizeof(HeifBlank)) != sizeof(HeifBlank))
                {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cerr << "Exiv2::HeifImage:: Failed to create HEIF image on memory" << std::endl;
#endif
                }
            }
        }
    } // HeifImage::HeifImage

    HeifImage::~HeifImage()
    {
    }

    std::string HeifImage::mimeType() const
    {
        return "image/heif";
    }

    void HeifImage::setComment(const std::string& /*comment*/)
    {
        throw(Error(kerInvalidSettingForImage, "Image comment", "HEIF"));
    } // HeifImage::setComment

    void HeifImage::readMetadata()
    {
#ifdef EXIV2_DEBUG_MESSAGES
        std::cerr << "Exiv2::HeifImage::readMetadata: Reading HEIF file " << io_->path() << std::endl;
#endif
        if (io_->open() != 0)
        {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        // Ensure that this is the correct image type
        if (!isHeifType(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(kerFailedToReadImageData);
            throw Error(kerNotAnImage, "HEIF");
        }

        std::shared_ptr<heif_context> ctx(heif_context_alloc(),
                                    [] (heif_context* c) { heif_context_free(c); });
        if (!ctx) throw Error(kerFailedToReadImageData);

        struct heif_error err;
        err = heif_context_read_from_file(ctx.get(), io_->path().c_str(), nullptr);

        if (err.code != 0)
        {
#ifdef EXIV2_DEBUG_MESSAGES
            std::cerr << "Exiv2::HeifImage::readMetadata: Could not read HEIF file: " << err.message << std::endl;
#endif
            throw Error(kerFailedToReadImageData);
        }

        int numImages = heif_context_get_number_of_top_level_images(ctx.get());
        heif_item_id* IDs = (heif_item_id*)alloca(numImages * sizeof(heif_item_id));
        heif_context_get_list_of_top_level_image_IDs(ctx.get(), IDs, numImages);

        for (int i = 0; i < numImages; i++)
        {
            struct heif_image_handle* handle;
            struct heif_error err = heif_context_get_image_handle(ctx.get(), IDs[i], &handle);
            if (err.code)
            {
#ifdef EXIV2_DEBUG_MESSAGES
                std::cerr << "Exiv2::HeifImage::readMetadata: " << err.message << std::endl;
#endif
                throw Error(kerFailedToReadImageData);
            }

            pixelWidth_ = heif_image_handle_get_width(handle);
            pixelHeight_ = heif_image_handle_get_height(handle);

	    int num_metadata;
	    heif_item_id metadata_id;
	    num_metadata = heif_image_handle_get_list_of_metadata_block_IDs(handle, "Exif", &metadata_id, 1);

	    if (num_metadata > 0)
	    {
		size_t data_size = heif_image_handle_get_metadata_size(handle, metadata_id);

		uint8_t* data = (uint8_t*) alloca(data_size);
		err = heif_image_handle_get_metadata(handle, metadata_id, data);
                if (err.code)
                {
#ifdef EXIV2_DEBUG_MESSAGES
                    std::cerr << "Exiv2::HeifImage::readMetadata: " << err.message << std::endl;
#endif
                    throw Error(kerFailedToReadImageData);
                }

		// hexdump (std::cerr, data, data_size);

		ByteOrder bo = ExifParser::decode(exifData_, data + 10, data_size - 10);
                setByteOrder(bo);
                if (data_size > 0 && byteOrder() == invalidByteOrder)
		{
#ifndef SUPPRESS_WARNINGS
                    EXV_WARNING << "Failed to decode Exif metadata.\n";
#endif
                    exifData_.clear();
                }
	    }
            heif_image_handle_release(handle);
        }

    } // HeifImage::readMetadata

    void HeifImage::printStructure(std::ostream& out, PrintStructureOption option, int depth)
    {
        if (io_->open() != 0)
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());

        // Ensure that this is the correct image type
        if (!isHeifType(*io_, false)) {
            if (io_->error() || io_->eof())
                throw Error(kerFailedToReadImageData);
            throw Error(kerNotExpectedFormat);
        }
    }  // HeifImage::printStructure

    void HeifImage::writeMetadata()
    {
        if (io_->open() != 0)
        {
            throw Error(kerDataSourceOpenFailed, io_->path(), strError());
        }
        IoCloser closer(*io_);
        BasicIo::UniquePtr tempIo(new MemIo);
        assert (tempIo.get() != 0);

        doWriteMetadata(*tempIo); // may throw
        io_->close();
        io_->transfer(*tempIo); // may throw

    } // HeifImage::writeMetadata

    void HeifImage::doWriteMetadata(BasicIo& outIo)
    {
        if (!io_->isopen()) throw Error(kerInputDataReadFailed);
        if (!outIo.isopen()) throw Error(kerImageWriteFailed);

#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::HeifImage::doWriteMetadata: Writing HEIF file " << io_->path() << std::endl;
        std::cout << "Exiv2::HeifImage::doWriteMetadata: tmp file created " << outIo.path() << std::endl;
#endif

        // Ensure that this is the correct image type
        if (!isHeifType(*io_, true))
        {
            if (io_->error() || io_->eof()) throw Error(kerInputDataReadFailed);
            throw Error(kerNoImageInInputData);
        }

        // Write HEIF Signature.
        if (outIo.write(HeifSignature, sizeof(HeifSignature)) != sizeof(HeifSignature)) throw Error(kerImageWriteFailed);
#ifdef EXIV2_DEBUG_MESSAGES
        std::cout << "Exiv2::HeifImage::doWriteMetadata: EOF" << std::endl;
#endif

    } // HeifImage::doWriteMetadata

    Image::UniquePtr newHeifInstance(BasicIo::UniquePtr io, bool create)
    {
        Image::UniquePtr image(new HeifImage(std::move(io), create));
        if (!image->good())
        {
            image.reset();
        }
        return image;
    }

    bool isHeifType(BasicIo& iIo, bool advance)
    {
        const int32_t len = sizeof(HeifSignature);
        byte buf[len];
        iIo.read(buf, len);
        if (iIo.error() || iIo.eof())
        {
            return false;
        }
        bool matched = (memcmp(buf, HeifSignature, len) == 0);
        if (!advance || !matched)
        {
            iIo.seek(-len, BasicIo::cur);
        }
        return matched;
    }

} // namespace Exiv2

#endif // EXIV2_ENABLE_HEIF
