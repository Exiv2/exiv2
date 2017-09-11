@echo off
setlocal enableextensions

rem  ----
rem always run this script in <exiv2dir>/contrib/cmake/msvc
cd %~dp0

set "_BUILDDIR_=%CD%"
call:echo calling cmakeDefaults.cmd
call cmakeDefaults.cmd
IF ERRORLEVEL 1 (
    echo "*** cmakeDefaults.cmd has failed ***" >&2
    GOTO error_end
)

:GETOPTS
set _CLAIMED_=0
if /I "%1" == "--bash"            call:set "_BASH_=%2"& shift
if /I "%1" == "--config"          call:set "_CONFIG_=%2"& shift
if /I "%1" == "--curl"            call:set "_CURL_=%2"& shift
if /I "%1" == "--exiv2"           call:set "_EXIV2_=%2"& shift
if /I "%1" == "--expat"           call:set "_EXPAT_=%2"& shift
if /I "%1" == "--generator"       call:set "_GENERATOR_=%2"& shift
if /I "%1" == "--openssl"         call:set "_OPENSSL_=%2"& shift
if /I "%1" == "--libssh"          call:set "_LIBSSH_=%2"& shift
if /I "%1" == "--work"            call:set "_WORK_=%2"& shift
if /I "%1" == "--zlib"            call:set "_ZLIB_=%2"& shift

if /I "%1" == "--help"            call:Help && goto end
if /I "%1" == "--dryrun"          call:set "_DRYRUN_=1"
if /I "%1" == "--build"           call:set "_BUILD_=1"
if /I "%1" == "--samples"         call:set "_SAMPLES_=1"
if /I "%1" == "--pause"           call:set "_PAUSE_=1"
if /I "%1" == "--silent"          call:set "_SILENT_=1"
if /I "%1" == "--static"          call:set "_SHARED_=0"
if /I "%1" == "--shared"          call:set "_SHARED_=1"
if /I "%1" == "--dll"             call:set "_SHARED_=1"
if /I "%1" == "--test"            call:set "_TEST_=1"
if /I "%1" == "--trace"           call:set "_TRACE_=1"
if /I "%1" == "--verbose"         call:set "_VERBOSE_=1"
if /I "%1" == "--video"           call:set "_VIDEO_=1"
if /I "%1" == "--webready"        call:set "_WEBREADY_=1"
if /I "%1" == "--unicode"         call:set "_UNICODE_=1"
if /I "%1" == "--ascii"           call:set "_UNICODE_=0"
if /I "%1" == "--nls"             call:set "_NLS_=1"
if /I "%1" == "--debug"           call:set "_CONFIG_=Debug"
if /I "%1" == "--release"         call:set "_CONFIG_=Release"

if /I "%_CLAIMED_%" == "0" (
	echo "*** unknown command argument %1 ***" >&2
	goto error_end
)

shift
if not (%1) EQU () goto GETOPTS
goto main

:set
set %1
set _CLAIMED_=1
exit /b 0

:help
echo Options: --help   ^| --silent ^| --verbose  ^| --pause   ^| --dryrun   ^| --trace ^| --test
echo.         --build  ^| --static ^| --unicode  ^| --nls     ^| --webready ^| --video ^| --samples 
echo.         --exiv2 %_EXIV2_% ^| --work %_WORK_% ^| --config %_CONFIG_% ^| --generator generator
echo.         --zlib %_ZLIB_% ^| --expat %_EXPAT_% ^| --curl %_CURL_% ^| --libssh %_LIBSSH_%
echo.         --bash %_BASH_%
exit /b 0

