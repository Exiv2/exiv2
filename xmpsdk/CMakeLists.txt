# CMake build system for exiv2 library and executables
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml at gmx dot de>

# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

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

foreach(_currentfile ${XMPSRC})
    if(UNIX AND NOT CYGWIN)
        set_source_files_properties(${_currentfile} PROPERTIES COMPILE_FLAGS "-fPIC")
    endif()
endforeach()

# We generate a CMake OBJECT LIBRARY (a bunch of object files)
add_library( xmp_object OBJECT ${XMPSRC} )

target_include_directories(xmp_object 
    PRIVATE 
        ${EXPAT_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}/xmpsdk/include
)

check_include_file( "stdint.h"  EXV_HAVE_STDINT_H )
if (EXV_HAVE_STDINT_H)
    target_compile_definitions(xmp_object PUBLIC EXV_HAVE_STDINT_H)
endif()

# TODO : We should only add this definition if EXPAT is static
target_compile_definitions(xmp_object PRIVATE XML_STATIC)

# http://stackoverflow.com/questions/10046114/in-cmake-how-can-i-test-if-the-compiler-is-clang
if ( NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang")
    # 1123 - hide xmpsdk symbols
    set_property(SOURCE ${XMPSRC} PROPERTY
        COMPILE_FLAGS "-fvisibility=hidden -fvisibility-inlines-hidden")
endif()

if( EXIV2_ENABLE_LIBXMP )
    add_library(xmp STATIC $<TARGET_OBJECTS:xmp_object>)
    target_link_libraries(xmp PUBLIC ${EXPAT_LIBRARY})

    # This is also needed for the xmp static library
    target_include_directories(xmp PUBLIC ${EXPAT_INCLUDE_DIR})
    target_compile_definitions(xmp PUBLIC XML_STATIC)

    if (BUILD_SHARED_LIBS)
        set_property(TARGET xmp_object PROPERTY COMPILE_FLAGS "-fPIC")
    endif()

    # 1119  Install libxmp.a for use by third party applications (Thanks, Emmanuel)
    install(TARGETS xmp
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()
