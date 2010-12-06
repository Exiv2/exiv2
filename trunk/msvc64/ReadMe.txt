exiv2\msvc64\ReadMe.txt
-----------------------

Updated: 2010-12-06

expat and zlib do not provide 64bit builds for DevStudio, so it has been necessary to create the build environments for exiv2.

This is "work in progress".  So far, I've only ported build environments for expat and zlib.

Robin Mills
http://clanmills.com
email: robin@clanmills.com

####
T A B L E  of  C O N T E N T S

1    Design
2    Acknowledgment of prior work
2.1  Differences between inherited project and the exiv2 projects


## end of table of contents ##
####


1    Design

However, we don't include the source code for zlib or expat - only the build environment.  You are expected to install the "vanilla" expat and zlib libraries in a directory at the same level as exiv2.  I personally always build in the directory c:\gnu - however the name/location/spaces of the build directory are all irrelevant, it's only the relative position of expat-2.0.1 and zlib-1.2.3 which matter.  The names expat-2.0.1 and zlib-1.2.3 are fixed.

c:\gnu>dir
 Directory of c:\gnu
2010-12-05  10:05    <DIR>          exiv2
2010-12-02  21:06    <DIR>          expat-2.0.1
2010-12-02  20:58    <DIR>          zlib-1.2.3
c:\gnu>

zlib and expat
msvc64\zlib123\zlib.{sln|vcproj}	DevStudio Solution and Project files
..\..\..\zlib-1.2.3\                Source code

msvc64\expat201\expat.sln expat.vcproj expat_static.vcproj DevStudio Solution and Project files
..\..\..\expat-2.0.1\               Source code


2    Acknowledgement of prior work
     This work is based on work by the following people:
     zlib 64 bit build

	    Copyright (C) 2000-2004 Simon-Pierre Cadieux.
	    Copyright (C) 2004 Cosmin Truta.
	    For conditions of distribution and use, see copyright notice in zlib.h.
	
	    And was obtained from:  http://www.achacha.org/vc2008_zlib123/
	

     expat 64 bit build
     http://www.start64.com/index.php?option=com_content&task=view&id=3461&Itemid=114

	 I recognise and respect the work performed by those individuals.
	
2.1  Differences between inherited project and the exiv2 projects
     There is no compatiblity.

# That's all Folks!
##
