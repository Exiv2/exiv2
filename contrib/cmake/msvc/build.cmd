@echo off
SETLOCAL

REM ---------------------------------------------------
REM -- Created by danielkaneider
REM ---------------------------------------------------

echo calling setenv
call setenv.cmd
IF ERRORLEVEL 1 (
	echo "*** setenv.cmd has failed ***"
	GOTO error_end
)

IF EXIST ..\msvc (
	echo.
	echo.ERROR: This file should NOT be executed within the exiv2 source directory,
	echo.       but in a new empty folder!
	echo.
	goto error_end
)

echo testing compiler
cl > NUL
IF ERRORLEVEL 1 (
	echo "*** ensure cl is not on path.  Run vcvars32.bat or vcvarsall.bat ***"
	GOTO error_end
)

echo testing VSINSTALLDIR
IF NOT EXIST "%VSINSTALLDIR%" (
	echo "VSINSTALLDIR %VSINSTALLDIR% does not exist.  Ruv vcvars32.bat or vcvarsall.bat ***"
	GOTO error_end
)

echo testing architecture
if "%PROCESSOR_ARCHITECTURE%" EQU "AMD64" ( 
	set Platform=x64
	set RawPlatform=x64
	set CpuPlatform=intel64
) ELSE (
	set Platform=Win32
	set RawPlatform=x86
	set CpuPlatform=ia32
)

echo testing svn is on path
svn --version > NUL
IF ERRORLEVEL 1 (
	echo "*** please ensure SVN.exe is on the PATH ***"
	GOTO error_end
)

echo testing cmake is on path
cmake --version > NUL
IF ERRORLEVEL 1 (
	echo "*** please ensure cmake.exe is on the PATH ***"
	GOTO error_end
)

echo testing curl is on path
curl --version > NUL
IF ERRORLEVEL 1 (
	echo "*** please ensure cmake.exe is on the PATH ***"
	GOTO error_end
)

echo testing the cygwin_dir exists
if NOT EXIST %CYGWIN_DIR% (
	echo "*** CYGWIN_DIR %CYGWIN_DIR% does not exist ***"
	GOTO error_end
)

IF %Platform% EQU x64 (
	set VS_CMAKE=%VS_CMAKE% Win64
)

echo testing cygwin tools are available
SET 7Z_PATH=%CYGWIN_DIR%\lib\p7zip\7z.exe
IF NOT EXIST %CYGWIN_DIR%\lib\p7zip\7z.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\cp.exe       GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\gzip.exe     GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\mv.exe       GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\svn.exe      GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\tar.exe      GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\unzip.exe    GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\wget.exe     GOTO cygwin_error
SET PATH=%PATH%;%CYGWIN_DIR%\bin
GOTO cygwin_ok

:cygwin_error
echo ERROR: Cygwin with 
echo    7z
echo    cp
echo    gzip 
echo    mv
echo    svn 
echo    tar 
echo    unzip 
echo    wget
echo is required
GOTO error_end

:cygwin_ok


echo setting configuration and creating working directories
IF NOT DEFINED Configuration (
	set Configuration=Release
)

echo.
echo.--- %VS_CMAKE% ---
echo.Configuration = %Configuration%
echo.Platform = %Platform% (%RawPlatform%)
echo.

IF NOT EXIST %TEMP_DIR% (
	mkdir %TEMP_DIR%
)

IF NOT EXIST vcDlls (
	mkdir vcDlls
	robocopy "%vcinstalldir%redist\%RawPlatform%" vcDlls /MIR >nul
)

IF NOT EXIST vcDlls\selected (
	mkdir vcDlls\selected
	%CYGWIN_DIR%\bin\cp.exe vcDlls/**/vcomp* vcDlls/selected
	%CYGWIN_DIR%\bin\cp.exe vcDlls/**/msv*   vcDlls/selected
)

echo setting INSTALL_DIR
SET INSTALL_DIR=dist
IF NOT EXIST %INSTALL_DIR% (
	mkdir %INSTALL_DIR%
	pushd %INSTALL_DIR%
	set "INSTALL_DIR=%CD%"
	popd
)
echo INSTALL_DIR = %INSTALL_DIR%


rem ---- location of library code ---------
rem  https://github.com/madler/zlib/commits
SET ZLIB_COMMIT_LONG=50893291621658f355bc5b4d450a8d06a563053d

rem https://github.com/pol51/OpenSSL-CMake
SET SSL_COMMIT_LONG=595a9e4384e1280659080f7c7029e66544b772a8

rem https://github.com/bagder/curl
SET CURL_COMMIT_LONG=dd39a671019d713bd077be9eed511c2dc6013598

