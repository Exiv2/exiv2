MinGW/msys2 Exiv2 v0.27 Release Bundle
--------------------------------------

Structure of the bundle:
------------------------

bin/exiv2                                 exiv2 and sample applications
bin/msys-exiv2-0.dll                      exiv2 dll
lib/libexiv2.dll.a & libxmp.a             link libraries
lib/pkgconfig/exiv2.pc                    pkg-config file
include/exiv2/                            include files
share/                                    man pages
samples/exifprint.cpp                     sample code

ReadMe.txt                                This file
license.txt                               GPLv2.0 Software License
releasenotes.txt                          Late breaking news
README.md                                 User Manual
README-CONAN.md                           User Manual Appendix

To run exiv2 from the bundle
----------------------------
$ cd <bundle>
$ env PATH="$PWD/bin:$PATH" bin/exiv2

To build samples/exiftool.cpp from the bundle
---------------------------------------------
$ cd <bundle>
$ g++ -std=c++98 samples/exifprint.cpp -Llib -Iinclude -lexiv2 -o exifprint
$ ./exifprint

To install for use by all users
-------------------------------
$ cd <bundle>
$ for i in bin include lib ; do mkdir -p /usr/local/$i ; cp -R $i/* /usr/local/$i ; done

To compile and link your own code using installed library and include files
---------------------------------------------------------------------------
$ cd <bundle>
$ g++ -std=c++98 samples/exifprint.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -o exifprint
$ export LD_LIBRARY_PATH="/usr/local/lib:$LD_LIBRARY_PATH"
$ ./exifprint --version
exiv2=0.27.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
$
