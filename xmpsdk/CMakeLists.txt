# CMake build system for exiv2 library and executables
# Copyright 2012-     Robin Mills      <robin@clanmills.com>
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml at gmx dot de>

# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(../CMake_msvc.txt)
msvc_runtime_configure(${EXIV2_ENABLE_SHARED} ${EXIV2_ENABLE_DYNAMIC_RUNTIME})

if( POLICY CMP0026 )
    cmake_policy(SET CMP0026 OLD) # Something to do with location
endif()

FOREACH(_currentfile ${XMPSRC})
    # http://www.openguru.com/2009/04/cmake-detecting-platformoperating.html
    IF(NOT MSVC AND NOT CYGWIN AND NOT MSYS AND NOT MINGW)
        SET_SOURCE_FILES_PROPERTIES(${_currentfile} PROPERTIES COMPILE_FLAGS "-fPIC")
    ENDIF()
ENDFOREACH()

if(NOT MSVC)
  # http://stackoverflow.com/questions/10046114/in-cmake-how-can-i-test-if-the-compiler-is-clang
  if ( NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang")
    # 1123 - hide xmpsdk symbols
    add_definitions( -fvisibility=hidden -fvisibility-inlines-hidden )
  endif()
endif()

IF( EXIV2_ENABLE_XMP AND EXIV2_ENABLE_LIBXMP )
    ADD_LIBRARY( xmp STATIC ${XMPSRC} )
    GET_TARGET_PROPERTY( XMPLIB xmp LOCATION )
    if ( NOT MSVC )
        TARGET_LINK_LIBRARIES(xmp ${EXPAT_LIBRARIES})
    else()
        TARGET_LINK_LIBRARIES(xmp Threads::Threads ${EXPAT_LIBRARIES})
    endif()
    # 1119  Install libxmp.a for use by third party applications (Thanks, Emmanuel)
    INSTALL(TARGETS xmp ${INSTALL_TARGET_STANDARD_ARGS} )
ENDIF()

# That's all Folks!
##
