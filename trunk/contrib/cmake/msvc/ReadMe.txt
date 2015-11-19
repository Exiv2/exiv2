contrib/cmake/msvc/ReadMe.txt
-----------------------------

These scripts are "in development".
They are derived from Daniels contrib/build scripts.

How to use this
---------------

1 Setting up your machine
  You need cmake.exe, svn.exe, 7z.exe and curl.exe on your PATH.
  
  Please get "Windows" versions of cmake etc (NOT Cygwin versions)
  
  You should initialize the Visual Studio environment
  using the version of vcvars32.bat or vcvarsall.bat
  installed with Visual Studio. For example
  
  call "C:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
  
  The batch file contrib\cmake\msvc\vcvars.bat is designed to take the pain
  out of this - provided Visual Studio's installed in c:\Program Files (x86)
  vcvars 2005        # sets 2005 x86
  vcvars 2010 64     # sets 2010 x86_amd64
  
2 Always build "out of source".  I recommend:
  cd <exiv2dir>
  mkdir ..\build
  
  Ensure that build.cmd and setenv.cmd are on your path (eg your build directory)
  copy  contrib\cmake\msvc\* ..\build
  cd    ..\build
  build --help
  
  You should never have reason to modify the code in build.cmd
  You may wish to change the defaults in setenv.cmd
  You can change the defaults on the command-line (or modify setenv.cmd)
  You can also change defaults using the dos set command.  For example:
  set _CONFIG_=Debug
  
  To unset an environment string, set _CONFIG_=
  
  For your first build, I recommend the command:
  build --verbose --pause
  
  This will print out a lot of information, and pause after each build step.
  
  When you are building happily, you may prefer:
  build --silent
  
3 What gets built?
  The build is performed in  build\temp
  The output is generated in build\dist
                             build\dist\bin contains *.exe and *.dll files
                             build\dist\lib contains *.lib files
                             build\dist\include contains *.h and *.hpp files

4 Building manually with CMake
  The cmake option -G Generator should be chosen for the version of Visual Studio installed.
  cmake --help for more information
  
  I personally always build/test with Visual Studio 2005 in 64 bits.
  The generator is:  "Visual Studio 8 2005 Win64"

  cd <exiv2dir>
  mkdir ../build
  cd    ../build
  
  rem download support libraries
  svn export svn://dev.exiv2.org/svn/team/libraries/zlib-1.2.8
  svn export svn://dev.exiv2.org/svn/team/libraries/expat-2.1.0

  ...
      for webready 
      you also need curl-7.45.0 libssh-0.7.2 
      you will have to install openssl for you compiler before building curl
      see note below "About openssl"
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
  
5 About openssl
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
  
8 Building with different libraries
  You can change the standard libraries.  For example, to build with curl-7.39.0
  1) set _CURL_=curl-7.39.0
  2) put curl-7.39.0.tar.gz into your build directory
  
  To change the version of openssl:
  1) set _OPENSSL_=openssl-1.0.1j
  2) put openssl-1.0.1j-vs2015.zip into your build directory
  
  To build a version of openssl-foo-vs2012:
  Try to find it online
  Building this with Visual Studio is to to be documented

Status:
2015-11-19 "Work in Progress"
           Added a dependency for 7z.exe to decompress archives.
           Added downloading openssl
           Updated documentation

2015-11-18 "Work in Progress"
           Lots of simplification.
           Removed need for cygwin.
           Added user documentation to ReadMe.txt
           More work required on webready support.
           
2015-11-17 "Work in Progress"
           Added command-line parser
           Building exiv2 with zlib and expat and exiv2
           -webready = curl etc broken
           
2015-11-16 "Work in Progress" = Not working yet.
           These script are not for public use at the moment by Daniel or anybody else.

Rather than endlessly discuss these scripts with Daniel
I'm going to do parallel development and see how that goes.

I'll discuss this with Daniel when I'm convinced my versions are working well.

Robin Mills
robin@clanmills.com
