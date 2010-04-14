# CMake build system for exiv2 library and executables
# Copyright 2008 by Patrick Spendrin <ps_ml@gmx.de>
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
# Private headers which are only needed for the library itself
set( LIBEXIV2_PRIVATE_HDR   cr2image_int.hpp
                            crwimage_int.hpp
                            makernote_int.hpp
                            minoltasonyvalues.hpp
                            orfimage_int.hpp
                            pngchunk_int.hpp
                            rw2image_int.hpp
                            tiffcomposite_int.hpp
                            tifffwd_int.hpp
                            tiffimage_int.hpp
                            tiffvisitor_int.hpp
)
# Add standalone C++ header files to this list
set( LIBEXIV2_HDR   basicio.hpp
                    bmpimage.hpp
                    canonmn.hpp
                    convert.hpp
                    cr2image.hpp
                    crwimage.hpp
                    datasets.hpp
		    easyaccess.hpp
                    error.hpp
                    exif.hpp
                    fujimn.hpp
                    futils.hpp
                    gifimage.hpp
                    image.hpp
                    iptc.hpp
                    jp2image.hpp
                    jpgimage.hpp
                    metadatum.hpp
                    minoltamn.hpp
                    mn.hpp
                    mrwimage.hpp
                    nikonmn.hpp
                    olympusmn.hpp
                    orfimage.hpp
                    panasonicmn.hpp
                    pentaxmn.hpp
		    pgfimage.hpp
                    pngimage.hpp
		    preview.hpp
                    properties.hpp
                    psdimage.hpp
                    rafimage.hpp
                    rcsid.hpp
                    rw2image.hpp
                    sigmamn.hpp
                    sonymn.hpp
                    tags.hpp
                    tgaimage.hpp
                    tiffimage.hpp
                    types.hpp
                    value.hpp
                    version.hpp
                    xmp.hpp
                    xmpsidecar.hpp )

# Add library C++ source files to this list
set( LIBEXIV2_SRC   basicio.cpp
                    bmpimage.cpp
                    canonmn.cpp
                    convert.cpp
                    cr2image.cpp
                    crwedit.cpp
                    crwimage.cpp
                    datasets.cpp
		    easyaccess.cpp
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
                    xmpsidecar.cpp )

# Add source files of simple applications to this list
set( TAGLIST_SRC    taglist.cpp )

# Source files for the Exiv2 application
set( EXIV2_SRC      exiv2.cpp
                    actions.cpp
                    utils.cpp )

# Source files for the metacopy sample application
set( MC_SRC         metacopy.cpp
                    utils.cpp )

if( EXIV2_ENABLE_PNG )
    if( ZLIB_FOUND )
        set( LIBEXIV2_SRC ${LIBEXIV2_SRC}   pngchunk.cpp
                                            pngimage.cpp )
    endif( ZLIB_FOUND )
endif( EXIV2_ENABLE_PNG )

if(NOT HAVE_TIMEGM )
    set( LIBEXIV2_SRC   ${LIBEXIV2_SRC} localtime.c )
    set( EXIV2_SRC      ${EXIV2_SRC}    localtime.c )
endif( NOT HAVE_TIMEGM )

if( MSVC )
    set( MC_SRC         ${MC_SRC}       getopt_win32.c )
    set( EXIV2_SRC      ${EXIV2_SRC}    getopt_win32.c )
    set( LIBEXIV2_SRC   ${LIBEXIV2_SRC} getopt_win32.c )
endif( MSVC )


if( EXIV2_ENABLE_XMP )
    if( NOT EXIV2_ENABLE_LIBXMP )
        set( LIBEXIV2_SRC ${XMPSRC} ${LIBEXIV2_SRC} )
    endif( NOT EXIV2_ENABLE_LIBXMP )
endif( EXIV2_ENABLE_XMP )

# ******************************************************************************
# Library
add_library( exiv2 ${STATIC_FLAG} ${LIBEXIV2_SRC} )
set_target_properties( exiv2 PROPERTIES VERSION ${GENERIC_LIB_VERSION} 
                                        SOVERSION ${GENERIC_LIB_SOVERSION}
                                        DEFINE_SYMBOL EXV_BUILDING_LIB )

if( EXIV2_ENABLE_LIBXMP )
    target_link_libraries( exiv2 ${XMPLIB} )
endif( EXIV2_ENABLE_LIBXMP )
target_link_libraries( exiv2 ${EXPAT_LIBRARIES} )
if( EXIV2_ENABLE_PNG )
    if( ZLIB_FOUND )
        target_link_libraries( exiv2 ${ZLIB_LIBRARIES} )
    endif( ZLIB_FOUND )
endif( EXIV2_ENABLE_PNG )
if( EXIV2_ENABLE_NLS )
    target_link_libraries( exiv2 ${LIBINTL_LIBRARIES} )
endif( EXIV2_ENABLE_NLS )
if( ICONV_TEST )
    target_link_libraries( exiv2 ${ICONV_LIBRARIES} )
    message ( "----------------> ICONV_LIBRARIES : " ${ICONV_LIBRARIES} )
endif( ICONV_TEST )

install( TARGETS exiv2 ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# Binary
add_executable( exiv2bin ${EXIV2_SRC} )
set_target_properties( exiv2bin PROPERTIES OUTPUT_NAME exiv2 )
target_link_libraries( exiv2bin exiv2 )
install( TARGETS exiv2bin ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# Binary
add_executable( metacopy ${MC_SRC} )
target_link_libraries( metacopy exiv2 )
install( TARGETS metacopy ${INSTALL_TARGET_STANDARD_ARGS} )


# ******************************************************************************
# Binary
add_executable( taglist ${TAGLIST_SRC} )
target_link_libraries( taglist exiv2 )
install( TARGETS taglist ${INSTALL_TARGET_STANDARD_ARGS} )

# ******************************************************************************
# Headers
install( FILES ${LIBEXIV2_HDR} DESTINATION include/exiv2 )

# ******************************************************************************
# Man page
install( FILES exiv2.1 DESTINATION man/man1 )