:report
echo.&&echo.&&echo.
echo.------ cmakeBuild Settings ----------
echo.bash      = %_BASH_%
echo.binpath   = %_BINPATH_%
echo.build     = %_BUILD_%
echo.builddir  = %_BUILDDIR_%
echo.config    = %_CONFIG_%
echo.curl      = %_CURL_%
echo.dryrun    = %_DRYRUN_%
echo.exiv2     = %_EXIV2_%
echo.expat     = %_EXPAT_%
echo.generator = %_GENERATOR_%
echo.install   = %_INSTALL_%
echo.incpath   = %_INCPATH_%
echo.libpath   = %_LIBPATH_%
echo.libssh    = %_LIBSSH_%
echo.nls       = %_NLS_%
echo.openssl   = %_OPENSSL_%
echo.platform  = %PLATFORM%
echo.samples   = %_SAMPLES_%
echo.shared    = %_SHARED_%
echo.test      = %_TEST_%
echo.unicode   = %_UNICODE_%
echo.vc        = %_VC_%
echo.video     = %_VIDEO_%
echo.vs        = %_VS_%
echo.webready  = %_WEBREADY_%
echo.work      = %_WORK_%
echo.zlib      = %_ZLIB_%
echo.&&echo.&&echo.
exit /b 0

:main
call:report
if NOT DEFINED _SILENT_ set _VERBOSE_=1
set _UNSUPPORTED_=

rem  ----
call:echo _EXIV2_ = %_EXIV2_%

rem  ----
call:echo testing VSINSTALLDIR "%VSINSTALLDIR%"
IF NOT DEFINED VSINSTALLDIR (
    echo "VSINSTALLDIR not set.  Run vcvars32.bat or vcvarsall.bat or vcvars.bat ***"
    GOTO error_end
)
IF NOT EXIST "%VSINSTALLDIR%" (
    echo "VSINSTALLDIR %VSINSTALLDIR% does not exist.  Run vcvars32.bat or vcvarsall.bat ***"
    GOTO error_end
)

if /I "%VSINSTALLDIR%" == "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\" set "_VS_=2017" && set "_VC_=15"
if /I "%VSINSTALLDIR%" == "%ProgramFiles(x86)%\Microsoft Visual Studio 14.0\" set "_VS_=2015" && set "_VC_=14"
if /I "%VSINSTALLDIR%" == "%ProgramFiles(x86)%\Microsoft Visual Studio 12.0\" set "_VS_=2013" && set "_VC_=12"
if /I "%VSINSTALLDIR%" == "%ProgramFiles(x86)%\Microsoft Visual Studio 11.0\" set "_VS_=2012" && set "_VC_=11"
if /I "%VSINSTALLDIR%" == "%ProgramFiles(x86)%\Microsoft Visual Studio 10.0\" set "_VS_=2010" && set "_VC_=10"
if /I "%VSINSTALLDIR%" == "%ProgramFiles(x86)%\Microsoft Visual Studio 9.0"   set "_VS_=2008" && set "_VC_=9"
if /I "%VSINSTALLDIR%" == "%ProgramFiles(x86)%\Microsoft Visual Studio 8"     set "_VS_=2005" && set "_VC_=8"

if NOT DEFINED _VC_ set _VS_=
if NOT DEFINED _VS_ (
    echo "*** Unsupported version of Visual Studio in '%VSINSTALLDIR%' ***"
    GOTO error_end
)

call:echo testing architecture
set _ARCH_=64
if /I "%PROCESSOR_ARCHITECTURE%" == "x86" set _ARCH_=32
if /I "%VSCMD_ARG_HOST_ARCH%"    == "x86" set _ARCH_=32

if /I "%_ARCH_%" == "32" (
    set Platform=Win32
    set RawPlatform=x86
    set CpuPlatform=ia32
)

if NOT DEFINED Platform (
    set Platform=x64
    set RawPlatform=x64
    set CpuPlatform=intel64
)
call:echo Platform = %Platform% (%RawPlatform%)

