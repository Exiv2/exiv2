// SPDX-License-Identifier: GPL-2.0-or-later

// included header files
#include "error.hpp"
#include "i18n.h"  // NLS support.
#include "types.hpp"

// + standard includes
#include <array>
#include <iostream>

namespace {
//! Complete list of Exiv2 exception error messages
constexpr std::array errList{
    N_("Success"),                                   // kerSuccess
    N_("Error %0: arg2=%2, arg3=%3, arg1=%1."),      // KerGeneralError
    "%1",                                            //  kerErrorMessage,
    "%1: Call to `%3' failed: %2",                   // KerCallFailed, %1=path, %2=strerror, %3=function that failed
    N_("This does not look like a %1 image"),        // kerNotAnImage, %1=Image type
    N_("Invalid dataset name '%1'"),                 // kerInvalidDataset %1=dataset name
    N_("Invalid record name '%1'"),                  // kerInvalidRecord %1=record name
    N_("Invalid key '%1'"),                          // kerInvalidKey %1=key
    N_("Invalid tag name or ifdId `%1', ifdId %2"),  // kerInvalidTag %1=tag name, %2=ifdId
    N_("%1 value not set"),                          // kerValueNotSet %1=key name
    N_("%1: Failed to open the data source: %2"),    // kerDataSourceOpenFailed %1=path, %2=strerror
    N_("%1: Failed to open file (%2): %3"),          // kerFileOpenFailed %1=path, %2=mode, %3=strerror
    N_("%1: The file contains data of an unknown image type"),  // kerFileContainsUnknownImageType %1=path
    N_("The memory contains data of an unknown image type"),    // kerMemoryContainsUnknownImageType
    N_("Image type %1 is not supported"),                       // kerUnsupportedImageType %1=image type
    N_("Failed to read image data"),                            // kerFailedToReadImageData
    N_("This does not look like a JPEG image"),                 // kerNotAJpeg
    N_("%1: Failed to map file for reading and writing: %2"),   // kerFailedToMapFileForReadWrite %1=path,
                                                                // %2=strerror
    N_("%1: Failed to rename file to %2: %3"),        // kerFileRenameFailed %1=old path, %2=new path, %3=strerror
    N_("%1: Transfer failed: %2"),                    // kerTransferFailed %1=path, %2=strerror
    N_("Memory transfer failed: %1"),                 // kerMemoryTransferFailed %1=strerror
    N_("Failed to read input data"),                  // kerInputDataReadFailed
    N_("Failed to write image"),                      // kerImageWriteFailed
    N_("Input data does not contain a valid image"),  // kerNoImageInInputData
    N_("Invalid ifdId %1"),                           // kerInvalidIfdId %1=ifdId
    N_("Entry::setValue: Value too large (tag=%1, size=%2, requested=%3)"),     // kerValueTooLarge %1=tag,
                                                                                // %2=dataSize, %3=required size
    N_("Entry::setDataArea: Value too large (tag=%1, size=%2, requested=%3)"),  // kerDataAreaValueTooLarge %1=tag,
                                                                                // %2=dataAreaSize, %3=required size
    N_("Offset out of range"),                                                  // kerOffsetOutOfRange
    N_("Unsupported data area offset type"),                                    // kerUnsupportedDataAreaOffsetType
    N_("Invalid charset: `%1'"),                                                // kerInvalidCharset %1=charset name
    N_("Unsupported date format"),                                              // kerUnsupportedDateFormat
    N_("Unsupported time format"),                                              // kerUnsupportedTimeFormat
    N_("Writing to %1 images is not supported"),     // kerWritingImageFormatUnsupported %1=image format
    N_("Setting %1 in %2 images is not supported"),  // kerInvalidSettingForImage %1=metadata type, %2=image format
    N_("This does not look like a CRW image"),       // kerNotACrwImage
    N_("%1: Not supported"),                         // kerFunctionNotSupported %1=function
    N_("No namespace info available for XMP prefix `%1'"),  // kerNoNamespaceInfoForXmpPrefix %1=prefix
    N_("No prefix registered for namespace `%2', needed for property path `%1'"),  // kerNoPrefixForNamespace
                                                                                   // %1=namespace
    N_("Size of %1 JPEG segment is larger than 65535 bytes"),  // kerTooLargeJpegSegment %1=type of metadata (Exif,
                                                               // IPTC, JPEG comment)
    N_("Unhandled Xmpdatum %1 of type %2"),                    // kerUnhandledXmpdatum %1=key, %2=value type
    N_("Unhandled XMP node %1 with opt=%2"),  // kerUnhandledXmpNode %1=key, %2=XMP Toolkit option flags
    N_("XMP Toolkit error %1: %2"),           // kerXMPToolkitError %1=XMP_Error::GetID(), %2=XMP_Error::GetErrMsg()
    N_("Failed to decode Lang Alt property %1 with opt=%2"),   // kerDecodeLangAltPropertyFailed %1=property path,
                                                               // %3=XMP Toolkit option flags
    N_("Failed to decode Lang Alt qualifier %1 with opt=%2"),  // kerDecodeLangAltQualifierFailed %1=qualifier path,
                                                               // %3=XMP Toolkit option flags
    N_("Failed to encode Lang Alt property %1"),               // kerEncodeLangAltPropertyFailed %1=key
    N_("Failed to determine property name from path %1, namespace %2"),  // kerPropertyNameIdentificationFailed
                                                                         // %1=property path, %2=namespace
    N_("Schema namespace %1 is not registered with the XMP Toolkit"),    // kerSchemaNamespaceNotRegistered
                                                                         // %1=namespace
    N_("No namespace registered for prefix `%1'"),                       // kerNoNamespaceForPrefix %1=prefix
    N_("Aliases are not supported. Please send this XMP packet to ahuggel@gmx.net `%1', `%2', `%3'"),  // kerAliasesNotSupported
                                                                                                       // %1=namespace,
                                                                                                       // %2=property
                                                                                                       // path,
                                                                                                       // %3=value
    N_("Invalid XmpText type `%1'"),               // kerInvalidXmpText %1=type
    N_("TIFF directory %1 has too many entries"),  // kerTooManyTiffDirectoryEntries %1=TIFF directory name
    N_("Multiple TIFF array element tags %1 in one directory"),  // kerMultipleTiffArrayElementTagsInDirectory
                                                                 // %1=tag number
    N_("TIFF array element tag %1 has wrong type"),              // kerWrongTiffArrayElementTagType %1=tag number
    N_("%1 has invalid XMP value type `%2'"),                    // kerInvalidKeyXmpValue %1=key, %2=value type
    N_("Not a valid ICC Profile"),                               // kerInvalidIccProfile
    N_("Not valid XMP"),                                         // kerInvalidXMP
    N_("tiff directory length is too large"),                    // kerTiffDirectoryTooLarge
    N_("invalid type in tiff structure"),                        // kerInvalidTypeValue
    N_("Invalid LangAlt value `%1'"),                            // kerInvalidLangAltValue %1=value
    N_("invalid memory allocation request"),                     // kerInvalidMalloc
    N_("corrupted image metadata"),                              // kerCorruptedMetadata
    N_("Arithmetic operation overflow"),                         // kerArithmeticOverflow
    N_("Memory allocation failed"),                              // kerMallocFailed
    N_("Cannot convert text encoding from '%1' to '%2'"),        // kerInvalidIconvEncoding
    N_("%1: File access disabled in exiv2 build options"),       // kerFileAccessDisabled %1=path
};
static_assert(errList.size() == static_cast<size_t>(Exiv2::ErrorCode::kerErrorCount),
              "errList needs to contain a error msg for every ErrorCode defined in error.hpp");

}  // namespace

