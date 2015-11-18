@echo off
setlocal enableextensions

set "_BUILDDIR_=%CD%"

:GETOPTS
if /I "%1" == "--help" (
   call:Help
   exit /b
)
if /I "%1" == "--webready"        set "_WEBREADY_=1"
if /I "%1" == "--config"          set "_CONFIG_=%2"& shift
if /I "%1" == "--temp"            set "_TEMP_=%2"& shift
if /I "%1" == "--generator"       set "_GENERATOR_=%2"& shift
if /I "%1" == "--exiv2"           set "_EXIV2_=%2"& shift
if /I "%1" == "--verbose"         set "_VERBOSE_=1"
if /I "%1" == "--dryrun"          set "_DRYRUN_=1"
if /I "%1" == "--rebuild"         set "_REBUILD_=1"
if /I "%1" == "--silent"          set "_SILENT_=1"
if /I "%1" == "--silent"          set "_QUIET_=1"
if /I "%1" == "--quiet"           set "_QUIET_=1"
if /I "%1" == "--video"           set "_VIDEO_=1"
if /I "%1" == "--pause"           set "_PAUSE_=1"
if /I "%1" == "--zlib"            set "_ZLIB_=%2"& shift
if /I "%1" == "--expat"           set "_EXPAT_=%2"& shift
if /I "%1" == "--libssh"          set "_LIBSSH_=%2"& shift
if /I "%1" == "--curl"            set "_CURL_=%2"& shift
if /I "%1" == "--openssl"         set "_OPENSSL_=%2"& shift

shift
if not (%1) EQU () goto GETOPTS

rem  ----
call:echo calling setenv
call setenv.cmd
IF ERRORLEVEL 1 (
	echo "*** setenv.cmd has failed ***" >&2
	GOTO error_end
)

rem  ----
call:echo checking that %_EXIV2_% exists
if NOT EXIST %_EXIV2_% (
	echo "_EXIV2_ = %_EXIV2_% does not exist ***" >&2
	exit /b 1
)
pushd %_EXIV2_%
set _EXIV2_=%CD%
popd
call:echo _EXIV2_ = %_EXIV2_%

rem  ----
call:echo testing VSINSTALLDIR "%VSINSTALLDIR%"
IF NOT DEFINED VSINSTALLDIR (
	echo "VSINSTALLDIR not set.  Run vcvars32.bat or vcvarsall.bat ***"
	GOTO error_end
)
IF NOT EXIST "%VSINSTALLDIR%" (
	echo "VSINSTALLDIR %VSINSTALLDIR% does not exist.  Run vcvars32.bat or vcvarsall.bat ***"
	GOTO error_end
)

rem http://stackoverflow.com/questions/9252980/how-to-split-the-filename-from-a-full-path-in-batch
for %%A in ("%VSINSTALLDIR%") do (
    set "VS_PROG_FILES=%%~nA"
)
call:echo VS_PROG_FILES = "%VS_PROG_FILES%"

rem  ----
call:echo setting CMake command options
IF "%VS_PROG_FILES%" EQU "Microsoft Visual Studio 14" (
	rem Visual Studio 2015
	set VS_CMAKE=Visual Studio 14
) ELSE IF "%VS_PROG_FILES%" EQU "Microsoft Visual Studio 12" (
	rem Visual Studio 2013
	set VS_CMAKE=Visual Studio 12
) ELSE IF "%VS_PROG_FILES%" EQU "Microsoft Visual Studio 11" (
	rem Visual Studio 2012
	set VS_CMAKE=Visual Studio 11
) ELSE IF "%VS_PROG_FILES%" EQU "Microsoft Visual Studio 10" (
	rem Visual Studio 2010
	set VS_CMAKE=Visual Studio 10
) ELSE IF "%VS_PROG_FILES%" EQU "Microsoft Visual Studio 9" (
	rem Visual Studio 2008
	set VS_CMAKE=Visual Studio 9 2008
) ELSE IF "%VS_PROG_FILES%" EQU "Microsoft Visual Studio 8" (
	rem Visual Studio 2005
	set VS_CMAKE=Visual Studio 8 2005
) ELSE (
    echo "*** Unsupported version of Visual Studio in '%VSINSTALLDIR%' ***"
	GOTO error_end
)

