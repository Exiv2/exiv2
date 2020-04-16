@RV@ Visual Studio @VS@ Bundle @RC@

Structure of the bundle:
------------------------

bin/exiv2.exe                                 exiv2 and sample applications
bin/exiv2.dll                                 dll
lib/exiv2.lib & exiv2-xmp.lib                 link libraries
lib/cmake/exiv2                               CMake support/consume files
include/exiv2/                                include files
samples/exifprint.cpp                         sample code
logs                                          build/test log

ReadMe.txt                                    This file
license.txt                                   GPLv2.0 Software License
releasenotes.txt                              Late breaking news
README.md                                     Developer Manual
README-CONAN.md                               Developer Manual Appendix
README-SAMPLES.md                             Developer Sample Code Manual
exiv2.png                                     Exiv2 Logo

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

+-----------------------------------------------------------------------------------------------+
| Caution: You must use the same version of Visual Studio as the build                          |
|          You will need to use the "Visual Studio Command Prompt"                              |
|          or initialise the DOS environment by calling vcvars64.bat                            |
| c:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat |
+-----------------------------------------------------------------------------------------------+

c:\> cd <bundle>
<bundle>> cl /EHsc -Iinclude /MD samples\exifprint.cpp /link lib\exiv2.lib
<bundle>> exifprint --version
<bundle>>

Method 3: Use the CMake support/consume files
See file: README.md Section: 2.6 "Consuming Exiv2 with CMake"

More Documentation
------------------

https://www.exiv2.org/manpage.html

The following documents are included in your bundle:

README.md                                     Developer Manual
README-CONAN.md                               Developer Manual Appendix
README-SAMPLES.md                             Developer Sample Code Manual
