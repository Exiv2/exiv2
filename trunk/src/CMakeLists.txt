# CMake build system for exiv2 library and executables
# Copyright 2008      by Patrick Spendrin <ps_ml@gmx.de>
# Copyright 2010-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

# Private headers which are only needed for the library itself
SET( LIBEXIV2_PRIVATE_HDR canonmn_int.hpp
                          cr2image_int.hpp
                          crwimage_int.hpp
                          fujimn_int.hpp
                          makernote_int.hpp
                          minoltamn_int.hpp
                          nikonmn_int.hpp
                          olympusmn_int.hpp
                          orfimage_int.hpp
                          panasonicmn_int.hpp
                          pentaxmn_int.hpp
                          pngchunk_int.hpp
                          rcsid_int.hpp
                          rw2image_int.hpp
                          samsungmn_int.hpp
                          sigmamn_int.hpp
                          sonymn_int.hpp
                          tags_int.hpp
                          tiffcomposite_int.hpp
                          tifffwd_int.hpp
                          tiffimage_int.hpp
                          tiffvisitor_int.hpp
   )

# Add standalone C++ header files to this list
SET( LIBEXIV2_HDR         basicio.hpp
                          bmpimage.hpp
                          convert.hpp
                          cr2image.hpp
                          crwimage.hpp
                          datasets.hpp
                          easyaccess.hpp
                          epsimage.hpp
                          error.hpp
                          exif.hpp
                          exiv2.hpp
                          futils.hpp
                          gifimage.hpp
                          image.hpp
                          iptc.hpp
                          jp2image.hpp
                          jpgimage.hpp
                          metadatum.hpp
                          mrwimage.hpp
                          orfimage.hpp
                          pgfimage.hpp
                          preview.hpp
                          properties.hpp
                          psdimage.hpp
                          rafimage.hpp
                          rw2image.hpp
                          tags.hpp
                          tgaimage.hpp
                          tiffimage.hpp
                          types.hpp
                          value.hpp
                          version.hpp
                          xmp.hpp
                          xmpsidecar.hpp
   )

# Add library C++ source files to this list
SET( LIBEXIV2_SRC         basicio.cpp
                          bmpimage.cpp
                          canonmn.cpp
                          convert.cpp
                          cr2image.cpp
                          crwedit.cpp
                          crwimage.cpp
                          datasets.cpp
                          easyaccess.cpp
                          epsimage.cpp
                          error.cpp
                          exif.cpp
                          futils.cpp
                          fujimn.cpp
                          gifimage.cpp
                          image.cpp
                          iptc.cpp
                          jp2image.cpp
                          jpgimage.cpp
                          makernote.cpp
                          metadatum.cpp
                          minoltamn.cpp
                          mrwimage.cpp
                          nikonmn.cpp
                          olympusmn.cpp
                          orfimage.cpp
                          panasonicmn.cpp
                          pentaxmn.cpp
                          pgfimage.cpp
                          preview.cpp
                          properties.cpp
                          psdimage.cpp
                          rafimage.cpp
                          rw2image.cpp
                          samsungmn.cpp
                          sigmamn.cpp
                          sonymn.cpp
                          tags.cpp
                          tgaimage.cpp
                          tiffcomposite.cpp
                          tiffimage.cpp
                          tiffvisitor.cpp
                          types.cpp
                          value.cpp
                          version.cpp
                          xmp.cpp
                          xmpsidecar.cpp
   )

# Add source files of simple applications to this list
SET( TAGLIST_SRC          taglist.cpp )

# Source files for the Exiv2 application
SET( EXIV2_SRC            exiv2.cpp
                          actions.cpp
                          utils.cpp
   )

# Source files for the metacopy sample application
SET( MC_SRC               metacopy.cpp
                          utils.cpp
   )

IF( EXIV2_ENABLE_PNG )
    IF( ZLIB_FOUND )
        SET( LIBEXIV2_SRC ${LIBEXIV2_SRC}
                          pngchunk.cpp
                          pngimage.cpp
           )
        set( LIBEXIV2_HDR ${LIBEXIV2_HDR} pngimage.hpp )
    ENDIF( ZLIB_FOUND )
