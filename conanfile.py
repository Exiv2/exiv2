from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout

class Exiv2Conan(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'
    generators = 'cmake_find_package', 'cmake_paths'
    options = {'unitTests': [True, False],
               'xmp': [True, False],
               'iconv': [True, False],
               'webready': [True, False],
              }
    default_options = {'unitTests': True,
                       'xmp': False,
                       'iconv': False,
                       'webready': False,
                      }

   # def configure(self):
    #    self.options['libcurl'].shared = True
     #   self.options['gtest'].shared = False

    def requirements(self):
        self.requires('zlib/1.2.13')

        self.requires('brotli/1.0.9')

        self.requires('inih/55')

        if self.options.webready:
            self.requires('libcurl/7.85.0')

        if self.settings.os == "Windows" and self.options.iconv:
            self.requires('libiconv/1.17')

        if self.options.unitTests:
            self.requires('gtest/1.12.1')

        if self.options.xmp:
            self.requires('XmpSdk/2016.7@piponazo/stable') # from conan-piponazo
        else:
            self.requires('expat/2.4.9')

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy('*.dll', dst='bin', src='bin')
        self.copy('*.dylib', dst='bin', src='lib')
