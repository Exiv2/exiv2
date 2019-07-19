Unix Exiv2 v0.27.2-RC3 Bundle (FreeBSD and NetBSD)
--------------------------------------------------

Structure of the bundle:
------------------------

bin/exiv2                                     exiv2 and sample applications
lib/libexiv2.so.0.27.2.3 & libexiv2-xmp.a     libraries
lib/pkgconfig/exiv2.pc                        pkg-config file
lib/cmake/exiv2                               CMake support/consume files
include/exiv2/                                include files
share/man/                                    man pages
share/locale/                                 localisation files
samples/exifprint.cpp                         sample code
logs/                                         build and test logs

ReadMe.txt                                    This file
license.txt                                   GPLv2.0 Software License
releasenotes.txt                              Late breaking news
README.md                                     Developer Manual
README-CONAN.md                               Developer Manual Appendix
exiv2.png                                     Exiv2 Logo

To run exiv2 from the bundle
----------------------------
$ cd <bundle>
$ bin/exiv2

To build samples/exiftool.cpp from the bundle
---------------------------------------------
$ g++ -std=c++98 samples/exifprint.cpp -L$PWD/lib -I$PWD/include -lexiv2 -o exifprint
$ env LD_LIBRARY_PATH="$PWD/lib:$LD_LIBRARY_PATH" ./exifprint

To install for use by all users
-------------------------------
$ for i in bin include lib share ; do sudo mkdir -p /usr/local/$i ; sudo cp -R $i/* /usr/local/$i ; done

To compile and link your own code using installed library and include files
---------------------------------------------------------------------------
Method 1: Explicitly set include and linking options
$ cd <bundle>
$ g++ -std=c++98 samples/exifprint.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -o exifprint
$ export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
$ ./exifprint --version

Method 2: Use pkg-config to set include and linking options
$ cd <bundle>
$ export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
$ export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
$ g++ -std=c++98 samples/exifprint.cpp -o exifprint $(pkg-config exiv2 --libs --cflags)
$ ./exifprint

Method 3: Use the CMake support/consume files
See file: README.md Section: 2.6 "Consuming Exiv2 with CMake"

