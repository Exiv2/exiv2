# CMake build system for exiv2 library and executables
# Copyright 2012-     Robin Mills      <robin@clanmills.com>
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if ( NOT MSVC )
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

include_directories("${CMAKE_SOURCE_DIR}/include" "${CMAKE_SOURCE_DIR}/src")

SET( SAMPLES addmoddel.cpp
             convert-test.cpp
             easyaccess-test.cpp
             exifcomment.cpp
             exifdata-test.cpp
             exifdata.cpp
             exifprint.cpp
             exifvalue.cpp
             ini-test.cpp
             iotest.cpp
             iptceasy.cpp
             iptcprint.cpp
             iptctest.cpp
             key-test.cpp
             largeiptc-test.cpp
             mmap-test.cpp
             prevtest.cpp
             stringto-test.cpp
             taglist.cpp
             tiff-test.cpp
             werror-test.cpp
             write-test.cpp
             write2-test.cpp
             xmpparse.cpp
             xmpparser-test.cpp
             xmpsample.cpp
   )

##
# msvn tuning
include(../CMake_msvc.txt)
msvc_runtime_configure(${EXIV2_ENABLE_SHARED} ${EXIV2_ENABLE_DYNAMIC_RUNTIME})

FOREACH(entry ${SAMPLES})
    STRING( REPLACE ".cpp" "" target ${entry})
    ADD_EXECUTABLE( ${target} ${target}.cpp )
    ADD_TEST( ${target}_test ${target} )
    TARGET_LINK_LIBRARIES( ${target} ${PRIVATE_VAR} exiv2lib Threads::Threads ${EXPAT_LIBRARIES})
    INSTALL( TARGETS ${target} ${INSTALL_TARGET_STANDARD_ARGS} )
ENDFOREACH(entry ${SAMPLES})

###################################
# metacopy/path-test sample applications require utils.cpp support
IF( MSVC )
    SET( PATHTEST_SRC      ${PATHTEST_SRC} ../src/getopt_win32.c )
    SET( MC_SRC            ${MC_SRC}       ../src/getopt_win32.c )
ENDIF( MSVC )

SET( MC_SRC ${MC_SRC} metacopy.cpp ../src/utils.cpp )
ADD_EXECUTABLE( metacopy ${MC_SRC} )
TARGET_LINK_LIBRARIES( metacopy ${PRIVATE_VAR} exiv2lib Threads::Threads ${EXPAT_LIBRARIES} ${ZLIB_LIBRARIES})
INSTALL( TARGETS metacopy ${INSTALL_TARGET_STANDARD_ARGS} )

SET   ( PATHTEST_SRC             ${PATHTEST_SRC}  path-test.cpp ../src/utils.cpp )
ADD_EXECUTABLE       ( pathtest  ${PATHTEST_SRC} )
SET_TARGET_PROPERTIES( pathtest  PROPERTIES OUTPUT_NAME path-test )
TARGET_LINK_LIBRARIES( pathtest  ${PRIVATE_VAR} exiv2lib Threads::Threads ${EXPAT_LIBRARIES} ${ZLIB_LIBRARIES})
INSTALL      ( TARGETS pathtest  ${INSTALL_TARGET_STANDARD_ARGS} )

SET( EXIV2JSON_SRC     exiv2json.cpp Jzon.cpp )
ADD_EXECUTABLE(        exiv2json ${EXIV2JSON_SRC} )
SET_TARGET_PROPERTIES( exiv2json PROPERTIES OUTPUT_NAME exiv2json )
TARGET_LINK_LIBRARIES( exiv2json ${PRIVATE_VAR} ${PRIVATE_VAR} exiv2lib Threads::Threads ${EXPAT_LIBRARIES} ${ZLIB_LIBRARIES})
INSTALL( TARGETS       exiv2json ${INSTALL_TARGET_STANDARD_ARGS} )

SET( GEOTAG_SRC        geotag.cpp )
ADD_EXECUTABLE(        geotag    ${GEOTAG_SRC} )
SET_TARGET_PROPERTIES( geotag    PROPERTIES OUTPUT_NAME geotag )
TARGET_LINK_LIBRARIES( geotag    ${PRIVATE_VAR} exiv2lib Threads::Threads ${EXPAT_LIBRARIES} ${ZLIB_LIBRARIES})
INSTALL( TARGETS       geotag    ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# Man page
INSTALL( FILES exiv2samples.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

# That's all Folks!
##


