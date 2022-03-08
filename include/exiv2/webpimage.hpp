// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef WEBPIMAGE_HPP
#define WEBPIMAGE_HPP

// *****************************************************************************
#include "exiv2lib_export.h"

// included header files
#include "image.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    /*!
      @brief Class to access WEBP video files.
     */
    class EXIV2API WebPImage:public Image
    {
    public:
        //! @name Creators
        //@{
        /*!
          @brief Constructor for a WebP video. Since the constructor
              can not return a result, callers should check the good() method
              after object construction to determine success or failure.
          @param io An auto-pointer that owns a BasicIo instance used for
              reading and writing image metadata. \b Important: The constructor
              takes ownership of the passed in BasicIo instance through the
              auto-pointer. Callers should not continue to use the BasicIo
              instance after it is passed to this method. Use the Image::io()
              method to get a temporary reference.
         */
        explicit WebPImage(BasicIo::UniquePtr io);
        //@}

        //! @name Manipulators
        //@{
        void readMetadata() override;
        void writeMetadata() override;
        void printStructure(std::ostream& out, PrintStructureOption option, int depth) override;
        //@}

        /*!
          @brief Not supported. Calling this function will throw an Error(ErrorCode::kerInvalidSettingForImage).
         */
        void setComment(std::string_view comment) override;
        void setIptcData(const IptcData& /*iptcData*/) override;

        //! @name Accessors
        //@{
        std::string mimeType() const override;
        //@}

        //! Copy constructor
        WebPImage(const WebPImage& rhs) = delete;
        //! Assignment operator
        WebPImage& operator=(const WebPImage& rhs) = delete;
        //@}

    private:
        void doWriteMetadata(BasicIo& outIo);
        //! @name NOT Implemented
        //@{
        static long getHeaderOffset(const byte* data, long data_size, const byte* header, long header_size);
        static bool equalsWebPTag(Exiv2::DataBuf& buf, const char* str);
        void debugPrintHex(byte *data, long size);
        void decodeChunks(long filesize);
        void inject_VP8X(BasicIo& iIo, bool has_xmp, bool has_exif,
                         bool has_alpha, bool has_icc, int width,
                         int height);

        static const byte WEBP_PAD_ODD;
        static const int WEBP_TAG_SIZE;
        static const int WEBP_VP8X_ICC_BIT;
        static const int WEBP_VP8X_ALPHA_BIT;
        static const int WEBP_VP8X_EXIF_BIT;
        static const int WEBP_VP8X_XMP_BIT;
        static const char* const WEBP_CHUNK_HEADER_VP8X;
        static const char* const WEBP_CHUNK_HEADER_VP8L;
        static const char* const WEBP_CHUNK_HEADER_VP8;
        static const char* const WEBP_CHUNK_HEADER_ANMF;
        static const char* const WEBP_CHUNK_HEADER_ANIM;
        static const char* const WEBP_CHUNK_HEADER_ICCP;
        static const char* const WEBP_CHUNK_HEADER_EXIF;
        static const char* const WEBP_CHUNK_HEADER_XMP;

    }; //Class WebPImage

// *****************************************************************************
// template, inline and free functions

    // These could be static private functions on Image subclasses but then
    // ImageFactory needs to be made a friend.
    /*!
      @brief Create a new WebPImage instance and return an auto-pointer to it.
          Caller owns the returned object and the auto-pointer ensures that
          it will be deleted.
     */
    EXIV2API Image::UniquePtr newWebPInstance(BasicIo::UniquePtr io, bool create);

    //! Check if the file iIo is a WebP Video.
    EXIV2API bool isWebPType(BasicIo& iIo, bool advance);

}                                       // namespace Exiv2

#endif                                  // WEBPIMAGE_HPP