call:echo determine generator
if NOT DEFINED _GENERATOR_ (
    set   "_GENERATOR_=Visual Studio %_VC_% %_VS_%"
    IF /I "%Platform%" == "x64" set "_GENERATOR_=Visual Studio %_VC_% %_VS_% Win64"
)
call:echo GENERATOR = %_GENERATOR_%

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
call:echo fix ups
set _WORK_=%_WORK_%_%_CONFIG_%
if DEFINED _TRACE_ (
    set _VERBOSE_=1
    echo on
)
if DEFINED _TEST_ (
    set _SAMPLES_=1
    set _BUILD_=1
)
if /I "%_SHARED_%" == "1" set _MODE_=dll
if /I "%_SHARED_%" == "0" set _MODE_=static

rem  ----
call:echo testing work directory _WORK_ = %_WORK_%
if EXIST "%_WORK_%" rmdir/s/q "%_WORK_%"
del/s CMakeCache.txt >NUL 2>NUL
IF NOT EXIST "%_WORK_%" mkdir "%_WORK_%"
pushd        "%_WORK_%"
set           "_WORK_=%CD%"
popd
call:echo      _WORK_ = %_WORK_%

rem ----
call:echo testing INSTALL
SET _INSTALL_=dist\%_VS_%\%Platform%\%_MODE_%\%_CONFIG_%
if NOT EXIST %_INSTALL_% mkdir %_INSTALL_%
pushd        %_INSTALL_%
    set      "_INSTALL_=%CD%"
popd
call:echo     _INSTALL_ = %_INSTALL_%

set "_LIBPATH_=%_INSTALL_%\bin"
set "_INCPATH_=%_INSTALL_%\include"
set "_BINPATH_=%_INSTALL_%\bin"
set "_ONCPATH_=%_INCPATH_%"
set  _LIBPATH_=%_LIBPATH_:\=/%
set  _INCPATH_=%_INCPATH_:\=/%
set  _BINPATH_=%_BINPATH_:\=/%

if defined _TEST_ if NOT EXIST "%_BASH_%" (
    echo "*** bash does not exist %_BASH_% ***"
    GOTO error_end
)

if NOT DEFINED _GENERATOR_       set "_GENERATOR_=%VS_CMAKE%"
if /I "%_GENERATOR_%" == "NMake" set "_GENERATOR_=NMake Makefiles"
if /I "%_SHARED_%" == "0"        set _LINK_="-DCMAKE_LINK=static"

rem Fix up for openssl/vs 2017
@echo on
if /I "%_OPENSSL_%" == "openssl-1.0.1p" if /I "%_VS_%" == 2017 set _OPENSSL_ = openssl-1.1.0f

call:cltest
call:report

IF DEFINED _DRYRUN_  goto end
IF DEFINED _PAUSE_   pause

echo ---------- ZLIB building with cmake ------------------
call:buildLib %_ZLIB_% -DCMAKE_INSTALL_PREFIX=%_INSTALL_%

echo ---------- EXPAT building with cmake -----------------
set "_TARGET_=--target expat"
if /I "%_SHARED_%" == "0" (
  call:buildLib %_EXPAT_% -DCMAKE_INSTALL_PREFIX=%_INSTALL_% -DBUILD_shared=0 -DCMAKE_C_FLAGS_RELEASE=/MT -DCMAKE_C_FLAGS_DEBUG=/MTd -DBUILD_examples=0 -DBUILD_tests=0
) else (
  call:buildLib %_EXPAT_% -DCMAKE_INSTALL_PREFIX=%_INSTALL_% -DBUILD_examples=0 -DBUILD_tests=0
)
set  _TARGET_=

if DEFINED _WEBREADY_ (
    echo ---------- OPENSSL installing pre-built binaries -----------------
    call:getOPENSSL %_OPENSSL_%
    if errorlevel 1 set _OPENSSL_= && set _WEBREADY_=
)

if DEFINED _WEBREADY_ (
    echo ---------- LIBSSH building with cmake -----------------
    call:buildLib   %_LIBSSH_% -DCMAKE_INSTALL_PREFIX=%_INSTALL_% -DCMAKE_LIBRARY_PATH=%_LIBPATH_% -DCMAKE_INCLUDE_PATH=%_INCPATH_% -DWITH_GSSAPI=OFF -DWITH_ZLIB=ON -DWITH_SFTP=ON -DWITH_SERVER=OFF -DWITH_EXAMPLES=OFF -DWITH_NACL=OFF -DWITH_PCAP=OFF
    if errorlevel 1 set _LIBSSH_= && set _WEBREADY_=
)

