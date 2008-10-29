Notes about msvc build of exiv2
-------------------------------

CodeBase: exiv2 /trunk: 1641
Version : Tuesday 20081025@17:14PDT

Tools
-----

Created With:   VC9    (VS/2008 Std)
Tested  With:   VC7.1, VC8 Pro +SP1, VC9 Std and VC9Express
               (VC7.1 = VS/2003 .Net; VC8 = VS/2005; VC9 = VS/2008)


How to build and test
---------------------

1)  Dependancies
    You need expat-2.0.1 AND zlib which you can download from sourceforge
    expat-2.0.1, zlib and exiv2 should be in the same directory

    I usually use c:\gnu, however the name does not matter
    nor does it have to be a topmost directory

    c:\gnu\expat-2.0.1    <---- vanilla expat
    c:\gnu\zlib           <---- vanilla zlib
    c:\gnu\exiv2          <---- this directory

2)  Convert expat-2.0.1 VC6 work space your version of DevStudio
    Open expat-2.0.1\expat.dsw with DevStudio to convert and save the expat project files
    Say "Yes" to everything.
    
3)  Open the sln file exiv2\msvc\exiv2.sln
    On VS2005 and up, the Wizard will offer to upgrade the project files.
    Once more, say yes to everything.

4)  Build/Batch Build/Select All/Build

	This will build all the necessary libraries and all the test programs.
	At the end of building, you should see the beautiful output:

	========== Build: 98 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========

    The build take 7 minutes on my iMac with a 2.8GHz Duo Core Intel Processor and 4GBytes of Ram. (VS 2005 and VS 2008)

    And about 16 minutes on my elderly 2003 laptop (VS 2003 .Net)
    (DeLL Inspiron 8500 2.2GHz Pentium and 1.25GBytes of Ram)

    Both machines are running XP/Pro SP3.

	+--------------------------------------------+
	| VS/2005 WARNING                            |
	+--------------------------------------------+

	You will receive the nasty message:
	========== Build: 8 succeeded, 90 failed, 0 up-to-date, 0 skipped ==========

	Repeat the Build/Batch Build/Select All/Build
    And this time, you'll get the message:
	========== Build: 90 succeeded, 0 failed, 8 up-to-date, 0 skipped ==========

	And you've built it.
	I've added an appendix at the bottom with more information about this issue.
	I'll fix this for the 0.18final release.
    
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

    C:\gnu\exiv2\msvc> diff.exe   runner.txt new.txt
    --- no differences should be reported --
    
    C:\gnu\exiv2\msvc>
    
    This implies that the build is working and producing the same output as my computer.
    Test runs in seconds.


Where are the libraries and executables?
----------------------------------------

exiv2\msvc\bin\{Debug | DebugDLL | Release | ReleaseDLL }

What's been built
-----------------

Static and dynamic version of exiv2, expat, zlib and the exiv2 sample programs
Debug and Release versions of all targets
Everything is built as MT (MTd for Debug).
The objects are built with the Static C runtime Libraries.

DLL targets link dynamically (exiv2.exe requires libexpat.dll, exiv2.dll and zlib4exiv2.dll)
Static targets are statically linked (exiv2.exe has no dependancies.

I've added a note at the bottom about zlib4exiv2.dll

Version Information
-------------------

It is not recommended to build as and MD target.  If you require this,
please let me know and I'll add support to the build environment.

zlib is required for png support.  It can be disabled in exv_msvc.h

The expat dlls are 'vanilla' libexpat.dll
The zlib  dll is zlib4exiv2.dll and has been built as MT (MTd)
The 'vanilla' zlib (zlib1.dll and zlib1d.dll) are MD libraries.

Known issue
-----------

exiv2.dll is new with 0.18 and should be treated with caution until it matures.
The following test programs are known to crash and is being investigated.

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

Changes to the exiv2 source code since 1641
-------------------------------------------

The following files have been modified:
                      Type
canonmn.cpp              6         
datasets.hpp             2
error.hpp                2
exv_msvc.h               5,3
image.hpp                2
iptc.hpp                 2
jpgimage.hpp             2 
olympusmn.cpp            6
pngchunk.cpp             3
pngimage.cpp             3
preview.cpp              4
tags.hpp                 7
value.hpp                2
xmp.hpp                  2

Types of change
1) Additional Code: exv_msvc.h
2) changes to EXIVAPI dispositions
3) changes to support zlib (which can be disabled from exv_msvc.h)
4) Elimination of some 'unused variable' warnings (which are treated as errors)
5) In the MSVC project files, I've removed the 64 bit compatibility check to elimate warnings.
6) Changes between 1641 and 1643 (nothing to do with me.  Example: canonmn.cpp)
7) Member variable was required public

