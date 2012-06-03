# CMake build system for exiv2 library and executables
# Copyright 2012-     Robin Mills      <robin@clanmills.com>
# Copyright 2010-2012 Gilles Caulier   <caulier dot gilles at gmail dot com>
# Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

if (MSVC)
	include(../CMake_msvc.txt)
	msvc_runtime_configure(${EXIV2_ENABLE_SHARED})
	include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../msvc64/include )
	
	if ( NOT EXIV2_ENABLE_SHARED )		
		link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../zlib-1.2.7/$(ConfigurationName))
	endif()
endif()

SET( SAMPLES addmoddel.cpp
             convert-test.cpp
             easyaccess-test.cpp
             exifcomment.cpp
             exifdata-test.cpp
             exifprint.cpp
             iotest.cpp
             iptceasy.cpp
             iptcprint.cpp
             iptctest.cpp
             key-test.cpp
             largeiptc-test.cpp
             mmap-test.cpp
             prevtest.cpp 
             stringto-test.cpp
             tiff-test.cpp
             werror-test.cpp
             write-test.cpp
             write2-test.cpp
             xmpparse.cpp
             xmpparser-test.cpp
             xmpsample.cpp
   )

FOREACH(entry ${SAMPLES})
    STRING( REPLACE ".cpp" "" target ${entry})
    ADD_EXECUTABLE( ${target} ${target}.cpp )
    ADD_TEST( ${target}_test ${target} )
    TARGET_LINK_LIBRARIES( ${target} exiv2 )
ENDFOREACH(entry ${SAMPLES})

# That's all Folks!
##
