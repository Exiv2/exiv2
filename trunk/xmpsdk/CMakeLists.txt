# CMake build system for exiv2 library and executables
# Copyright 2008      by Patrick Spendrin <ps_ml at gmx dot de>
# Copyright 2010-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF( EXIV2_ENABLE_XMP AND EXIV2_ENABLE_LIBXMP )
    ADD_LIBRARY( xmp STATIC ${XMPSRC} )
    GET_TARGET_PROPERTY( XMPLIB xmp LOCATION )
    SET( XMPLIB ${XMPLIB} PARENT_SCOPE )

    INSTALL(TARGETS xmp ${INSTALL_TARGET_STANDARD_ARGS} )
ENDIF( EXIV2_ENABLE_XMP AND EXIV2_ENABLE_LIBXMP )
