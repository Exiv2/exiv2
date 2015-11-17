@echo off

if not defined Configuration SET _CONFIG_=Release
IF NOT DEFINED CYGWIN_DIR    SET CYGWIN_DIR=c:\cygwin64
if NOT DEFINED _TEMP_        SET _TEMP_=temp
if NOT DEFINED _EXIV2_       SET _EXIV2_=..\trunk

rem That's all Folks!
rem
