MinGW/msys2 Exiv2 v0.27 Release Bundle
--------------------------------------

Structure of the bundle:
------------------------

bin/exiv2                                 exiv2 and sample applications
bin/msys-exiv2lib-0.dll                   exiv2 dll
lib/libexiv2lib.dll.a & libxmp.a          link libraries
include/exiv2/                            include files
share/                                    man pages
contrib/Qt                                Qt code and notes
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
$ g++ -std=c++98 samples/exifprint.cpp -L$PWD/lib -I$PWD/include -lexiv2lib -o exifprint
$ env PATH="$PWD/bin:$PATH" ./exifprint

To install for use by all users
-------------------------------
$ for i in bin include lib ; do mkdir -p /usr/local/$i ; cp -R $i/* /usr/local/$i ; done

To compile and link your own code using installed library and include files
---------------------------------------------------------------------------
$ g++ -std=c++98 samples/exifprint.cpp -I/usr/local/include -L/usr/local/lib -lexiv2lib -o exifprint
$ export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/lib"
$ ./exifprint --version
exiv2=0.27.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
$
