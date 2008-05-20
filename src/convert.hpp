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
  @file    convert.hpp
  @brief   Exif and IPTC conversions to and from XMP
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a><BR>
           Vladimir Nadvornik (vn)
           <a href="mailto:nadvornik@suse.cz">nadvornik@suse.cz</a>
  @date    17-Mar-08, ahu: created basic converter framework<BR>
           20-May-08, vn:  added actual conversion logic
 */
#ifndef CONVERT_HPP_
#define CONVERT_HPP_

// *****************************************************************************
// included header files

// + standard includes

// *****************************************************************************
// namespace extensions
namespace Exiv2 {

// *****************************************************************************
// class declarations
    class ExifData;
    class IptcData;
    class XmpData;

// *****************************************************************************
// free functions, template and inline definitions

    //! Convert (copy) Exif tags to XMP properties.
    void copyExifToXmp(const ExifData& exifData, XmpData& xmpData);
    //! Convert (move) Exif tags to XMP properties, remove converted properties.
    void moveExifToXmp(ExifData& exifData, XmpData& xmpData);

    //! Convert (copy) XMP properties to Exif tags.
    void copyXmpToExif(const XmpData& xmpData, ExifData& exifData);
    //! Convert (move) XMP properties to Exif tags, remove converted properties.
    void moveXmpToExif(XmpData& xmpData, ExifData& exifData);

    //! Detect which metadata are newer and performs a copy in apropriate direction.
    void syncExifWithXmp(ExifData& exifData, XmpData& xmpData);

    //! Convert (copy) IPTC datasets to XMP properties.
    void copyIptcToXmp(const IptcData& iptcData, XmpData& xmpData);
    //! Convert (move) IPTC datasets to XMP properties, remove converted properties.
    void moveIptcToXmp(IptcData& iptcData, XmpData& xmpData);

    //! Convert (copy) XMP properties to IPTC datasets.
    void copyXmpToIptc(const XmpData& xmpData, IptcData& iptcData);
    //! Convert (move) XMP properties to IPTC tags, remove converted properties.
    void moveXmpToIptc(XmpData& xmpData, IptcData& iptcData);

}                                       // namespace Exiv2

#endif                                  // #ifndef CONVERT_HPP_
