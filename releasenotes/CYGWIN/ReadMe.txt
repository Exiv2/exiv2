CYGWIN Exiv2 v0.27 Release Bundle
---------------------------------

Structure of the bundle:
------------------------

bin/exiv2                                 exiv2 and sample applications
bin/cygexiv2-0.dll                        DLL
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
$ bin/exiv2

To build samples/exiftool.cpp from the bundle
---------------------------------------------
$ g++ -std=gnu++98 samples/exifprint.cpp -L$PWD/lib -I$PWD/include -lexiv2 -o exifprint
$ env PATH="$PWD/bin:$PATH" ./exifprint

To install for use by all users
-------------------------------
$ for i in bin include lib ; do cp -R $i/* /usr/local/$i ; done

To compile and link your own code using installed library and include files
---------------------------------------------------------------------------
$ g++ -std=gnu++98 samples/exifprint.cpp -I/usr/include -I/usr/local/include -L/usr/local/lib -lexiv2 -o exifprint
$ export PATH="/usr/local/bin:$PATH"
$ ./exifprint --version
exiv2=0.27.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
$
