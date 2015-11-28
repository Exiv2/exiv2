for %%a in (64 32) do for %%v in (2015 2013 2012 2010 2008 2005) do cmd /c "vcvars %%v %%a && cmakeBuild --rebuild --test 2>&1" 

rem cmd /c "vcvars 2012 64 && cmakeBuild --rebuild --test 2>&1"
