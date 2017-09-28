@echo off

rem set up some defaults to be used by cmakeBuild.cmd and related scripts
rem use environment strings to set defaults which will not be clobbered by this script

if NOT DEFINED _CONFIG_      set _CONFIG_=Release
if NOT DEFINED _WORK_        set _WORK_=work
if NOT DEFINED _EXIV2_       set _EXIV2_=..\..\..
if NOT DEFINED _CURL_        set _CURL_=curl-7.45.0
if NOT DEFINED _LIBSSH_      set _LIBSSH_=libssh-0.7.2
if NOT DEFINED _OPENSSL_     set _OPENSSL_=openssl-1.0.1p
if NOT DEFINED _ZLIB_        set _ZLIB_=zlib-1.2.8
if NOT DEFINED _EXPAT_       set _EXPAT_=expat-2.1.0
if NOT DEFINED _BASH_        set _BASH_=c:\cygwin64\bin\bash.exe
if NOT DEFINED _SHARED_      set _SHARED_=1
if NOT DEFINED _UNICODE_     set _UNICODE_=0
if NOT DEFINED _NLS_         set _NLS_=0
if NOT DEFINED  COPYCMD      set  COPYCMD=/Y

rem ---------- check that EXIV2   exists
echo checking that %_EXIV2_% exists
if NOT EXIST %_EXIV2_% (
    echo "_EXIV2_ = %_EXIV2_% does not exist ***" >&2
    exit /b 1
)
pushd %_EXIV2_%
set _EXIV2_=%CD%
popd
if NOT EXIST %_EXIV2_%\src\version.cpp (
    echo "_EXIV2_ = %_EXIV2_% %_EXIV2_%\src\version.cpp does not exist ***" >&2
    exit /b 1
)

rem That's all Folks!
rem
