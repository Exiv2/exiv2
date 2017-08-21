# CMake build system for exiv2 library and executables
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

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
             xmpprint.cpp
             xmpsample.cpp
   )

foreach(entry ${SAMPLES})
    string( REPLACE ".cpp" "" target ${entry})
    add_executable( ${target} ${target}.cpp )
    add_test( ${target}_test ${target} )
    target_link_libraries( ${target} PRIVATE exiv2lib)
    install( TARGETS ${target} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()

###################################

add_executable( metacopy metacopy.cpp ../src/utils.cpp)
target_link_libraries( metacopy PRIVATE exiv2lib)
target_include_directories(metacopy PRIVATE ${CMAKE_SOURCE_DIR}/src) # To find utils.hpp

add_executable       ( pathtest  path-test.cpp ../src/utils.cpp)
set_target_properties( pathtest  PROPERTIES OUTPUT_NAME path-test )
target_link_libraries( pathtest  PRIVATE exiv2lib)
target_include_directories(pathtest PRIVATE ${CMAKE_SOURCE_DIR}/src) # To find utils.hpp

if(NOT EXV_HAVE_TIMEGM )
    target_sources(pathtest PRIVATE ../src/localtime.c)
endif()

add_executable(        exiv2json exiv2json.cpp Jzon.cpp)
target_link_libraries( exiv2json PRIVATE exiv2lib)

install( TARGETS metacopy pathtest exiv2json RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if (MSVC)
    target_sources(pathtest PRIVATE ../src/getopt_win32.c)
    target_sources(metacopy PRIVATE ../src/getopt_win32.c)
endif()

if( EXIV2_ENABLE_XMP )
    add_executable(        geotag    geotag.cpp)
    target_link_libraries( geotag    PRIVATE exiv2lib ${ZLIB_LIBRARIES})
    install( TARGETS       geotag    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# ******************************************************************************
# connection test application
add_executable(conntest conntest.cpp)
target_link_libraries(conntest PRIVATE  exiv2lib)

# ******************************************************************************
# exifprint application
# ADD_EXECUTABLE( exifprint exifprint.cpp )
# TARGET_LINK_LIBRARIES( exifprint exiv2lib )

# ******************************************************************************
# remotetest application
add_executable(remotetest remotetest.cpp)
target_link_libraries(remotetest exiv2lib)

# ******************************************************************************
# Man page
INSTALL( FILES exiv2samples.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )
