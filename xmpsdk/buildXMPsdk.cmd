@echo off
REM buildXMPsdk.cmd
setlocal enableextensions

GOTO main
:help
echo Options: --help   ^| --2016 ^| --2014 ^| --2013  ^| --64 ^| --32
exit /b 0

:main
REM
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

if /I "%1" == "--help"            call:help && goto end

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

REM
rem if it's already built, we're done
if EXIST Adobe\%_SDK_%\libXMPCore.lib (
  dir    Adobe\%_SDK_%\libXMPCore.lib
  exit /b 0
)

REM
rem Test the VC Environment
IF NOT DEFINED VSINSTALLDIR (
    echo "VSINSTALLDIR not set.  Run vcvars32.bat or vcvarsall.bat or vcvars.bat ***"
    GOTO error_end
)
IF NOT EXIST "%VSINSTALLDIR%" (
    echo "VSINSTALLDIR %VSINSTALLDIR% does not exist.  Run vcvars32.bat or vcvarsall.bat ***"
    GOTO error_end
)

call:report

REM
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
    echo *** ERROR SDK = Adobe\%_SDK_% not found" >2
    GOTO error_end
)

REM
rem Copy in the third-party files
xcopy/yesihq third-party\zlib                            Adobe\%_SDK_%\third-party\zlib
xcopy/yesihq third-party\expat\lib                       Adobe\%_SDK_%\third-party\expat\lib
xcopy/yesihq third-party\zuid\interfaces                 Adobe\%_SDK_%\third-party\zuid\interfaces

REM
rem  generate the build the SDK
cd   Adobe\%_SDK_%\build
dir
c:\cygwin64\bin\echo -n 5 | call GenerateXMPToolkitSDK_win.bat
cd ..
REM
rem build it!
echo --------------------------------------- 
echo devenv XMPCore\build\vc14\static\windows_x64\XMPCore64.sln /Build "Release|x64"
echo --------------------------------------- 
     devenv XMPCore\build\vc14\static\windows_x64\XMPCore64.sln /Build "Release|x64"
dir/s *.lib


REM
rem normal end
:end
endlocal
exit /b 0

REM -----------------------------------------
rem Functions
REM
rem echo (or don't if --silent).  syntax: call:echo args ...
:echo
if NOT DEFINED _SILENT_ echo %*%
exit /b 0

REM
rem end with an error syntax: call:error_end
:error_end
endlocal
exit /b 1

REM
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
REM
