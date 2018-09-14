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
#include <iostream>
#include <string>
#include <cassert>

// *****************************************************************************
namespace {

    //! Helper structure defining an error message.
    struct ErrMsg {
        //! Comparison operator
        bool operator==(int code) const { return code_ == code; }

        int code_;                              //!< Error code
        const char* message_;                   //!< Error message
    };

    //! Complete list of Exiv2 exception error messages
    const ErrMsg errList[] = {
        { Exiv2::kerGeneralError,
          N_("Error %0: arg2=%2, arg3=%3, arg1=%1.") },
        { Exiv2::kerSuccess,
          N_("Success") },
        { Exiv2::kerErrorMessage,
          "%1" }, // %1=error message
        { Exiv2::kerCallFailed,
          "%1: Call to `%3' failed: %2" }, // %1=path, %2=strerror, %3=function that failed
        { Exiv2::kerNotAnImage,
          N_("This does not look like a %1 image") }, // %1=Image type
        { Exiv2::kerInvalidDataset,
          N_("Invalid dataset name `%1'") }, // %1=dataset name
        { Exiv2::kerInvalidRecord,
          N_("Invalid record name `%1'") }, // %1=record name
        { Exiv2::kerInvalidKey,
          N_("Invalid key `%1'") }, // %1=key
        { Exiv2::kerInvalidTag,
          N_("Invalid tag name or ifdId `%1', ifdId %2") }, // %1=tag name, %2=ifdId
        { Exiv2::kerValueNotSet,
          N_("Value not set") },
        { Exiv2::kerDataSourceOpenFailed,
          N_("%1: Failed to open the data source: %2") }, // %1=path, %2=strerror
        { Exiv2::kerFileOpenFailed,
          N_("%1: Failed to open file (%2): %3") }, // %1=path, %2=mode, %3=strerror
        { Exiv2::kerFileContainsUnknownImageType,
          N_("%1: The file contains data of an unknown image type") }, // %1=path
        { Exiv2::kerMemoryContainsUnknownImageType,
          N_("The memory contains data of an unknown image type") },
        { Exiv2::kerUnsupportedImageType,
          N_("Image type %1 is not supported") }, // %1=image type
        { Exiv2::kerFailedToReadImageData,
          N_("Failed to read image data") },
        { Exiv2::kerNotAJpeg,
          N_("This does not look like a JPEG image") },
        { Exiv2::kerFailedToMapFileForReadWrite,
          N_("%1: Failed to map file for reading and writing: %2") }, // %1=path, %2=strerror
        { Exiv2::kerFileRenameFailed,
          N_("%1: Failed to rename file to %2: %3") }, // %1=old path, %2=new path, %3=strerror
        { Exiv2::kerTransferFailed,
          N_("%1: Transfer failed: %2") }, // %1=path, %2=strerror
        { Exiv2::kerMemoryTransferFailed,
          N_("Memory transfer failed: %1") }, // %1=strerror
        { Exiv2::kerInputDataReadFailed,
          N_("Failed to read input data") },
        { Exiv2::kerImageWriteFailed,
          N_("Failed to write image") },
        { Exiv2::kerNoImageInInputData,
          N_("Input data does not contain a valid image") },
        { Exiv2::kerInvalidIfdId,
          N_("Invalid ifdId %1") }, // %1=ifdId
        { Exiv2::kerValueTooLarge,
          N_("Entry::setValue: Value too large (tag=%1, size=%2, requested=%3)") }, // %1=tag, %2=dataSize, %3=required size
        { Exiv2::kerDataAreaValueTooLarge,
          N_("Entry::setDataArea: Value too large (tag=%1, size=%2, requested=%3)") }, // %1=tag, %2=dataAreaSize, %3=required size
        { Exiv2::kerOffsetOutOfRange,
          N_("Offset out of range") },
        { Exiv2::kerUnsupportedDataAreaOffsetType,
          N_("Unsupported data area offset type") },
        { Exiv2::kerInvalidCharset,
          N_("Invalid charset: `%1'") }, // %1=charset name
        { Exiv2::kerUnsupportedDateFormat,
          N_("Unsupported date format") },
        { Exiv2::kerUnsupportedTimeFormat,
          N_("Unsupported time format") },
        { Exiv2::kerWritingImageFormatUnsupported,
          N_("Writing to %1 images is not supported") }, // %1=image format
        { Exiv2::kerInvalidSettingForImage,
          N_("Setting %1 in %2 images is not supported") }, // %1=metadata type, %2=image format
        { Exiv2::kerNotACrwImage,
          N_("This does not look like a CRW image") },
        { Exiv2::kerFunctionNotSupported,
          N_("%1: Not supported") }, // %1=function
        { Exiv2::kerNoNamespaceInfoForXmpPrefix,
          N_("No namespace info available for XMP prefix `%1'") }, // %1=prefix
        { Exiv2::kerNoPrefixForNamespace,
          N_("No prefix registered for namespace `%2', needed for property path `%1'") }, // %1=namespace
        { Exiv2::kerTooLargeJpegSegment,
          N_("Size of %1 JPEG segment is larger than 65535 bytes") }, // %1=type of metadata (Exif, IPTC, JPEG comment)
        { Exiv2::kerUnhandledXmpdatum,
          N_("Unhandled Xmpdatum %1 of type %2") }, // %1=key, %2=value type
        { Exiv2::kerUnhandledXmpNode,
          N_("Unhandled XMP node %1 with opt=%2") }, // %1=key, %2=XMP Toolkit option flags
        { Exiv2::kerXMPToolkitError,
          N_("XMP Toolkit error %1: %2") }, // %1=XMP_Error::GetID(), %2=XMP_Error::GetErrMsg()
        { Exiv2::kerDecodeLangAltPropertyFailed,
          N_("Failed to decode Lang Alt property %1 with opt=%2") }, // %1=property path, %3=XMP Toolkit option flags
        { Exiv2::kerDecodeLangAltQualifierFailed,
          N_("Failed to decode Lang Alt qualifier %1 with opt=%2") }, // %1=qualifier path, %3=XMP Toolkit option flags
        { Exiv2::kerEncodeLangAltPropertyFailed,
          N_("Failed to encode Lang Alt property %1") }, // %1=key
        { Exiv2::kerPropertyNameIdentificationFailed,
          N_("Failed to determine property name from path %1, namespace %2") }, // %1=property path, %2=namespace
        { Exiv2::kerSchemaNamespaceNotRegistered,
          N_("Schema namespace %1 is not registered with the XMP Toolkit") }, // %1=namespace
        { Exiv2::kerNoNamespaceForPrefix,
          N_("No namespace registered for prefix `%1'") }, // %1=prefix
        { Exiv2::kerAliasesNotSupported,
          N_("Aliases are not supported. Please send this XMP packet to ahuggel@gmx.net `%1', `%2', `%3'") }, // %1=namespace, %2=property path, %3=value
        { Exiv2::kerInvalidXmpText,
          N_("Invalid XmpText type `%1'") }, // %1=type
        { Exiv2::kerTooManyTiffDirectoryEntries,
          N_("TIFF directory %1 has too many entries") }, // %1=TIFF directory name
        { Exiv2::kerMultipleTiffArrayElementTagsInDirectory,
          N_("Multiple TIFF array element tags %1 in one directory") }, // %1=tag number
        { Exiv2::kerWrongTiffArrayElementTagType,
          N_("TIFF array element tag %1 has wrong type") }, // %1=tag number
        { Exiv2::kerInvalidKeyXmpValue,
          N_("%1 has invalid XMP value type `%2'") }, // %1=key, %2=value type
        { Exiv2::kerInvalidIccProfile,
          N_("Not a valid ICC Profile") },
        { Exiv2::kerInvalidXMP,
          N_("Not valid XMP") },
        { Exiv2::kerTiffDirectoryTooLarge,
          N_("tiff directory length is too large") },
        { Exiv2::kerInvalidTypeValue,
          N_("invalid type value detected in Image::printIFDStructure") },
        { Exiv2::kerInvalidMalloc,
          N_("invalid memory allocation request") },
        { Exiv2::kerCorruptedMetadata,
          N_("corrupted image metadata") },
        { Exiv2::kerArithmeticOverflow,
          N_("Arithmetic operation overflow") },
        { Exiv2::kerMallocFailed,
          N_("Memory allocation failed")}
    };

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

