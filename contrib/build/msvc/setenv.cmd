@echo off

REM ---------------------------------------------------
REM -- Created by danielkaneider for the exiv2 project
REM ---------------------------------------------------


SET CYGWIN_DIR=c:\cygwin64
SET TEMP_DIR=.downloaddir

SET CMAKE_EXE=C:\Data\Programs\cmake-3.3.2-win32-x86\bin\cmake.exe

SET SVN_DIR=C:\Data\Programs\Apache-Subversion-1.8.13\bin

SET EXIV_DIR=exiv2-trunk

REM Options:  -------------------------------------------

REM enable another configuration (defaulting to Release)
REM SET Configuration=Debug

IF DEFINED VS140COMNTOOLS (
	REM Visual Studio 2015
	set VS_SHORT=vc14
	set VS_CMAKE=Visual Studio 14
	set VS_PROG_FILES=Microsoft Visual Studio 14.0
	set VS_OPENSSL=vs2015
) ELSE IF DEFINED VS120COMNTOOLS (
	REM Visual Studio 2013
	set VS_SHORT=vc12
	set VS_CMAKE=Visual Studio 12
	set VS_PROG_FILES=Microsoft Visual Studio 12.0
	set VS_OPENSSL=vs2013
) ELSE IF DEFINED VS110COMNTOOLS (
	REM Visual Studio 2012
	set VS_SHORT=vc11
	set VS_CMAKE=Visual Studio 11
	set VS_PROG_FILES=Microsoft Visual Studio 11.0
	set VS_OPENSSL=vs2012
) ELSE IF DEFINED VS100COMNTOOLS (
	REM Visual Studio 2010
	set VS_SHORT=vc10
	set VS_CMAKE=Visual Studio 10
	set VS_PROG_FILES=Microsoft Visual Studio 10.0
	set VS_OPENSSL=vs2010
) ELSE (
	REM Visual Studio 2008
	set VS_SHORT=vc9
	set VS_CMAKE=Visual Studio 9 2008
	set VS_PROG_FILES=Microsoft Visual Studio 9.0
	set VS_OPENSSL=vs2008
)
