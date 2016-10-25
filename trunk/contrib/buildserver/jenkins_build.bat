@echo off

rem --
rem
rem build exiv2 library from Jenkins (or the command-line)
rem
rem --

rem --
rem set up defaults for undefined variables used by this script
if NOT DEFINED ACTION        set ACTION=/build
if NOT DEFINED COMPILER      set COMPILER=G++
if NOT DEFINED BuildEnv      set BuildEnv=native
if NOT DEFINED Builder       set Builder=2005

if NOT DEFINED release       set release=false
if NOT DEFINED debug         set debug=false
if NOT DEFINED static        set static=false
if NOT DEFINED dll           set dll=false
if NOT DEFINED x64           set x64=false
if NOT DEFINED Win32         set Win32=false

if NOT DEFINED tests         set tests=false
if NOT DEFINED teste         set teste=false
if NOT DEFINED testv         set testv=false
if NOT DEFINED testr         set testr=false

if NOT DEFINED expat         set expat=true
if NOT DEFINED zlib          set zlib=true
if NOT DEFINED curl          set curl=false
if NOT DEFINED libssh        set libssh=false
if NOT DEFINED openssl       set openssl=false
if %openssl%==true           set curl=true

if %ACTION%==/clean          set tests=false
if %ACTION%==/upgrade        set tests=false

rem --
rem init the build environment for the builder
rem currently, only 2003 (32 bit) and 2005 (32 and 64 bit) are supported
rem adding additional MSVC compilers (2008, 2010, 2012, etc) should be simple
rem and has not been done for expediency
rem should only require:
rem 1    Invoke the appropriate vsvars32.bat script
rem 2    Run devenv/upgrade
rem 3    set Builder=2005
rem and all should be good
set BGOOD=0
if %Builder%==2003 (
  set   x64=false
  set   Win32=true
  set   curl=false
  set   openssl=false
  set   libssh=false
  call "C:\Program Files (x86)\Microsoft Visual Studio .NET 2003\Common7\Tools\vsvars32.bat"
  pushd msvc2003
  set   BGOOD=1
)

if %Builder%==2005 (
  call "C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\Tools\vsvars32.bat"
  pushd msvc
  set   BGOOD=1
)

rem --
rem check we have a good builder
rem we could test label and msvc
rem if NOT %label%==MSVC exit/b 1
rem if NOT %msvc%==true  exit/b 1
if %BGOOD%==0 (
  echo "MSVC Builder %Builder% is not supported"
  exit/b 1
)

rem --
rem FOO is the current directory in cygwin (/cygdrive/c/users/shared/workspace/exiv2-trunk/label/msvc)
rem we need this to set the correct directory when we run the test suite from Cygwin
for /f "tokens=*" %%a in ('cygpath -au ..') do set FOO=%%a

rem --
rem  copy external library code and other files appropriate for the build requested
copy exiv2.sln e.sln
call copylibs.bat

set webready=false
if %curl% == true if %libssh% == true if %openssl% == true set webready=true
if %webready% == true (
  copy/y exiv2-webready.sln e.sln
  copy/y ..\include\exiv2\exv_msvc-webready.h ..\include\exiv2\exv_msvc.h
)

rem --
rem let the user know what's going on!
echo ------- from jenkins_build.bat ------------
set PATH=c:\perl64\bin;%PATH%
set | sort
perl --version

rem --
rem Now build and test

if %Win32%==true (
  if %debug%==true (
    if %static%==true (
      if %Builder%==2003 (
        devenv e.sln %ACTION% "Debug"
        if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2003/bin/Debug'
      )
      if %Builder%==2005 (
        devenv e.sln %ACTION% "Debug|Win32"
        if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc/bin/win32/Debug'
      )
) ) )

if %Win32%==true (
  if %release%==true (
    if %static%==true  (
      if %Builder%==2003 (
        devenv e.sln %ACTION% "Release"
        if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2003/bin/Release'
      )
      if %Builder%==2005 (
        devenv e.sln %ACTION% "Release|Win32"
        if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc/bin/win32/Release'
      )
) ) )

if %Win32%==true (
  if %debug%==true (
    if %dll%==true   (
      if %Builder%==2003 (
        devenv e.sln %ACTION% "DebugDLL"
        if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2003/bin/DebugDLL'
      )
      if %Builder%==2005 (
        devenv e.sln %ACTION% "DebugDLL|Win32"
        if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc/bin/win32/DebugDLL'
      )
) ) )

if %Win32%==true (
  if %release%==true (
    if %dll%==true     (
      if %Builder%==2003 (
        devenv e.sln %ACTION% "ReleaseDLL"
        if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc2003/bin/ReleaseDLL'
      )
      if %Builder%==2005 (
        devenv e.sln %ACTION% "ReleaseDLL|Win32"
        if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc/bin/win32/ReleaseDLL'
      )
) ) )

if %x64%==true (
  if %debug%==true (
    if %static%==true (
      devenv e.sln %ACTION% "Debug|x64"
      if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc/bin/x64/Debug'
) ) )

if %x64%==true (
  if %release%==true (
    if %static%==true  (
      devenv e.sln %ACTION% "Release|x64"
      if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc/bin/x64/Release'
) ) )

if %x64%==true (
  if %debug%==true (
    if %dll%==true   (
      devenv e.sln %ACTION% "DebugDLL|x64"
      if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc/bin/x64/DebugDLL'
) ) )

if %x64%==true   (
  if %release%==true (
    if %dll%==true     (
      devenv e.sln %ACTION% "ReleaseDLL|x64"
      if NOT ERRORLEVEL 1 if %tests%==true call bash -c 'cd %FOO%;cd test;./testMSVC.sh ../msvc/bin/x64/ReleaseDLL'
) ) )

rem --
rem cleanup time
del e.sln
popd

rem --
rem delete support libraries (with mozilla's native rm utility)
if %Builder%==2003 msvc\tools\bin\rm.exe -rf ..\expat-2.0.1 ..\zlib-1.2.3
if %Builder%==2005 msvc\tools\bin\rm.exe -rf ..\expat ..\zlib ..\openssl ..\libssh ..\curl
if %Builder%==2005 if EXIST openssl (
  cd ..
  cygwin64\bin\rm.exe -rf openssl
  cd msvc
)

rem That's all Folks!
rem -----------------
