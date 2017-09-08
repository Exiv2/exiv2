for %%a in (64 32) do for %%v in (2017 2015 2005) do cmd /c "vcvars %%v %%a && cmakeBuild --build %* 2>&1" 