if DEFINED _WEBREADY_ (
    set        CURL_CMAKE=
    if DEFINED CURL_CMAKE (
        echo ---------- CURL building with cmake -----------------
        call:buildLib   %_CURL_% -DCMAKE_INSTALL_PREFIX=%_INSTALL_% -DCMAKE_LIBRARY_PATH=%_LIBPATH_% -DCMAKE_INCLUDE_PATH=%_INCPATH_% -DWITH_GSSAPI=OFF -DWITH_ZLIB=OFF -DWITH_SFTP=OFF -DWITH_SERVER=OFF -DWITH_EXAMPLES=OFF -DWITH_NACL=OFF -DWITH_PCAP=OFF -DCMAKE_USE_LIBSSH2=OFF -DCMAKE_USE_LIBSSH=OFF
        if errorlevel 1 set _WEBREADY_=
    ) ELSE (
        rmdir/s/q "%_ONCPATH_%\curl" >NUL 2>NUL
        if NOT EXIST "%_ONCPATH_%"\curl (
            echo ---------- CURL building with nmake -----------------
            IF NOT EXIST %_CURL_%.tar.gz  svn export svn://dev.exiv2.org/svn/team/libraries/%_CURL_%.tar.gz >NUL
            pushd  "%_WORK_%"

            IF     EXIST %_CURL_%         rmdir/s/q  %_CURL_%
            IF NOT EXIST %_CURL_%.tar.gz  copy "%_BUILDDIR_%\%_CURL_%.tar.gz" >NUL
            IF NOT EXIST %_CURL_%.tar     7z x %_CURL_%.tar.gz
            7z x         %_CURL_%.tar

            cd "%_CURL_%\winbuild"
            call:run nmake /f Makefile.vc mode=%_MODE_% vc=%_VC_% machine=%RawPlatform% "WITH_DEVEL=%_INSTALL_%" WITH_ZLIB=%_MODE_% ENABLE_WINSSL=yes ENABLE_IDN=no
            if errorlevel 1 set _CURL_= && set _WEBREADY_=
            if DEFINED _WEBREADY_ (
                cd ..
                call:run copy         builds\libcurl-vc%_VC_%-%RawPlatform%-release-%_MODE_%-zlib-%_MODE_%-ipv6-sspi-winssl\lib\*        "%_LIBPATH_%"
                call:run copy         builds\libcurl-vc%_VC_%-%RawPlatform%-release-%_MODE_%-zlib-%_MODE_%-ipv6-sspi-winssl\bin\*        "%_BINPATH_%"
                call:run xcopy/yesihq builds\libcurl-vc%_VC_%-%RawPlatform%-release-%_MODE_%-zlib-%_MODE_%-ipv6-sspi-winssl\include\curl "%_ONCPATH_%"\curl
            )
            popd
        )
    )
)

if NOT DEFINED _WEBREADY_ set _CURL_= && set _LIBSSH_=

echo ---------- EXIV2 building with cmake ------------------
set          "EXIV_B=%_WORK_%\exiv2"
IF EXIST     "%EXIV_B%"  rmdir/s/q "%EXIV_B%"
IF NOT EXIST "%EXIV_B%"  mkdir     "%EXIV_B%"

