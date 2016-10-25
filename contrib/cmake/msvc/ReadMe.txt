    @@@Marco@@@@@b                   ;mm                       /##Gilles###\
    j@@@#Robin",                     Brad                     /@@@Thomas@@@@Q
     @@@#       \                     ##                     @@@b     |@@@b
     @@@#          .;;;;,     ,;;;, ,;;;;  ,;;;p      .;;;   7@@      ]Alan
     @@@#           j@@@@,   ]@@#/  '@@@#  j@@@#      ]@@^           ;@@@"
     @@@Andreas@C     "@@@p @@@"     @@@b   j@@@p     @@b           @@@#/
     @@@#^7"7%#\       ^@@@@@#~      Benb    1@@@    {@#          s@@@#
     @@@#                Niels       @@@b     @@@Q  ]@#         ;@@@#/
     @@@#              ,@@##@@m      @@@b      @@@p @@C        #@@#C
     @@@#       ,/    s@@#  @@@@     @@@b       Volker       @Tuan@
    ]@@@Abhinav@@\   /@@@\    \@@@Q  @@@Q       %@@@#      /@@@@Mahesh@@#
   /@@Raphael@@@@@\ /@@@@@\     C++  Metadata  Library    /@Sridhar@@@v0.26\

exiv2/contrib/cmake/msvc/ReadMe.txt
-----------------------------------

How to use this
---------------

1 Setting up your machine
  You need cmake.exe, svn.exe and 7z.exe on your PATH.

  Please get "Windows" versions of cmake/svn/7z etc (NOT Cygwin or MinGW versions)

  You should initialize the Visual Studio environment
  using the version of vcvars32.bat or vcvarsall.bat
  installed with Visual Studio. For example:

  call "C:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"

  The batch file contrib\cmake\msvc\vcvars.bat is designed to take the pain
  out of this - provided Visual Studio is installed in %ProgramFiles(x86)%
  %ProgramFiles(x86)% is usually c:\Program Files (x86)

  vcvars 2005        # sets 2005 x86
  vcvars 2010 64     # sets 2010 x86_amd64

2 Always build "out of source".  I recommend:
  cd <exiv2dir>
  mkdir build

  +-------------------------------------------------------+
  | Never attempt to build in a directory with a space in |
  | the path name.  Example c:\My Build Tree\exiv2\build  |
  +-------------------------------------------------------+

  Ensure that cmakeBuild.cmd and cmakeDefaults.cmd are on your path (eg your build directory)
  copy  contrib\cmake\msvc\* build
  cd    build
  cmakeBuild --help

  You should never have reason to modify the code in cmakeBuild.cmd
  You may wish to change the defaults in cmakeDefaults.cmd
  You can change the defaults on the command-line (or modify cmakeDefaults.cmd)
  You can also change defaults using the dos set command.  For example:
  set _CONFIG_=Debug

  To unset an environment string, set _CONFIG_=

  For your first build, I recommend the command:
  cmakeBuild --pause --verbose

  This will print out a lot of information, and pause after each build step.

  When you are building happily, you may prefer:
  cmakeBuild --silent

3 What gets built?
  The build is performed in  build\temp
  The output is generated in build\dist
                             build\dist\..mumble..\bin contains *.exe and *.dll files
                             build\dist\..mumble..\lib contains *.lib files
                             build\dist\..mumble..\include contains *.h and *.hpp files

  mumble identifies the compiler and build.
  Example C:\gnu\exiv2\build\dist\2013\x64\dll\Release\bin
  2013    = Visual Studio      Choices: 2005/2008/2010/2012/2013/2015
  x64     = 64 bit build                Win32/x64
  dll     = shared library              dll/static
  Release = configuration               Release/Debug/RelWithDebInfo/MinSizeRel

4 Building manually with CMake
  The cmake option -G Generator should be chosen for the version of Visual Studio installed.
  cmake --help for more information

  I personally always build/test with Visual Studio 2005 in 64 bits.
  The generator is:  "Visual Studio 8 2005 Win64"

  cd <exiv2dir>
  mkdir ../build
  cd    ../build

  rem download support libraries
  svn export svn://dev.exiv2.org/svn/team/libraries/zlib-1.2.8.tar.gz
  svn export svn://dev.exiv2.org/svn/team/libraries/expat-2.1.0.tar.gz

  ...
      for webready
      you need curl-7.45.0 libssh-0.7.2 and openssl-1.0.1p
      See below: "About webready support libraries (openssl, libssh and curl)
  ...

  rem create a temp directory and a dist (distribution) directory
  mkdir temp  # build, compile and link in this directory
  mkdir dist  # the output artifacts are stored here

  rem  build zlib-1.2.8
  mkdir temp\zlib-1.2.8
  cd    temp\zlib-1.2.8
  cmake -G "Visual Studio 8 2005 Win64" "-DCMAKE_INSTALL_PREFIX=..\..dist" ..\..\zlib-1.2.8
  cmake --build .                  # TAKE CARE with expat-2.1.0 use: cmake --build . --target expat
  cmake --build . --target install
  cd ..\..

  rem  build expat-2.1.0 and other required libraries

  rem  build exiv2
  mkdir temp\exiv2
  cd    temp\exiv2
  cmake -G "Visual Studio 8 2005 Win64" "-DCMAKE_INSTALL_PREFIX=..\..\dist"                ^
           "-DCMAKE_LIBRARY_PATH=..\..\dist\lib" "-DCMAKE_INCLUDE_PATH=..\..\dist\include" ^
            -DEXIV2_ENABLE_SHARED=ON ^
            ..\..\..\<exiv2dir>
  cmake --build . --config Release
  cmake --build . --config Release --target install