// *****************************************************************************
// class member definitions
namespace Exiv2 {
LogMsg::Level LogMsg::level_ = LogMsg::warn;  // Default output level
LogMsg::Handler LogMsg::handler_ = LogMsg::defaultHandler;

LogMsg::LogMsg(LogMsg::Level msgType) : msgType_(msgType) {
}

LogMsg::~LogMsg() {
  if (msgType_ >= level_ && handler_)
    handler_(msgType_, os_.str().c_str());
}

std::ostringstream& LogMsg::os() {
  return os_;
}

void LogMsg::setLevel(LogMsg::Level level) {
  level_ = level;
}

void LogMsg::setHandler(LogMsg::Handler handler) {
  handler_ = handler;
}

LogMsg::Level LogMsg::level() {
  return level_;
}

LogMsg::Handler LogMsg::handler() {
  return handler_;
}

void LogMsg::defaultHandler(int level, const char* s) {
  switch (static_cast<LogMsg::Level>(level)) {
    case LogMsg::debug:
      std::cerr << "Debug: ";
      break;
    case LogMsg::info:
      std::cerr << "Info: ";
      break;
    case LogMsg::warn:
      std::cerr << "Warning: ";
      break;
    case LogMsg::error:
      std::cerr << "Error: ";
      break;
    default:
      break;
  }
  std::cerr << s;
}

Error::Error(ErrorCode code) : code_(code) {
  setMsg(0);
}

ErrorCode Error::code() const noexcept {
  return code_;
}

const char* Error::what() const noexcept {
  return msg_.c_str();
}

void Error::setMsg(int count) {
  std::string msg{_(errList.at(static_cast<size_t>(code_)))};
  auto pos = msg.find("%0");
  if (pos != std::string::npos) {
    msg.replace(pos, 2, std::to_string(static_cast<int>(code_)));
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
}

}  // namespace Exiv2
