macOS (Darwin) Exiv2 v0.27 Release Bundle
-----------------------------------------

Structure of the bundle
-----------------------

bin/exiv2                                     exiv2 and sample applications
lib/libexiv2.0.27.1.0.dylib & libexiv2-xmp.a  libraries
lib/pkgconfig/exiv2.pc                        pkg-config file
lib/cmake/exiv2                               CMake support/consume files
include/exiv2/                                include files
share/man/                                    man pages
share/locale/                                 localisation files
samples/exifprint.cpp                         sample code
logs/                                         build and test logs

ReadMe.txt                                    This file
exiv2.png                                     Exiv2 Logo
license.txt                                   GPLv2.0 Software License
README.md                                     Developer Manual
README-CONAN.md                               Developer Manual Appendix
releasenotes.txt                              Late breaking news

To run exiv2 from the bundle
----------------------------
$ cd <bundle>
$ env DYLD_LIBRARY_PATH="$PWD/lib:$DYLD_LIBRARY_PATH" bin/exiv2

To build samples/exiftool.cpp from the bundle
---------------------------------------------
$ g++ -std=c++11 samples/exifprint.cpp -L$PWD/lib -I$PWD/include -lexiv2 -o exifprint
$ env DYLD_LIBRARY_PATH="$PWD/lib:$DYLD_LIBRARY_PATH" ./exifprint

To install for use by all users
-------------------------------
$ for i in bin include lib share ; do sudo ditto -vV $i /usr/local/$i ; done

To compile and link your own code using installed library and include files
---------------------------------------------------------------------------
Method 1: Explicitly set include and linking options
$ g++ -std=c++11 samples/exifprint.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -o exifprint
$ ./exifprint --version
exiv2=0.27.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
$

Method 2: Use pkg-config to set include and linking options
$ cd <bundle>
$ export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
$ export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
$ g++ -std=c++11 samples/exifprint.cpp -o exifprint $(pkg-config exiv2 --libs --cflags)
$ ./exifprint

Method 3: Use the CMake support/consume files
See file: README.md Section: 2.6 "Consuming Exiv2 with CMake"
