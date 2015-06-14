@echo off
SETLOCAL

REM ---------------------------------------------------
REM -- Created by danielkaneider for the exiv2 project
REM ---------------------------------------------------


rem  https://github.com/madler/zlib/commits
SET ZLIB_COMMIT_LONG=50893291621658f355bc5b4d450a8d06a563053d

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

IF EXIST ..\..\..\exiv2-trunk (
	CD ..\..\..
	call exiv2-trunk\build\msvc\setenv.cmd
) ELSE (
	call setenv.cmd
)


IF %Platform% EQU x64 (
	set VS_CMAKE=%VS_CMAKE% Win64
)

IF NOT EXIST %CMAKE_DIR%\bin\cmake.exe (
	echo.
	echo.ERROR: CMake not found: %CMAKE_DIR%\bin\cmake.exe
	echo.
	goto error_end
)

IF NOT EXIST %CYGWIN_DIR%\bin\cp.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\gzip.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\mv.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\svn.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\tar.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\unzip.exe GOTO cygwin_error
IF NOT EXIST %CYGWIN_DIR%\bin\wget.exe GOTO cygwin_error
GOTO cygwin_ok

:cygwin_error
echo ERROR: Cygwin with 
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
	%CMAKE_DIR%\bin\cmake.exe -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=..\%INSTALL_DIR% ..\zlib-%ZLIB_COMMIT%
	IF errorlevel 1 goto error_end
	%CMAKE_DIR%\bin\cmake.exe --build . --config %Configuration%
	IF errorlevel 1 goto error_end
	%CMAKE_DIR%\bin\cmake.exe --build . --config %Configuration% --target install
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
	%CMAKE_DIR%\bin\cmake.exe -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=..\%INSTALL_DIR% ..\expat-2.1.0
	IF errorlevel 1 goto error_end
	%CMAKE_DIR%\bin\cmake.exe --build . --config %Configuration%
	IF errorlevel 1 goto error_end
	%CMAKE_DIR%\bin\cmake.exe --build . --config %Configuration% --target install
	IF errorlevel 1 goto error_end
	popd
)

IF NOT EXIST exiv2-trunk (
    %CYGWIN_DIR%\bin\svn.exe co svn://dev.exiv2.org/svn/trunk exiv2-trunk
) ELSE (
    %CYGWIN_DIR%\bin\svn.exe update exiv2-trunk
)

IF NOT EXIST exiv2-trunk.build (
    mkdir exiv2-trunk.build
)
    
pushd exiv2-trunk.build

%CMAKE_DIR%\bin\cmake.exe -G "%VS_CMAKE%" -DCMAKE_INSTALL_PREFIX=..\%INSTALL_DIR% -DCMAKE_PROGRAM_PATH=%SVN_DIR% -DEXIV2_ENABLE_BUILD_SAMPLES=ON -DEXIV2_ENABLE_CURL=OFF -DEXIV2_ENABLE_SSH=OFF ..\exiv2-trunk

IF errorlevel 1 goto error_end

%CMAKE_DIR%\bin\cmake.exe --build . --config %Configuration%
IF errorlevel 1 goto error_end
%CMAKE_DIR%\bin\cmake.exe --build . --config %Configuration% --target install
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