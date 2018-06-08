prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=@CMAKE_INSTALL_FULL_LIBDIR@
includedir=@CMAKE_INSTALL_FULL_INCLUDEDIR@

Name: exiv2
Description: Exif and IPTC metadata library and tools
Version: @PACKAGE_VERSION@
URL: @PACKAGE_URL@
Libs: -L${libdir} -lexiv2 @EXV_LIB_STATIC@
Cflags: -I${includedir}
