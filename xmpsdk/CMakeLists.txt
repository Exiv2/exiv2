add_library(
        exiv2-xmp OBJECT
        XMP-Toolkit-SDK/public/include/XMP_Const.h
        XMP-Toolkit-SDK/public/include/XMP_Environment.h
        XMP-Toolkit-SDK/public/include/XMP_Version.h
        XMP-Toolkit-SDK/public/include/TXMPFiles.hpp
        XMP-Toolkit-SDK/public/include/TXMPIterator.hpp
        XMP-Toolkit-SDK/public/include/TXMPMeta.hpp
        XMP-Toolkit-SDK/public/include/TXMPUtils.hpp
        XMP-Toolkit-SDK/public/include/XMP.hpp
        XMP-Toolkit-SDK/public/include/XMP_IO.hpp

        XMP-Toolkit-SDK/source/Endian.h
        XMP-Toolkit-SDK/source/EndianUtils.hpp
        XMP-Toolkit-SDK/source/ExpatAdapter.hpp
        XMP-Toolkit-SDK/source/Host_IO.hpp
        XMP-Toolkit-SDK/source/IOUtils.cpp
        XMP-Toolkit-SDK/source/IOUtils.hpp
        XMP-Toolkit-SDK/source/PerfUtils.cpp
        XMP-Toolkit-SDK/source/PerfUtils.hpp
        XMP-Toolkit-SDK/source/SafeStringAPIs.cpp
        XMP-Toolkit-SDK/source/SafeStringAPIs.h
        XMP-Toolkit-SDK/source/SafeTypes.h
        XMP-Toolkit-SDK/source/SuppressSAL.h
        XMP-Toolkit-SDK/source/UnicodeConversions.cpp
        XMP-Toolkit-SDK/source/UnicodeConversions.hpp
        XMP-Toolkit-SDK/source/UnicodeInlines.incl_cpp
        XMP-Toolkit-SDK/source/XIO.cpp
        XMP-Toolkit-SDK/source/XIO.hpp
        XMP-Toolkit-SDK/source/XML_Node.cpp
        XMP-Toolkit-SDK/source/XMLParserAdapter.hpp
        XMP-Toolkit-SDK/source/XMPFiles_IO.cpp
        XMP-Toolkit-SDK/source/XMPFiles_IO.hpp
        XMP-Toolkit-SDK/source/XMP_LibUtils.cpp
        XMP-Toolkit-SDK/source/XMP_LibUtils.hpp
        XMP-Toolkit-SDK/source/XMP_ProgressTracker.cpp
        XMP-Toolkit-SDK/source/XMP_ProgressTracker.hpp

        XMP-Toolkit-SDK/XMPCore/source/ExpatAdapter.cpp
        XMP-Toolkit-SDK/XMPCore/source/ParseRDF.cpp
        XMP-Toolkit-SDK/XMPCore/source/WXMPMeta.cpp
        XMP-Toolkit-SDK/XMPCore/source/WXMPUtils.cpp
        XMP-Toolkit-SDK/XMPCore/source/XMPCore_Impl.cpp
        XMP-Toolkit-SDK/XMPCore/source/XMPIterator.cpp
        XMP-Toolkit-SDK/XMPCore/source/WXMPIterator.cpp
        XMP-Toolkit-SDK/XMPCore/source/XMPMeta.cpp
        XMP-Toolkit-SDK/XMPCore/source/XMPMeta-GetSet.cpp
        XMP-Toolkit-SDK/XMPCore/source/XMPMeta-Parse.cpp
        XMP-Toolkit-SDK/XMPCore/source/XMPMeta-Serialize.cpp
        XMP-Toolkit-SDK/XMPCore/source/XMPUtils.cpp
        XMP-Toolkit-SDK/XMPCore/source/XMPUtils-FileInfo.cpp

        XMP-Toolkit-SDK/third-party/zuid/interfaces/MD5.cpp
)

target_compile_features(exiv2-xmp PUBLIC cxx_std_20)

# there is an include for /third-party/expat/lib/expat.h in XMP-Toolkit-SDK
# we do these shenanigans to create a fake /third-party/expat/ in build directory
if (NOT EXISTS ${CMAKE_BINARY_DIR}/fake_expat/third-party/expat/lib)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/fake_expat/third-party/expat/lib)
endif ()
file(GLOB_RECURSE EXPAT_H ${EXPAT_INCLUDE_DIRS}*/expat.h)
list(LENGTH EXPAT_H EXPAT_H_ITEM_CNT)
if(EXPAT_H_ITEM_CNT EQUAL 1)
    list(GET EXPAT_H 0 EXPAT_H)
    message(STATUS "The list contains one item: ${EXPAT_H}")
    file(COPY_FILE ${EXPAT_H} ${CMAKE_BINARY_DIR}/fake_expat/third-party/expat/lib/expat.h)
else()
    message(FATAL_ERROR "Cannot find expat.h")
endif()

target_include_directories(exiv2-xmp PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/public/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/source/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/third-party/zuid/>
        ${EXPAT_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}/fake_expat # this is the fake include directory needed for expat
)

# Prevent a denial-service-attack related to XML entity expansion ("billion laughs attack"). See https://bugzilla.redhat.com/show_bug.cgi?id=888769
target_compile_definitions(exiv2-xmp PUBLIC BanAllEntityUsage=1)

# need to be careful not to expose any xmp includes in exiv2 headers
target_include_directories(exiv2-xmp PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/> # md5.h seems to need this
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/public/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/third-party/zuid/interfaces>
)

if (WIN32)
    target_sources(exiv2-xmp PRIVATE XMP-Toolkit-SDK/source/Host_IO-Win.cpp)
    target_compile_definitions(exiv2-xmp PRIVATE XML_STATIC)
    target_compile_definitions(exiv2-xmp PUBLIC WIN_ENV)
    target_compile_definitions(exiv2-xmp PRIVATE WIN32 UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)
endif ()

if (MINGW)
    # https://stackoverflow.com/questions/18551409/localtime-r-support-on-mingw
    target_compile_definitions(exiv2-xmp PRIVATE _POSIX_THREAD_SAFE_FUNCTIONS)
endif ()

if (APPLE)
    target_sources(exiv2-xmp PRIVATE XMP-Toolkit-SDK/source/Host_IO-POSIX.cpp)
    target_compile_definitions(exiv2-xmp PUBLIC MAC_ENV)
elseif (UNIX)
    target_sources(exiv2-xmp PRIVATE XMP-Toolkit-SDK/source/Host_IO-POSIX.cpp)
    target_compile_definitions(exiv2-xmp PUBLIC UNIX_ENV)
endif ()

if (BUILD_SHARED_LIBS)
    set_property(TARGET exiv2-xmp PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()