To be done (hopefully for 0.18final)
-----------------------------------

1 Add expat4exiv2 project (see note below)
2 Make the test program much longer and deeper
3 Investigate the crashes in the DLLs (see Known Issue)
4 Build and test pyexiv2 and exiv2net
- We're not going to distribute this
- I'd like to test that those work correctly with our builds
5 Maybe build and test MD (MDd) libraries

Robin Mills
robin@clanmills.com
http://clanmills.com

VS2005 Users
------------

The build issue is being caused by how DevStudio 2005 manages dependancies.
(VS .Net 2003 and VS 2008 are unaffected by this).

Because we are using a 'vanilla' expat directory, we are converting expat.dsw to a visual studio project.  DevStudio maintains the dependancy information using a GUID which is assigned to every project file.  As you are creating a new expat project, the supplied file exiv2.sln cannot know the dependancy in advance!

The consequence is that project dependancy isn't understood and the projects are not being built in the correct order.  So most of them fail.  On the second build, the dependancy objects are in place (from the failed build) and the build succeeds.

This has the consequence that if you wish to use these projects to develop and debug the beautiful exiv2 source, you should firstly open Project/Dependancy, select 'exiv2lib' and check the dependancy for expat and expat-static (the one which failed the incorrect guid test).  Now it will build correctly and debug fine.

The solution to this is to add an additional project expat4exiv2.  

expat4exiv2 project (to be added)
---------------------------------

I intend to add a new project expat4exiv2 and this will build libexpat using the expat sources.  This will the following consequences:

1) We won't need to run the project conversion wizard on expat
2) exiv2.sln will understand it's dependance on expat4exiv2 and everything will be fine.
3) It'll eliminate the warnings on VC71 during the linking of exiv2.lib (Debug/Release)

These warnings are very ugly and being caused by exiv2lib being dependant on both expat-static.vcproj (lib) and expat (dll).  Visual Studio's linking both and finds no work for the DLL (thank goodness).  The depends1.exe test program confirms the correct linkage.

expat4exiv2 will build libexpat.dll.  Please read the following note about this.

zlib4exiv2.dll
--------------

The 'vanilla' zlib projects build zlib1.dll and zlib1d.dll. (d=debug).  These are built with the MD (and MDd) options.  We are building with MT and MTd.  When we link, we get heart-thumping warnings from the linker about malloc and free.  I don't think it's a good idea to link those.  So, I added a project zlib4exiv2 to build the MT (and MTd) version of zlib.  I resisted the temptation to call those zlib1.dll (and zlib1d.dll) as they are different from the 'vanilla' versions.  While I believe zlib1.dll and zlib4exiv2.dll are probably interchangeable, I decided not to take the risk of breaking another product by redefining a 'standard' dll.  Rule 1 in life 'do no harm'.

An additional complication of the 'vanilla' zlib projects is that some of the targets fail to build.  They have assembler code which is no longer supported!  So if we do a "Build All" we then have to document the failures (which come from zlib, not our code).  Alternatively, we have to document the projects which are to be 'de-selected'.  And there's the possibility that VC71 will try to link unbuildable targets.  A mess?  I'd say so.

When I add the expat4exiv2 project, I will build libexpat.dll as that will be identical with the 'vanilla' libexpat.dll which is built with the MT and MTd options.

Last observation
----------------

I think it's a 'bad thing' to build other people's project files.  Although DevStudio has been designed to enable this, there are consequences.  There are really only two ways to deal with this:

1) Require the external libraries to be pre-built.
- This is what was always done at my company
- I'd never thought about the reasons for this
- The builds were so long, that you wouldn't even consider building anybody elses code!
- 100 million+ lines of code.

2) Provide your own project to build the external code.

We can really only use option 2, because zlib does not provide an MT/MTd build environment.  If we returned to building MD/MDd, expat only provides an MT/MTd build.

-- end --
