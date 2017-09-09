for %%a in (64 32) do for %%v in (2008) do for %%s in (--static --dll) do for %%r in (--release --debug) do cmd /c "vcvars %%v %%a && cmakeBuild --build %%s %%r %* 2>&1" 
