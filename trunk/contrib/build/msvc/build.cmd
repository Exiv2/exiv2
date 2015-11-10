@echo off
SETLOCAL

REM ---------------------------------------------------
REM -- Created by danielkaneider for the exiv2 project
REM ---------------------------------------------------


rem  https://github.com/madler/zlib/commits
SET ZLIB_COMMIT_LONG=50893291621658f355bc5b4d450a8d06a563053d

rem https://github.com/pol51/OpenSSL-CMake
SET SSL_COMMIT_LONG=595a9e4384e1280659080f7c7029e66544b772a8

rem https://github.com/bagder/curl
SET CURL_COMMIT_LONG=dd39a671019d713bd077be9eed511c2dc6013598

rem http://www.npcglib.org/~stathis/blog/precompiled-openssl/
SET OPENSSL_VERSION=openssl-1.0.2d


IF EXIST ..\msvc (
	echo.
	echo.ERROR: This file should NOT be executed within the exiv2 source directory,
	echo.       but in a new empty folder!
	echo.
	goto error_end
)

ml64.exe > NUL
IF ERRORLEVEL 1 (
	set Platform=Win32
	set RawPlatform=x86
	set CpuPlatform=ia32
) ELSE (
	set Platform=x64
	set RawPlatform=x64
	set CpuPlatform=intel64
)

call setenv.cmd


IF %Platform% EQU x64 (
	set VS_CMAKE=%VS_CMAKE% Win64
)

IF NOT EXIST %CMAKE_EXE% (
	echo.
	echo.ERROR: CMake not found: %CMAKE_EXE%
	echo.
	goto error_end
)

SET 7Z_PATH=%CYGWIN_DIR%\lib\p7zip\7z.exe
IF NOT EXIST %CYGWIN_DIR%\lib\p7zip\7z.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\cp.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\gzip.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\mv.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\svn.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\tar.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\unzip.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\wget.exe GOTO cygwin_error
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


IF NOT DEFINED Configuration (
	set Configuration=Release
)

cls
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
	%CYGWIN_DIR%\bin\cp.exe vcDlls/**/msv* vcDlls/selected
)

SET INSTALL_DIR=dist
IF NOT EXIST %INSTALL_DIR% (
	mkdir %INSTALL_DIR%
)



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
	%CMAKE_EXE% -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=..\%INSTALL_DIR% ..\zlib-%ZLIB_COMMIT%
	IF errorlevel 1 goto error_end
	%CMAKE_EXE% --build . --config %Configuration%
	IF errorlevel 1 goto error_end
	%CMAKE_EXE% --build . --config %Configuration% --target install
	IF errorlevel 1 goto error_end
    
    popd
)

IF NOT EXIST %TEMP_DIR%\expat-2.1.0.tar (
	%CYGWIN_DIR%\bin\wget.exe -O %TEMP_DIR%/expat-2.1.0.tar.gz http://sourceforge.net/projects/expat/files/expat/2.1.0/expat-2.1.0.tar.gz/download
	%CYGWIN_DIR%\bin\gzip.exe -d %TEMP_DIR%/expat-2.1.0.tar.gz
)
IF NOT EXIST expat-2.1.0 (
	%CYGWIN_DIR%\bin\tar.exe -xf %TEMP_DIR%/expat-2.1.0.tar   
)

IF NOT EXIST expat-2.1.0.build (
    mkdir expat-2.1.0.build
    
    pushd expat-2.1.0.build
	%CMAKE_EXE% -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=..\%INSTALL_DIR% ..\expat-2.1.0
	IF errorlevel 1 goto error_end
	%CMAKE_EXE% --build . --config %Configuration%
	IF errorlevel 1 goto error_end
	%CMAKE_EXE% --build . --config %Configuration% --target install
	IF errorlevel 1 goto error_end
	popd
)



REM SET SSL_COMMIT=%SSL_COMMIT_LONG:~0,7%
REM IF NOT EXIST %TEMP_DIR%\OpenSSL-%SSL_COMMIT%.zip (
REM 	%CYGWIN_DIR%\bin\wget.exe -O %TEMP_DIR%/OpenSSL-%SSL_COMMIT%.zip --no-check-certificate http://github.com/pol51/OpenSSL-CMake/zipball/%SSL_COMMIT_LONG%
REM )
REM 
REM IF NOT EXIST OpenSSL-%SSL_COMMIT% (
REM 	%CYGWIN_DIR%\bin\unzip.exe -q %TEMP_DIR%/OpenSSL-%SSL_COMMIT%.zip
REM 	%CYGWIN_DIR%\bin\mv.exe pol51-OpenSSL-CMake-* OpenSSL-%SSL_COMMIT%
REM )
REM 
REM REM IF NOT EXIST OpenSSL-%SSL_COMMIT%.build (
REM REM     mkdir OpenSSL-%SSL_COMMIT%.build
REM     
REM     pushd OpenSSL-%SSL_COMMIT%
REM 	%CMAKE_EXE% -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=..\%INSTALL_DIR% .
REM 	IF errorlevel 1 goto error_end
REM 	%CMAKE_EXE% --build . --config %Configuration%
REM 	IF errorlevel 1 goto error_end
REM 	%CMAKE_EXE% --build . --config %Configuration% --target install
REM 	IF errorlevel 1 goto error_end
REM     
REM REM     popd
REM REM )


