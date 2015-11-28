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
if /I "%1" == "--test"            set "_TEST_=1"
if /I "%1" == "--bash"            set "_BASH_=%2"& shift

shift
if not (%1) EQU () goto GETOPTS

set _VERBOSE_=1

rem  ----
call:echo calling cmakeDefaults.cmd
call cmakeDefaults
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
if /I "%VSINSTALLDIR%" == "C:\Program Files (x86)\Microsoft Visual Studio 14.0\" set "VS_PROG_FILES=Microsoft Visual Studio 14"
if /I "%VSINSTALLDIR%" == "C:\Program Files (x86)\Microsoft Visual Studio 12.0\" set "VS_PROG_FILES=Microsoft Visual Studio 12"
if /I "%VSINSTALLDIR%" == "C:\Program Files (x86)\Microsoft Visual Studio 11.0\" set "VS_PROG_FILES=Microsoft Visual Studio 11"
if /I "%VSINSTALLDIR%" == "C:\Program Files (x86)\Microsoft Visual Studio 10.0\" set "VS_PROG_FILES=Microsoft Visual Studio 10"
if /I "%VSINSTALLDIR%" == "C:\Program Files (x86)\Microsoft Visual Studio 9.0\"  set "VS_PROG_FILES=Microsoft Visual Studio 9"
if /I "%VSINSTALLDIR%" == "C:\Program Files (x86)\Microsoft Visual Studio 8.0\"  set "VS_PROG_FILES=Microsoft Visual Studio 8"
call:echo VS_PROG_FILES = "%VS_PROG_FILES%"

