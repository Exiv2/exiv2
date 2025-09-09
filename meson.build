project(
  'exiv2',
  'cpp',
  version: '1.0.0',
  meson_version: '>=0.54.1',
  default_options: ['warning_level=0', 'cpp_std=c++20'],
)

cpp = meson.get_compiler('cpp')
if host_machine.system() == 'windows' and get_option('default_library') != 'static'
  cargs = '-DEXIV2API=__declspec(dllexport)'
elif cpp.has_function_attribute('visibility:default')
  cargs = '-DEXIV2API=__attribute__((visibility("default")))'
else
  cargs = '-DEXIV2API='
endif

if host_machine.system() == 'windows'
  if cpp.get_argument_syntax() == 'msvc'
    add_project_arguments('-DNOMINMAX', language: 'cpp')
  elif cpp.compiles('#include <time.h>\n#ifdef _UCRT\n#error\n#endif')
    error('Non UCRT MinGW is unsupported. Please update toolchain')
  endif
endif

exiv_api = configuration_data()
exiv_conf = configure_file(output: 'exiv2lib_export.h', configuration: exiv_api)

cdata = configuration_data()
cdata.set('EXV_PACKAGE_NAME', meson.project_name())
ver = meson.project_version().split('.')
if ver[0] == '0'
  sover = ver[1].to_int()
else
  sover = 30 + (ver[0].to_int() - 1)
endif
cdata.set('PROJECT_VERSION_MAJOR', ver[0])
cdata.set('PROJECT_VERSION_MINOR', ver[1])
cdata.set('PROJECT_VERSION_PATCH', ver[2])
cdata.set('PROJECT_VERSION_TWEAK', 9)
cdata.set('PROJECT_VERSION', '@0@.@1@'.format(meson.project_version(), cdata.get('PROJECT_VERSION_TWEAK')))
cdata.set('EXV_PACKAGE_VERSION', '@0@.@1@'.format(meson.project_version(), cdata.get('PROJECT_VERSION_TWEAK')))
cdata.set('EXV_PACKAGE_STRING', '@0@ @1@'.format(meson.project_name(), cdata.get('PROJECT_VERSION')))

cdata.set('EXV_HAVE_STRERROR_R', cpp.has_function('strerror_r'))
cdata.set('EXV_STRERROR_R_CHAR_P', not cpp.compiles('#define _GNU_SOURCE\n#include <string.h>\nint strerror_r(int,char*,size_t);int main(){}'))
cdata.set('EXV_HAVE_STD_FORMAT', cpp.has_header_symbol('format', 'std::format'))

cdata.set('EXV_ENABLE_BMFF', get_option('bmff'))
cdata.set('EXV_HAVE_LENSDATA', get_option('lensdata'))
cdata.set('EXV_ENABLE_VIDEO', get_option('video'))

deps = []
foreach d, os : {'procstat': 'freebsd', 'socket': 'sunos', 'ws2_32': 'windows'}
  if host_machine.system() == os
    deps += cpp.find_library(d)
  endif
endforeach

fmt_dep = []
if not cdata.get('EXV_HAVE_STD_FORMAT')
  fmt_dep = dependency('fmt', version: '>=5.0.0')
  deps += fmt_dep
endif

brotli_dep = dependency('libbrotlidec', disabler: true, required: get_option('brotli'))
if brotli_dep.found()
  deps += brotli_dep
endif

if get_option('webready')
  curl_dep = dependency('libcurl', required: get_option('curl'))
  web_dep = declare_dependency(dependencies: [curl_dep])
  deps += web_dep
else
  web_dep = dependency('', disabler: true, required: false)
  curl_dep = web_dep
endif

expat_dep = dependency('expat', disabler: true, required: get_option('xmp'))
if expat_dep.found()
  deps += expat_dep
endif

inih_dep = dependency('INIReader', disabler: true, required: get_option('inih'))
if inih_dep.found()
  deps += inih_dep
endif

zlib_dep = dependency('zlib', disabler: true, required: get_option('png'))
if zlib_dep.found()
  deps += zlib_dep
endif

