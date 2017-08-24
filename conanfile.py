from conans import ConanFile
from conans.tools import os_info

class Exiv2Conan(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'

    def requirements(self):
        self.requires('Expat/2.2.1@pix4d/stable') # From pix4d
        self.requires('zlib/1.2.11@conan/stable') # From conan-center
        self.requires('libcurl/7.47.1@lasote/stable') # From conan-transit (It also brings OpenSSL)