call:echo testing architecture
if "%PROCESSOR_ARCHITECTURE%" EQU "AMD64" ( 
	set Platform=x64
	set RawPlatform=x64
	set CpuPlatform=intel64
) ELSE (
	set Platform=Win32
	set RawPlatform=x86
	set CpuPlatform=ia32
)

IF %Platform% EQU x64 (
	set "VS_CMAKE=%VS_CMAKE% Win64"
)
call:echo Platform = %Platform% (%RawPlatform%)

rem  ----
call:echo testing out of source build
dir/s exiv2.cpp >NUL 2>NUL
IF NOT ERRORLEVEL 1 (
	echo "*** error: do not execute this script within the exiv2 source directory ***"
	goto error_end
)

rem  ----
call:echo testing compiler
cl > NUL 2>NUL
IF ERRORLEVEL 1 (
	echo "*** ensure cl is on path.  Run vcvars32.bat or vcvarsall.bat ***"
	GOTO error_end
)
if NOT DEFINED _SILENT_ cl>/NUL

rem  ----
call:echo testing svn is on path
svn --version > NUL
IF ERRORLEVEL 1 (
	echo "*** please ensure SVN.exe is on the PATH ***"
	GOTO error_end
)

rem  ----
call:echo testing cmake is on path
cmake --version > NUL
IF ERRORLEVEL 1 (
	echo "*** please ensure cmake.exe is on the PATH ***"
	GOTO error_end
)

rem  ----
call:echo testing temporary directory
IF NOT EXIST "%_TEMP_%" mkdir "%_TEMP_%"
pushd        "%_TEMP_%"
set           "_TEMP_=%CD%"
popd
call:echo      _TEMP_ = %_TEMP_% 

rem ----
call:echo testing INSTALL
SET _INSTALL_=dist
IF NOT EXIST %_INSTALL_% mkdir %_INSTALL_%
pushd        %_INSTALL_%
set          "_INSTALL_=%CD%"
popd
call:echo     _INSTALL_ = %_INSTALL_%

if NOT DEFINED _GENERATOR_       set "_GENERATOR_=%VS_CMAKE%"
if /I "%_GENERATOR_%" == "NMake" set "_GENERATOR_=NMake Makefiles"

if defined _VIDEO_ set _VIDEO_=-DEXIV2_ENABLE_VIDEO=ON

rem  ----
echo.
echo.config   = %_CONFIG_%
echo.video    = %_VIDEO_%
echo.webready = %_WEBREADY_%
echo.exiv2    = %_EXIV2_%"

IF DEFINED _DRYRUN_ exit /b 1

echo ---------- building ZLIB ------------------
call:buildLib %_ZLIB_%

echo ---------- building EXPAT -----------------
set "TARGET=--target expat"
call:buildLib %_EXPAT_%
set  TARGET=


if DEFINED _WEBREADY_ (
	echo ---------- building LIBSSH -----------------
	set _SSH_=-DEXIV2_ENABLE_SSH=ON
	call:buildLib "%_LIBSSH_%"
	if errorlevel 1 set _SSH_=-DEXIV2_ENABLE_SSH=OFF

	echo ---------- building OPENSSL -----------------
	call:buildLib "%_OPENSSL_%"

	echo ---------- building CURL -----------------
	set _CURL_=-DEXIV2_ENABLE_CURL=ON
	call:buildLib %_CURL_%"
	if errorlevel 1 set _CURL_=-DEXIV2_ENABLE_CURL=OFF
	
	set _WEBREADY_=-DEXIV2_ENABLE_WEBREADY=ON
) else (
	set _WEBREADY_=-DEXIV2_ENABLE_WEBREADY=OFF
	set _CURL_=-DEXIV2_ENABLE_CURL=OFF
	set _SSH_=-DEXIV2_ENABLE_SSH=OFF
)

echo ---------- building EXIV2 ------------------
set          "EXIV_BUILD=%_TEMP_%\exiv2"

