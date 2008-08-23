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
  File:      error.cpp
  Version:   $Rev$
  Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
  History:   02-Apr-05, ahu: created
 */
// *****************************************************************************
#include "rcsid.hpp"
EXIV2_RCSID("@(#) $Id$")

// *****************************************************************************
// included header files
#include "error.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <string>

// *****************************************************************************
namespace {

    //! Helper structure defining an error message.
    struct ErrMsg {
        int code_;                              //!< Error code
        const char* message_;                   //!< Error message
    };

    const ErrMsg errMsg[] = {
        { -1, N_("Error %0: arg1=%1, arg2=%2, arg3=%3.") },
        {  0, N_("Success") },
        {  1, "%1" }, // %1=error message
        {  2, "%1: %2 (%3)" }, // %1=path, %2=strerror, %3=function that failed
        {  3, N_("This does not look like a %1 image") }, // %1=Image type
        {  4, N_("Invalid dataset name `%1'") }, // %1=dataset name
        {  5, N_("Invalid record name `%1'") }, // %1=record name
        {  6, N_("Invalid key `%1'") }, // %1=key
        {  7, N_("Invalid tag name or ifdId `%1', ifdId %2") }, // %1=tag name, %2=ifdId
        {  8, N_("Value not set") },
        {  9, N_("%1: Failed to open the data source: %2") }, // %1=path, %2=strerror
        { 10, N_("%1: Failed to open file (%2): %3") }, // %1=path, %2=mode, %3=strerror
        { 11, N_("%1: The file contains data of an unknown image type") }, // %1=path
        { 12, N_("The memory contains data of an unknown image type") },
        { 13, N_("Image type %1 is not supported") }, // %1=image type
        { 14, N_("Failed to read image data") },
        { 15, N_("This does not look like a JPEG image") },
//      { 16, N_("") }, -- currently not used
        { 17, N_("%1: Failed to rename file to %2: %3") }, // %1=old path, %2=new path, %3=strerror
        { 18, N_("%1: Transfer failed: %2") }, // %1=path, %2=strerror
        { 19, N_("Memory transfer failed: %1") }, // %1=strerror
        { 20, N_("Failed to read input data") },
        { 21, N_("Failed to write image") },
        { 22, N_("Input data does not contain a valid image") },
        { 23, N_("Invalid ifdId %1") }, // %1=ifdId
        { 24, N_("Entry::setValue: Value too large (tag=%1, size=%2, requested=%3)") }, // %1=tag, %2=dataSize, %3=required size
        { 25, N_("Entry::setDataArea: Value too large (tag=%1, size=%2, requested=%3)") }, // %1=tag, %2=dataAreaSize, %3=required size
        { 26, N_("Offset out of range") },
        { 27, N_("Unsupported data area offset type") },
        { 28, N_("Invalid charset: `%1'") }, // %1=charset name
        { 29, N_("Unsupported date format") },
        { 30, N_("Unsupported time format") },
        { 31, N_("Writing to %2 images is not supported") }, // %2=image format
        { 32, N_("Setting %1 in %2 images is not supported") }, // %1=metadata type, %2=image format
        { 33, N_("This does not look like a CRW image") },
        { 34, N_("%1: Not supported") }, // %1=function
        { 35, N_("No namespace info available for XMP prefix `%1'") }, // %1=prefix
        { 36, N_("No prefix registered for namespace `%1'") }, // %1=namespace
        { 37, N_("Size of %1 JPEG segment is larger than 65535 bytes") }, // %1=type of metadata (Exif, IPTC, JPEG comment)
        { 38, N_("Unhandled Xmpdatum %1 of type %2") }, // %1=key, %2=value type
        { 39, N_("Unhandled XMP node %1 with opt=%2") }, // %1=key, %2=XMP Toolkit option flags
        { 40, N_("XMP Toolkit error %1: %2") }, // %1=XMP_Error::GetID(), %2=XMP_Error::GetErrMsg()
        { 41, N_("Failed to decode Lang Alt property %1 with opt=%2") }, // %1=property path, %3=XMP Toolkit option flags
        { 42, N_("Failed to decode Lang Alt qualifier %1 with opt=%2") }, // %1=qualifier path, %3=XMP Toolkit option flags
        { 43, N_("Failed to encode Lang Alt property %1") }, // %1=key
        { 44, N_("Failed to determine property name from path %1, namespace %2") }, // %1=property path, %2=namespace
        { 45, N_("Schema namespace %1 is not registered with the XMP Toolkit") }, // %1=namespace
        { 46, N_("No namespace registered for prefix `%1'") }, // %1=prefix
        { 47, N_("Aliases are not supported. Please send this XMP packet to ahuggel@gmx.net `%1', `%2', `%3'") }, // %1=namespace, %2=property path, %3=value
        { 48, N_("Invalid XmpText type `%1'") }, // %1=type
        { 49, N_("TIFF directory %1 has too many entries") }, // %1=TIFF directory name
        { 50, N_("Multiple TIFF array element tags %1 in one directory") }, // %1=tag number
        { 51, N_("TIFF array element tag %1 has wrong type") }, // %1=tag number
        // Last error message (message is not used)
        { -2, N_("(Unknown Error)") }
    };

}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    int Error::errorIdx(int code)
    {
        int idx;
        for (idx = 0; errMsg[idx].code_ != code; ++idx) {
            if (errMsg[idx].code_ == -2) return 0;
        }
        return idx;
    }

    void Error::setMsg()
    {
        int idx = errorIdx(code_);
        msg_ = std::string(_(errMsg[idx].message_));
        std::string::size_type pos;
        pos = msg_.find("%0");
        if (pos != std::string::npos) {
            msg_.replace(pos, 2, toString(code_));
        }
        if (count_ > 0) {
            pos = msg_.find("%1");
            if (pos != std::string::npos) {
                msg_.replace(pos, 2, arg1_);
            }
        }
        if (count_ > 1) {
            pos = msg_.find("%2");
            if (pos != std::string::npos) {
                msg_.replace(pos, 2, arg2_);
            }
        }
        if (count_ > 2) {
            pos = msg_.find("%3");
            if (pos != std::string::npos) {
                msg_.replace(pos, 2, arg3_);
            }
        }
    }

}                                       // namespace Exiv2
