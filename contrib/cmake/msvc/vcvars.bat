@echo off

if defined VSINSTALLDIR (
	echo already installed
    goto eof
)
set "_ARG_=%1"

rem help
if "%_ARG_%" EQU "" (
	echo syntax: %0 [2005 ^| 2008 ^| 2010 ^| 2012 ^| 2013 ^| 2015] [64]
	goto eof
)

if NOT DEFINED ProgramFiles(x86) set ProgramFiles(x86)=C:\Program Files (x86)
if NOT DEFINED VS80COMNTOOLS set VS80COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\Tools\
if NOT DEFINED VS90COMNTOOLS set VS90COMNTOOLS=c:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\
rem VS100COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 10.0\Common7\Tools\
rem VS110COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\Tools\
rem VS120COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\Tools\
rem VS140COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\
rem VS71COMNTOOLS=c:\Program Files (x86)\Microsoft Visual Studio .NET 2003\Common7\Tools\
rem VS80COMNTOOLS=C:\Program Files (x86)\Microsoft Visual Studio 8\Common7\Tools\
rem VS90COMNTOOLS=c:\Program Files (x86)\Microsoft Visual Studio 9.0\Common7\Tools\

set "_OPT_=%2"
if NOT DEFINED _OPT_ set _OPT_=64
if "%_OPT_%" EQU "x64" set "_OPT_=64"

if "%_ARG_%" EQU "2005" (
  if "%_OPT_%" EQU "64" (
    pushd "%ProgramFiles(x86)%\Microsoft Visual Studio 8\VC\bin\amd64\"
    call vcvarsamd64.bat
    set PROCESSOR_ARCHITECTURE=AMD64
  ) else (
    pushd "%ProgramFiles(x86)%\Microsoft Visual Studio 8\VC\bin"
    call vcvars32.bat
    set PROCESSOR_ARCHITECTURE=x86
  )
  popd
  goto eof
)

rem set the variable _VC_
set _VC_=

if "%_ARG_%" EQU "2008" set "_VC_=9.0"
if "%_ARG_%" EQU "2010" set "_VC_=10.0"
if "%_ARG_%" EQU "2012" set "_VC_=11.0"
if "%_ARG_%" EQU "2013" set "_VC_=12.0"
if "%_ARG_%" EQU "2015" set "_VC_=14.0"

if DEFINED _VC_ (
  pushd "%ProgramFiles(x86)%\Microsoft Visual Studio %_VC_%\VC\"
  if "%_OPT_%" EQU "64" (
    vcvarsall.bat x86_amd64
    set PROCESSOR_ARCHITECTURE=AMD64
  ) else (
    vcvarsall.bat x86
    set PROCESSOR_ARCHITECTURE=x86
  )
  if errorlevel 1 (
  	echo.*** error - failed ***
  ) else (
  	echo Success Environment for Visual Studio %_VC_% = %*
  )
  popd
) else (
  echo unknown version of Visual Studio %*
  set _ARG_=
  call:help
)

rem
rem That's all Folks!
:eof
