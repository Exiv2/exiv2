Visual Studio 2017 Release DLL Bundle
-------------------------------------

Structure of the bundle:
------------------------

bin/exiv2                                 exiv2 and sample applications
bin/exiv2lib.dll                          dll
lib/exiv2lib.lib & xmp.lib                link libraries
include/exiv2/                            include files
share/                                    man pages
samples/exifprint.cpp                     sample code

ReadMe.txt                                This file
license.txt                               GPLv2.0 Software License
releasenotes.txt                          Late breaking news
README.md                                 User Manual
README-CONAN.md                           User Manual Appendix

+----------------------------------------------------------------------------+
| Caution: Use a Windows unzip utility such as 7z or winzip                  |
| Cygwin unzip utilities can result in incorrect security with bin/exiv2.dll |
+----------------------------------------------------------------------------+

To run exiv2.exe from the bundle:
c:\> cd <bundle>\bin
<bundle>\bin> exiv2

Add the bin to your PATH:
c:\> cd <bundle>\bin
<bundle>\bin>set PATH=%CD%;%PATH%

To compile and link your own code:

+----------------------------------------------------------------------------+
| Caution: You must use the same version of Visual Studio as the build       |
|          You will need to use the "Visual Studio Command Prompt"           |
|          or initialise the DOS environment by calling vcvarsall.bat        |
+----------------------------------------------------------------------------+

c:\> cd <bundle>
<bundle>> cl /EHsc -Iinclude /MD samples\exifprint.cpp /link lib\exiv2lib.lib
<bundle>> exifprint --version
exiv2=0.27.0
...
xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
<bundle>>