SET OPENSSL_LONG=%OPENSSL_VERSION%-%VS_OPENSSL%
IF NOT EXIST %TEMP_DIR%\%OPENSSL_LONG%.7z (
	%CYGWIN_DIR%\bin\wget.exe -O %TEMP_DIR%/%OPENSSL_LONG%.7z --no-check-certificate http://www.npcglib.org/~stathis/downloads/%OPENSSL_LONG%.7z
)
IF NOT EXIST %OPENSSL_LONG% (
	%CYGWIN_DIR%\lib\p7zip\7z.exe x %TEMP_DIR%/%OPENSSL_LONG%.7z
	IF %Platform% EQU x64 (
		ren %OPENSSL_LONG%\lib xxlib
		ren %OPENSSL_LONG%\lib64 lib
		ren %OPENSSL_LONG%\bin xxbin
		ren %OPENSSL_LONG%\bin64 bin
	)
)
robocopy %OPENSSL_LONG%\bin %INSTALL_DIR%\bin libeay32MD.dll /MIR /NJS >nul
robocopy %OPENSSL_LONG%\bin %INSTALL_DIR%\bin ssleay32MD.dll /MIR /NJS >nul


SET CURL_COMMIT=%CURL_COMMIT_LONG:~0,7%
IF NOT EXIST %TEMP_DIR%\curl-%CURL_COMMIT%.zip (
	%CYGWIN_DIR%\bin\wget.exe -O %TEMP_DIR%/curl-%CURL_COMMIT%.zip --no-check-certificate http://github.com/bagder/curl/zipball/%CURL_COMMIT_LONG%
)

IF NOT EXIST curl-%CURL_COMMIT% (
	%CYGWIN_DIR%\bin\unzip.exe -q %TEMP_DIR%/curl-%CURL_COMMIT%.zip
	%CYGWIN_DIR%\bin\mv.exe bagder-curl-* curl-%CURL_COMMIT%
)

IF NOT EXIST curl-%CURL_COMMIT%.build (
    mkdir curl-%CURL_COMMIT%.build
    
    pushd curl-%CURL_COMMIT%.build
	%CMAKE_EXE% -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=..\%INSTALL_DIR% -DCMAKE_PREFIX_PATH=..\%OPENSSL_LONG% -DBUILD_CURL_TESTS=OFF -DCMAKE_USE_OPENSSL=ON -DCMAKE_USE_LIBSSH2=OFF ..\curl-%CURL_COMMIT%
	IF errorlevel 1 goto error_end
	%CMAKE_EXE% --build . --config %Configuration%
	IF errorlevel 1 goto error_end
	%CMAKE_EXE% --build . --config %Configuration% --target install
	IF errorlevel 1 goto error_end
    
    popd
)

SET SSH_VERSION=0.7.2
IF NOT EXIST %TEMP_DIR%\libssh-%SSH_VERSION%.zip (
	%CYGWIN_DIR%\bin\wget.exe -O %TEMP_DIR%/libssh-%SSH_VERSION%.zip --no-check-certificate https://git.libssh.org/projects/libssh.git/snapshot/libssh-%SSH_VERSION%.zip
)
IF NOT EXIST libssh-%SSH_VERSION% (
	%CYGWIN_DIR%\bin\unzip.exe -q %TEMP_DIR%/libssh-%SSH_VERSION%.zip
)
IF NOT EXIST libssh-%SSH_VERSION%.build (
    mkdir libssh-%SSH_VERSION%.build
    
    pushd libssh-%SSH_VERSION%.build
	
	%CMAKE_EXE% -G "%VS_CMAKE%" -DWITH_GSSAPI=OFF -DWITH_ZLIB=ON -DWITH_SFTP=ON -DWITH_SERVER=OFF -DWITH_EXAMPLES=OFF -DWITH_NACL=OFF -DCMAKE_PREFIX_PATH=..\%OPENSSL_LONG% -DWITH_PCAP=OFF -DCMAKE_INSTALL_PREFIX=..\%INSTALL_DIR%  ..\libssh-%SSH_VERSION%

	IF errorlevel 1 goto error_end
	%CMAKE_EXE% --build . --config %Configuration%
	IF errorlevel 1 goto error_end
	%CMAKE_EXE% --build . --config %Configuration% --target install
	IF errorlevel 1 goto error_end
    
    popd
)


IF NOT EXIST %EXIV_DIR% (
    %CYGWIN_DIR%\bin\svn.exe co svn://dev.exiv2.org/svn/trunk %EXIV_DIR%
) ELSE (
REM    %CYGWIN_DIR%\bin\svn.exe update %EXIV_DIR%
)

IF NOT EXIST %EXIV_DIR%.build (
    mkdir %EXIV_DIR%.build
)
    
pushd %EXIV_DIR%.build

%CMAKE_EXE% -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=..\%INSTALL_DIR% -DCMAKE_PROGRAM_PATH=%SVN_DIR% -DEXIV2_ENABLE_WEBREADY=ON -DEXIV2_ENABLE_BUILD_SAMPLES=ON -DEXIV2_ENABLE_CURL=ON -DEXIV2_ENABLE_SSH=ON -DEXIV2_ENABLE_NLS=ON -DEXIV2_ENABLE_WIN_UNICODE=ON -DEXIV2_ENABLE_SHARED=ON ..\%EXIV_DIR%

IF errorlevel 1 goto error_end

%CMAKE_EXE% --build . --config %Configuration%
IF errorlevel 1 goto error_end
%CMAKE_EXE% --build . --config %Configuration% --target install
IF errorlevel 1 goto error_end
popd  
goto end
    

IF errorlevel 1 goto error_end
popd

goto end

:error_end
REM pause

:end

endlocal