    AnyError::AnyError(): std::exception()
    {

    }

    AnyError::AnyError(const AnyError &o): std::exception(o)
    {

    }

    AnyError::~AnyError() throw()
    {
    }

    //! @cond IGNORE
    template<>
    void BasicError<char>::setMsg()
    {
        std::string msg = _(errMsg(code_));
        std::string::size_type pos;
        pos = msg.find("%0");
        if (pos != std::string::npos) {
            msg.replace(pos, 2, toString(code_));
        }
        if (count_ > 0) {
            pos = msg.find("%1");
            if (pos != std::string::npos) {
                msg.replace(pos, 2, arg1_);
            }
        }
        if (count_ > 1) {
            pos = msg.find("%2");
            if (pos != std::string::npos) {
                msg.replace(pos, 2, arg2_);
            }
        }
        if (count_ > 2) {
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
    //! @endcond

#ifdef EXV_UNICODE_PATH
    template<>
    void BasicError<wchar_t>::setMsg()
    {
        std::string s = _(errMsg(code_));
        std::wstring wmsg(s.begin(), s.end());
        std::wstring::size_type pos;
        pos = wmsg.find(L"%0");
        if (pos != std::wstring::npos) {
            wmsg.replace(pos, 2, toBasicString<wchar_t>(code_));
        }
        if (count_ > 0) {
            pos = wmsg.find(L"%1");
            if (pos != std::wstring::npos) {
                wmsg.replace(pos, 2, arg1_);
            }
        }
        if (count_ > 1) {
            pos = wmsg.find(L"%2");
            if (pos != std::wstring::npos) {
                wmsg.replace(pos, 2, arg2_);
            }
        }
        if (count_ > 2) {
            pos = wmsg.find(L"%3");
            if (pos != std::wstring::npos) {
                wmsg.replace(pos, 2, arg3_);
            }
        }
        wmsg_ = wmsg;
        msg_ = ws2s(wmsg);
    }
#endif

    const char* errMsg(int code)
    {
        const ErrMsg* em = find(errList, code);
        return em ? em->message_ : "";
    }

}                                       // namespace Exiv2
