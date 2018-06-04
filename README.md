| Travis        | AppVeyor      | GitLab|
|:-------------:|:-------------:|:-----:|
| [![Build Status](https://travis-ci.org/Exiv2/exiv2.svg?branch=master)](https://travis-ci.org/Exiv2/exiv2) | [![Build status](https://ci.appveyor.com/api/projects/status/d6vxf2n0cp3v88al/branch/master?svg=true)](https://ci.appveyor.com/project/piponazo/exiv2-wutfp/branch/master) | [![pipeline status](https://gitlab.com/D4N/exiv2/badges/master/pipeline.svg)](https://gitlab.com/D4N/exiv2/commits/master) |

# Building Exiv2 Library and Command-line tools

Welcome to Exiv2, a C++ library and a command line utility to read and
write Exif, IPTC and XMP image metadata. The homepage of Exiv2 is: [http://exiv2.org](http://exiv2.org)

<pre><code>
    @@@Luis@@@@@b                    ;mm                        /#####Dan###\
    j@@@#Robin",                     ;MM                      /@@@Michal@@@@Q
     @@@#       \                     ##                     @@@b     |@@@b
     @@@#          .;;;;,     ,;;;, ,;;;;  ,;;;p      .;;;   7@@      |@@#
     @@@#           j@@@@,   ]@@#/  '@@@#  j@@@#      ]@@^           ;@@@"
     @@@Andreas@C     "@@@p @@@"     @@@b   j@@@p     @@b           @@@#/
     @@@#^7"7%#\       ^@@@@@#~      @@@b    1@@@    {@#          s@@@#
     @@@#                Henri       @@@b     @@@Q  ]@#         ;@@@#/
     @@@#              ,@@##@@m      @@@b      @@@p @@C        #@@#C
     @@@#       ,/    s@@#   @@@@    @@@b       @@@@@@       @@@@@@@K
    ]@@Elizabeth@\   /@@@\    \@@@Q  @@@Q       %@@@#      /@@@Alison@@#
    /http://www.exiv2.org/\    C++  Metadata   Library    /@Exiv2@@@v0.27\
</code></pre>

<name id="TOC"></a>
----
### T A B L E &nbsp;&nbsp;&nbsp; O&nbsp;&nbsp;F &nbsp;&nbsp;&nbsp; C O N T E N T S

1. [Building and Installing Exiv2](#1)
    1. [Build and install dependent libraries](#1-1)
    2. [Using CMake](#1-2)
    3. [Exiv2 Build Options](#1-3)
    4. [Build, test and install on *ix* Platforms](#1-4)
    5. [Building with Visual Studio](#1-5)
    6. [Building Exiv2 using conan](#1-6)

2. [Linking your own code to the Exiv2 library] (#2)
3. [Trouble Shooting and Support] (#3)
4. [Building Exiv2 Documentation] (#4)
5. [License] (#5)

<name id="1"></a>
----
## 1) Building and Installing

You build Exiv2 using CMake with the compiler tool-chain on your platform.
Exiv2 supports Linux, MacOS-X, Cygwin and Visual Studio (2015 and 2017).

CMake is freely available from [http://www.cmake.org/](http://www.cmake.org/).

<name id="1-1"></a>
### 1.1) Build and install dependent libraries

| Library | Available | Purpose |
|:--------|:----------|:--------|
| _**expat**_       | http://expat.sourceforge.net/ | XML Support (required by XMP) |
| _**zlib**_        |  http://zlib.net/ | PNG Support |
| _**gettext**_ \*  |  http://www.gnu.org/software/gettext/ | Natural Language Support |
| _**libiconv**_ \* | http://www.gnu.org/software/libiconv/ | Character set conversions |

\* On many Linux distros, gettext and libiconv are included in the default libraries, or within libc and can be used without special installation.

To install libraries, download the source bundle and build with the usual:

```bash
$ cd <expatdir>
$ ./configure
$ make
$ sudo make install
```

If you encounter difficulties when building a dependent library, consult the documentation included with the source code of the library.

An alternative way to use the cross-platform package manage conan to install the  dependent libraries is to use conan (except Cygwin).  See [README-CONAN](README-CONAN.md)

<name id="1-2"></a>
### 1.2) Using CMake
CMake has many options and the following are very useful with Exiv2 :

<name id="install-prefix"></a>

<table>
<tr><th>cmake option&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </th><th> Meaning </th><th> More </th></tr>
<tr><td>&#x2011;DCMAKE_INSTALL_PREFIX=</td><td colspan="2""> Installation tree for the build.  Default: <i>/usr/local/</i></td></tr>
<tr><td>&#x2011;DCMAKE_BUILD_TYPE=     <td colspan=2> Type of build. Choices: </td></tr>
<tr><td></td><td>                         debugfull     </td><td> Include all debug information. </td></tr>
<tr><td></td><td>                         debug</td><td></td></tr>
<tr><td></td><td>                         profile</td><td></td></tr>
<tr><td></td><td>                         relwithdebinfo </td><td>Default: use gcc -O2 -g options.</td></tr>
<tr><td></td><td>                         release        </td><td>generate stripped and optimized bin files. For packaging.</td></tr>
<tr><td>&#x2011;DBUILD_SHARED_LIBS=ON <td> Build DLL </td><td>OFF for static library.</td></tr>
<tr><td>&#x2011;DCMAKE_CXX_FLAGS="xxxxx"  <td colspan=2> Pass specific compiler flags or definitions to the
compiler.<br>Example: &#x2011;DCMAKE_CXX_FLAGS="-DDEBUG" to compile debug blocks of code. </td></tr>
</table>

<name id="1-3"></a>
### 1.3) Exiv2 Build Options

Exiv2 has many build options:

```bash
$ cd <exiv2dir>
$ grep ^option CMakeLists.txt
option( BUILD_SHARED_LIBS             "Build exiv2lib as a shared library"                    ON  )
option( EXIV2_ENABLE_XMP              "Build with XMP metadata support"                       ON  )
option( EXIV2_ENABLE_EXTERNAL_XMP     "Use external version of XMP"                           OFF )
option( EXIV2_ENABLE_PNG              "Build with png support (requires libz)"                ON  )
option( EXIV2_ENABLE_NLS              "Build native language support (requires gettext)"      ON  )
option( EXIV2_ENABLE_PRINTUCS2        "Build with Printucs2"                                  ON  )
option( EXIV2_ENABLE_LENSDATA         "Build including lens data"                             ON  )
option( EXIV2_ENABLE_VIDEO            "Build video support into library"                      OFF )
option( EXIV2_ENABLE_WEBREADY         "Build webready support into library"                   OFF )
option( EXIV2_ENABLE_DYNAMIC_RUNTIME  "Use dynamic runtime (used for static libs)"            OFF )
option( EXIV2_ENABLE_WIN_UNICODE      "Use Unicode paths (wstring) on Windows"                OFF )
option( EXIV2_ENABLE_CURL             "USE Libcurl for HttpIo"                                OFF )
option( EXIV2_ENABLE_SSH              "USE Libssh for SshIo"                                  OFF )
option( EXIV2_BUILD_SAMPLES           "Build sample applications"                             ON  )
option( EXIV2_BUILD_PO                "Build translations files"                              OFF )
option( EXIV2_BUILD_EXIV2_COMMAND     "Build exiv2 command-line executable"                   ON  )
option( EXIV2_BUILD_UNIT_TESTS        "Build unit tests"                                      OFF )
option( EXIV2_TEAM_EXTRA_WARNINGS     "Add more sanity checks using compiler flags"           OFF )
option( EXIV2_TEAM_WARNINGS_AS_ERRORS "Treat warnings as errors"                              OFF )
option( BUILD_WITH_CCACHE             "Use ccache to speed up compilations"                   OFF )
```

You specify and option with the syntax:

```bash
$ cmake .. -DBUILD_SHARED_LIBS=OFF .. -DEXIV2_ENABLE_XMP=OFF
```

<name id="1-4"></a>
### 1.4) Build, test and install on _***ix**_ platforms (Linux, Cygwin and MacOS-X)
On _***ix**_ p platforms such as Linux, Cygwin and MacOS-X, the recommended way to build Exiv2 is:


```bash
$ cd <exiv2dir>
$ mkdir build
$ cd build
$ cmake ..
$ cmake --build . --config Release
$ make test
$ sudo make install
```

##### Uninstall:

```bash
$ sudo make uninstall
```

<name id="1-5"></a>
### 1.5) Build and install using Visual Studio

See [README-CONAN](README-CONAN.md)

<name id="1-6"></a>
### 1.6) Building Exiv2 with CMake and conan

See [README-CONAN](README-CONAN.md)



<name id="2"></a>
## 2) Linking your own code with Exiv2


A pkg-config .pc file is installed together with the library.
Application developers can use `pkg-config(1)` to obtain correct
compile and link time flags for the Exiv2 library. See
[samples/Makefile](https://github.com/Exiv2/exiv2/blob/master/samples/Makefile)
for an example.

If you downloaded Exiv2 directly from the git repository, and you want
to build it using the GNU configure script, then you need to have GNU
Autoconf installed on your system and create the configure script as
the first step:

    $ make config

Then run the usual `./configure; make; make install` commands.

Exiv2 uses GNU Libtool in order to build shared libraries on a variety
of systems. While this is very nice for making usable binaries, it can
be a pain when trying to debug a program. For that reason, compilation
of shared libraries can be turned off by specifying the
`--disable-shared` option to the configure script.

[TOC](#TOC)

<name id="3"></a>
## 3) Troubleshooting and Support

<name id="3-1"></a>
### 3.1) Troubleshooting

If you have problems building Exiv2 on UNIX-like systems, check the
generated `config/config.mk` and `config/config.h` files. You should *not*
need to modify any Makefile directly, in particular not `src/Makefile`!

<name id="3-2"></a>
### 3.2) Support

All project resources are accessible from the project website at
http://dev.exiv2.org/projects/exiv2/wiki

Please send feedback or queries to the Exiv2 forum. For new bug reports
and feature requests, please open an issue.

[TOC](#TOC)

<name id="4"></a>
## 4) Building Exiv2 Documentation

To generate the documentation (`make doc`), you will further need
`doxygen`, `graphviz`, `python` and `xsltproc`.

| Tool | Available | Purpose |
|:---- | :-------- | :------ |
|	doxygen    |  http://www.doxygen.org/ | Generates documentation from code comments |
|	graphviz   |  http://www.graphviz.org/ | Graphics generator used by doxygen |
|	python     |  http://www.python.org/ | Scripting language |
|	xsltproc   |  http://xmlsoft.org/XSLT/ | XML Manipulation Language Processor |
|	md5sum     |  http://www.microbrew.org/tools/md5sha1sum/ | Checksum calculator |

[TOC](#TOC)

<name id="5"></a>
## 5) License

Exiv2 is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your
option) any later version.

Exiv2 is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, 5th Floor, Boston,
MA 02110-1301 USA.

[TOC](#TOC)
