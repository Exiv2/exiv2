@echo off

rem ---------------------------------------------------
rem -- Originallly created by danielkaneider
rem ---------------------------------------------------


rem Options: -- begin --

rem enable another configuration (defaulting to Release)
rem SET Configuration=Debug
SET CYGWIN_DIR=c:\cygwin64
SET TEMP_DIR=temp
SET EXIV_DIR=..\trunk
SET BUILD_DIR=%CD%

rem Options: --  end  --


rem  -------------------------------------
echo checking that EXIV_DIR exists
if NOT EXIST %EXIV_DIR% (
	echo "EXIV_DIR = %EXIV_DIR% does not exist ***"
	exit /b 1
)
pushd %EXIV_DIR%
set EXIV_DIR=%CD%
popd
echo EXIV_DIR = %EXIV_DIR%


rem  -------------------------------------
echo testing VSINSTALLDIR "%VSINSTALLDIR%"
rem http://stackoverflow.com/questions/9252980/how-to-split-the-filename-from-a-full-path-in-batch
for %%A in ("%VSINSTALLDIR%") do (
    set "VSSTUDIO=%%~nA"
)
echo VSSTUDIO = "%VSSTUDIO%"

rem  -------------------------------------
echo setting CMake command options
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
    exit /b 1
)

rem That's all Folks!
rem
