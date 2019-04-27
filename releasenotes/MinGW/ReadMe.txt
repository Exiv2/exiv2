MinGW/msys2 Exiv2 v0.27 Release Bundle
--------------------------------------

Structure of the bundle:
------------------------

bin/exiv2.exe                                 exiv2 and sample applications
bin/msys-exiv2-27.dll                         exiv2 dll
lib/libexiv2.dll.a & libexiv2-xmp.a           link libraries
lib/pkgconfig/exiv2.pc                        pkg-config file
lib/cmake/exiv2                               CMake support/consume files
include/exiv2/                                include files
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
$ env PATH="$PWD/bin:$PATH" bin/exiv2

To build samples/exiftool.cpp from the bundle
---------------------------------------------
$ cd <bundle>
$ g++ -std=c++11 samples/exifprint.cpp -Llib -Iinclude -lexiv2 -o exifprint
$ ./exifprint

To install for use by all users
-------------------------------
$ cd <bundle>
$ for i in bin include lib share ; do mkdir -p /usr/local/$i ; cp -R $i/* /usr/local/$i ; done

To compile and link your own code using installed library and include files
---------------------------------------------------------------------------
Method 1: Explicitly set include and linking options
$ cd <bundle>
$ g++ -std=c++11 samples/exifprint.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -o exifprint
$ export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
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
