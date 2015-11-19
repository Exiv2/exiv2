@echo off

if defined VSINSTALLDIR (
	echo already installed
    goto eof
)

rem help
if "%1" EQU "" (
	echo syntax: %0 [2005 ^| 2008 ^| 2010 ^| 2012 ^| 2013 ^| 2015] [64]
	goto eof
)

if "%1" EQU "2005" (
  if "%2" EQU "64" (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\amd64\"
    call vcvarsamd64.bat
  ) else (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\"
    vcvar32.bat
  )
  popd
  goto eof
)

rem set the variable _VC_
set _VC_=

if "%1" EQU "2008" set "_VC_=9.0"
if "%1" EQU "2010" set "_VC_=10.0"
if "%1" EQU "2012" set "_VC_=11.0"
if "%1" EQU "2013" set "_VC_=12.0"
if "%1" EQU "2015" set "_VC_=14.0"

if DEFINED _VC_ (
  @echo on
  pushd "%ProgramFiles(x86)%\Microsoft Visual Studio %_VC_%\VC\"
  if "%2" EQU "64" (
    vcvarsall.bat x86_amd64
  ) else (
    vcvarsall.bat x86  
  )
  if errorlevel 1 (
  	echo.*** error - failed ***
  ) else (
  	echo Success Environment for Visual Studio %_VC_% = %*
  )
  popd
) else (
  echo unknown version of Visual Studio %*
)

:eof