if defined _REBUILD_        rmdir/s/q "%EXIV_BUILD%"
IF NOT EXIST "%EXIV_BUILD%" mkdir     "%EXIV_BUILD%"
pushd        "%EXIV_BUILD%"
	call:run cmake -G "%_GENERATOR_%" ^
	         "-DCMAKE_INSTALL_PREFIX=%_INSTALL_%"      "-DCMAKE_PROGRAM_PATH=%SVN_DIR%"           ^
	         "-DCMAKE_LIBRARY_PATH=%_INSTALL_%\lib"    "-DCMAKE_INCLUDE_PATH=%_INSTALL_%\include" ^
	          -DEXIV2_ENABLE_NLS=OFF                    -DEXIV2_ENABLE_BUILD_SAMPLES=ON           ^
	          -DEXIV2_ENABLE_WIN_UNICODE=OFF            -DEXIV2_ENABLE_SHARED=ON ^
	          %_WEBREADY_%  %_CURL_%  %_SSH_% %_VIDEO_% ^
	         "%_EXIV2_%"

	IF errorlevel 1 (
		echo "*** cmake errors in EXIV2 ***" >&2
	    popd
		goto error_end
	)

	call:run cmake --build . --config %_CONFIG_%
	IF errorlevel 1 (
		echo "*** build errors in EXIV2 ***" >&2
	    popd
		goto error_end
	)

	call:run cmake --build . --config %_CONFIG_% --target install
	IF errorlevel 1 (
		echo "*** install errors in EXIV2 ***" >&2
	    popd
		goto error_end
	)
	if     defined _SILENT_ copy/y "samples\%_CONFIG_%\"*.exe "%_INSTALL_%\bin" >nul
	if NOT defined _SILENT_ copy/y "samples\%_CONFIG_%\"*.exe "%_INSTALL_%\bin"
popd

rem -----------------------------------------
rem Exit
:end
endlocal
exit /b 0

:error_end
endlocal
exit /b 1

rem -----------------------------------------
rem Functions
:help
echo Options: --help ^| --pause ^| --webready ^| --dryrun ^| --verbose ^| --rebuild ^| --silent ^| --verbose ^| --video
echo.         --exiv2 directory ^| --temp directory ^| --config name ^| --generator generator
echo.         --zlib zlib.1.2.8 ^| --expat expat-2.1.0 ^| --curl curl-7.39.0 ^| --libssh libssh-0.5.5 ^| --openssl openssl-1.0.1j
exit /b 0

:echo
if NOT DEFINED _SILENT_ echo %*%
exit /b 0

:run
if defined _VERBOSE_ (
	echo.
	echo ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	echo CD = %CD%
	echo %*%
)	
if     defined _SILENT_ %*% >nul 2>nul
if NOT defined _SILENT_ %*%

set _RESULT_=%ERRORLEVEL%
if DEFINED _PAUSE_ pause
exit /b %_RESULT_%

rem -------------------------------------------------------------
:buildLib
cd  "%_BUILDDIR_%"
set "LIB=%1%"
set "LIB_B=%_TEMP_%\%LIB%"

if defined _REBUILD_   rmdir/s/q "%LIB%" "%LIB_B%"
IF NOT EXIST "%LIB%"   svn export svn://dev.exiv2.org/svn/team/libraries/%LIB% >NUL
IF NOT EXIST "%LIB_B%" mkdir "%LIB_B%"  
pushd "%LIB_B%"

    call:run cmake -G "%_GENERATOR_%" -DCMAKE_INSTALL_PREFIX=%_INSTALL_% ..\..\%LIB%
	IF errorlevel 1 (
		echo "*** cmake errors in %LIB% ***"
	    popd
		exit /b 1
	)

	call:run cmake --build . --config %_CONFIG_% %TARGET%
	IF errorlevel 1 (
		echo "*** warning: build errors in %LIB% ***"
	)

	call:run cmake --build . --config %_CONFIG_% --target install
	IF errorlevel 1 (
		echo "*** warning: install errors in %LIB% ***"
	)
popd
exit /b 0

rem That's all Folks!
rem -------------------------------------------------------------
