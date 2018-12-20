from conans import ConanFile
from conans.tools import os_info

class Exiv2Conan(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake'
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
        self.options['libcurl'].with_openssl = True
        self.options['gtest'].shared = False

    def requirements(self):
        self.requires('zlib/1.2.11@conan/stable')

        if os_info.is_windows and self.options.iconv:
            self.requires('libiconv/1.15@bincrafters/stable')

        if self.options.unitTests:
            self.requires('gtest/1.8.1@bincrafters/stable')

        if self.options.webready:
            self.requires('libcurl/7.60.0@bincrafters/stable')

        if self.options.xmp:
            self.requires('XmpSdk/2016.7@piponazo/stable') # from conan-piponazo
        else:
            self.requires('Expat/2.2.6@pix4d/stable')

    def imports(self):
        self.copy('*.dll', dst='conanDlls', src='bin')
        self.copy('*.dylib', dst='bin', src='lib')
