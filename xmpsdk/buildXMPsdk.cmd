@echo off
rem  ----
rem buildXMPsdk.cmd
setlocal enableextensions
set "_THIS_=%0%"

GOTO main
:help
echo %_THIS_% [Options]
echo.Options: --help   ^| --2016 ^| --2014 ^| --2013  ^| --64 ^| --32 ^| --rebuild ^| --dryrun ^| --distclean
echo          --sdk [2013^|2014^|2016] ^| --bit [32^|64] ^| --vs [2005^|2008^|2010^|2012^|2013^|2015^|2017] 
exit /b 0

:main
rem  ----
rem always run this script in <exiv2dir>/xmpsdk
cd %~dp0

set "_BUILDDIR_=%CD%"
set  _SDK_=2016
set  _BIT_=64
set  _VS_=2015

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

if /I "%1" == "--vs"              set "_VS_=%2"&shift
if /I "%1" == "--sdk"             set "_SDK_=%2"&shift
if /I "%1" == "--bit"             set "_BIT_=%2"&shift

if /I "%1" == "--verbose"         echo on
if /I "%1" == "--silent"          set _SILENT_=1
if /I "%1" == "--help"            call:help && goto end
if /I "%1" == "--rebuild"         set _REBUILD_=1
if /I "%1" == "--distclean"       set _DISTCLEAN_=1
if /I "%1" == "--dryrun"          set _DRYRUN_=1

shift
if not (%1) EQU () goto GETOPTS

if /I "%_SDK_%" == "2013" (
  set "_SDK_=XMP-Toolkit-SDK-CC201306"
  set "_ZIP_=XMP-Toolkit-SDK-CC-201306"
)
if /I "%_SDK_%" == "2014" (
  set "_SDK_=XMP-Toolkit-SDK-CC201412" 
  set "_ZIP_=XMP-Toolkit-SDK-CC201412"
)
if /I "%_SDK_%" == "2016" (
  set "_SDK_=XMP-Toolkit-SDK-CC201607"
  set "_ZIP_=XMP-Toolkit-SDK-CC201607"
)

if /I "%_VS_%" == "2017" set "_VC_=15"
if /I "%_VS_%" == "2015" set "_VC_=14"
if /I "%_VS_%" == "2013" set "_VC_=12"
if /I "%_VS_%" == "2012" set "_VC_=11"
if /I "%_VS_%" == "2010" set "_VC_=10"
if /I "%_VS_%" == "2008" set "_VC_=9"
if /I "%_VS_%" == "2005" set "_VC_=8"
                        set "_GENERATOR_=Visual Studio %_VC_% %_VS_%"
IF /I "%_BIT_%" == "64" set "_GENERATOR_=Visual Studio %_VC_% %_VS_% Win64"
if DEFINED _DRYRUN_ (
	call:report
	GOTO end
)

if DEFINED _REBUILD_   if EXIST Adobe\%_SDK_% rmdir/s/q Adobe\%_SDK_%
if DEFINED _DISTCLEAN_ if EXIST Adobe         rmdir/s/q Adobe

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
call:echo testing cmake.exe is on path
cmake.exe > NUL
IF ERRORLEVEL 1 (
    echo "*** please ensure cmake.exe is on the PATH ***"
    GOTO error_end
)


call:report

rem  ----
rem Download the code from Adobe
if NOT EXIST     Adobe\%_SDK_% (
    if NOT EXIST Adobe mkdir Adobe
    cd           Adobe
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
del CMakeCache.txt
if EXIST CMakeFiles rmdir/s/q CMakeFiles

rem ------------------------------------------------------------------------------
rem The Adobe script GeneratXMPToolkitSDK_win.bat
rem use the CMake Generators "Visual Studio 14 2015 Win64" and "Visual Studio 14 2015"
rem Adobe generate the Visual Studio Solution with the CMake command:
rem       cmake ../../../. ^
rem           -G"Visual Studio 14 2015 Win64" ^ or -G"Visual Studio 14 2015"
rem             -DXMP_CMAKEFOLDER_NAME="vc14/static/windows_x64" ^
rem             -DCMAKE_CL_64=ON ^
rem             -DCMAKE_ARCH=x64 ^         or  -DCMAKE_ARCH=x86
rem             -DXMP_BUILD_WARNING_AS_ERROR=ON ^
rem             -DXMP_BUILD_STATIC=ON
rem             -DCMAKE_BUILD_TYPE=Release
rem ------------------------------------------------------------------------------
rem Building with the Adobe batch file GenerateXMPToolkitSDK_win.bat
rem TODO: Test the CMake.exe code and decide how to proceed
rem       It's possible SDK=2016 demands VS=2015 (SDK=2014 & VS=2012) (SDK 2013 & VS=2010)
rem if /I %_BIT_% == 64 ( 
rem     echo 5|GenerateXMPToolkitSDK_win.bat
rem     call "%_BUILDDIR_%\..\contrib\cmake\msvc\vcvars.bat" %_VS_% %_BIT_%
rem     devenv vc%_VC_%\static\windows_x64\XMPToolkitSDK64.sln /Build "Release|x64" /ProjectConfig XMPCoreStatic
rem )
rem  
rem if /I %_BIT_% == 32 (
rem     echo 3|GenerateXMPToolkitSDK_win.bat
rem     call "%_BUILDDIR_%\..\contrib\cmake\msvc\vcvars.bat" %_VS_% %_BIT_%
rem     devenv vc%_VC_%\static\windows\XMPToolkitSDK.sln /Build "Release|Win32" /ProjectConfig XMPCoreStatic
rem )

    set "_CL64_=-DCMAKE_CL_64=ON"
    set "_ARCH_=-DCMAKE_ARCH=x64"
    set "_OUT_=%_VS_%/static/windows_x64"
    set "_BUILD_=Release|x64"
    set "_SLN_=XMPToolkitSDK64.sln"
if /I "%_BIT_%" == "32" (
    set "_CL64_=-DCMAKE_CL_64=OFF"
    set "_ARCH_=-DCMAKE_ARCH=x86"
    set "_OUT_=%_VS_%/static/windows"
    set "_BUILD_=Release|Win32"
    set "_SLN_=XMPToolkitSDK.sln"
)
@echo on
cmake.exe   --version
call      "%_BUILDDIR_%\..\contrib\cmake\msvc\vcvars.bat" %_VS_% %_BIT_%
cmake.exe . "-G%_GENERATOR_%" "%_CL64_%" "%_ARCH_%" -DXMP_BUILD_STATIC=ON -DCMAKE_BUILD_TYPE=Release "-DXMP_CMAKEFOLDER_NAME=%_OUT_%"
rem cmake.exe . --build
devenv    "%_SLN_%"     /Build "%_BUILD_%"    /ProjectConfig XMPCoreStatic

cd   ..\..\..
@echo off

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
cmake.exe > NUL
IF NOT ERRORLEVEL 1 cmake.exe --version
echo.vs           = %_VS_%
echo.bit          = %_BIT_%
echo.sdk          = %_SDK_%
echo.generator    = %_GENERATOR_%
echo.zip          = %_ZIP_%
echo.builddir     = %_BUILDDIR_%
echo ----------------------------------
exit /b 0

rem That's all Folks!
rem  ----
