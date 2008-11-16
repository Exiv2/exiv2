Notes about msvc build of exiv2
-------------------------------

Tools
-----

Created With:   VC9    (VS/2008 Std)
Tested  With:   VC7.1, VC8 Pro +SP1, VC9 Std and VC9Express
               (VC7.1 = VS/2003 .Net; VC8 = VS/2005; VC9 = VS/2008)

How to build and test
---------------------

1)  Dependancies
    You need expat-2.0.1 and zlib-1.2.3 which you can download from
    http://expat.sourceforge.net/ and http://zlib.net/, respectively.
    expat-2.0.1, zlib-1.2.3 and exiv2 should be in the same directory.

    I usually use c:\gnu, however the name 'c:\gnu' does not matter
    nor does it have to be a topmost directory.

    c:\gnu\expat-2.0.1    <---- vanilla expat
    c:\gnu\zlib-1.2.3     <---- vanilla zlib
    c:\gnu\exiv2          <---- exiv2 top level directory

2)  Build expat. See expat documents for details.
    This is what I do:
    a) Open expat-2.0.1\expat.dsw with DevStudio
       This converts the expat-2.0.1 VC6 work space to DevStudio project files
       Say "Yes" to everything.

    b) Build/Batch Build/Select
       { debug | release } { expat | expat-static } (4 targets)
       / Build

    Rattle roll.... less than a minute ...... rattle grrrrrrrr rump.
    ========== Build: 4 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
    
3)  Build zlib.  See zlib documents for details.
    This is what I do:
    a) Open zlib\zlib-1.2.3\projects\visualc6\zlib.dsw with DevStudio
       This converts the zlib-1.2.3 VC6 workspace to DevStudio project files
       Say "Yes" to everything.

    b) Build/Batch Build/Select
       {DLL Debug | DLL Release | LIB Debug | LIB Release } (4 targets)
       Build
      
    Rattle roll.... less than a minute ...... snap, crackle, pop (lots of warnings)
    ========== Build: 4 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========

4)  Open the sln file exiv2\msvc\exiv2.sln
    On VS2005 and up, the Wizard will offer to upgrade the project files.
    Once more, say yes to everything.

5)  Build/Batch Build/Select All/Build

	This will build all the necessary libraries and all the test programs.
	At the end of building, you should see the beautiful output:

    ========== Build: 92 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
    
    The build takes 6 minutes on my iMac with a 2.8GHz Duo Core Intel
    Processor and 4GBytes of Ram. (VS 2005 and VS 2008)

    And about 16 minutes on my elderly 2003 laptop (VS 2003 .Net)
    (DeLL Inspiron 8500 2.2GHz Pentium and 1.25GBytes of Ram)

    Both machines are running XP/Pro SP3.

	+--------------------------------------------+
	|         DISK SPACE WARNING                 |
	+--------------------------------------------+

    The total build consumes 1.0GBytes of disk space.
    There are 100 targets (4 expats, 4 zlibs and 92 exiv2 targets)
    The pdb (debugging) files consume 300MB.
    The debug exiv2 library alone (exiv2.lib) is 35MB

    A more modest build is to build exiv2
    { Debug | Release | DebugDLL | ReleaseDLL }
    This consumes: 400MB

    A minimum build is to build exiv2/Release
    This consumes: 100MB

5)  Test from the Command Line (cmd.exe):

    cd exiv2\msvc
    C:\gnu\exiv2\msvc>runner.bat > new.txt
    exiv2.exe: An action must be specified
    exiv2.exe: At least one file is required
    metacopy.exe: Read and write files must be specified
    bla bla white sheep......

    C:\gnu\exiv2\msvc>diff.exe   runner.txt new.txt
    --- no differences should be reported --
    
    The test takes less than 1 minute and runs 200 different test
    programs.
    
Where are the libraries and executables?
----------------------------------------

exiv2\msvc\bin\{Debug | DebugDLL | Release | ReleaseDLL }

What's been built?
------------------

Static and dynamic version of exiv2, expat, zlib and the exiv2 sample programs
Debug and Release versions of all targets.

The static libraries link with MT (MTd for Debug).
The objects are built to use the Static C runtime Libraries.

DLL targets link dynamically
- exiv2.exe requires libexpat.dll, exiv2.dll and zlib1.dll (zlib1d.dll for debug)
- the objects use the dynamic c runtime dlls (MSVCR70.dll and friends)

Linking Applications with the exiv2 libraries
---------------------------------------------

The following 'flavors' have been built:

a) DLLs + MD build and work

DebugDLL | ReleaseDLL = MD{d} : link exiv2.lib, xmpsdk.lib, libexpat.lib
                                and zlib1{d}.lib
          Runtime DLLS        : exiv2.dll, libexpat.dll, zlib1{d}.dll
                                and MSVC{R|P}{_0D.dll)
                                _ = 7 for VS2003, 8 for VS2005 and 9 for VS2008
          The MSVC*.dll's are on most machines and a free download from Microsoft
                        
b) Static + MT build and work

Debug | Release      = MT{d} : link exiv2.lib, libexpatMT.lib, zlib{d}.lib
                               xmpsdk.lib 
          Runtime DLLs       : none

c) Static + MD will work (but not built)

During the development of the build environment, this was working.
However it was dropped from the release because expat-2.0.1 does not
support an MD build.

When you choose to link your application with the exiv2 library, can
choose any of the 3 configurations.  Please use Google to find
additional advice about which flavor is best for your needs.

d) DLLs + MT WILL NOT FUNCTION even if you build it

Do not build DLLs + MT.  Exiv2.dll exports STL template objects which
will crash when linked with the MT run-time.  This is because
Exiv2.dll is linked with it's own (static) c-runtime library.
Exiv2.exe is linked with a different c-runtime library.  The two
cannot co-exist.

This is not a bug.  There is no workaround.

Bugs
----

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

The following topics are documented for additional information about
the status of the build.

A) Build warning messages
B) Partial Build errors
C) Debugging exiv2 library code

A) Build warning messages
-------------------------

The following warning and message are part of the build:

1) exiv2.lib(xmlparse.obj) : warning LNK4075: ignoring
   '/EDITANDCONTINUE' due to '/INCREMENTAL:NO' specification To
   eliminate this message, you will have to modify project settings in
   expat-2.0.1 and rebuild expat.

2) zlib.lib(zutil.obj) : warning LNK4217: locally defined symbol
   _malloc imported in function _zcalloc I believe this is coming from
   zlib which uses the local (MD) c-runtime library.  It's harmless
   (although frightening)

B) Partial Build errors
-----------------------

Andreas assures me that if you cannot build some targets, e.g.,
exifprint/ReleaseDLL on a clean VC71 machine.  I haven't been able to
reproduce this.  Please drop me an email if you find any of those.
I'm cross-eyed building different flavors of this on different version
of DevStudio.

C) Debugging exiv2 library code
-------------------------------

You may wish to debug the exiv2 library code using one of the test
utilities.  Examples: exiv2.exe and exifprint.exe.  You'll find this a
little frustrating because the exiv2.sln file contains all the test
programs.  When you change the library code, DevStudio will relink all
20 test programs.  To avoid this frustration, unload the project
files which you are not using.  For example, if you are debugging
exiv2.exe, you only require the following projects to be loaded:

xmpsdk
exiv2lib
exiv2

(when exiting DevStudio, say "No" when it asks you if you wish to save
changes to exiv2.sln)
  
-- end --
