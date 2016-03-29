for %%a in (64 32) do for %%v in (2008 2005) do cmd /c "dir && set && call ..\contrib\cmake\msvc\vcvars.bat %%v %%a && cmakeBuild %* 2>&1" 
