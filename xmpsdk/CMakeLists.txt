if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/public/include/XMP_Const.h")
    message(FATAL_ERROR
        "XMP-Toolkit-SDK submodule not found. Please run:\n"
        "  git submodule update --init --recursive")
endif()

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

# XMP-Toolkit-SDK includes expat as "third-party/expat/lib/expat.h".
# A wrapper header at xmpsdk/third-party/expat/lib/expat.h redirects
# this to the system expat via #include <expat.h> (angle brackets).
# Note: XMP-Toolkit-SDK/source/ is intentionally NOT on the include path.
# SDK files use "source/..." relative includes (resolved via the SDK root),
# and adding source/ directly causes a case-insensitive collision on Windows
# (Cygwin/MSYS2) between SDK's Endian.h and system <endian.h>.
target_include_directories(exiv2-xmp PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/public/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/third-party/zuid/>
        ${EXPAT_INCLUDE_DIRS}
)

# Prevent a denial-service-attack related to XML entity expansion ("billion laughs attack"). See https://bugzilla.redhat.com/show_bug.cgi?id=888769
target_compile_definitions(exiv2-xmp PUBLIC BanAllEntityUsage=1)

# Mark XMP SDK headers as SYSTEM includes for consuming targets (e.g. exiv2lib).
# This suppresses most compiler warnings originating from third-party SDK headers
# when exiv2 is built with -Werror.
target_include_directories(exiv2-xmp SYSTEM PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/public/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/XMP-Toolkit-SDK/third-party/zuid/interfaces>
)

# Interface library grouping compiler workarounds for targets that include XMP
# SDK headers and are built with -Werror. Link against this alongside exiv2-xmp.
#
# GCC never suppresses #warning directives from system headers (unlike Clang),
# so the SYSTEM include dirs above are not enough. XMPCommonDefines.h includes
# <ciso646>, which GCC 15+ flags as deprecated in C++20 via #warning. With
# -Werror this becomes fatal (-Werror=cpp).
add_library(exiv2-xmp-interface INTERFACE)
target_compile_options(exiv2-xmp-interface INTERFACE $<$<CXX_COMPILER_ID:GNU>:-Wno-error=cpp>)

if (WIN32)
    target_sources(exiv2-xmp PRIVATE XMP-Toolkit-SDK/source/Host_IO-Win.cpp)
    target_compile_definitions(exiv2-xmp PRIVATE XML_STATIC)
    target_compile_definitions(exiv2-xmp PUBLIC WIN_ENV)
    target_compile_definitions(exiv2-xmp PRIVATE WIN32 UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)
endif ()

if (WIN32 AND NOT MSVC)
    # Non-MSVC compilers (Clang, GCC) on Windows lack MSVC's <sal.h> header
    # which defines SAL annotations used by the XMP SDK's SafeStringAPIs.h.
    # Force-include our compatibility shim that defines them as empty.
    # See sal_compat.h for details.
    target_compile_options(exiv2-xmp PRIVATE -include ${CMAKE_CURRENT_SOURCE_DIR}/sal_compat.h)
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
    # The SDK's EndianUtils.hpp only auto-detects endianness for GCC on
    # x86/x86_64/SPARC under UNIX_ENV. For other architectures (ARM, RISC-V,
    # etc.) or non-GCC compilers, define it explicitly.
    include(TestBigEndian)
    test_big_endian(IS_BIG_ENDIAN)
    if (IS_BIG_ENDIAN)
        target_compile_definitions(exiv2-xmp PRIVATE kBigEndianHost=1)
    else ()
        target_compile_definitions(exiv2-xmp PRIVATE kBigEndianHost=0)
    endif ()
endif ()

if (BUILD_SHARED_LIBS)
    set_property(TARGET exiv2-xmp PROPERTY POSITION_INDEPENDENT_CODE ON)
endif ()
