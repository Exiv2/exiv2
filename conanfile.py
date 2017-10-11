from conans import ConanFile
from conans.tools import os_info

class Exiv2Conan(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'
    options = {'unitTests': [True, False]}
    default_options = 'unitTests=True'

    def configure(self):
        # Note : The linking in exiv2lib fails if we try to use the static version of libcurl.
        # The libcurl CMake code is not mature enough and therefore the conan recipe for
        # Windows also has some problems (since it uses CMake for configuring the project).
        if os_info.is_windows:
            self.options['libcurl'].shared = True

    def requirements(self):
        self.requires('Expat/2.2.1@pix4d/stable') # From pix4d
        self.requires('zlib/1.2.8@lasote/stable') # From conan-center
        self.requires('libcurl/7.50.3@lasote/stable') # From conan-transit (It also brings OpenSSL)

        if self.options.unitTests:
            self.requires('gtest/1.8.0@lasote/stable') #From conan-transit

    def imports(self):
        self.copy('*.dll', dst='bin', src='bin')
        self.copy('*.dylib', dst='bin', src='lib')
