This is to be written for Linux from contrib/buildserver/dailyReadMe.txt

Structure of the bundle:
------------------------

bin/exiv2                                 exiv2 and sample applications
lib/libexiv2.so.0.27.0.1                  shared library
include/exiv2/                            include files
share/                                    man pages
samples/                                  sample code
contrib/Qt                                Qt code and notes
samples/exifprint.cpp                     sample code

ReadMe.txt                                This file
license.txt                               GPLv2.0 Software License
releasenotes.txt
README-CMAKE.md
README.md
README-CONAN.md

To run exiv2 from the bundle
----------------------------
$ cd <bundle>
$ env LD_LIBRARY_PATH="$PWD/lib:$LD_LIBRARY_PATH" bin/exiv2

To build samples/exiftool.cpp from the bundle
---------------------------------------------
$ g++ -std=c++98 samples/exifprint.cpp -L$PWD/lib -I$PWD/include -lexiv2 -o exifprint
$ env LD_LIBRARY_PATH="$PWD/lib:$LD_LIBRARY_PATH" ./exifprint

To install for use by all users
-------------------------------
$ for i in bin lib include ; do sudo cp -R $i /usr/local/$i ; done

To compile and link your own code using installed library and include files
---------------------------------------------------------------------------
$ g++ -std=c++98 samples/exifprint.cpp -I/usr/local/include -L/usr/local/lib -lexiv2 -o exifprint
$ ./exifprint --version
exiv2=0.27.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
$
