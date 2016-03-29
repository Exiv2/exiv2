for %%a in (32) do for %%v in (2010 2008 2005) do cmd /c "vcvars %%v %%a && cmakeBuild %* 2>&1" 
