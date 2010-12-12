exiv2\msvc64\ReadMe.txt
--------------------------

Updated: 2010-12-12

Work in progress.
1) Working.
   Build 32 bit and 64bit builds of exiv2 (and libraries exiv2lib,expat,zlib,xmpsdk) in 32bit and 64bits.
2) TODO:
   Test Applications (exifprint etc)
   Test build with VS2008 and VS2010

Robin Mills
http://clanmills.com
email: robin@clanmills.com

####
T A B L E  of  C O N T E N T S

1    Design
1.1  Architecture
1.2  Changing the version of expat/zlib/xmpsdk
1.3  Relationship with msvc build environment

2    Acknowledgment of prior work
2.1  Differences between inherited project and the exiv2 projects

3    Build Instructions

## end of table of contents ##
####


1    Design

expat and zlib do not provide 64bit builds for DevStudio, so it has been necessary to create the build environments for exiv2.  However, we don't include the source code for zlib or expat - only the build environment.  You are expected to install the "vanilla" expat and zlib libraries in a directory at the same level as exiv2.  I personally always build in the directory c:\gnu - however the name/location/spaces of the build directory are all irrelevant, it's only the relative position of expat-2.0.1 and zlib-1.2.3 which matter.  The names expat-2.0.1 and zlib-1.2.3 are fixed.

c:\gnu>dir
 Directory of c:\gnu
2010-12-05  10:05    <DIR>             exiv2
2010-12-02  21:06    <DIR>             expat-2.0.1
2010-12-02  20:58    <DIR>             zlib-1.2.3
c:\gnu>

zlib and expat
msvc64\zlib\zlib.{sln|vcproj}	   DevStudio Solution and Project files
..\..\..\zlib-1.2.3\                   Source code

msvc64\expat\expat.sln expat.vcproj DevStudio Solution and Project files
..\..\..\expat-2.0.1\                  Source code

1.1  Architecture
     There are directories for every component:
     The libraries: zlib, expat, xmpsdk, exiv2lib
     Applications:  exiv2, exifprint etc......
     
     For each component, there are three build directories:
     exiv2lib\build											intermediate results
     exiv2lib\Win32\{Debug|Release|DebugDLL|ReleaseDLL}     32 bit builds
     exiv2lib\x64\{Debug|Release|DebugDLL|ReleaseDLL}       64 bit builds
     
1.2  Changing the version of expat/zlib/xmpsdk
     The directory expat is a copy of expat, zlib is a copy of zlib
     
     As new versions of the libraries become available (eg zlib125) it will be added.
     To build exiv2 with zlib125, the directory zlib125 should be copied to zlib.

1.3  Relationship with msvc build environment
     This environment is similar to msvc (same build engineer).  However there are significant differences:
     1) Support 64bit and 32bit builds
     2) Provides projects to build expat and zlib
     3) Is designed to accomodate new versios of expat and zlib when they become available.
     4) Does not require you to build 'vanilla' expat and zlib projects in advance
     5) Does not support the organize application
     6) Supports DevStudio 2008 and 2010 (no support for 2003 and 2005)
     
     msvc will continue to be supported for 32 bit builds using DevStudio 2003 and 2005,
     however there is no plan to enhance or develop that environment going forward.

2    Acknowledgement of prior work
     This work is based on work by the following people:
     zlib 64 bit build

	    Copyright (C) 2000-2004 Simon-Pierre Cadieux.
	    Copyright (C) 2004 Cosmin Truta.
	    For conditions of distribution and use, see copyright notice in zlib.h.
	
	    And was obtained from:  http://www.achacha.org/vc2008_zlib/

     expat 64 bit build
     http://www.start64.com/index.php?option=com_content&task=view&id=3461&Itemid=114

	 I recognise and respect the work performed by those individuals.
	
2.1  Differences between inherited project and the exiv2 projects
     There is no compatiblity.

3    Build Instructions
     1) Install zlib and expat sources
     2) Open exiv2.sln, batch build, select all, build
     
     If you don't have the x64 compiler available, don't select the 64 bit configurations!

# That's all Folks!
##