pushd        "%EXIV_B%"
    set ENABLE_CURL=-DEXIV2_ENABLE_CURL=OFF
    set ENABLE_LIBSSH=-DEXIV2_ENABLE_SSH=OFF
    set ENABLE_WEBREADY=-DEXIV2_ENABLE_WEBREADY=OFF
    set ENABLE_VIDEO=-DEXIV2_ENABLE_VIDEO=OFF
	set ENABLE_SHARED=ON
	set ENABLE_DYNAMIC=ON

    if defined _CURL_     set ENABLE_CURL=-DEXIV2_ENABLE_CURL=ON
    if defined _LIBSSH_   set ENABLE_SSH=-DEXIV2_ENABLE_SSH=ON
    if defined _WEBREADY_ set ENABLE_WEBREADY=-DEXIV2_ENABLE_WEBREADY=ON
    if defined _VIDEO_    set ENABLE_VIDEO=-DEXIV2_ENABLE_VIDEO=ON
	if /I "%_MODE_%" == "static" (
	    set ENABLE_SHARED=OFF
	    set ENABLE_DYNAMIC=OFF
	)
	                      set BUILD_SAMPLES=OFF
	if DEFINED _SAMPLES_  set BUILD_SAMPLES=ON

    call:run cmake -G "%_GENERATOR_%"     -DCMAKE_BUILD_TYPE=%_CONFIG_% %_LINK_% -DCMAKE_INSTALL_PREFIX=%_INSTALL_% -DCMAKE_LIBRARY_PATH=%_LIBPATH_% -DCMAKE_INCLUDE_PATH=%_INCPATH_% ^
              -DEXIV2_ENABLE_NLS=%_NLS_%  -DEXIV2_ENABLE_BUILD_SAMPLES=%BUILD_SAMPLES% ^
              -DEXIV2_ENABLE_WIN_UNICODE=%_UNICODE_% -DBUILD_SHARED_LIBS=%ENABLE_SHARED% ^
              -DEXIV2_ENABLE_DYNAMIC_RUNTIME=%ENABLE_DYNAMIC% ^
              %ENABLE_WEBREADY%  %ENABLE_CURL%  %ENABLE_LIBSSH% %ENABLE_VIDEO% ^
             "%_EXIV2_%"

    IF errorlevel 1 (
        echo "*** cmake errors in EXIV2 ***" >&2
        popd
        goto error_end
    )

    rem if DEFINED _BUILDX_ devenv %_WORK_%\exiv2\exiv2.sln /Build "%_CONFIG_%|%Platform%" /ProjectConfig INSTALL

    if DEFINED _BUILD_ call:run cmake --build . --config %_CONFIG_%
    IF errorlevel 1 (
        echo "*** build errors in EXIV2 ***" >&2
        popd
        goto error_end
    )

    if DEFINED _BUILD_ call:run cmake --build . --config %_CONFIG_% --target install
    IF errorlevel 1 (
        echo "*** install errors in EXIV2 ***" >&2
        popd
        goto error_end
    )
popd

if defined _TEST_ (
    pushd "%_EXIV2_%\test"
    "%_BASH_%" -c "export 'PATH=/usr/bin:$PATH' ; ./testMSVC.sh $(cygpath -au '%_BINPATH_%')"
    popd
    exit /b 0
)

rem -----------------------------------------
rem Exit
rem end  syntax: GOTO end
:end
endlocal
exit /b 0

rem end with an error syntax: GOTO error_end
:error_end
endlocal
exit /b 1

