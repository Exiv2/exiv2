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
if /I "%1" == "--cygwin"          set "_CYGWIN_=%2"& shift
if /I "%1" == "--exiv2"           set "_EXIV2_=%2"& shift
if /I "%1" == "--verbose"         set "_VERBOSE_=1"
if /I "%1" == "--dryrun"          set "_DRYRUN_=1"
if /I "%1" == "--rebuild"         set "_REBUILD_=1"
if /I "%1" == "--silent"          set "_SILENT_=1"
if /I "%1" == "--silent"          set "_QUIET_=1"
if /I "%1" == "--quiet"           set "_QUIET_=1"

shift
if not (%1) EQU () goto GETOPTS

rem  ----
call:echo calling setenv
call setenv.cmd
IF ERRORLEVEL 1 (
	echo "*** setenv.cmd has failed ***"
	GOTO error_end
)

rem if NOT DEFINED _DRYRUN_ if DEFINED _REBUILD_ (
rem 	for /F "delims=" %%i in ('dir /b') do (rmdir "%%i" /s/q || del "%%i" /s/q)  	
rem )

rem  ----
call:echo checking that %_EXIV2_% exists
if NOT EXIST %_EXIV2_% (
	echo "_EXIV2_ = %_EXIV2_% does not exist ***"
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
    set "VSSTUDIO=%%~nA"
)
call:echo VSSTUDIO = "%VSSTUDIO%"

