# CMake build system for exiv2 library and executables
# Copyright 2008      by Patrick Spendrin <ps_ml@gmx.de>
# Copyright 2010-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

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