5 About webready support libraries (openssl, libssh and curl)

  a) openssl
  You cannot build openssl with CMake.  However we have prebuilt binaries which
  you can download and extract into your build tree.

  You will have to match the version to your compiler.
  In this example: vs2015/64 bit

  svn export svn://dev.exiv2.org/svn/team/libraries/openssl-1.0.1p-vs2015.7z
  7z x openssl-1.0.1p-vs2015.7z
  xcopy/yesihq openssl-1.0.1p-vs2015\bin64      dist\bin"
  xcopy/yesihq openssl-1.0.1p-vs2015\lib64      dist\bin"
  xcopy/yesihq openssl-1.0.1p-vs2015\include64  dist\include"

  In this example: vs2008/32 bit
  svn export svn://dev.exiv2.org/svn/team/libraries/openssl-1.0.1p-vs2008.7z
  7z x openssl-1.0.1p-vs2015.7z
  xcopy/yesihq openssl-1.0.1p-vs2008\bin        dist\bin"
  xcopy/yesihq openssl-1.0.1p-vs2008\lib        dist\bin"
  xcopy/yesihq openssl-1.0.1p-vs2008\include    dist\include"

  The script contrib/cmake/msvc/cmakeOpenssl was used to create the vs2005.7z file
  from a complete build performed by msvc/exiv2-webready.sln and openssl-1.0.1p source

  b) curl
  curl does not seem to build with CMake.
  It announces itself "the curl cmake build system is poorly maintained. Be aware"

  I have given up trying to get this to work and used nmake in the winbuild directory.
  For more information, read:  winbuild\BUILD.WINDOWS.txt

  c) libssh
  Three changes have been made to libssh to build with VS2015, VS2008, VS2995
  These have been reported (with fixes)
  VS2015: 	https://red.libssh.org/issues/214
  VS2005/8: https://red.libssh.org/issues/2205

  The fixes are included in svn://dev.exiv2.org/svn/team/libraries/libssh-0.7.2.tar.gz
  A 'vanilla' version of libssh will may require those fixes to be applied.

6 Build options
  You can inspect CMake options by running grep OPTION on CMakeLists.txt in <exiv2dir>
  C:\cygwin64\home\rmills\gnu\exiv2\build>cd ..\trunk

  C:\cygwin64\home\rmills\gnu\exiv2\trunk>grep OPTION CMakeLists.txt
  OPTION( EXIV2_ENABLE_SHARED        "Build exiv2 as a shared library (dll)"                 ON  )
  OPTION( EXIV2_ENABLE_XMP           "Build with XMP metadata support"                       ON  )
  OPTION( EXIV2_ENABLE_LIBXMP        "Build a static convenience Library for XMP"            ON  )
  OPTION( EXIV2_ENABLE_VIDEO         "Build with video support"                              OFF )
  OPTION( EXIV2_ENABLE_PNG           "Build with png support (requires libz)"                ON  )
  OPTION( EXIV2_ENABLE_NLS           "Build native language support (requires gettext)"      ON  )
  OPTION( EXIV2_ENABLE_PRINTUCS2     "Build with Printucs2"                                  ON  )
  OPTION( EXIV2_ENABLE_LENSDATA      "Build including lens data"                             ON  )
  OPTION( EXIV2_ENABLE_COMMERCIAL    "Build with the EXV_COMMERCIAL_VERSION symbol set"      OFF )
  OPTION( EXIV2_ENABLE_BUILD_SAMPLES "Build the unit tests"                                  ON  )
  OPTION( EXIV2_ENABLE_BUILD_PO      "Build translations files"                              OFF )
  OPTION( EXIV2_ENABLE_VIDEO         "Build video support into library"                      OFF )
  OPTION( EXIV2_ENABLE_WEBREADY      "Build webready support into library"                   OFF )
  OPTION( EXIV2_ENABLE_WIN_UNICODE   "Use Unicode paths (wstring) on Windows"                OFF )
  OPTION( EXIV2_ENABLE_CURL          "USE Libcurl for HttpIo"                                OFF )
  OPTION( EXIV2_ENABLE_SSH           "USE Libssh for SshIo"                                  OFF )

  C:\cygwin64\home\rmills\gnu\exiv2\trunk>

7 Running the test suite
  http://dev.exiv2.org/projects/exiv2/wiki/How_do_I_run_the_test_suite_for_Exiv2

  You can run the test-suite directly from cmakeBuild.cmd with the argument --test
  You need cygwin's bash.exe to run the test suite.

8 Building with different versions of the support libraries
  You can change the standard libraries.  For example, to build with curl-7.39.0
  1) set _CURL_=curl-7.39.0
  2) add curl-7.39.0.tar.gz in your build directory

  To change the version of openssl:
  1) set _OPENSSL_=openssl-1.0.1j
  2) add openssl-1.0.1j-vs2015.zip into your build directory

9 Rebuilding with VS 2005/8/10/12/13/15 32/64
  The script cmakeBuildAll.cmd is provided for convenience:
  cmakeBuildAll.cmd --test > rebuildAll.txt
  To view progress, open another shell: tail -f rebuildAll.txt

  cmakeBuildAll.cmd takes about a hour if you don't specify --webready
  12 build+test cycles of about 5 minutes each.
  With webready, 12 build+test cycles of 12 minutes = 2.5 hours

Robin Mills
robin@clanmills.com
