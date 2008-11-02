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
    You need expat-2.0.1 AND zlib-1.2.3 which you can download from
    http://expat.sourceforge.net/ and http://zlib.net/, respectively.
    expat-2.0.1, zlib and exiv2 should be in the same directory

    I usually use c:\gnu, however the path 'c:\gnu' does not matter
    nor does it have to be a topmost directory.

    c:\gnu\expat-2.0.1    <---- vanilla expat
    c:\gnu\zlib           <---- vanilla zlib
    c:\gnu\exiv2          <---- this directory

2)  Build expat. See there for details. Something like this usually works:
    Convert expat-2.0.1 VC6 work space your version of DevStudio
    Open expat-2.0.1\expat.dsw with DevStudio to convert and save the expat
    project files
    Say "Yes" to everything.

    Build/Batch Build/Select { debug | release } { expat | expat-static }
    (4 targets) / Build

    Rattle roll.... less than a minute ...... rattle grrrrrrrr rump.
	========== Build: 4 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========

3)  Open the sln file exiv2\msvc\exiv2.sln
    On VS2005 and up, the Wizard will offer to upgrade the project files.
    Once more, say yes to everything.

4)  Build/Batch Build/Select All/Build

	This will build all the necessary libraries and all the test programs.
	At the end of building, you should see the beautiful output:

	========== Build: 94 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========

    The build take 6 minutes on my iMac with a 2.8GHz Duo Core Intel Processor
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
    The debug exiv2 library (exiv2.lib) is 40MB

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

What's been built
-----------------

Static and dynamic version of exiv2, expat, zlib and the exiv2 sample programs
Debug and Release versions of all targets
Everything is built as MT (MTd for Debug).
The objects are built with the Static C runtime Libraries.

DLL targets link dynamically (exiv2.exe requires libexpat.dll, exiv2.dll and
zlib4exiv2.dll)
Static targets are statically linked (exiv2.exe has no dependancies.

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

To be done (hopefully for 0.18final)
-----------------------------------

1 Remove zlib4exiv2 project            (see note below)
2 Tidy up a little
- Fix the linker editcontinue warning  (trivial)
- Reduce the disk demands of the build (see note below)
- Do some debugging (check that it debugs, links, builds OK)
- Test more 'partial' targets
- See what happens when expat isn't built!
3 Investigate the crashes in the DLLs (see Known Issue)
4 Make the test program much longer and deeper
5 Build and test pyexiv2 and exiv2net
- We're not going to distribute pyexiv2 or exiv2net
- I'd like to test that those work correctly with our builds
6 Maybe build and test MD (MDd) libraries (probably quite a lot of work)

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

For example:  exiv2/msvc/exiv2.lib -> debug/exiv2.lib  = copy => exiv2/msvc/bin/debug/exiv2.lib

I did this for a good reasons at the time.  I think I'll revisit this and build into the bin.
This will save about 100-200MB of disk space on a complete build (10%-20%)

-- end --
