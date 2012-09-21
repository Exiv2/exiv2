# CMake build system for exiv2 library and executables
# Copyright 2012-     Robin Mills      <robin@clanmills.com>
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml at gmx dot de>

# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

PROJECT( exiv2 )

CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

SET( PACKAGE_COPYRIGHT      "Andreas Huggel" )
SET( PACKAGE_BUGREPORT      "ahuggel@gmx.net" )
SET( PACKAGE                "exiv2" )
SET( PACKAGE_NAME           "exiv2" )
SET( PACKAGE_STRING         "exiv2 0.23" )
SET( PACKAGE_TARNAME        "exiv2" )
SET( PACKAGE_VERSION        "0.23" )
SET( PACKAGE_URL            "http://www.exiv2.org" )
SET( GENERIC_LIB_VERSION    "12.0.0" )
SET( GENERIC_LIB_SOVERSION  "12" )

# options and their default values
OPTION( EXIV2_ENABLE_SHARED        "Build exiv2 as a shared library (dll)"                 ON  )
OPTION( EXIV2_ENABLE_XMP           "Build with XMP metadata support"                       ON  )
OPTION( EXIV2_ENABLE_LIBXMP        "Build a static convenience Library for XMP"            ON  )
OPTION( EXIV2_ENABLE_PNG           "Build with png support (requires libz)"                ON  )
OPTION( EXIV2_ENABLE_NLS           "Build native language support (requires gettext)"      OFF )
OPTION( EXIV2_ENABLE_PRINTUCS2     "Build with Printucs2"                                  ON  )
OPTION( EXIV2_ENABLE_LENSDATA      "Build including lens data"                             ON  )
OPTION( EXIV2_ENABLE_COMMERCIAL    "Build with the EXV_COMMERCIAL_VERSION symbol set"      OFF )
OPTION( EXIV2_ENABLE_BUILD_SAMPLES "Build the unit tests"                                  ON  )
OPTION( EXIV2_ENABLE_BUILD_PO      "Build translations files"                              OFF )

IF( MINGW OR UNIX )
    ADD_DEFINITIONS(-Wall
                    -Wcast-align
                    -Wpointer-arith
                    -Wformat-security
                    -Wmissing-format-attribute
                    -Woverloaded-virtual
                    -W
                   )
ENDIF( MINGW OR UNIX )

if( MSVC )
	include(CMake_msvc.txt)
	msvc_runtime_report()
	msvc_runtime_configure(${EXIV2_ENABLE_SHARED})
	msvc_runtime_report()
	
	set(E expat-2.1.0)
	set(Z zlib-1.2.7)
	
	if( NOT IS_DIRECTORY ./../${E} )
		set(OUT_OF_SOURCE 1)
	endif()
	
    if ( NOT EXPAT_LIBRARY  )
		# link dependent libraries
    	if (OUT_OF_SOURCE)
    		set(EXPAT_LIBRARY      ../../../${E}/$(ConfigurationName)/expat)
    		set(ZLIB_LIBRARY       ../../../${Z}/$(ConfigurationName)/zlib )
    		set(EXPAT_INCLUDE_DIR  ../${E}/lib )
    		set(ZLIB_INCLUDE_DIR   ../${Z}     )
		else()
    		set(EXPAT_LIBRARY      ../../${E}/$(ConfigurationName)/expat)
    		set(ZLIB_LIBRARY       ../../${Z}/$(ConfigurationName)/zlib)
    		set(EXPAT_INCLUDE_DIR  ../${E}/lib )
    		set(ZLIB_INCLUDE_DIR   ../${Z}     )
    	endif()
    	include_directories( ${EXPAT_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} msvc64\\include src\\include)
	endif()
	
	if ( EXIV2_ENABLE_SHARED )
	    add_library(${E} SHARED IMPORTED)
	    add_library(${Z} SHARED IMPORTED)
	endif()

	##
    # Set the output directory
    # x64/Win32
    if( CMAKE_SIZEOF_VOID_P MATCHES 8 )
        SET( P_DIR x64)
    else()
	    set( P_DIR Win32 )
    endif()

    # DLL/static build
    if( EXIV2_ENABLE_SHARED )
        set( T_DIR Dynamic )
    else()
	    set( T_DIR Static)
    endif()

    set(B_DIR ${PROJECT_BINARY_DIR}/bin/${P_DIR}/${T_DIR})
    set(CMAKE_CURRENT_BINARY_DIR ${B_DIR})
    set(EXECUTABLE_OUTPUT_PATH   ${B_DIR})
    set(LIBRARY_OUTPUT_PATH      ${B_DIR})
endif( MSVC )

IF( EXIV2_ENABLE_XMP )
	SET( HAVE_XMP_TOOLKIT ON )
	SET( XMPLIB "do" )
ENDIF( EXIV2_ENABLE_XMP )

INCLUDE( config/ConfigureChecks.cmake )

SET( INSTALL_TARGET_STANDARD_ARGS
     RUNTIME DESTINATION bin
     LIBRARY DESTINATION lib
     ARCHIVE DESTINATION lib
)

##
# add sources from XMPSDK directory
# this needs to be done here to be visible to xmpsdk, src and sample sub directories
SET( XMPSRC ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/ExpatAdapter.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/MD5.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/ParseRDF.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/UnicodeConversions.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/WXMPIterator.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/WXMPMeta.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/WXMPUtils.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XML_Node.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPCore_Impl.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPIterator.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPMeta-GetSet.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPMeta-Parse.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPMeta-Serialize.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPMeta.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPUtils-FileInfo.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/xmpsdk/src/XMPUtils.cpp
)

# the following lines need to be added to circumvent the error in xmpsdk\src\MD5.h
IF( ${EXV_HAVE_STDINT_H} )
    ADD_DEFINITIONS( -DEXV_HAVE_STDINT_H )
ENDIF( ${EXV_HAVE_STDINT_H} )

ADD_SUBDIRECTORY( xmpsdk )
ADD_SUBDIRECTORY( src )

IF( EXIV2_ENABLE_BUILD_SAMPLES )
    ADD_SUBDIRECTORY( samples )
ENDIF( EXIV2_ENABLE_BUILD_SAMPLES )

IF( EXIV2_ENABLE_BUILD_PO )
    ADD_SUBDIRECTORY( po )
ENDIF( EXIV2_ENABLE_BUILD_PO )

# That's all Folks!
##
