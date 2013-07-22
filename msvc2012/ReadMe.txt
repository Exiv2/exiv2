exiv2\msvc2012\ReadMe.txt
-------------------------

+-----------------------------------------------------------+
| msvc2012 builds 32bit and 64bit binaries                  |
|          with Visual Studio 2012                          |
| msvc64   builds 32bit and 64bit binaries                  |
|          with Visual Studio 2005/8/10                     |
| msvc     builds 32bit binaries                            |
|          with Visual Studio 2003/5/8                      |
+-----------------------------------------------------------+

Updated: 2012-07-22 

The build environment for Visual Studio 2012 is basically the same
as msvc64 and you should consult msvc64\ReadMe.txt.

The project file convertor for 2012 created a new build environment
which fails to build several of the Release and Debug (static) applications.

The project files in msvc2012 have been hand modified to successfully build
exiv2 and all the sample applications.  The environment can build 32bit and 64bit
binaries in four flavors:
    Debug and Release       (static)
    DebugDLL and ReleaseDLL (shared libraries)
    
The static builds are linked with the static C-runtime libraries.
The shared builds are linked with the shared C-runtime libraries.

Robin Mills
http://clanmills.com
email: robin@clanmills.com

# That's all Folks!
##