rem -----------------------------------------
rem Functions
rem echo (or don't if --silent).  syntax: call:echo args ...
:echo
if NOT DEFINED _SILENT_ echo %*%
exit /b 0

rem run a command. syntax call:run args
:run
if defined _VERBOSE_ (
    echo.
    echo ++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    echo CD = %CD%
    echo %*%
    if     DEFINED _PAUSE_ pause
)
if     defined _SILENT_ %*% >nul 2>nul
if NOT defined _SILENT_ %*%

set _RESULT_=%ERRORLEVEL%
if     DEFINED _PAUSE_ pause
exit /b %_RESULT_%

rem -----------------------------------------
rem build a library with CMake.  syntax: call:buildLib name cmake-args ...
:buildLib
cd  "%_BUILDDIR_%"
set "LOB=%1"
shift

set "LOB_B=%_WORK_%\%LOB%"
set "LOB_TAR=%LOB%.tar"
set "LOB_TAR_GZ=%LOB_TAR%.gz"

IF NOT EXIST "%LOB_TAR_GZ%"  svn export svn://dev.exiv2.org/svn/team/libraries/%LOB_TAR_GZ% >NUL
IF NOT EXIST "%LOB_TAR%"     7z x "%LOB_TAR_GZ%"
IF NOT EXIST "%LOB%"         7z x "%LOB_TAR%"
if NOT EXIST "%LOB_B%"       mkdir "%LOB_B%"

pushd "%LOB_B%"
    call:run cmake -G "%_GENERATOR_%" -DCMAKE_BUILD_TYPE=%_CONFIG_% %_LINK_% %* ..\..\%LOB%
    IF errorlevel 1 (
        echo "*** cmake errors in %LOB% ***"
        popd
        exit /b 1
    )

    call:run cmake --build . --config %_CONFIG_% %_TARGET_%
    IF errorlevel 1 (
        echo "*** warning: build errors in %LOB% ***"
    )

    rem --static expat fails to install on VS 2010+
    set buildLibInstallExpatStatic=0
    if /I "%_TARGET_%" == "--target expat" if /I "%_MODE_%" == "static" set buildLibInstallExpatStatic=1

    if /I "%buildLibInstallExpatStatic%" == "1" (
        rem msvc\expat-2.1.0\lib\expat*.h                => msvc\dist\2005\x64\static\Release\include
        call:run copy/y "%_BUILDDIR_%\%LOB%\lib\expat*.h"   "%_ONCPATH_%"

        rem msvc\work\expat-2.1.0\release\*.lib          => msvc\dist\2005\x64\static\Release\lib
        call:run copy/y "%_WORK_%\%LOB%\%_CONFIG_%\*.lib"   "%_INSTALL_%\lib"
    ) else (
        call:run cmake --build . --config %_CONFIG_% --target install
    )

    IF errorlevel 1 (
        echo "*** warning: install errors in %LOB% ***"
    )
popd
exit /b 0

rem -----------------------------------------
rem get pre-built openssl binaries syntax: call:getOPENSSL version
:getOPENSSL
cd  "%_BUILDDIR_%"

set "LOB=%1-vs%_VS_%"
set "LOB_7Z=%LOB%.7z"

IF NOT EXIST "%LOB_7Z%"      svn export svn://dev.exiv2.org/svn/team/libraries/%LOB_7Z% >NUL
IF NOT EXIST "%LOB%"         7z x      "%LOB_7Z%" >nul

set _BINARY_=bin
set _LIBRARY_=lib
set _INCLUDE_=include
if /I "%Platform%" == "x64" (
    set "_BINARY_=bin64"
    set "_LIBRARY_=lib64"
    set "_INCLUDE_=include64"
)

xcopy/yesihq "%LOB%\%_BINARY_%"  "%_INSTALL_%\bin"
xcopy/yesihq "%LOB%\%_LIBRARY_%" "%_INSTALL_%\lib"
xcopy/yesihq "%LOB%\%_INCLUDE_%" "%_INSTALL_%\include"

rem curl requires libeay32 and ssleay32 (and not libeay32MD and ssleay32MD)
pushd "%_INSTALL_%\lib"
    copy libeay32MD.lib  libeay32.lib
    copy ssleay32MD.lib  ssleay32.lib
popd
pushd "%_INSTALL_%\bin"
    copy libeay32MD.dll  libeay32.dll
    copy ssleay32MD.dll  ssleay32.dll
popd

exit /b 0

rem -----------------------------------------
rem this runs the compiler and reports _MSC_VER and sizeof(void*)
:cltest
pushd    "%_EXIV2_%\contrib\cmake\msvc"
nmake -a cltest.exe
cltest.exe
popd
exit /b %ERRORLEVEL%

rem That's all Folks!
rem -----------------------------------------