rem  ----
call:echo setting CMake command options
IF "%VSSTUDIO%" EQU "Microsoft Visual Studio 14" (
	rem Visual Studio 2015
	set VS_SHORT=vc14
	set VS_CMAKE=Visual Studio 14
	set VS_PROG_FILES=Microsoft Visual Studio 14.0
	set VS_OPENSSL=vs2015
) ELSE IF "%VSSTUDIO%" EQU "Microsoft Visual Studio 12" (
	rem Visual Studio 2013
	set VS_SHORT=vc12
	set VS_CMAKE=Visual Studio 12
	set VS_PROG_FILES=Microsoft Visual Studio 12.0
	set VS_OPENSSL=vs2013
) ELSE IF "%VSSTUDIO%" EQU "Microsoft Visual Studio 11" (
	rem Visual Studio 2012
	set VS_SHORT=vc11
	set VS_CMAKE=Visual Studio 11
	set VS_PROG_FILES=Microsoft Visual Studio 11.0
	set VS_OPENSSL=vs2012
) ELSE IF "%VSSTUDIO%" EQU "Microsoft Visual Studio 10" (
	rem Visual Studio 2010
	set VS_SHORT=vc10
	set VS_CMAKE=Visual Studio 10
	set VS_PROG_FILES=Microsoft Visual Studio 10.0
	set VS_OPENSSL=vs2010
) ELSE IF "%VSSTUDIO%" EQU "Microsoft Visual Studio 9" (
	rem Visual Studio 2008
	set VS_SHORT=vc9
	set VS_CMAKE=Visual Studio 9 2008
	set VS_PROG_FILES=Microsoft Visual Studio 9.0
	set VS_OPENSSL=vs2008
) ELSE IF "%VSSTUDIO%" EQU "Microsoft Visual Studio 8" (
	rem Visual Studio 2005
	set VS_SHORT=vc8
	set VS_CMAKE=Visual Studio 8 2005
	set VS_PROG_FILES=Microsoft Visual Studio 8.0
	set VS_OPENSSL=vs2005
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
	set VS_CMAKE=%VS_CMAKE% Win64
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
call:echo testing that _CYGWIN_ exists
if NOT EXIST %_CYGWIN_% (
	echo "*** _CYGWIN_ %_CYGWIN_% does not exist ***"
	GOTO error_end
)

rem  ----
call:echo testing cygwin tools are available
IF NOT EXIST %_CYGWIN_%\bin\cp.exe       GOTO cygwin_error
IF NOT EXIST %_CYGWIN_%\bin\tar.exe      GOTO cygwin_error
GOTO cygwin_ok

:cygwin_error
echo "*** Cygwin with bin/cp and bin/tar are required ***"
GOTO error_end

:cygwin_ok
pushd %_CYGWIN_%
set _CYGWIN_=%CD%
popd

rem  ----
call:echo testing temporary directory
IF NOT EXIST "%_TEMP_%" mkdir "%_TEMP_%"
pushd        "%_TEMP_%"
set    "_TEMP_=%CD%"
popd
call:echo _TEMP_ = %_TEMP_% 

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

rem  ----
echo.
echo config = %_CONFIG_% webready = %_WEBREADY_% _EXIV2_= %_EXIV2_%"

IF DEFINED _DRYRUN_ exit /b 1

echo ---------- building ZLIB ------------------
call:buildLib zlib-1.2.8

echo ---------- building EXPAT -----------------
set "TARGET=--target expat"
call:buildLib expat-2.1.0
set  TARGET=

if DEFINED _WEBREADY_ (
	echo ---------- building LIBSSH -----------------
	set _SSH_=-DEXIV2_ENABLE_SSH=ON
	call:buildLib libssh-0.5.5
	if errorlevel 1 set _SSH_=-DEXIV2_ENABLE_SSH=OFF

	echo ---------- building OPENSSL -----------------
	call:buildLib openssl-1.0.1j

	echo ---------- building CURL -----------------
	set _CURL_=-DEXIV2_ENABLE_CURL=ON
	call:buildLib curl-7.39.0
	if errorlevel 1 set _CURL_=-DEXIV2_ENABLE_CURL=OFF
	
	set _WEBREADY_=-DEXIV2_ENABLE_WEBREADY=ON
) else (
	set _WEBREADY_=-DEXIV2_ENABLE_WEBREADY=OFF
	set _CURL_=-DEXIV2_ENABLE_CURL=OFF
	set _SSH_=-DEXIV2_ENABLE_SSH=OFF
)

echo ---------- building EXIV2 ------------------

set EXIV_BUILD="%_TEMP_%\exiv2"

if defined _REBUILD_ "%EXIV_BUILD%"
IF NOT EXIST         "%EXIV_BUILD%" mkdir "%EXIV_BUILD%"
pushd "%EXIV_BUILD%"
	call:run cmake -G "%_GENERATOR_%" ^
	         "-DCMAKE_INSTALL_PREFIX=%_INSTALL_%"      "-DCMAKE_PROGRAM_PATH=%SVN_DIR%" ^
	         "-DCMAKE_LIBRARY_PATH=%INSTALL_PATH%\lib" "-DCMAKE_INCLUDE_PATH=%INSTALL_PATH%\include" ^
	          -DEXIV2_ENABLE_NLS=OFF                    -DEXIV2_ENABLE_BUILD_SAMPLES=ON ^
	          -DEXIV2_ENABLE_WIN_UNICODE=OFF            -DEXIV2_ENABLE_SHARED=ON ^
	          %_WEBREADY_%  %_CURL_%  %_SSH_% ^
	         "%_EXIV2_%"

	IF errorlevel 1 (
		echo "*** errors in EXIV2 build [1] ***"
	    popd
		goto error_end
	)

	call:run cmake --build . --config %_CONFIG_%
	IF errorlevel 1 (
		echo "*** errors in EXIV2 build [2] ***"
	    popd
		goto error_end
	)

	call:run cmake --build . --config %_CONFIG_% --target install
	IF errorlevel 1 (
		echo "*** errors in EXIV2 build [3] ***"
	    popd
		goto error_end
	)
	copy/y "samples\%_CONFIG_%\"*.exe "%_INSTALL_%\bin"
popd

rem -----------------------------------------
rem Exit
:error_end
endlocal
REM pause
exit /b 1

:end
endlocal
exit /b 0

rem -----------------------------------------
rem Functions
:help
echo Options: --help ^| --webready ^| --dryrun ^| --verbose ^| --rebuild ^| --silent ^| --verbose
echo.         --exiv2 directory ^| --cygwin directory ^| --temp directory ^| --config name ^| --generator generator
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
exit /b %ERRORLEVEL%



rem -------------------------------------------------------------
:buildLib
cd "%_BUILDDIR_%"

set LIB=%1%
set LIB_T=%LIB%.tar
set LIB_Z=%LIB%.tar.Z
set LIB_B="%_TEMP_%\%LIB%"

if defined _REBUILD_ del "%LIB_Z%"
IF NOT EXIST             "%LIB_Z%" (
	call:run svn export svn://dev.exiv2.org/svn/team/libraries/%LIB_Z%
)

if defined _REBUILD_ rmdir/s/q  "%LIB%"
IF NOT EXIST                    "%LIB%" "%_CYGWIN_%\bin\tar.exe" xzf "%LIB_Z%"

if defined _REBUILD_  rmdir/s/q "%LIB_B%"
IF NOT EXIST %LIB_B%  mkdir     "%LIB_B%"

    pushd "%LIB_B%"

    call:run cmake -G "%_GENERATOR_%" -DCMAKE_INSTALL_PREFIX=%_INSTALL_% ..\..\%LIB%
	IF errorlevel 1 (
		echo "*** errors in %LIB% build [1] ***"
	    popd
		exit /b 1
	)

	call:run cmake --build . --config %_CONFIG_% %TARGET%
	IF errorlevel 1 (
		echo "*** warning: errors in %LIB% build [2] ***"
	    popd
		exit /b 0
	)

	call:run cmake --build . --config %_CONFIG_% --target install
	IF errorlevel 1 (
		echo "*** warning: errors in %LIB% build [3] ***"
	    popd
		exit /b 0
	)
    
    popd

exit /b 0

rem That's all Folks!
rem -------------------------------------------------------------
