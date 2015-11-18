contrib/cmake/msvc/ReadMe.txt
-----------------------------

These scripts are "in development".
They are derived from Daniels contrib/build scripts.

How to use this
---------------

1 Ensure that build.cmd and setenv.cmd are on your path (eg the build directory)

2 Your machine setup:
  You need cmake.exe and svn.exe on your PATH.
  Please get "Windows" versions of cmake and svn (NOT Cygwin versions)
  You need a "Windows" version of perl.exe on the path to build openssl
  
  You should initialize the Visual Studio environment
  using the version vcvars32.bat or vcvarsall.bat
  installed with Visual Studio. For example
  
  call "C:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\vcvars32.bat"
  
  The cmake option -G Generator should be chosen for the version of Visual Studio installed.
  cmake --help for more information
  
3 Always build "out of source".  I recommend:
  cd <exiv2dir>
  mkdir ..\build
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
  
4 What gets built?
  The build is performed in  build\temp
  The output is generated in build\dist
                             build\dist\bin contains *.exe and *.dll files
                             build\dist\lib contains *.lib files
                             build\dist\include contains *.h and *.hpp files

5 Building manually with CMake
  cd <exiv2dir>
  mkdir ../build
  cd    ../build
  
  rem download support libraries
  svn export svn:/dev.exiv2.org/team/libraries/zlib-1.2.8
  svn export svn:/dev.exiv2.org/team/libraries/expat-2.1.0
  ... for webready, you also need libssh-0.5.5 curl-7.39.0 openssl-1.0.1j ...
  
  rem create a temp directory and a dist (distribution) directory 
  mkdir temp
  mkdir dist
  
  rem  build zlib-1.2.8
  mkdir temp\zlib-1.2.8
  cd    temp\zlib-1.2.8
  cmake -G "Visual Studio 8 2005 Win64" -DCMAKE_INSTALL_PREFIX=..\..dist ..\..\zlib-1.2.8
  cmake --build   . --config Release  # TAKE CARE with expat-2.1.0 use: cmake --build . --config Release --target expat
  cmake --build . --target install --config Release    
  cd ..\..
  
  rem  build expat-2.1.0 and other required libraries 
  
  rem  build exiv2
  mkdir temp\exiv2
  cd    temp\exiv2
  cmake -G "Visual Studio 8 2005 Win64" -DCMAKE_INSTALL_PREFIX=..\..\dist ^
           -DCMAKE_LIBRARY_PATH=..\..\dist\lib -DCMAKE_INCLUDE_PATH=..\..\dist\include ^
           -DEXIV2_ENABLE_SHARED=ON
           ..\..\..\<exiv2dir>
  cmake --build . --config Release
  cmake --build . --config Release --target install
  
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
  OPTION( EXIV2_ENABLE_CURL          "USE Libcurl for HttpIo"                                ON  )
  OPTION( EXIV2_ENABLE_SSH           "USE Libssh for SshIo"                                  ON  )

  C:\cygwin64\home\rmills\gnu\exiv2\trunk>
  
7 Running the test suite
  http://dev.exiv2.org/projects/exiv2/wiki/How_do_I_run_the_test_suite_for_Exiv2
  
Status:
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
