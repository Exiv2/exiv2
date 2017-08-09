# CMake build system for exiv2 library and executables
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
    check_include_file( "stdint.h"  EXV_HAVE_STDINT_H )

    ADD_LIBRARY( xmp STATIC ${XMPSRC} )

    target_include_directories(xmp PRIVATE ${CMAKE_SOURCE_DIR}/xmpsdk/include)
    if (EXV_HAVE_STDINT_H)
        target_compile_definitions(xmp PRIVATE EXV_HAVE_STDINT_H)
    endif()

    GET_TARGET_PROPERTY( XMPLIB xmp LOCATION )
    if ( NOT MSVC )
        TARGET_LINK_LIBRARIES(xmp ${EXPAT_LIBRARIES})
    else()
        TARGET_LINK_LIBRARIES(xmp ${CMAKE_THREAD_LIBS_INIT} ${EXPAT_LIBRARIES})
    endif()
    # 1119  Install libxmp.a for use by third party applications (Thanks, Emmanuel)
    INSTALL(TARGETS xmp ${INSTALL_TARGET_STANDARD_ARGS} )
ENDIF()

# That's all Folks!
##
