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
        self.options['gtest'].shared = True

    def requirements(self):
        self.requires('Expat/2.2.5@pix4d/stable')
        self.requires('zlib/1.2.11@conan/stable')
        self.requires('libcurl/7.56.1@bincrafters/stable') # from conan-bincrafters

        if self.options.unitTests:
            self.requires('gtest/1.8.0@bincrafters/stable')

    def imports(self):
        self.copy('*.dll', dst='bin', src='bin')
        self.copy('*.dylib', dst='bin', src='lib')
