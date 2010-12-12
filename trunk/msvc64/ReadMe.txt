exiv2\msvc64\ReadMe.txt
-----------------------

Updated: 2010-12-12 
This is work in progress.
However it is sufficiently robust to be in current use by a commercial licensee of exiv2.

1) Working
   Build 32 bit and 64bit builds of exiv2.exe (and libraries exiv2lib,expat,zlib,xmpsdk) in 32bit and 64bits.
   Builds with VS2005, 2008 and 2010.

2) TODO (in priority order)
   a) Provide support for zlib1.2.5
   b) Use .vsprop files to hold "common" project settings
   c) Build sample/test applications (exifprint etc)
   d) Provide a "runner.bat" build verification test script
   
   Assistance appreciated if you wish to become involved.

Robin Mills
http://clanmills.com
email: robin@clanmills.com

####
T A B L E  o f  C O N T E N T S

1    Build Instructions
1.1  Install zlib and expat sources.
1.2  Open exiv2\msvc64\exiv2.sln
1.3  What is build

2    Design
2.1  Architecture
2.2  Changing the version of expat/zlib/xmpsdk
2.3  Relationship with msvc build environment

3    Acknowledgment of prior work
3.1  Differences between inherited project and the exiv2 projects

## end Table of Contents ##
####


1    Build Instructions
1.1  Install zlib and expat sources.
     I use the directory c:\gnu for this purpose, however the name isn't important.

     c:\gnu>dir
      Directory of c:\gnu
     2010-12-05  10:05    <DIR>             exiv2                <--- this tree
     2010-12-02  21:06    <DIR>             expat-2.0.1          <--- "vanilla" expat 2.0.1 source tree
     2010-12-02  20:58    <DIR>             zlib-1.2.3           <--- "vanilla" zlib 1.2.3 source tree
     c:\gnu>
     
     The URLs from which to obtain zlib and expat are documented in exiv2\msvc\README-MSVC.txt

1.2  Open exiv2\msvc64\exiv2.sln
     batch build, select all, build
     - you will build 5 projects x 2 Platforms (x64|Win32) x 4 Configurations (Debug|Release|DebugDLL|ReleaseDLL)
     = 5x2*4 = 40 builds.
     
     If you don't have the x64 compiler available, don't select the 64 bit configurations!
     
1.3  What is built
     The DLL builds use the DLL version of the C runtime libraries
     The Debug|Release builds use static C runtime libraries
     This is discussed in exiv2\msvc\README-MSVC.txt 

2    Design

expat and zlib do not provide 64bit builds for DevStudio, so it has been necessary to create the build environments for exiv2.  However, we don't include the source code for zlib or expat - only the build environment.  You are expected to install the "vanilla" expat and zlib libraries in a directory at the same level as exiv2.  I personally always build in the directory c:\gnu - however the name/location/spaces of the build directory are all irrelevant, it's only the relative position of expat-2.0.1 and zlib-1.2.3 which matter.  The names expat-2.0.1 and zlib-1.2.3 are fixed.

zlib and expat
msvc64\zlib\zlib.{sln|vcproj}	       DevStudio Solution and Project files
..\..\..\zlib-1.2.3\                   Source code

msvc64\expat\expat.sln expat.vcproj DevStudio Solution and Project files
..\..\..\expat-2.0.1\                  Source code

2.1  Architecture
     There are directories for every component:
     The libraries: zlib, expat, xmpsdk, exiv2lib
     Applications:  exiv2 (exifprint.exe and other sample apps can be added on request)
     
     For each component, there are three build directories:
     exiv2lib\build											intermediate results
     exiv2lib\Win32\{Debug|Release|DebugDLL|ReleaseDLL}     32 bit builds
     exiv2lib\x64\{Debug|Release|DebugDLL|ReleaseDLL}       64 bit builds
     
     Final builds and include directories (for export to "foreign" projects)
     bin\{win32|x84}\Win32\{Debug|Release|DebugDLL|ReleaseDLL} 
     include
     
2.2  Changing the version of expat/zlib/xmpsdk
     At the moment, zlib contains references to zlib-1.2.3 and expat to expat-2.0.1
     It is planned to support zlib 1.2.5 shortly and these notes will be updated at that time.

2.3  Relationship with msvc build environment
     This environment is similar to msvc (same build engineer).
     However there are significant differences:
     1) Support 64bit and 32bit builds
     2) Provides projects to build expat and zlib
     3) Is designed to accomodate new versions of expat and zlib when they become available.
     4) Supports DevStudio 2005, 2008 and 2010 (no support for 2003)
     5) msvc64 does not require you to build 'vanilla' expat and zlib projects in advance
     6) msvc64 does not support the organize application
     7) msvc64 does not build the sample/test applications (such as exfprint.exe)
     8) msvc64 has no test/build verification scripts
     
     msvc will continue to be supported for 32 bit builds using DevStudio 2003 and 2005
     however there is no plan to enhance or develop that environment going forward.

3    Acknowledgement of prior work
     This work is based on work by the following people:
     zlib 64 bit build

	    Copyright (C) 2000-2004 Simon-Pierre Cadieux.
	    Copyright (C) 2004 Cosmin Truta.
	    For conditions of distribution and use, see copyright notice in zlib.h.
	
	    And was obtained from:  http://www.achacha.org/vc2008_zlib/

     expat 64 bit build
     http://www.start64.com/index.php?option=com_content&task=view&id=3461&Itemid=114

	 I recognise and respect the work performed by those individuals.
	
3.1  Differences between inherited project and the exiv2 projects
     There is no compatiblity.

# That's all Folks!
##
