@RV@ macOS (Darwin) Bundle @RC@

Document                                        Purpose
--------                                        -------
ReadMe.txt                                      This file
README.md                                       Developer Manual
README-CONAN.md                                 Developer Manual Appendix
README-SAMPLES.md                               Developer Sample Code Manual
releasenotes.txt                                Late breaking news
exiv2.png                                       Exiv2 Logo
COPYING                                         GPLv2.0 Software License

Deliverable                                     Location
-----------                                     --------
exiv2 and sample applications                   bin/exiv2
libraries                                       lib/libexiv2.@VM@.@VN@.@VD@@VR@.dylib & libexiv2-xmp.a
pkg-config file                                 lib/pkgconfig/exiv2.pc
CMake support/consume files                     lib/cmake/exiv2
include files                                   include/exiv2/
man pages                                       share/man/
localisation files                              share/locale/
sample code                                     samples/exifprint.cpp
build/test log                                  logs/build.txt

To run exiv2 from the bundle
----------------------------
$ cd <bundle>
$ env DYLD_LIBRARY_PATH="$PWD/lib:$DYLD_LIBRARY_PATH" bin/exiv2

To build samples/exiftool.cpp from the bundle
---------------------------------------------
$ g++ -std=c++98 samples/exifprint.cpp -L$PWD/lib -I$PWD/include -lexiv2 -o exifprint
$ env DYLD_LIBRARY_PATH="$PWD/lib:$DYLD_LIBRARY_PATH" ./exifprint

To install for use by all users
-------------------------------
$ for i in bin include lib share ; do sudo ditto -vV $i /usr/local/$i ; done

To compile and link your own code using installed library and include files
---------------------------------------------------------------------------
Method 1: Explicitly set include and linking options
$ g++ -std=c++98 samples/exifprint.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -o exifprint
$ ./exifprint --version

Method 2: Use pkg-config to set include and linking options
$ cd <bundle>
$ export PKG_CONFIG_PATH="/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH"
$ g++ -std=c++98 samples/exifprint.cpp -o exifprint $(pkg-config exiv2 --libs --cflags)
$ ./exifprint

Method 3: Use the CMake support/consume files
See file: README.md Section: 2.6 "Consuming Exiv2 with CMake"

More Documentation
------------------

Project Website: https://exiv2.org

$ export "MANPATH=/usr/local/share/man:$MANPATH"
$ man exiv2

Robin Mills
robin@clanmills.com
Updated: 2020-04-22