rem http://www.npcglib.org/~stathis/blog/precompiled-openssl/
SET OPENSSL_VERSION=openssl-1.0.2d

echo ---------- building ZLIB ------------------
cd "%BUILD_DIR%"

SET ZLIB_COMMIT=%ZLIB_COMMIT_LONG:~0,7%
IF NOT EXIST %TEMP_DIR%\zlib-%ZLIB_COMMIT%.zip (
	%CYGWIN_DIR%\bin\wget.exe -O %TEMP_DIR%/zlib-%ZLIB_COMMIT%.zip --no-check-certificate http://github.com/madler/zlib/zipball/%ZLIB_COMMIT_LONG%
)

IF NOT EXIST zlib-%ZLIB_COMMIT% (
	%CYGWIN_DIR%\bin\unzip.exe -q %TEMP_DIR%/zlib-%ZLIB_COMMIT%.zip
	%CYGWIN_DIR%\bin\mv.exe madler-zlib-* zlib-%ZLIB_COMMIT%
)

IF NOT EXIST zlib-%ZLIB_COMMIT%.build (
    mkdir zlib-%ZLIB_COMMIT%.build
    
    pushd zlib-%ZLIB_COMMIT%.build
	cmake -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR% ..\zlib-%ZLIB_COMMIT%
	IF errorlevel 1 (
		echo "*** errors in ZLIB build [1] ***"
	    popd
		goto error_end
	)

	cmake --build . --config %Configuration%
	IF errorlevel 1 (
		echo "*** errors in ZLIB build [2] ***"
	    popd
		goto error_end
	)

	cmake --build . --config %Configuration% --target install
	IF errorlevel 1 (
		echo "*** errors in ZLIB build [3] ***"
	    popd
		goto error_end
	)
    
    popd
)

echo ---------- building EXPAT ------------------
cd "%BUILD_DIR%"

IF NOT EXIST %TEMP_DIR%\expat-2.1.0.tar (
	if NOT EXIST %TEMP_DIR%\expat-2.1.0.tar.gz (
		%CYGWIN_DIR%\bin\wget.exe -O %TEMP_DIR%/expat-2.1.0.tar.gz http://sourceforge.net/projects/expat/files/expat/2.1.0/expat-2.1.0.tar.gz/download
	)
	%CYGWIN_DIR%\bin\gzip.exe -d %TEMP_DIR%/expat-2.1.0.tar.gz
)
IF NOT EXIST expat-2.1.0 (
	%CYGWIN_DIR%\bin\tar.exe -xf %TEMP_DIR%/expat-2.1.0.tar   
)

IF NOT EXIST expat-2.1.0.build (
    mkdir expat-2.1.0.build
)    
pushd expat-2.1.0.build

	cmake -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR% ..\expat-2.1.0
	IF errorlevel 1 (
		echo "*** errors in expat build [1] ***"
	    popd
		goto error_end
	)

	cmake --build . --config %Configuration%
	IF errorlevel 1 (
		echo "*** WARNING errors in expat build [2] ***"
	)

	cmake --build . --config %Configuration% --target install
	IF errorlevel 1 (
		echo "*** WARNING errors in expat build [3] ***"
	)
popd



