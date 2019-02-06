// ***************************************************************** -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
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
 */
// *****************************************************************************
// included header files
#include "error.hpp"
#include "i18n.h"                // NLS support.

// + standard includes
#include <array>
#include <cassert>
#include <iostream>
#include <string>

// *****************************************************************************
namespace {

    //! Complete list of Exiv2 exception error messages
    constexpr std::array<const char*, static_cast<size_t>(Exiv2::ErrorCode::kerErrorCount)> errList = {
        {// Exiv2::ErrorCode::kerSuccess
         N_("Success"),
         // Exiv2::ErrorCode::kerGeneralError
         N_("Error %0: arg2=%2, arg3=%3, arg1=%1."),
         // ErrorCode::kerErrorMessage
         // %1=error message
         "%1",
         // ErrorCode::kerCallFailed
         // %1=path, %2=strerror, %3=function that failed
         "%1: Call to `%3' failed: %2",
         // ErrorCode::kerNotAnImage
         // %1=Image type
         N_("This does not look like a %1 image"),
         // ErrorCode::kerInvalidDataset
         // %1=dataset name
         N_("Invalid dataset name `%1'"),
         // ErrorCode::kerInvalidRecord
         // %1=record name
         N_("Invalid record name `%1'"),
         // ErrorCode::kerInvalidKey
         // %1=key
         N_("Invalid key `%1'"),
         // ErrorCode::kerInvalidTag
         // %1=tag name, %2=ifdId
         N_("Invalid tag name or ifdId `%1', ifdId %2"),
         // ErrorCode::kerValueNotSet
         N_("Value not set"),
         // ErrorCode::kerDataSourceOpenFailed
         // %1=path, %2=strerror
         N_("%1: Failed to open the data source: %2"),
         // ErrorCode::kerFileOpenFailed
         // %1=path, %2=mode, %3=strerror
         N_("%1: Failed to open file (%2): %3"),
         // ErrorCode::kerFileContainsUnknownImageType
         // %1=path
         N_("%1: The file contains data of an unknown image type"),
         // ErrorCode::kerMemoryContainsUnknownImageType
         N_("The memory contains data of an unknown image type"),
         // ErrorCode::kerUnsupportedImageType
         // %1=image type
         N_("Image type %1 is not supported"),
         // ErrorCode::kerFailedToReadImageData
         N_("Failed to read image data"),
         // ErrorCode::kerNotAJpeg
         N_("This does not look like a JPEG image"),
         // ErrorCode::kerFailedToMapFileForReadWrite
         // %1=path, %2=strerror
         N_("%1: Failed to map file for reading and writing: %2"),
         // ErrorCode::kerFileRenameFailed
         // %1=old path, %2=new path, %3=strerror
         N_("%1: Failed to rename file to %2: %3"),
         // ErrorCode::kerTransferFailed
         // %1=path, %2=strerror
         N_("%1: Transfer failed: %2"),
         // ErrorCode::kerMemoryTransferFailed
         // %1=strerror
         N_("Memory transfer failed: %1"),
         // ErrorCode::kerInputDataReadFailed
         N_("Failed to read input data"),
         // ErrorCode::kerImageWriteFailed
         N_("Failed to write image"),
         // ErrorCode::kerNoImageInInputData
         N_("Input data does not contain a valid image"),
         // ErrorCode::kerInvalidIfdId
         // %1=ifdId
         N_("Invalid ifdId %1"),
         // ErrorCode::kerValueTooLarge
         // %1=tag, %2=dataSize, %3=required size
         N_("Entry::setValue: Value too large (tag=%1, size=%2, requested=%3)"),
         // ErrorCode::kerDataAreaValueTooLarge
         // %1=tag, %2=dataAreaSize, %3=required size
         N_("Entry::setDataArea: Value too large (tag=%1, size=%2, requested=%3)"),
         // ErrorCode::kerOffsetOutOfRange
         N_("Offset out of range"),
         // ErrorCode::kerUnsupportedDataAreaOffsetType
         N_("Unsupported data area offset type"),
         // ErrorCode::kerInvalidCharset
         // %1=charset name
         N_("Invalid charset: `%1'"),
         // ErrorCode::kerUnsupportedDateFormat
         N_("Unsupported date format"),
         // ErrorCode::kerUnsupportedTimeFormat
         N_("Unsupported time format"),
         // ErrorCode::kerWritingImageFormatUnsupported
         // %1=image format
         N_("Writing to %1 images is not supported"),
         // ErrorCode::kerInvalidSettingForImage
         // %1=metadata type, %2=image format
         N_("Setting %1 in %2 images is not supported"),
         // ErrorCode::kerNotACrwImage
         N_("This does not look like a CRW image"),
         // ErrorCode::kerFunctionNotSupported
         // %1=function
         N_("%1: Not supported"),
         // ErrorCode::kerNoNamespaceInfoForXmpPrefix
         // %1=prefix
         N_("No namespace info available for XMP prefix `%1'"),
         // ErrorCode::kerNoPrefixForNamespace
         // %1=namespace
         N_("No prefix registered for namespace `%2', needed for property path `%1'"),
         // ErrorCode::kerTooLargeJpegSegment
         // %1=type of metadata (Exif, IPTC, JPEG comment)
         N_("Size of %1 JPEG segment is larger than 65535 bytes"),
         // ErrorCode::kerUnhandledXmpdatum
         // %1=key, %2=value type
         N_("Unhandled Xmpdatum %1 of type %2"),
         // ErrorCode::kerUnhandledXmpNode
         // %1=key, %2=XMP Toolkit option flags
         N_("Unhandled XMP node %1 with opt=%2"),
         // ErrorCode::kerXMPToolkitError
         // %1=XMP_Error::GetID(), %2=XMP_Error::GetErrMsg()
         N_("XMP Toolkit error %1: %2"),
         // ErrorCode::kerDecodeLangAltPropertyFailed
         // %1=property path, %3=XMP Toolkit option flags
         N_("Failed to decode Lang Alt property %1 with opt=%2"),
         // ErrorCode::kerDecodeLangAltQualifierFailed
         // %1=qualifier path, %3=XMP Toolkit option flags
         N_("Failed to decode Lang Alt qualifier %1 with opt=%2"),
         // ErrorCode::kerEncodeLangAltPropertyFailed
         // %1=key
         N_("Failed to encode Lang Alt property %1"),
         // ErrorCode::kerPropertyNameIdentificationFailed
         // %1=property path, %2=namespace
         N_("Failed to determine property name from path %1, namespace %2"),
         // ErrorCode::kerSchemaNamespaceNotRegistered
         // %1=namespace
         N_("Schema namespace %1 is not registered with the XMP Toolkit"),
         // ErrorCode::kerNoNamespaceForPrefix
         // %1=prefix
         N_("No namespace registered for prefix `%1'"),
         // ErrorCode::kerAliasesNotSupported
         // %1=namespace, %2=property path, %3=value
         N_("Aliases are not supported. Please post this XMP to packet to https://github.com/Exiv2/exiv2/issues `%1', "
            "`%2', `%3'"),
         // ErrorCode::kerInvalidXmpText
         // %1=type
         N_("Invalid XmpText type `%1'"),
         // ErrorCode::kerTooManyTiffDirectoryEntries
         // %1=TIFF directory name
         N_("TIFF directory %1 has too many entries"),
         // ErrorCode::kerMultipleTiffArrayElementTagsInDirectory
         // %1=tag number
         N_("Multiple TIFF array element tags %1 in one directory"),
         // ErrorCode::kerWrongTiffArrayElementTagType
         // %1=tag number
         N_("TIFF array element tag %1 has wrong type"),
         // ErrorCode::kerInvalidKeyXmpValue
         // %1=key, %2=value type
         N_("%1 has invalid XMP value type `%2'"),
         // ErrorCode::kerInvalidIccProfile
         N_("Not a valid ICC Profile"),
         // ErrorCode::kerInvalidXMP
         N_("Not valid XMP"),
         // ErrorCode::kerTiffDirectoryTooLarge
         N_("tiff directory length is too large"),
         // ErrorCode::kerInvalidTypeValue
         N_("invalid type value detected in Image::printIFDStructure"),
         // ErrorCode::kerInvalidMalloc
         N_("invalid memory allocation request"),
         // ErrorCode::kerCorruptedMetadata
         N_("corrupted image metadata"),
         // ErrorCode::kerArithmeticOverflow
         N_("Arithmetic operation overflow"),
         // ErrorCode::kerMallocFailed
         N_("Memory allocation failed")}};
}

