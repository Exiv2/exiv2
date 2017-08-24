@echo off
rem  ----
rem buildXMPsdk.cmd
setlocal enableextensions
set "_THIS_=%0%"

GOTO main
:help
echo %_THIS_% [Options]
echo Options: --help   ^| --2016 ^| --2014 ^| --2013  ^| --64 ^| --32 ^| --rebuild
exit /b 0

:main
rem  ----
rem always run this script in <exiv2dir>/xmpsdk
cd %~dp0

set "_BUILDDIR_=%CD%"
set  _SDK_=2016
set  _BIT_=64

:GETOPTS
if /I "%1" == "--2013"            set "_SDK_=2013"
if /I "%1" == "--2014"            set "_SDK_=2014"
if /I "%1" == "--2016"            set "_SDK_=2016"
if /I "%1" ==   "2013"            set "_SDK_=2013"
if /I "%1" ==   "2014"            set "_SDK_=2014"
if /I "%1" ==   "2016"            set "_SDK_=2016"

if /I "%1" == "--32"              set "_BIT_=32"
if /I "%1" == "--64"              set "_BIT_=64"
if /I "%1" ==   "32"              set "_BIT_=32"
if /I "%1" ==   "64"              set "_BIT_=64"

if /I "%1" == "--verbose"         echo on
if /I "%1" == "--silent"          set _SILENT_=1
if /I "%1" == "--help"            call:help && goto end
if /I "%1" == "--rebuild"         set _REBUILD_=1

shift
if not (%1) EQU () goto GETOPTS

if /I "%_SDK_%" == "2013" (
  set "_SDK_=XMP-Toolkit-SDK-CC201306"
  set "_ZIP_=XMP-Toolkit-SDK-CC-201306"
  set "_VC_=vc10"
)
if /I "%_SDK_%" == "2014" (
  set "_SDK_=XMP-Toolkit-SDK-CC201412" 
  set "_ZIP_=XMP-Toolkit-SDK-CC201412"
  set "_VC_=vc11"
)
if /I "%_SDK_%" == "2016" (
  set "_SDK_=XMP-Toolkit-SDK-CC201607"
  set "_ZIP_=XMP-Toolkit-SDK-CC201607"
  set "_VC_=vc14"
)

if DEFINED _REBUILD_ if EXIST Adobe\%_SDK_% rmdir/s/q Adobe\%_SDK_%

rem  ----
rem if it's already built, we're done
set "_TARGET_=Adobe\%_SDK_%\public\doesntexist
if /I %_BIT_% == 64 set "_TARGET_=Adobe\%_SDK_%\public\libraries\windows_x64\Release\XMPCoreStatic.lib"
if /I %_BIT_% == 32 set "_TARGET_=Adobe\%_SDK_%\public\libraries\windows\Release\XMPCoreStatic.lib"
if EXIST %_TARGET_% (
   dir %_TARGET_%  
   GOTO end
)

rem  ----
rem Test the VC Environment
IF NOT DEFINED VSINSTALLDIR (
    echo "VSINSTALLDIR not set.  Run vcvars32.bat or vcvarsall.bat or vcvars.bat ***"
    GOTO error_end
)
IF NOT EXIST "%VSINSTALLDIR%" (
    echo "VSINSTALLDIR %VSINSTALLDIR% does not exist.  Run vcvars32.bat or vcvarsall.bat ***"
    GOTO error_end
)

rem  ----
call:echo testing 7z is on path
7z > NUL
IF ERRORLEVEL 1 (
    echo "*** please ensure 7z.exe is on the PATH ***"
    GOTO error_end
)

rem  ----
call:echo testing curl is on path
curl --version > NUL
IF ERRORLEVEL 1 (
    echo "*** please ensure cmake.exe is on the PATH ***"
    GOTO error_end
)

rem  ----
call:echo testing my_echo is on path
if NOT EXIST my_echo.exe cl my_echo.cpp /o my_echo.exe
	
my_echo --version > NUL
IF ERRORLEVEL 1 (
    echo "*** please ensure my_echo.exe is on the PATH ***"
    GOTO error_end
)

call:report

