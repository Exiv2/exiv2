from conans import ConanFile
from conans.tools import os_info

class Exiv2Conan(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake_find_package', 'cmake_paths'
    options = {'unitTests': [True, False],
               'xmp': [True, False],
               'iconv': [True, False],
               'webready': [True, False],
              }
    default_options = ('unitTests=True',
                       'xmp=False',
                       'iconv=False',
                       'webready=False',
                      )

    def configure(self):
        self.options['libcurl'].shared = True
        self.options['gtest'].shared = False

    def requirements(self):
        self.requires('zlib/1.2.13')

        self.requires('brotli/1.0.9')

        self.requires('inih/55')

        self.requires('fmt/10.1.1')

        if self.options.webready:
            self.requires('libcurl/7.85.0')

        if os_info.is_windows and self.options.iconv:
            self.requires('libiconv/1.17')

        if self.options.unitTests:
            self.requires('gtest/1.12.1')

        if self.options.xmp:
            self.requires('XmpSdk/2016.7@piponazo/stable') # from conan-piponazo
        else:
            self.requires('expat/2.4.9')

    def imports(self):
        self.copy('*.dll', dst='bin', src='bin')
        self.copy('*.dylib', dst='bin', src='lib')