if meson.version().version_compare('>= 0.60')
  iconv_dep = dependency('iconv', required: get_option('iconv'))
elif not get_option('iconv').disabled() and not cpp.links('#include <iconv.h>\nint main(){iconv_open("", "");}')
  iconv_dep = cpp.find_library('iconv', required: get_option('iconv'))
else
  iconv_dep = get_option('iconv').disabled() ? dependency('', required: false) : declare_dependency()
endif
if iconv_dep.found()
  deps += iconv_dep
endif

if meson.version().version_compare('>= 0.60')
  intl_dep = dependency('intl', required: get_option('nls'))
else
  intl_dep = dependency('', required: false)
endif
if intl_dep.found()
  add_project_arguments('-DEXV_LOCALEDIR="@0@"'.format(get_option('prefix') / get_option('localedir')), language: 'cpp')
  deps += intl_dep
endif

cdata.set('EXV_ENABLE_INIH', inih_dep.found())
cdata.set('EXV_HAVE_XMP_TOOLKIT', expat_dep.found())
cdata.set('EXV_HAVE_BROTLI', brotli_dep.found())
cdata.set('EXV_HAVE_ICONV', iconv_dep.found())
cdata.set('EXV_HAVE_LIBZ', zlib_dep.found())
cdata.set('EXV_ENABLE_WEBREADY', web_dep.found())
cdata.set('EXV_USE_CURL', curl_dep.found())
cdata.set('EXV_ENABLE_NLS', intl_dep.found())
cdata.set('EXV_ENABLE_FILESYSTEM', true)

cfile = configure_file(
  input: 'cmake/config.h.cmake',
  output: 'exv_conf.h',
  format: 'cmake@',
  configuration: cdata,
)

libinc = include_directories('.', 'include/exiv2')
depinc = include_directories('.', 'include')
subdir('xmpsdk')
subdir('src')
subdir('include')

install_man('man/man1/exiv2.1')

exiv2inc = include_directories('src', 'include/exiv2')

subdir('unitTests')
subdir('po')

if get_option('app')
  exiv2_sources = files(
    'app/actions.cpp',
    'app/app_utils.cpp',
    'app/exiv2.cpp',
    'app/getopt.cpp',
  )

  if host_machine.system() == 'windows'
    windows = import('windows')
    exiv2_sources += windows.compile_resources('app/utf8.rc', depend_files: 'app/utf8.manifest')
  endif

  executable(
    'exiv2',
    exiv2_sources,
    include_directories: exiv2inc,
    dependencies: exiv2_dep,
    install: true,
  )
endif

if get_option('app')
  samples = {
    'addmoddel': [],
    'conntest': web_dep,
    'convert-test': [],
    'easyaccess-test': [],
    'exifcomment': [],
    'exifdata-test': [],
    'exifdata': [],
    'exifprint': [],
    'exifvalue': [],
    'geotag': expat_dep,
    'ini-test': inih_dep,
    'iotest': [],
    'iptceasy': [],
    'iptcprint': [],
    'iptctest': [],
    'jpegparsetest': [],
    'key-test': [],
    'largeiptc-test': [],
    'mmap-test': [],
    'mrwthumb': [],
    'prevtest': [],
    'remotetest': [],
    'stringto-test': [],
    'taglist': [],
    'tiff-test': [],
    'write-test': [],
    'write2-test': [],
    'xmpparse': [],
    'xmpparser-test': [],
    'xmpprint': [],
    'xmpsample': [],
    'xmpdump': [],
  }

  foreach s, d : samples
    executable(s, 'samples/@0@.cpp'.format(s), dependencies: [exiv2_dep, d], include_directories: exiv2inc)
  endforeach

  gopt = [
   'getopt-test',
   'metacopy',
   'path-test',
  ]

  ginc = include_directories('app')
  foreach g : gopt
    executable(g, 'samples/@0@.cpp'.format(g), 'app/getopt.cpp', dependencies: exiv2_dep, include_directories: [exiv2inc, ginc])
  endforeach

  bindir = 'EXIV2_BINDIR=@0@'.format(meson.current_build_dir())
  subdir('tests')
endif
