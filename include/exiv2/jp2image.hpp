// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef JP2IMAGE_HPP_
#define JP2IMAGE_HPP_

#include "exiv2lib_export.h"

#include "image.hpp"

namespace Exiv2
{
    /// @brief Class to access JPEG-2000 images.
    class EXIV2API Jp2Image : public Image {
    public:
        //! @name Creators
        //@{
        /// @brief Constructor to open a JPEG-2000 image. Since the constructor can not return a result, callers should
        /// check the good() method after object construction to determine success or failure.
        /// @param io An auto-pointer that owns a BasicIo instance used for reading and writing image metadata.
        ///  \b Important: The constructor takes ownership of the passed in BasicIo instance through the auto-pointer.
        ///  Callers should not continue to use the BasicIo instance after it is passed to this method. Use the Image::io()
        ///  method to get a temporary reference.
        /// @param create Specifies if an existing image should be read (false) or if a new file should be created (true).
        Jp2Image(BasicIo::UniquePtr io, bool create);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata() override;
        void writeMetadata() override;

        /// @brief Print out the structure of image file.
        /// @throw Error if reading of the file fails or the image data is not valid
        /// @warning This function is not thread safe and intended for exiv2 -pS for debugging.
        void printStructure(std::ostream& out, PrintStructureOption option, int depth) override;

        void setComment(std::string_view comment) override;
        //@}

        //! @name Accessors
        //@{
        std::string mimeType() const override;
        //@}

        Jp2Image(const Jp2Image& rhs) = delete;
        Jp2Image& operator=(const Jp2Image& rhs) = delete;
    private:
        /// @brief Main implementation of writeMetadata() by writing all buffered metadata to the provided BasicIo.
        /// @param oIo BasicIo instance to write to (a temporary location).
        void doWriteMetadata(BasicIo& outIo);

        /// @brief reformats the Jp2Header to store iccProfile
        /// @param oldData DataBufRef to data in the file.
        /// @param newData DataBufRef with updated data
        void encodeJp2Header(const DataBuf& boxBuf, DataBuf& outBuf);
    }; // class Jp2Image

// *****************************************************************************
// template, inline and free functions

    /// @brief Create a new Jp2Image instance and return an auto-pointer to it.
    /// Caller owns the returned object and the auto-pointer ensures that it will be deleted.
    EXIV2API Image::UniquePtr newJp2Instance(BasicIo::UniquePtr io, bool create);

    //! Check if the file iIo is a JPEG-2000 image.
    EXIV2API bool isJp2Type(BasicIo& iIo, bool advance);
} // namespace Exiv2

#endif // #ifndef JP2IMAGE_HPP_
