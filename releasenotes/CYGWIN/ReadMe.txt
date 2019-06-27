CYGWIN Exiv2 v0.27.2-RC2 Bundle
-------------------------------

Structure of the bundle:
------------------------

bin/exiv2.exe                                 exiv2 and sample applications
bin/cygexiv2-27.dll                           DLL
lib/libexiv2.dll.a & libexiv2-xmp.a           link libraries
lib/cmake/exiv2                               CMake support/consume files
lib/pkgconfig/exiv2.pc                        pkg-config file
share/man/                                    man pages
share/locale/                                 localisation files
samples/exifprint.cpp                         sample code
logs                                          build and test logs

ReadMe.txt                                    This file
exiv2.png                                     Exiv2 Logo
license.txt                                   GPLv2.0 Software License
README.md                                     Developer Manual
README-CONAN.md                               Developer Manual Appendix
releasenotes.txt                              Late breaking news

To run exiv2 from the bundle
----------------------------
$ cd <bundle>
$ bin/exiv2

To build samples/exiftool.cpp from the bundle
---------------------------------------------
$ g++ -std=gnu++98 samples/exifprint.cpp -L$PWD/lib -I$PWD/include -lexiv2 -o exifprint
$ env PATH="$PWD/bin:$PATH" ./exifprint

To install for use by all users
-------------------------------
$ for i in bin include lib share ; do cp -R $i/* /usr/local/$i ; done

To compile and link your own code using installed library and include files
---------------------------------------------------------------------------
Method 1: Explicitly set include and linking options
$ cd <bundle>
$ g++ -std=gnu++98 samples/exifprint.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -o exifprint
$ export PATH="/usr/local/bin:$PATH"
$ ./exifprint --version

Method 2: Use pkg-config to set include and linking options
$ cd <bundle>
$ export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
$ export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
$ g++ -std=gnu++98 samples/exifprint.cpp -o exifprint $(pkg-config exiv2 --libs --cflags)
$ ./exifprint

