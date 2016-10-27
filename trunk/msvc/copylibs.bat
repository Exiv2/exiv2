@echo off
rem copylibs.bat
rem get the support libraries
rem
setlocal
set EXIV2LIBS=c:\exiv2libs

xcopy/yesihq  %EXIV2LIBS%\expat   ..\..\expat
xcopy/yesihq  %EXIV2LIBS%\zlib    ..\..\zlib
xcopy/yesihq  %EXIV2LIBS%\openssl ..\..\openssl 
xcopy/yesihq  %EXIV2LIBS%\libssh  ..\..\libssh
xcopy/yesihq  %EXIV2LIBS%\curl    ..\..\curl


rem That's all Folks!
rem -----------------