rem  ----
call:echo setting CMake Generator
if        /I "%VS_PROG_FILES%" == "Microsoft Visual Studio 14" (
        set   "VS_CMAKE=Visual Studio 14 2015"
        set   "VS_OPENSSL=vs2015"
) else if /I "%VS_PROG_FILES%" == "Microsoft Visual Studio 12" (
        set   "VS_CMAKE=Visual Studio 12 2013"
        set   "VS_OPENSSL=vs2013"
) else if /I "%VS_PROG_FILES%" == "Microsoft Visual Studio 11" (
        set   "VS_CMAKE=Visual Studio 11 2012"
        set   "VS_OPENSSL=vs2012"
) else if /I "%VS_PROG_FILES%" == "Microsoft Visual Studio 10" (
        set   "VS_CMAKE=Visual Studio 10 2010"
        set   "VS_OPENSSL=vs2010"
) else if /I "%VS_PROG_FILES%" == "Microsoft Visual Studio 9"  (
        set   "VS_CMAKE=Visual Studio 9 2008"
        set   "VS_OPENSSL=vs2008"
) else if /I "%VS_PROG_FILES%" == "Microsoft Visual Studio 8"  (
        set   "VS_CMAKE=Visual Studio 8 2005"
        set   "VS_OPENSSL=vs2005"
) else (
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
if NOT DEFINED _SILENT_ cl

rem  ----
call:echo testing svn is on path
svn --version > NUL
IF ERRORLEVEL 1 (
	echo "*** please ensure svn.exe is on the PATH ***"
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
set "_LIBPATH_=%_INSTALL_%\bin"
set "_INCPATH_=%_INSTALL_%\include"
set "_BINPATH_=%_INSTALL_%\bin"
set  _LIBPATH_=%_LIBPATH_:\=/%
set  _INCPATH_=%_INCPATH_:\=/%
set  _BINPATH_=%_BINPATH_:\=/%

if defined _TEST_ if NOT EXIST "%_BASH_%" (
	echo "*** bash does not exist %_BASH_% ***"
	GOTO error_end
)

if NOT DEFINED _GENERATOR_       set "_GENERATOR_=%VS_CMAKE%"
if /I "%_GENERATOR_%" == "NMake" set "_GENERATOR_=NMake Makefiles"

if defined _VIDEO_ set _VIDEO_=-DEXIV2_ENABLE_VIDEO=ON

rem  ------
echo.
echo.config    = %_CONFIG_%
echo.video     = %_VIDEO_%
echo.webready  = %_WEBREADY_%
echo.exiv2     = %_EXIV2_%
echo.generator = %_GENERATOR_%
echo.expat     = %_EXPAT_%
echo.zlib      = %_ZLIB_%
echo.libssh    = %_LIBSSH_%
echo.curl      = %_CURL_%
echo.openssh   = %_OPENSSL_%
echo.libpat    = %_LIBPATH_%
echo.incpat    = %_INCPATH_%
echo.binpat    = %_BINPATH_%
echo.test      = %_TEST_%
echo.bash      = %_BASH_%
echo.

IF DEFINED _DRYRUN_  exit /b 1
IF DEFINED _REBUILD_ rmdir/s/q "%_TEMP_%"

echo ---------- ZLIB building with cmake ------------------
call:buildLib %_ZLIB_%

echo ---------- EXPAT building with cmake -----------------
set "TARGET=--target expat"
call:buildLib %_EXPAT_%
set  TARGET=


if DEFINED _WEBREADY_ (
	echo ---------- OPENSSL installing pre-built binaries -----------------
	call:getOPENSSL %_OPENSSL_%
	if errorlevel 1 set _OPENSSL_=

	echo ---------- LIBSSH building with cmake -----------------
	call:buildLib   %_LIBSSH_% -DWITH_GSSAPI=OFF -DWITH_ZLIB=ON -DWITH_SFTP=ON -DWITH_SERVER=OFF -DWITH_EXAMPLES=OFF -DWITH_NACL=OFF -DWITH_PCAP=OFF
	if errorlevel 1 set _LIBSSH_=

	echo ---------- CURL building with cmake -----------------
	call:buildLib   %_CURL_% -DBUILD_CURL_TESTS=OFF -DCMAKE_USE_OPENSSL=ON -DCMAKE_USE_LIBSSH2=OFF
	if errorlevel 1 set _CURL_=

	rem echo ---------- CURL building with nmake -----------------
	rem if     exist %_CURL_%         rmdir/s/q %_CURL_% 
	rem IF NOT EXIST %_CURL_%.tar.gz  svn export svn://dev.exiv2.org/svn/team/libraries/%_CURL_%.tar.gz >NUL
	rem IF NOT EXIST %_CURL_%.tar     7z x %_CURL_%.tar.gz
    rem 7z x %_CURL_%.tar

	rem cd %_CURL_%
	rem cd winbuild
	rem nmake /f Makefile.vc mode=dll VC=12 machine=x64
	rem cd ..
	rem xcopy/yesihq builds\libcurl-vc14-x64-release-dll-ipv6-sspi-winssl\lib "%_LIBPATH_%"  
	rem xcopy/yesihq builds\libcurl-vc14-x64-release-dll-ipv6-sspi-winssl\bin "%_BINPATH_%"  
	rem xcopy/yesihq builds\libcurl-vc14-x64-release-dll-ipv6-sspi-winssl\inc "%_INCPATH_%"
	rem cd ..
	
) else (
	set _WEBREADY_=
	set _CURL_=
	set _LIBSSH_=
)

echo ---------- EXIV2 building with cmake ------------------
set          "EXIV_B=%_TEMP_%\exiv2"

if defined _REBUILD_        rmdir/s/q "%EXIV_B%"
IF NOT EXIST "%EXIV_B%"     mkdir     "%EXIV_B%"
pushd        "%EXIV_B%"
	set ENABLE_CURL=-DEXIV2_ENABLE_CURL=OFF
	set ENABLE_LIBSSH=-DEXIV2_ENABLE_SSH=OFF
	set ENABLE_OPENSSL=-DEXIV2_ENABLE_WEBREADY=OFF
	set ENABLE_WEBREADY=-DEXIV2_ENABLE_VIDEO=OFF
	
	if defined _CURL_     set ENABLE_CURL=-DEXIV2_ENABLE_CURL=ON
	if defined _LIBSSH_   set ENABLE_LIBSSH=-DEXIV2_ENABLE_LIBSSH=ON
	if defined _WEBREADY_ set ENABLE_WEBREADY=-DEXIV2_ENABLE_WEBREADY=ON
	if defined _VIDEO_    set ENABLE_VIDEO=-DEXIV2_ENABLE_VIDEO=ON
	
	call:run cmake -G "%_GENERATOR_%" ^
	         "-DCMAKE_INSTALL_PREFIX=%_INSTALL_%"  "-DCMAKE_LIBRARY_PATH=%_LIBPATH_%" ^
	         "-DCMAKE_INCLUDE_PATH=%_INCPATH_%" ^
	          -DEXIV2_ENABLE_NLS=OFF                -DEXIV2_ENABLE_BUILD_SAMPLES=ON ^
	          -DEXIV2_ENABLE_WIN_UNICODE=OFF        -DEXIV2_ENABLE_SHARED=ON ^
	          %ENABLE_WEBREADY%  %ENABLE_CURL%  %ENABLE_LIBSSH% %ENABLE_VIDEO% ^
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

if defined _TEST_ (
	for /f "tokens=*" %%a in ('cygpath -au .') do set BUILDDIR=%%a
	pushd "%_EXIV2_%\test"
	"%_BASH_%" -c "export 'PATH=/usr/bin:$PATH' ; ./testMSVC.sh ${BUILDDIR}/dist/bin"	
	popd
	exit /b 0
)

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
echo Options: --help ^| --pause ^| --webready ^| --dryrun ^| --verbose ^| --rebuild ^| --silent ^| --verbose ^| --video ^| --test
echo.         --exiv2 directory ^| --temp directory ^| --config name ^| --generator generator
echo.         --zlib zlib.1.2.8 ^| --expat expat-2.1.0 ^| --curl curl-7.45.0 ^| --libssh libssh-0.7.2 ^| --openssl openssl-1.0.1p ^| --bash c:\cygwin64\bin\bash
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

rem -----------------------------------------
:buildLib
cd  "%_BUILDDIR_%"
set "LIB=%1"
shift

set "LIB_B=%_TEMP_%\%LIB%"
set "LIB_TAR=%LIB%.tar"
set "LIB_TAR_GZ=%LIB_TAR%.gz"

IF NOT EXIST "%LIB_TAR_GZ%"  svn export svn://dev.exiv2.org/svn/team/libraries/%LIB_TAR_GZ% >NUL
IF NOT EXIST "%LIB_TAR%"     7z x "%LIB_TAR_GZ%"
IF NOT EXIST "%LIB%"         7z x "%LIB_TAR%"
if NOT EXIST "%LIB_B%"       mkdir "%LIB_B%"

pushd "%LIB_B%"

    call:run cmake -G "%_GENERATOR_%" 	                    ^
                      "-DCMAKE_INSTALL_PREFIX=%_INSTALL_%"  ^
                      "-DCMAKE_LIBRARY_PATH=%_LIBPATH_%"    ^
                      "-DCMAKE_INCLUDE_PATH=%_INCPATH_%"    ^
                      %* ..\..\%LIB%
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

rem -----------------------------------------
:getOPENSSL
cd  "%_BUILDDIR_%"
set "LIB=%1-%VS_OPENSSL%"
set "LIB_7Z=%LIB%.7z"

IF NOT EXIST "%LIB_7Z%"      svn export svn://dev.exiv2.org/svn/team/libraries/%LIB_7Z% >NUL
IF NOT EXIST "%LIB%"         7z x      "%LIB_7Z%" >nul

set BINARY=bin
set LIBRARY=lib
set INCLUDE=include
if /I "%Platform%" == "x64" (
	set "BINARY=%BINARY%64"
	set "LIBRARY=%LIBRARY%64"
	set "INCLUDE=%INCLUDE%64"
)

xcopy/yesihq "%LIB%\%BINARY%"  "%_INSTALL_%\bin"
xcopy/yesihq "%LIB%\%LIBRARY%" "%_INSTALL_%\lib"
xcopy/yesihq "%LIB%\%INCLUDE%" "%_INSTALL_%\include"

exit /b 0
	
rem That's all Folks!
rem -----------------------------------------