ENDIF( EXIV2_ENABLE_PNG )

IF(NOT HAVE_TIMEGM )
    SET( LIBEXIV2_SRC     ${LIBEXIV2_SRC} localtime.c    )
    SET( EXIV2_SRC        ${EXIV2_SRC} localtime.c       )
ENDIF( NOT HAVE_TIMEGM )

IF( MSVC )
    SET( MC_SRC           ${MC_SRC} getopt_win32.c       )
    SET( EXIV2_SRC        ${EXIV2_SRC} getopt_win32.c    )
    SET( LIBEXIV2_SRC     ${LIBEXIV2_SRC} getopt_win32.c )
ENDIF( MSVC )

IF( EXIV2_ENABLE_XMP )
    IF( NOT EXIV2_ENABLE_LIBXMP )
        SET( LIBEXIV2_SRC ${XMPSRC} ${LIBEXIV2_SRC} )
    ENDIF( NOT EXIV2_ENABLE_LIBXMP )
ENDIF( EXIV2_ENABLE_XMP )

# ******************************************************************************
# Library
ADD_LIBRARY( exiv2 ${STATIC_FLAG} ${LIBEXIV2_SRC} )

SET_TARGET_PROPERTIES( exiv2 PROPERTIES VERSION ${GENERIC_LIB_VERSION}
                                        SOVERSION ${GENERIC_LIB_SOVERSION}
                                        DEFINE_SYMBOL EXV_BUILDING_LIB
                     )

TARGET_LINK_LIBRARIES( exiv2 ${EXPAT_LIBRARIES} )

# IF( MINGW OR UNIX )
	if( EXIV2_ENABLE_LIBXMP )
		TARGET_LINK_LIBRARIES( exiv2 ${XMPLIB} )
	ENDIF( EXIV2_ENABLE_LIBXMP )

	IF( EXIV2_ENABLE_PNG )
		IF( ZLIB_FOUND )
#			IF ( MINGW OR UNIX )
				TARGET_LINK_LIBRARIES( exiv2 ${ZLIB_LIBRARIES} )
#			ELSE ( MINGW OR UNIX )
#				TARGET_LINK_LIBRARIES( exiv2 zlibd )
#			ENDIF( MINGW OR UNIX )
		ENDIF( ZLIB_FOUND )
	ENDIF( EXIV2_ENABLE_PNG )

	IF( EXIV2_ENABLE_NLS )
		TARGET_LINK_LIBRARIES( exiv2 ${LIBINTL_LIBRARIES} )
	ENDIF( EXIV2_ENABLE_NLS )

	IF( ICONV_FOUND )
		TARGET_LINK_LIBRARIES( exiv2 ${ICONV_LIBRARIES} )
	ENDIF( ICONV_FOUND )
# ENDIF( MINGW OR UNIX )

INSTALL( TARGETS exiv2 ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# Binary
ADD_EXECUTABLE( exiv2bin ${EXIV2_SRC} )
SET_TARGET_PROPERTIES( exiv2bin PROPERTIES OUTPUT_NAME exiv2 )
TARGET_LINK_LIBRARIES( exiv2bin exiv2 )
INSTALL( TARGETS exiv2bin ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# Binary
ADD_EXECUTABLE( metacopy ${MC_SRC} )
TARGET_LINK_LIBRARIES( metacopy exiv2 )
INSTALL( TARGETS metacopy ${INSTALL_TARGET_STANDARD_ARGS} )


# ******************************************************************************
# Binary
ADD_EXECUTABLE( taglist ${TAGLIST_SRC} )
TARGET_LINK_LIBRARIES( taglist exiv2 )
INSTALL( TARGETS taglist ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# Headers
INSTALL( FILES ${LIBEXIV2_HDR} DESTINATION include/exiv2 )

# ******************************************************************************
# Man page
INSTALL( FILES exiv2.1 DESTINATION man/man1 )
