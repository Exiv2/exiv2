from conans import ConanFile
from conans.tools import os_info
from conans.model.version import Version

class Exiv2Conan(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake_paths'
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
        self.options['gtest'].shared = True

    def requirements(self):
        self.requires('zlib/1.2.11')
        self.requires('libcurl/7.75.0')

        if os_info.is_windows and self.options.iconv:
            self.requires('libiconv/1.16')

        if self.options.unitTests:
            self.requires('gtest/1.10.0')
            if self.settings.build_type == "Debug":
                self.options['gtest'].debug_postfix = ''

        if self.options.xmp:
            self.requires('XmpSdk/2016.7@piponazo/stable') # from conan-piponazo
        else:
            if os_info.is_windows:
                self.requires('Expat/2.2.6@pix4d/stable')
            else:
                self.requires('expat/2.3.0')

    def imports(self):
        self.copy('*.dll', dst='bin', src='bin')
        self.copy('*.dylib', dst='bin', src='lib')
