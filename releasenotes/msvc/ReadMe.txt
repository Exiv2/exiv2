MSVC 2017 Release DLL Bundle
----------------------------

Structure of the bundle:
------------------------

bin/exiv2                                 exiv2 and sample applications
bin/libexiv2lib.dll and libcurl.dll       dlls
include/exiv2/                            include files
lib/exiv2lib.lib (and xmp.lib)            link libraries
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

+----------------------------------------------------------------------------+
| Caution: Use a Windows unzip utility such as 7z or winzip                  |
| Cygwin unzip utilities can result in incorrect security with bin/exiv2.dll |
+----------------------------------------------------------------------------+

To run exiv2.exe from the bundle:
c:\temp> cd <exiv2-0.27.0.1-msvc>\bin
c:\temp\exiv2-0.27.0.1-msvc\bin> exiv2

Add the bin to your PATH:
c:\temp\exiv2-0.27.0.1-msvc\bin>set PATH=%CD%;%PATH%


To compile and link your own code:

+----------------------------------------------------------------------------+
| Caution: You must use the same version of Visual Studio as the build       |
|          You will need to use the "Visual Studio Command Prompt"           |
|          or initialise the DOS environment by calling vcvarsall.bat        |
+----------------------------------------------------------------------------+

c:\temp> cd exiv2-0.27.0.1-msvc
c:\temp\exiv2-0.27.0.1-msvc> cl /EHsc -Iinclude /MD samples\exifprint.cpp /link lib\exiv2lib.lib
c:\temp\exiv2-0.27.0.1-msvc> exifprint --version
exiv2=0.27.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
c:\temp\exiv2-0.27.0.1-msvc>