rem SET OPENSSL_LONG=%OPENSSL_VERSION%-%VS_OPENSSL%
rem IF NOT EXIST %TEMP_DIR%\%OPENSSL_LONG%.7z (
rem 	%CYGWIN_DIR%\bin\wget.exe -O %TEMP_DIR%/%OPENSSL_LONG%.7z --no-check-certificate http://www.npcglib.org/~stathis/downloads/%OPENSSL_LONG%.7z
rem )
rem IF NOT EXIST %OPENSSL_LONG% (
rem 	%CYGWIN_DIR%\lib\p7zip\7z.exe x %TEMP_DIR%/%OPENSSL_LONG%.7z
rem 	IF %Platform% EQU x64 (
rem 		ren %OPENSSL_LONG%\lib xxlib
rem 		ren %OPENSSL_LONG%\lib64 lib
rem 		ren %OPENSSL_LONG%\bin xxbin
rem 		ren %OPENSSL_LONG%\bin64 bin
rem 	)
rem )
rem robocopy %OPENSSL_LONG%\bin %INSTALL_DIR%\bin libeay32MD.dll /MIR /NJS >nul
rem robocopy %OPENSSL_LONG%\bin %INSTALL_DIR%\bin ssleay32MD.dll /MIR /NJS >nul
rem 
rem 
rem SET CURL_COMMIT=%CURL_COMMIT_LONG:~0,7%
rem IF NOT EXIST %TEMP_DIR%\curl-%CURL_COMMIT%.zip (
rem 	%CYGWIN_DIR%\bin\wget.exe -O %TEMP_DIR%/curl-%CURL_COMMIT%.zip --no-check-certificate http://github.com/bagder/curl/zipball/%CURL_COMMIT_LONG%
rem )
rem 
rem IF NOT EXIST curl-%CURL_COMMIT% (
rem 	%CYGWIN_DIR%\bin\unzip.exe -q %TEMP_DIR%/curl-%CURL_COMMIT%.zip
rem 	%CYGWIN_DIR%\bin\mv.exe bagder-curl-* curl-%CURL_COMMIT%
rem )
rem 
rem IF NOT EXIST curl-%CURL_COMMIT%.build (
rem     mkdir curl-%CURL_COMMIT%.build
rem     
rem     pushd curl-%CURL_COMMIT%.build
rem 	cmake -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=..\%INSTALL_DIR% -DCMAKE_PREFIX_PATH=..\%OPENSSL_LONG% -DBUILD_CURL_TESTS=OFF -DCMAKE_USE_OPENSSL=ON -DCMAKE_USE_LIBSSH2=OFF ..\curl-%CURL_COMMIT%
rem 	IF errorlevel 1 goto error_end
rem 	cmake --build . --config %Configuration%
rem 	IF errorlevel 1 goto error_end
rem 	cmake --build . --config %Configuration% --target install
rem 	IF errorlevel 1 goto error_end
rem     
rem     popd
rem )
rem 
rem SET SSH_VERSION=0.7.2
rem IF NOT EXIST %TEMP_DIR%\libssh-%SSH_VERSION%.zip (
rem 	%CYGWIN_DIR%\bin\wget.exe -O %TEMP_DIR%/libssh-%SSH_VERSION%.zip --no-check-certificate https://git.libssh.org/projects/libssh.git/snapshot/libssh-%SSH_VERSION%.zip
rem )
rem IF NOT EXIST libssh-%SSH_VERSION% (
rem 	%CYGWIN_DIR%\bin\unzip.exe -q %TEMP_DIR%/libssh-%SSH_VERSION%.zip
rem )
rem IF NOT EXIST libssh-%SSH_VERSION%.build (
rem     mkdir libssh-%SSH_VERSION%.build
rem     
rem     pushd libssh-%SSH_VERSION%.build
rem 	
rem cmake -G "%VS_CMAKE%" -DWITH_GSSAPI=OFF -DWITH_ZLIB=ON -DWITH_SFTP=ON -DWITH_SERVER=OFF -DWITH_EXAMPLES=OFF -DWITH_NACL=OFF -DCMAKE_PREFIX_PATH=..\%OPENSSL_LONG% -DWITH_PCAP=OFF -DCMAKE_INSTALL_PREFIX=..\%INSTALL_DIR%  ..\libssh-%SSH_VERSION%
rem 
rem 	IF errorlevel 1 goto error_end
rem 	cmake --build . --config %Configuration%
rem 	IF errorlevel 1 goto error_end
rem 	cmake --build . --config %Configuration% --target install
rem 	IF errorlevel 1 goto error_end
rem     
rem     popd
rem )


rem IF NOT EXIST %EXIV_DIR% (
rem     %CYGWIN_DIR%\bin\svn.exe co svn://dev.exiv2.org/svn/trunk %EXIV_DIR%
rem ) ELSE (
rem     %CYGWIN_DIR%\bin\svn.exe update %EXIV_DIR%
rem )

echo. ---------- building EXIV2 ------------------
cd "%BUILD_DIR%"

IF NOT EXIST exiv2.build (
    mkdir exiv2.build
)
pushd exiv2.build
	cmake -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=%INSTALL_DIR% -DCMAKE_PROGRAM_PATH=%SVN_DIR% -DEXIV2_ENABLE_WEBREADY=OFF -DEXIV2_ENABLE_BUILD_SAMPLES=ON -DEXIV2_ENABLE_CURL=OFF -DEXIV2_ENABLE_SSH=OFF -DEXIV2_ENABLE_NLS=OFF -DEXIV2_ENABLE_WIN_UNICODE=OFF -DEXIV2_ENABLE_SHARED=ON %EXIV_DIR%

	IF errorlevel 1 (
		echo "*** errors in EXIV2 build [1] ***"
	    popd
		goto error_end
	)

	cmake --build . --config %Configuration%
	IF errorlevel 1 (
		echo "*** errors in EXIV2 build [2] ***"
	    popd
		goto error_end
	)

	cmake --build . --config %Configuration% --target install
	IF errorlevel 1 (
		echo "*** errors in EXIV2 build [3] ***"
	    popd
		goto error_end
	)
popd
goto end
    

:error_end
REM pause

:end
endlocal