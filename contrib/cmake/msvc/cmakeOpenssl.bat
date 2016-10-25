@echo off
rem  I have been unable to find public builds of openssl for VS2005
rem  this script is used to export openssl builds from VS
rem  I am keeping this in our code base for future use, although I hope never to need it again!
rem  this is a throw away script and not intended for public use

rem trg=the .7z being built
rem dst=directory in which to create trg.7z
set trg=openssl-1.0.1p-vs2005
set dst=\gnu\exiv2\team\libraries\%trg%\

call cmakeDefaults >NUL 2>NUL
if ERRORLEVEL 1 echo *** cmakeDefaults failed *** && exit /b 1

rem src=pre-built.  Use msvc/exiv2-webready.sln openssl.vcproj to build 32/64 openssl Release and ReleaseDLL
set src=%_EXIV2_%\msvc\openssl\

if NOT EXIST %src%\x64\ReleaseDLL\ssl\openssl.cnf echo *** file %src%\x64\ReleaseDLL\ssl\openssl.cnf does not exist *** && exit /b 1
if NOT EXIST %dst% echo "*** destination %dst% does not exist && exit /b 1

echo copying %src% to %dst%
pause
copy         %src%Win32\ReleaseDLL\bin\libeay32.dll      %dst%bin\libeay32MD.dll
copy         %src%Win32\ReleaseDLL\bin\ssleay32.dll      %dst%bin\libeay32MDd.dll
copy           %src%x64\ReleaseDLL\bin\libeay32.dll    %dst%bin64\libeay32MD.dll
copy           %src%x64\ReleaseDLL\bin\ssleay32.dll    %dst%bin64\libeay32MDd.dll

copy         %src%Win32\ReleaseDLL\lib\libeay32.lib      %dst%lib\libeay32MD.lib
copy         %src%Win32\ReleaseDLL\lib\ssleay32.lib      %dst%lib\libeay32MDd.lib
copy           %src%x64\ReleaseDLL\lib\libeay32.lib    %dst%lib64\libeay32MD.lib
copy           %src%x64\ReleaseDLL\lib\ssleay32.lib    %dst%lib64\libeay32MDd.lib

copy         %src%Win32\Release\lib\libeay32.lib         %dst%lib\libeay32MT.lib
copy         %src%Win32\Release\lib\ssleay32.lib         %dst%lib\libeay32MTd.lib
copy           %src%x64\Release\lib\libeay32.lib       %dst%lib64\libeay32MT.lib
copy           %src%x64\Release\lib\ssleay32.lib       %dst%lib64\libeay32MTd.lib

xcopy/yesihq %src%Win32\Release\include\openssl      %dst%include\openssl
xcopy/yesihq   %src%x64\Release\include\openssl    %dst%include64\openssl

copy           %src%x64\ReleaseDLL\ssl\*.cnf       %dst%ssl

rem create trg.7z
pushd %dst%..
    del/s *.exe *.pdb *.exp
    if EXIST %trg%.7z del %trg%.7z
    7z a %trg%.7z     %trg%\
    dir  %trg%.7z
popd

rem That's all folks
rem ------------------------------------------
