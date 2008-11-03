Notes about msvc build of exiv2
-------------------------------

Code base: trunk/1657 
Sunday 20081101 22:30PST

Changes since 20081101 17.00PST
-------------------------------

1 Don't copy src\*_int.hpp to msvc/include/exiv2 directory

2 Test Builds            2008 2005 2003
  Batch Build All        Y    Y    Y

3 Partial builds: select target/config + build (clean build)
  exifprint Debug        Y    Y    Y+W
            DebugDLL     Y    Y    Y
            Release      Y    Y    Y+W
            ReleaseDLL   Y    Y    Y
            
  exiv2     Debug        Y    Y    Y+W
            DebugDLL     Y    Y
            Release      Y    Y    Y+W
            ReleaseDLL   Y    Y    Y

  W = Warnings about multiple-defined externals
      MSVC is linking expat twice.

Changes since 20081030
----------------------

1) zlib directory
   zlib now lives in c:\gnu\zlib-1.2.3
   
2) Removed unnecessary copying of exv_msvc.h to msvc/include directory
  
3) Investigated linker /EDITANDCONTINUE warning
   For example:  exiv2.lib(xmlparse.obj) : warning LNK4075: ignoring '/EDITANDCONTINUE' due to '/INCREMENTAL:NO' specification
   Cannot fix.  It's coming from expat/expat-static Debug builds
   Fix is to modify expat/expat-static.vcproj Compiler/Generaral/Debug Information ZI (not Zi)

4) Investigated what happens when expat isn't built!
   - documented below.

5) Tested more 'partial' target builds
   Tested:
   { exiv2   |exiv2print  }
   { Debug   | Release    | DebugDLL | ReleaseDLL } Manual build (F7)
   { Debug   | Release    | DebugDLL | ReleaseDLL } using Batch/Build
   { VS 2003 | 2005       | 2008 }
   
   2*3*8 = 48 partial builds
   (and of course 3 full builds)
   
   VC71 (VS 2003) in not good at dependancies.  On a 'clean' directory:
   If I select exifprint+Debug Build (or Release or DebugDLL or ReleaseDLL)
   it builds successfully
   If I use Batch Build/ and select exifprint { Debug etc } all 4 targets fail
   to build!
   VC71 is not attempting to build the dependant sub projects.
   
   VC8 and VC9 (VS 2005 and 2008) are working fine.  Both report
   ========== Build: 14 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========   
   
   I think the Batch/Build feature in VC71 simply builds (It omits dependancy
   analysis). I don't intend to do any more work on this on VC71.
                             
6) I did some debugging
   exiv2.exe and exifprint.exe (Debug/DebugDLL on VS/2005)
   
   I was surprised when all .exe's relinked when I changed library code
   (not only the target .exe)
   I'll have to investigate this.  I'll fix this when I investigate the
   crashing DLLs

Priorities for 0.18final
------------------------

1 Remove zlib4exiv2 project           (see note below)
2 Get rid of the linker warnings with VS/2003
3 Investigate the crashes in the DLLs (see Known Issue)
4 Improve the debugging experience
- dont relink every .exe when library code modified
5 Reduce the disk footprint           (use less disk space)
6 Make the test program longer and deeper
7 Build and test pyexiv2 and exiv2net
- We're not going to distribute pyexiv2 or exiv2net
- I'd like to test that those work correctly with our builds
8 Maybe build and test MD (MDd) libraries
- this probably quite a lot of work
- would really like a solid 'customer' request for this

Tools
-----

Created With:   VC9    (VS/2008 Std)
Tested  With:   VC7.1, VC8 Pro +SP1, VC9 Std and VC9Express
               (VC7.1 = VS/2003 .Net; VC8 = VS/2005; VC9 = VS/2008)


How to build and test
---------------------

1)  Dependancies
    You need expat-2.0.1 AND zlib-1.2.3 which you can download from
    http://expat.sourceforge.net/ and http://zlib.net/, respectively.
    expat-2.0.1, zlib and exiv2 should be in the same directory

    I usually use c:\gnu, however the path 'c:\gnu' does not matter
    nor does it have to be a topmost directory.

    c:\gnu\expat-2.0.1    <---- vanilla expat
    c:\gnu\zlib-1.2.3     <---- vanilla zlib
    c:\gnu\exiv2          <---- this directory

2)  Build expat. See there for details. Something like this usually works:
    Convert expat-2.0.1 VC6 work space your version of DevStudio
    Open expat-2.0.1\expat.dsw with DevStudio to convert and save the expat
    project files
    Say "Yes" to everything.

    Build/Batch Build/Select
    { debug | release } { expat | expat-static } (4 targets)
    / Build

    Rattle roll.... less than a minute ...... rattle grrrrrrrr rump.
    ========== Build: 4 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========

3)  Open the sln file exiv2\msvc\exiv2.sln
    On VS2005 and up, the Wizard will offer to upgrade the project files.
    Once more, say yes to everything.

