@echo off

rem ##
rem buildall [/rebuild]

setlocal
set "ACTION=%1%"
if NOT DEFINED ACTION set "ACTION=/build"

devenv exiv2.sln %ACTION% "Debug|Win32" 
devenv exiv2.sln %ACTION% "DebugDLL|Win32" 
devenv exiv2.sln %ACTION% "Release|Win32" 
devenv exiv2.sln %ACTION% "ReleaseDLL|Win32" 
devenv exiv2.sln %ACTION% "Debug|x64" 
devenv exiv2.sln %ACTION% "DebugDLL|x64" 
devenv exiv2.sln %ACTION% "Release|x64" 
devenv exiv2.sln %ACTION% "ReleaseDLL|x64" 

endlocal

rem That's all Folks!
rem ##
