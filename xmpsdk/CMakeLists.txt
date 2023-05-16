add_library(exiv2-xmp OBJECT
    src/ExpatAdapter.cpp
    src/MD5.cpp
    src/ParseRDF.cpp
    src/UnicodeConversions.cpp
    src/WXMPIterator.cpp
    src/WXMPMeta.cpp
    src/WXMPUtils.cpp
    src/XML_Node.cpp
    src/XMPCore_Impl.cpp
    src/XMPIterator.cpp
    src/XMPMeta-GetSet.cpp
    src/XMPMeta-Parse.cpp
    src/XMPMeta-Serialize.cpp
    src/XMPMeta.cpp
    src/XMPUtils-FileInfo.cpp
    src/XMPUtils.cpp
    include/MD5.h
    include/TXMPIterator.hpp
    include/TXMPMeta.hpp
    include/TXMPUtils.hpp
    include/XMP_Const.h
    include/XMP_Environment.h
    include/XMP.incl_cpp
    include/XMPSDK.hpp
    include/XMP_Version.h
)

target_include_directories(exiv2-xmp SYSTEM
    PRIVATE
        ${PROJECT_SOURCE_DIR}/xmpsdk/include
        ${EXPAT_INCLUDE_DIRS}
)

# Prevent a denial-service-attack related to XML entity expansion
# ("billion laughs attack").
# See https://bugzilla.redhat.com/show_bug.cgi?id=888769
target_compile_definitions(exiv2-xmp PRIVATE BanAllEntityUsage=1)

if (WIN32)
    target_compile_definitions(exiv2-xmp PRIVATE XML_STATIC)
endif()

if (MINGW)
    # https://stackoverflow.com/questions/18551409/localtime-r-support-on-mingw
    target_compile_definitions(exiv2-xmp PRIVATE _POSIX_THREAD_SAFE_FUNCTIONS)
endif()

if (BUILD_SHARED_LIBS)
    set_property(TARGET exiv2-xmp PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()
