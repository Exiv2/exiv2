# CMakeLists.txt exiv2/xmpsdk

# list files to be use to build XMPsdk
set(XMPSRC src/ExpatAdapter.cpp
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

add_library(xmp STATIC ${XMPSRC})

target_link_libraries(xmp 
    PRIVATE 
        ${EXPAT_LIBRARY}
)

target_include_directories(xmp
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/xmpsdk/include>
    PRIVATE 
        ${EXPAT_INCLUDE_DIR}
)

# TODO : We should only add this definition for Windows
target_compile_definitions(xmp PRIVATE XML_STATIC)

check_include_file( "stdint.h"  EXV_HAVE_STDINT_H )
if (EXV_HAVE_STDINT_H)
    target_compile_definitions(xmp PRIVATE EXV_HAVE_STDINT_H)
endif()

# http://stackoverflow.com/questions/10046114/in-cmake-how-can-i-test-if-the-compiler-is-clang
if ( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # 1123 - hide xmpsdk symbols
    set_property(SOURCE ${XMPSRC} PROPERTY
        COMPILE_FLAGS "-fvisibility=hidden -fvisibility-inlines-hidden")
endif()

if (BUILD_SHARED_LIBS)
    set_property(TARGET xmp PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

# 1119  Install libxmp.a for use by third party applications (Thanks, Emmanuel)
install(TARGETS xmp EXPORT exiv2Config
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# That's all Folks!
##
