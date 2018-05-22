from conans import ConanFile
from conans.tools import os_info

class Exiv2Conan(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'
    options = {'unitTests': [True, False]}
    default_options = 'unitTests=True'

    def configure(self):
        if not os_info.is_macos:
            self.options['libcurl'].shared = True
        self.options['gtest'].shared = True

    def requirements(self):
        self.requires('Expat/2.2.5@pix4d/stable')
        self.requires('zlib/1.2.11@conan/stable')
        self.requires('libcurl/7.60.0@bincrafters/stable')

        if self.options.unitTests:
            self.requires('gtest/1.8.0@bincrafters/stable')

    def imports(self):
        self.copy('*.dll', dst='conanDlls', src='bin')
        self.copy('*.dylib', dst='bin', src='lib')
