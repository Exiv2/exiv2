@echo off

if defined VSINSTALLDIR (
	echo already installed
    goto eof
)

if "%1" EQU "2005_64" (
	pushd "c:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\amd64\"
	call vcvarsamd64.bat
	popd
	goto eof
)

if "%1" EQU "2005"    (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 8\VC\bin\"
    vcvar32.bat
    popd
    goto eof
)

if "%1" EQU "2008_64" (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin\amd64\"
    vcvarsamd64.bat 
    popd
    goto eof
)

if "%1" EQU "2008"    (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\bin\"
    vcvar32.bat
    popd
    goto eof
)

if "%1" EQU "2010_64" (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\"
    vcvarsall.bat x86_amd
    popd
    goto eof
)

if "%1" EQU "2010"    (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\"
    vcvarsall.bat x86
    popd
    goto eof
)

if "%1" EQU "2012_64" (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\"
    vcvarsall.bat x86_amd64 
    popd
    goto eof
)

if "%1" EQU "2012"    (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 11.0\VC\"
    vcvarsall.bat x86
    popd
    goto eof
)

if "%1" EQU "2013_64" (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\"
    vcvarsall.bat x86_amd64 
    popd
    goto eof
)

if "%1" EQU "2013"    (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\"
    vcvarsall.bat x86
    popd
    goto eof
)

if "%1" EQU "2015_64" (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\"
    vcvarsall.bat x86_amd64 
    popd
    goto eof
)

if "%1" EQU "2015"    (
    pushd "c:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\"
    vcvarsall.bat x86
    popd
    goto eof
)

:eof