4)  Build/Batch Build/Select All/Build

	This will build all the necessary libraries and all the test programs.
	At the end of building, you should see the beautiful output:

    ========== Build: 94 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
    
    If the build finishes:
    Linking...
    LINK : fatal error LNK1181: cannot open input file '..\exiv2lib\Debug\exiv2.lib'
    Creating browse information file...
    Microsoft Browse Information Maintenance Utility Version 8.00.50727
    Copyright (C) Microsoft Corporation. All rights reserved.
    Build log was saved at "file://c:\gnu\exiv2\msvc\exifprint\Debug\BuildLog.htm"
    exifprint - 1 error(s), 0 warning(s)
    ========== Build: 6 succeeded, 88 failed, 0 up-to-date, 0 skipped ==========    
    
    Then you probably forgot to build expat.  Repeat steps 2 and 4.

    The build takes 6 minutes on my iMac with a 2.8GHz Duo Core Intel Processor
    and 4GBytes of Ram. (VS 2005 and VS 2008)

    And about 16 minutes on my elderly 2003 laptop (VS 2003 .Net)
    (DeLL Inspiron 8500 2.2GHz Pentium and 1.25GBytes of Ram)

    Both machines are running XP/Pro SP3.

	+--------------------------------------------+
	|         DISK SPACE WARNING                 |
	+--------------------------------------------+

    The total build consumes 1.2GBytes of disk space.
    There are 94 targets.
    The pdb (debugging) files consume 400MB.
    The debug exiv2 library alone (exiv2.lib) is 40MB

    A more modest build is to build exiv2 { Debug | Release | DebugDLL 
    | ReleaseDLL } This consumes: 420MB
    
    A minimum build is to build exiv2/Release
    This consumes: 100MB

    I will reduce this by about 10-20% in 0.18final (see appendix)

5)  Test from the Command Line (cmd.exe):
    
    cd exiv2\msvc
    C:\gnu\exiv2\msvc>runner.bat > new.txt
    exiv2.exe: An action must be specified
    exiv2.exe: At least one file is required
    metacopy.exe: Read and write files must be specified
    exiv2.exe: An action must be specified
    exiv2.exe: At least one file is required
    metacopy.exe: Read and write files must be specified
    exiv2.exe: An action must be specified
    exiv2.exe: At least one file is required
    metacopy.exe: Read and write files must be specified
    exiv2.exe: An action must be specified
    exiv2.exe: At least one file is required
    metacopy.exe: Read and write files must be specified
    test.png: No Exif data found in the file
    test.png: No Exif data found in the file
    test.png: No Exif data found in the file
    test.png: No Exif data found in the file

    C:\gnu\exiv2\msvc>diff.exe   runner.txt new.txt
    --- no differences should be reported --
    
Where are the libraries and executables?
----------------------------------------

exiv2\msvc\bin\{Debug | DebugDLL | Release | ReleaseDLL }

What's been built?
------------------

Static and dynamic version of exiv2, expat, zlib and the exiv2 sample programs
Debug and Release versions of all targets
Everything is built as MT (MTd for Debug).
The objects are built with the Static C runtime Libraries.

DLL targets link dynamically
- exiv2.exe requires libexpat.dll, exiv2.dll and zlib4exiv2.dll
Static targets are statically linked
- exiv2.exe has no dependant DLLs.

I've added a note at the bottom about zlib4exiv2.dll

Version Information
-------------------

It is not recommended to build as an MD target.  If you require this,
please let me know and I'll add support to the build environment.

zlib is required for png support.  It can be disabled in exv_msvc.h

The expat dlls are 'vanilla' libexpat.dll
The zlib  dll is zlib4exiv2.dll and has been built as MT (MTd)
The 'vanilla' zlib (zlib1.dll and zlib1d.dll) are MD libraries.

Known issue
-----------

exiv2.dll is new with 0.18 and should be treated with caution until it matures.
The following test programs are known to crash and are being investigated.

bin\DebugDLL\xmpsample.exe
bin\ReleaseDLL\stringto-test.exe
bin\ReleaseDLL\taglist.exe
bin\ReleaseDLL\xmpsample.exe
bin\DebugDLL\exiv2.exe

Other Bugs
----------

Please consult the issue database:
http://dev.robotbattle.com/bugs/my_view_page.php

Support and Questions
---------------------

Information/Documentation  : http://www.exiv2.org/index.html
Discussion Forum           : http://uk.groups.yahoo.com/group/exiv2/
The MSVC build environment : Please contact me directly by email.

Robin Mills
robin@clanmills.com
http://clanmills.com

APPENDIX

zlib4exiv2.dll
--------------

The 'vanilla' zlib projects build zlib1.dll and zlib1d.dll. (d=debug).
These are built with the MD (and MDd) options.  We are building with
MT and MTd.  When we link, we get heart-thumping warnings from the
linker about malloc and free.  I don't think it's a good idea to link
those.  So, I added a project zlib4exiv2 to build the MT (and MTd)
version of zlib.  I resisted the temptation to call those zlib1.dll
(and zlib1d.dll) as they are different from the 'vanilla' versions.
While I believe zlib1.dll and zlib4exiv2.dll are probably
interchangeable, I decided not to take the risk of breaking another
product by redefining a 'standard' dll.  Rule 1 in life 'do no harm'.

I need to do more work in this area.

Reducing the disk demands
-------------------------

At the moment, I build every target into it's own directory and then copy it to the bin directory.

For example:  exiv2/msvc/exiv2.lib --outputs--> debug/exiv2.lib ==copies==> exiv2/msvc/bin/debug/exiv2.lib

I did this for a good reasons at the time.  I think I'll revisit this and build into the bin.
This will save about 100-200MB of disk space on a complete build (10%-20%)

-- end --