// *****************************************************************************
// class member definitions
namespace Exiv2 {

    LogMsg::Level LogMsg::level_ = LogMsg::warn; // Default output level
    LogMsg::Handler LogMsg::handler_ = LogMsg::defaultHandler;

    LogMsg::LogMsg(LogMsg::Level msgType) : msgType_(msgType)
    {}

    LogMsg::~LogMsg()
    {
        if (msgType_ >= level_ && handler_)
            handler_(msgType_, os_.str().c_str());
    }

    std::ostringstream &LogMsg::os() { return os_; }

    void LogMsg::setLevel(LogMsg::Level level) { level_ = level; }

    void LogMsg::setHandler(LogMsg::Handler handler) { handler_ = handler; }

    LogMsg::Level LogMsg::level() { return level_; }

    LogMsg::Handler LogMsg::handler() { return handler_; }

    void LogMsg::defaultHandler(int level, const char* s)
    {
        switch (static_cast<LogMsg::Level>(level)) {
        case LogMsg::debug: std::cerr << "Debug: "; break;
        case LogMsg::info:  std::cerr << "Info: "; break;
        case LogMsg::warn:  std::cerr << "Warning: "; break;
        case LogMsg::error: std::cerr << "Error: "; break;
        case LogMsg::mute:  assert(false);
        }
        std::cerr << s;
    }

    Error::Error(ErrorCode code)
        : code_(code)
    {
        setMsg(0);
    }

    Error::~Error() noexcept
    {
    }

    ErrorCode Error::code() const noexcept
    {
        return code_;
    }

    const char* Error::what() const noexcept
    {
        return msg_.c_str();
    }

#ifdef EXV_UNICODE_PATH
    const wchar_t* Error::wwhat() const noexcept
    {
        return wmsg_.c_str();
    }
#endif

    void Error::setMsg(int count)
    {
        std::string msg = _(errMsg(code_));
        std::string::size_type pos;
        pos = msg.find("%0");
        if (pos != std::string::npos) {
            msg.replace(pos, 2, toString(static_cast<int>(code_)));
        }
        if (count > 0) {
            pos = msg.find("%1");
            if (pos != std::string::npos) {
                msg.replace(pos, 2, arg1_);
            }
        }
        if (count > 1) {
            pos = msg.find("%2");
            if (pos != std::string::npos) {
                msg.replace(pos, 2, arg2_);
            }
        }
        if (count > 2) {
            pos = msg.find("%3");
            if (pos != std::string::npos) {
                msg.replace(pos, 2, arg3_);
            }
        }
        msg_ = msg;
#ifdef EXV_UNICODE_PATH
        wmsg_ = s2ws(msg);
#endif
    }

    const char* errMsg(Exiv2::ErrorCode code)
    {
        return errList.at(static_cast<size_t>(code));
    }

} // namespace Exiv2
