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
/*!
  @file    preview.hpp
  @brief   Classes to access preview images embedded in an image.
  @version $Rev$
  @author  Vladimir Nadvornik (vn)
           <a href="mailto:nadvornik@suse.cz">nadvornik@suse.cz</a>
  @date    18-Sep-08, vn: created
 */
#ifndef PREVIEW_HPP_
#define PREVIEW_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"
#include "image.hpp"
#include "basicio.hpp"

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class definitions

    //! Type of preview image.
    typedef int PreviewId;

    /*!
      @brief Preview image properties.
     */
    struct EXIV2API PreviewProperties {
        //! Preview image mime type.
        const char* mimeType_;

        //! Preview image extension.
        const char* extension_;

        //! Preview image size in bytes.
        uint32_t size_;

        //! Preview image width in pixels or 0 for unknown width.
        uint32_t width_;
        
        //! Preview image height in pixels or 0 for unknown height.
        uint32_t height_;
        
        //! Preview image compression - true means uncompressed image.
        bool uncompressed_;

        //! Identifies type of preview image.
        PreviewId id_;
    };

    //! Container type to hold all preview images metadata.
    typedef std::vector<PreviewProperties> PreviewPropertiesList;

    /*!
      @brief Class that holds preview image properties and data buffer.
     */
    class EXIV2API PreviewImage {
    public:
        //@{
        //! Constructor.
        PreviewImage(const PreviewProperties& properties, DataBuf& data);

        //! Copy constructor
        PreviewImage(const PreviewImage& src);
        //@}

        //! @name Accessors
        //@{

        /*!
          @brief Return a reference to image data.
         */
        DataBuf& data();

        /*!
          @brief Write the thumbnail image to a file.

          A filename extension is appended to \em path according to the image
          type of the thumbnail, so \em path should not include an extension.
          The function will overwrite an existing file of the same name.

          @param path File name of the thumbnail without extension.
          @return The number of bytes written.
        */
        long writeFile(const std::string& path) const;
        /*!
          @brief Return the MIME type of the preview image, usually either
                 \c "image/tiff" or \c "image/jpeg".
         */
        const char* mimeType() const;
        /*!
          @brief Return the file extension for the format of the preview image
                 (".tif" or ".jpg").
         */
        const char* extension() const;

        /*!
          @brief Return the size of the preview image in bytes.
         */
        uint32_t size() const;
        //@}

    private:
        PreviewProperties properties_;
        DataBuf data_;
    }; // class PreviewImage

    /*!
      @brief Class for extracting preview images from image metadata.
     */
    class EXIV2API PreviewImageLoader {
    public:
        //@{
        //! Constructor.
        PreviewImageLoader(const Image& image);

        /*!
          @brief Return list of preview properties.
         */
        PreviewPropertiesList getPreviewPropertiesList() const;

        /*!
          @brief Return image data for given properties.
         */
        PreviewImage getPreviewImage(const PreviewProperties& properties) const;

        /*!
          @brief Read image dimensions if they are not available directly.
         */
        bool readDimensions(PreviewProperties& properties) const;
        
        //@}

    private:
	const Image& image_;

    }; // class PreviewImageLoader
}                                       // namespace Exiv2

#endif                                  // #ifndef PREVIEW_HPP_