rem  ----
rem Download the code from Adobe
if NOT EXIST     Adobe\%_SDK_% (
    if NOT EXIST Adobe mkdir Adobe
    cd           Adobe
    if NOT EXIST %_ZIP_%.zip  copy/y y:\temp\XMP-Toolkit-SDK-CC201607.zip
    if NOT EXIST %_ZIP_%.zip  curl -O http://download.macromedia.com/pub/developer/xmp/sdk/%_ZIP_%.zip
    IF NOT ERRORLEVEL 1       7z x %_ZIP_%.zip 2>nul
    cd ..
)

if NOT EXIST Adobe\%_SDK_%  (
    echo *** ERROR SDK = Adobe\%_SDK_% not found"
    GOTO error_end
)

rem  ----
rem Copy in the third-party files
xcopy/yesihq third-party\zlib                            Adobe\%_SDK_%\third-party\zlib
xcopy/yesihq third-party\expat\lib                       Adobe\%_SDK_%\third-party\expat\lib
xcopy/yesihq third-party\zuid\interfaces                 Adobe\%_SDK_%\third-party\zuid\interfaces

rem  ----
rem  generate and build the SDK
cd   Adobe\%_SDK_%\build
set  

if /I %_BIT_% == 64 ( 
    %_BUILDDIR_%\my_echo 5 | call GenerateXMPToolkitSDK_win.bat
    devenv %_VC_%\static\windows_x64\XMPToolkitSDK64.sln /Build "Release|x64" /ProjectConfig XMPCoreStatic
)

if /I %_BIT_% == 32 (
    %_BUILDDIR_%\my_echo 3 | call GenerateXMPToolkitSDK_win.bat
    devenv %_VC_%\static\windows\XMPToolkitSDK.sln /Build "Release|Win32" /ProjectConfig XMPCoreStatic
)
cd   ..\..\..

rem ------------------------------------------------------------------------------
rem The Adobe script GeneratXMPToolkitSDK_win.bat
rem use the CMake Generators "Visual Studio 14 2015 Win64" and "Visual Studio 14 2015"
rem CMake provides more generators
rem       Visual Studio 14 2015 [arch] = Generates Visual Studio 2015 project files.
rem       Visual Studio 12 2013 [arch] = Generates Visual Studio 2013 project files.
rem       Visual Studio 11 2012 [arch] = Generates Visual Studio 2012 project files.
rem       Visual Studio 10 2010 [arch] = Generates Visual Studio 2010 project files.
rem       Visual Studio 9 2008 [arch]  = Generates Visual Studio 2008 project files.
rem       Visual Studio 8 2005 [arch]  = Generates Visual Studio 2005 project files.
rem Adobe generate the Visual Studio Solution with the CMake command:
rem       cmake ../../../. ^
rem           -G"Visual Studio 14 2015 Win64" ^ or -G"Visual Studio 14 2015 Win64"
rem             -DXMP_CMAKEFOLDER_NAME="vc14/static/windows_x64" ^
rem             -DCMAKE_CL_64=ON ^
rem             -DCMAKE_ARCH=x64 ^         or  -DCMAKE_ARCH=x64=x86
rem             -DXMP_BUILD_WARNING_AS_ERROR=ON ^
rem             -DXMP_BUILD_STATIC=ON
rem             -DCMAKE_BUILD_TYPE=Release
rem I believe it's possible to build for other versions of Visual Studio
rem However I have never been tested this
rem ------------------------------------------------------------------------------

cd "%_BUILDDIR_%"
dir/s XMPCoreStatic.lib
GOTO end

rem -----------------------------------------
rem Functions

rem  ----
rem normal end
:end
endlocal
exit /b 0

rem  ----
rem echo (or don't if --silent).  syntax: call:echo args ...
:echo
if NOT DEFINED _SILENT_ echo %*%
exit /b 0

rem  ----
rem end with an error syntax: call:error_end
:error_end
endlocal
exit /b 1

rem  ----
rem report settings
:report
echo.sdk          = %_SDK_%
echo.bit          = %_BIT_%
echo.zip          = %_ZIP_%
echo.builddir     = %_BUILDDIR_%
echo.vsinstalldir = %VSINSTALLDIR%
echo ----------------------------------
exit /b 0

rem That's all Folks!
rem  ----
