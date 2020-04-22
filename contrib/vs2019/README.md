# exiv2-x86_x64
Visual Studio project files to compile exiv2 on Windows (x86/x64/Release/Debug)

## Background ##
One of the main building blocks for a personal product that I was
developing was exiv2. When I started this project, support for
building on Windows using Visual Studio was quite good and in
particular, [Robin Mills](https://github.com/clanmills), the then
maintainer, was excellent in helping resolve questions.

However, as time progressed, the maintainers changed and their primary
focus turned to Linux and Windows compilation was an afterthought
accomplished by the swiss knife CMake. In reality, CMake, produces
horrible Visual Studio solution/projects that just creates wrappers on
cmake commands, tries to enforce the Linux custom of all dependencies
installed in a single location like /usr/local as well as not
retaining the flexibility of targeting separate x86/x64/Release/Debug
builds while using the same solution/projects.

The native solution/project files were removed from the exiv2
repository and not even added to a contrib folder for others to use, a
restriction that I found particularly galling. The fact that the main
audience didn't squeal much at these steps is not the answer as they
may not care about compiling on Windows using VS. This was a
retrograde step, but they believed that they were going forward.

One can never predict what the world wants. If one is not open to
possibilities and instead, claims that my way is the highway, the
utility of the product is diminished. This project is to help people
who would like to build exiv2 using Visual Studio on Windows, to do
so.

## Building Philosophy ##
Unlike the linux-style philosophy of installing everything in and then,
linking from, /usr/local-type folders, typically source trees on
Windows are not organized that way. At least, I don't.

I prefer to git clone from the target repository onto my workspace. I
also prefer to get the latest git versions of exiv2's dependencies as
well. This is an important distinction. The usual philosophy would
install released packages (say into C:\Program Files\...) and the
compiled package to link against them.

The advantage with my approach is that I have a transparent solution
with all the dependencies visible explicitly instead of needing to be
added in areas like Additional Libraries. The debugger is able to step
into any area of the code, including the dependencies easily instead
of worrying whether the dependency is release/debug compiled and is
consistent with all the choices that I make for the application (like
static/runtime CRT or x86/x64 or Release/Debug).

This involves creating one solution and including in it, multiple
projects (its own as well as those of the dependencies) and linking
the dependencies through references instead of explicit identification
through path/folder names.

For e.g. exiv2 will be git-cloned into E:\Projects\exiv2. libexpat (a
dependency that is required for compiling exiv2) will be git-cloned to
E:\Projects\libexpat. The exiv2 solution will have the exiv2 project
(it has many more, but mentioning just one for simplifying the
exposition) and the expat-static project (from libexpat). The
expat-static project is added as a reference to the exiv2
project. This make Visual Studio generate the correct dependency
heirarchy and compile correctly.

# Folder Structure #

Because these are project files that are pre-generated, they expect
exiv2 and its dependencies to be placed in a certain hierarchy.

Each of the dependencies will have to be placed at the same level as
the exiv2 source tree.

For e.g.

C:\Sources\exiv2
C:\Sources\zlib
C:\Sources\openssl
C:\Sources\libssh2

and so on. The requirement that it be at the same level as the exiv2
source tree is only mandated in a relative sense. That is, you could
have your exiv2 sources in C:\Sources or E:\Projects or
C:\Users\Sridhar\Documents\VS\exiv2. There are problems with spaces in
directory names, because there are some commands executed through
utilities like perl which may not handle them well. Please don't use
spaces.

## VS solution/projects for dependencies ##

Just like the solution/project file to compile exiv2 in VS (the one
that you are reading about now), most dependencies also have similar
VS solution/project files, because their VS-build infrastructure is
deficient. Specific instructions on fetching these project files
separately and copying them to the source tree is given below for each
dependency. I fetch these *-x86_x64 repositories into a separate
folder from these other source folders. For e.g., I keep these in
E:\Projects\github. Whereas the sources are in E:\Projects.

Theoretically, just fetching these dependency sources, placing them in
the correct folder hierarchies, fetching their associated VS project
files and placing them in the source tree in specific folders
(detailed below) should be enough to compile exiv2 directly. exiv2
links against static libraries of these dependencies. However, these
dependencies are full-fledged distributions that contain, apart from
those libraries that exiv2 needs, other executables and tests. These
can be compiled, optionally, if one needs to test the integrity of
those projects.

## Dependency Order ##

Some of these dependencies depend on other dependencies. Thus, it is
best if they are executed in the order shown below.

# Dependencies #
  * [Strawberry Perl, used v5.30.1.1 x64](strawberryperl.com)
  * [NASM, used 2.14.02 x64](https://www.nasm.us/)
  * [Visual Studio, used Community 2019 edition](https://visualstudio.microsoft.com/)
  * [zlib, tested w/ v1.2.11](https://github.com/madler/zlib.git)
    * Optionally, switch the git tree to the 1.2.11 branch.
    * Use my [zlib-x86_x64](https://github.com/sridharb1/zlib-x86_x64)
      to compile on Windows using Visual Studio
    * Copy the contents of this folder into the contrib folder
      of the zlib repository. Thus, the contrib/vc14 folder of
      zlib-x86_x64 should overwrite the same in zlib.
    * Optionally, use zlibvc.sln to compile.
  * [libexpat, tested w/ v2.2.9](https://github.com/libexpat/libexpat)
    * Optionally, switch the git tree to the R_2_2_9 branch.
    * Use my
      [libexpat-x86_x64](https://github.com/sridharb1/libexpat-x86_x64)
      to compile on Windows using Visual Studio 
    * Copy the build folder of libexpat-x86_x64 into the libexpat
      source tree.
    * Optionally, use expat.sln to compile.
  * [OpenSSL, tested w/ v1.1.1g-DEV](https://github.com/openssl/openssl)
    * Optionally, switch the git tree to the OpenSSL_1_1_1-stable branch.
    * Use my
      [openssl-x86_x64](https://github.com/sridharb1/openssl-x86_x64)
      to compile on Windows using Visual Studio
    * Copy the build folder of openssl-x86_x64 into the openssl tree.
    * Optionally, use openssl1_1.sln to compile.
  * [libssh, tested w/ v0.9.3](https://git.libssh.org/projects/libssh.git/)
    * Optionally, switch the git tree to the stable-0.9 branch.
    * Use my
      [libssh-x86_x64](https://github.com/sridharb1/libssh-x86_x64.git)
      to compile on Windows using Visual Studio
    * Copy the build folder of libssh-x86_x64 into the libssh source tree.
    * Optionally, use libssh.sln to compile.
  * [libssh2, tested w/ v1.9.0](https://github.com/libssh2/libssh2.git)
    * Optionally, switch the git tree to the libssh2-1.9.0 branch.
    * Use my
      [libssh2-x86_x64](https://github.com/sridharb1/libssh2-x86_x64.git)
      to compile on Windows using Visual Studio
    * Copy the build folder of libssh2-x86_x64 into the libssh2 source tree.
    * Optionally, use libssh2.sln to compile.
  * [brotli, tested w/ v1.0.7+](https://github.com/google/brotli)
    * Use my
      [brotli-x86_x64](https://github.com/sridharb1/brotli-x86_x64.git)
      to compile on Windows using Visual Studio
    * Copy the build_folder folder of brotli-x86_x64 into the brotli source tree.
    * Optionally, use brotli.sln to compile.
  * [curl, tested w/ v7.69.1](https://github.com/curl/curl.git)
    * Optionally, switch the git tree to the 7_69_1 branch.
    * Use my
      [curl-x86_x64](https://github.com/sridharb1/curl-x86_x64.git)
      to compile on Windows using Visual Studio
    * Copy the contents of this folder into the **projects/Windows/VC15**
      folder of the curl source tree. *Note: instructions different
      from other projects*
    * Optionally, use curl_all.sln to compile.
  * [googletest, tested w/ v1.10.x](https://github.com/google/googletest.git)[^8]
    * Optionally, switch the git tree to the v1.10.x branch.
    * Use my
      [googletest-x86_x64](https://github.com/sridharb1/googletest-x86_x64.git)
      to compile on Windows using Visual Studio
    * Copy the build folder of googletest-x86_x64 into the googletest source tree.
    * Optionally, use googletest-distribution.sln to compile.
  * [libintl (aka gettext), tested with v0.20.1](https://git.savannah.gnu.org/git/gettext.git)
    * When you clone gettext, you might also clone a submodule called
      gnulib. This is not necessary. You can turn off the recursive
      flag while cloning.
    * Optionally, switch the git tree to the 0.20.1 branch.
    * Use my
      [gettext-x86_x64](https://github.com/sridharb1/gettext-x86_x64)
      to compile on Windows using Visual Studio
    * Copy the build folder into the root folder of gettext source tree.
    * Optionally, use gettext.sln to compile.
  * [libiconv, tested w/ v1.16](https://github.com/sridharb1/libiconv-x86_x64)
    * This dependency is a little different from the others in the
      sense that in the others, you fetch the source and the VS
      project files separately. In this case, this repository provides
      both the source and the VS project files.
    * Optionally, use libiconv.sln to compile.

# Sources #
  * [Exiv2, tested w/ v0.27.2](https://github.com/Exiv2/exiv2)
  * In v0.27.3, my solution/project files can be found in contrib/vs2019/solution
  * For other versions, use my
    [exiv2-x86_x64](https://github.com/sridharb1/exiv2-x86_x64) to
    compile on Windows 
  * It may, in particular, not work for the HEAD or 0.28+ branches as
    there have been incompatible changes made, that have not been
    incorporated in these project files.
  * Place the contents of exiv2-x86_x64 in a folder called
    contrib/vs2019/solution in the exiv2 repository and build using
    the provided solution. Please note that the dependencies listed
    above are needed. 
  * `exiv2 -vV` (output of generated exiv2.exe on my machine for reference)

  ``` shell
  exiv2 0.27.2
  
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public
  License along with this program; if not, write to the Free
  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301 USA
  exiv2=0.27.2
  platform=windows
  compiler=MSVC
  bits=64
  dll=0
  debug=0
  cplusplus=199711
  version=13.25 (2017/x64)
  date=Apr  6 2020
  time=11:28:24
  processpath=E:\My Projects\exiv2\contrib\vs2019\solution\src\x64\Release
  localedir=/../share/locale
  package_name=exiv2
  curlprotocols=dict file ftp ftps gopher http https imap imaps ldap ldaps pop3 pop3s rtsp scp sftp smb smbs smtp smtps telnet tftp
  curl=1
  executable=E:\Projects\exiv2\contrib\vs2019\solution\src\x64\Release\exiv2.exe
  library=C:\WINDOWS\SYSTEM32\ntdll.dll
  library=C:\Program Files\AVAST Software\Avast\aswhook.dll
  library=C:\WINDOWS\System32\KERNEL32.DLL
  library=C:\WINDOWS\System32\KERNELBASE.dll
  library=C:\WINDOWS\System32\ucrtbase.dll
  library=C:\WINDOWS\System32\PSAPI.DLL
  library=C:\WINDOWS\System32\WS2_32.dll
  library=C:\WINDOWS\System32\RPCRT4.dll
  library=C:\WINDOWS\System32\SHELL32.dll
  library=C:\WINDOWS\System32\cfgmgr32.dll
  library=C:\WINDOWS\System32\shcore.dll
  library=C:\WINDOWS\System32\msvcrt.dll
  library=C:\WINDOWS\System32\combase.dll
  library=C:\WINDOWS\System32\bcryptPrimitives.dll
  library=C:\WINDOWS\System32\windows.storage.dll
  library=C:\WINDOWS\System32\msvcp_win.dll
  library=C:\WINDOWS\System32\sechost.dll
  library=C:\WINDOWS\System32\advapi32.dll
  library=C:\WINDOWS\System32\profapi.dll
  library=C:\WINDOWS\System32\powrprof.dll
  library=C:\WINDOWS\System32\UMPDC.dll
  library=C:\WINDOWS\System32\shlwapi.dll
  library=C:\WINDOWS\System32\GDI32.dll
  library=C:\WINDOWS\System32\win32u.dll
  have_strerror_r=0
  have_gmtime_r=0
  have_inttypes=0
  have_libintl=0
  have_lensdata=1
  have_iconv=1
  have_memory=1
  have_lstat=0
  have_regex=0
  have_regex_h=0
  have_stdbool=1
  have_stdint=1
  have_stdlib=0
  have_strlib=0
  have_strerror_r=0
  have_strings_h=0
  have_mmap=0
  have_munmap=0
  have_sys_stat=1
  have_unistd_h=0
  have_sys_mman=0
  have_libz=1
  have_xmptoolkit=1
  adobe_xmpsdk=0
  have_bool=0
  have_strings=0
  have_sys_types=1
  have_unistd=0
  have_unicode_path=1
  enable_video=1
  enable_webready=1
  enable_nls=1
  use_curl=1
  use_ssh=1
  config_path=C:\Users\Sridhar\exiv2.ini
  xmlns=DICOM:http://ns.adobe.com/DICOM/
  xmlns=GPano:http://ns.google.com/photos/1.0/panorama/
  xmlns=Iptc4xmpCore:http://iptc.org/std/Iptc4xmpCore/1.0/xmlns/
  xmlns=Iptc4xmpExt:http://iptc.org/std/Iptc4xmpExt/2008-02-29/
  xmlns=MP:http://ns.microsoft.com/photo/1.2/
  xmlns=MPRI:http://ns.microsoft.com/photo/1.2/t/RegionInfo#
  xmlns=MPReg:http://ns.microsoft.com/photo/1.2/t/Region#
  xmlns=MicrosoftPhoto:http://ns.microsoft.com/photo/1.0/
  xmlns=acdsee:http://ns.acdsee.com/iptc/1.0/
  xmlns=album:http://ns.adobe.com/album/1.0/
  xmlns=asf:http://ns.adobe.com/asf/1.0/
  xmlns=audio:http://www.audio/
  xmlns=aux:http://ns.adobe.com/exif/1.0/aux/
  xmlns=bmsp:http://ns.adobe.com/StockPhoto/1.0/
  xmlns=creatorAtom:http://ns.adobe.com/creatorAtom/1.0/
  xmlns=crs:http://ns.adobe.com/camera-raw-settings/1.0/
  xmlns=crss:http://ns.adobe.com/camera-raw-saved-settings/1.0/
  xmlns=dc:http://purl.org/dc/elements/1.1/
  xmlns=dcterms:http://purl.org/dc/terms/
  xmlns=digiKam:http://www.digikam.org/ns/1.0/
  xmlns=dwc:http://rs.tdwg.org/dwc/index.htm
  xmlns=exif:http://ns.adobe.com/exif/1.0/
  xmlns=exifEX:http://cipa.jp/exif/1.0/
  xmlns=expressionmedia:http://ns.microsoft.com/expressionmedia/1.0/
  xmlns=iX:http://ns.adobe.com/iX/1.0/
  xmlns=jp2k:http://ns.adobe.com/jp2k/1.0/
  xmlns=jpeg:http://ns.adobe.com/jpeg/1.0/
  xmlns=kipi:http://www.digikam.org/ns/kipi/1.0/
  xmlns=lr:http://ns.adobe.com/lightroom/1.0/
  xmlns=mediapro:http://ns.iview-multimedia.com/mediapro/1.0/
  xmlns=mwg-kw:http://www.metadataworkinggroup.com/schemas/keywords/
  xmlns=mwg-rs:http://www.metadataworkinggroup.com/schemas/regions/
  xmlns=pdf:http://ns.adobe.com/pdf/1.3/
  xmlns=pdfaExtension:http://www.aiim.org/pdfa/ns/extension/
  xmlns=pdfaField:http://www.aiim.org/pdfa/ns/field#
  xmlns=pdfaProperty:http://www.aiim.org/pdfa/ns/property#
  xmlns=pdfaSchema:http://www.aiim.org/pdfa/ns/schema#
  xmlns=pdfaType:http://www.aiim.org/pdfa/ns/type#
  xmlns=pdfaid:http://www.aiim.org/pdfa/ns/id/
  xmlns=pdfx:http://ns.adobe.com/pdfx/1.3/
  xmlns=pdfxid:http://www.npes.org/pdfx/ns/id/
  xmlns=photoshop:http://ns.adobe.com/photoshop/1.0/
  xmlns=plus:http://ns.useplus.org/ldf/xmp/1.0/
  xmlns=png:http://ns.adobe.com/png/1.0/
  xmlns=rdf:http://www.w3.org/1999/02/22-rdf-syntax-ns#
  xmlns=stArea:http://ns.adobe.com/xmp/sType/Area#
  xmlns=stDim:http://ns.adobe.com/xap/1.0/sType/Dimensions#
  xmlns=stEvt:http://ns.adobe.com/xap/1.0/sType/ResourceEvent#
  xmlns=stFnt:http://ns.adobe.com/xap/1.0/sType/Font#
  xmlns=stJob:http://ns.adobe.com/xap/1.0/sType/Job#
  xmlns=stMfs:http://ns.adobe.com/xap/1.0/sType/ManifestItem#
  xmlns=stRef:http://ns.adobe.com/xap/1.0/sType/ResourceRef#
  xmlns=stVer:http://ns.adobe.com/xap/1.0/sType/Version#
  xmlns=tiff:http://ns.adobe.com/tiff/1.0/
  xmlns=video:http://www.video/
  xmlns=wav:http://ns.adobe.com/xmp/wav/1.0/
  xmlns=xml:http://www.w3.org/XML/1998/namespace
  xmlns=xmp:http://ns.adobe.com/xap/1.0/
  xmlns=xmpBJ:http://ns.adobe.com/xap/1.0/bj/
  xmlns=xmpDM:http://ns.adobe.com/xmp/1.0/DynamicMedia/
  xmlns=xmpG:http://ns.adobe.com/xap/1.0/g/
  xmlns=xmpGImg:http://ns.adobe.com/xap/1.0/g/img/
  xmlns=xmpMM:http://ns.adobe.com/xap/1.0/mm/
  xmlns=xmpNote:http://ns.adobe.com/xmp/note/
  xmlns=xmpRights:http://ns.adobe.com/xap/1.0/rights/
  xmlns=xmpT:http://ns.adobe.com/xap/1.0/t/
  xmlns=xmpTPg:http://ns.adobe.com/xap/1.0/t/pg/
  xmlns=xmpidq:http://ns.adobe.com/xmp/Identifier/qual/1.0/
  ```
