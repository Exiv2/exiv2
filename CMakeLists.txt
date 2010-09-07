# CMake build system for exiv2 library and executables
# Copyright 2008 by Patrick Spendrin <ps_ml@gmx.de>
# Copyright 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    1. Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#    3. The name of the author may not be used to endorse or promote
#       products derived from this software without specific prior
#       written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

PROJECT( exiv2 )

CMAKE_MINIMUM_REQUIRED( VERSION 2.6 )

SET( PACKAGE_COPYRIGHT      "Andreas Huggel" )
SET( PACKAGE_BUGREPORT      "ahuggel@gmx.net" )
SET( PACKAGE                "exiv2" )
SET( PACKAGE_NAME           "exiv2" )
SET( PACKAGE_STRING         "exiv2 0.21" )
SET( PACKAGE_TARNAME        "exiv2" )
SET( PACKAGE_VERSION        "0.21" )
SET( PACKAGE_URL            "http://www.exiv2.org" )
SET( GENERIC_LIB_VERSION    "9.0.0" )
SET( GENERIC_LIB_SOVERSION  "9" )

# options and there default values
OPTION( EXIV2_ENABLE_SHARED        "Build exiv2 as a shared library (dll)"                 ON  )
OPTION( EXIV2_ENABLE_XMP           "Build with XMP metadata support"                       ON  )
OPTION( EXIV2_ENABLE_LIBXMP        "Build a static convenience Library for XMP"            OFF )
OPTION( EXIV2_ENABLE_PNG           "Build with png support (requires libz)"                ON  )
OPTION( EXIV2_ENABLE_NLS           "Build native language support (requires gettext)"      ON  )
OPTION( EXIV2_ENABLE_PRINTUCS2     "Build with Printucs2"                                  ON  )
OPTION( EXIV2_ENABLE_LENSDATA      "Build including lens data"                             ON  )
OPTION( EXIV2_ENABLE_COMMERCIAL    "Build with the EXV_COMMERCIAL_VERSION symbol set"      OFF )
OPTION( EXIV2_ENABLE_BUILD_SAMPLES "Build the unit tests"                                  OFF )
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

# add sources from XMPSDK directory - this needs to be done here
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

IF( EXIV2_BUILD_PO )
    ADD_SUBDIRECTORY( po )
ENDIF( EXIV2_BUILD_PO )
