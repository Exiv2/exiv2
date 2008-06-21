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
  @file    tifffwd_int.hpp
  @brief   Internal TIFF parser related typedefs and forward definitions.
  @version $Rev$
  @author  Andreas Huggel (ahu)
           <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  @date    24-Jun-06, ahu: created
 */
#ifndef TIFFFWD_INT_HPP_
#define TIFFFWD_INT_HPP_

// *****************************************************************************
// included header files
#include "types.hpp"

// + standard includes
#include <memory>
#include <stack>

// *****************************************************************************
// Exiv2 namespace extensions
namespace Exiv2 {

    class Exifdatum;

    namespace Internal {

    class TiffHeaderBase;
    class TiffComponent;
    struct TiffStructure;
    class TiffEntryBase;
    class TiffEntry;
    class TiffDataEntry;
    class TiffSizeEntry;
    class TiffDirectory;
    class TiffSubIfd;
    class TiffMnEntry;
    class TiffArrayEntry;
    class TiffArrayElement;

    class TiffIfdMakernote;

    class TiffVisitor;
    class TiffFinder;
    class TiffDecoder;
    class TiffEncoder;
    class TiffReader;
    class TiffPrinter;

    class TiffRwState;
    struct TiffMappingInfo;

// *****************************************************************************
// type definitions

    /*!
      @brief Function pointer type for a TiffDecoder member function
             to decode a TIFF component.
     */
    typedef void (TiffDecoder::*DecoderFct)(const TiffEntryBase*);
    /*!
      @brief Function pointer type for a TiffDecoder member function
             to decode a TIFF component.
     */
    typedef void (TiffEncoder::*EncoderFct)(TiffEntryBase*, const Exifdatum*);
    /*!
      @brief Type for a function pointer for a function to decode a TIFF component.
     */
    typedef DecoderFct (*FindDecoderFct)(const std::string& make,
                                               uint32_t     extendedTag,
                                               uint16_t     group);
    /*!
      @brief Type for a function pointer for a function to encode a TIFF component.
     */
    typedef EncoderFct (*FindEncoderFct)(
        const std::string& make,
              uint32_t     extendedTag,
              uint16_t     group
    );
    /*!
      @brief Type for a function pointer for a function to create a TIFF component.
             Use TiffComponent::AutoPtr, it is not used in this declaration only
             to reduce dependencies.
     */
    typedef std::auto_ptr<TiffComponent> (*NewTiffCompFct)(      uint16_t       tag,
                                                           const TiffStructure* ts);
    //! Stack to hold a path from the TIFF root element to a TIFF entry
    typedef std::stack<const TiffStructure*> TiffPath;

    /*!
      @brief Type for a factory function to create new TIFF components.
             Use TiffComponent::AutoPtr, it is not used in this declaration only
             to reduce dependencies.
     */
    typedef std::auto_ptr<Internal::TiffComponent> (*TiffCompFactoryFct)(uint32_t extendedTag,
                                                                         uint16_t group);

}}                                      // namespace Internal, Exiv2

#endif                                  // #ifndef TIFFFWD_INT_HPP_
