project(
  'exiv2',
  'cpp',
  version: '1.0.0',
  meson_version: '>=0.54.1',
  default_options: ['warning_level=0', 'cpp_std=c++17'],
)

cargs = []
cpp = meson.get_compiler('cpp')
if host_machine.system() == 'windows' and get_option('default_library') != 'static'
  cargs += '-DEXIV2API=__declspec(dllexport)'
elif cpp.has_function_attribute('visibility')
  cargs += '-DEXIV2API=__attribute__((visibility("default")))'
else
  cargs += '-DEXIV2API='
endif

if host_machine.system() == 'windows'
  if cpp.get_argument_syntax() == 'gcc'
    if cpp.compiles('#include <time.h>\n#ifdef _UCRT\n#error\n#endif')
      add_project_arguments('-D__USE_MINGW_ANSI_STDIO', '-D__MINGW_USE_VC2005_COMPAT', language: 'cpp')
    endif
  else
    add_project_arguments('-DNOMINMAX', language: 'cpp')
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
cdata.set('EXV_STRERROR_R_CHAR_P', not cpp.compiles('#include <string.h>\nint strerror_r(int,char*,size_t);int main(){}'))

cdata.set('EXV_ENABLE_BMFF', get_option('bmff'))
cdata.set('EXV_HAVE_LENSDATA', get_option('lensdata'))
cdata.set('EXV_ENABLE_VIDEO', get_option('video'))

deps = []
deps += cpp.find_library('ws2_32', required: host_machine.system() == 'windows')
deps += cpp.find_library('procstat', required: host_machine.system() == 'freebsd')

if cpp.get_argument_syntax() == 'gcc' and cpp.version().version_compare('<9')
  if host_machine.system() == 'linux' and cpp.get_define('_LIBCPP_VERSION', prefix: '#include <new>') == ''
    deps += cpp.find_library('stdc++fs')
  elif cpp.get_id() == 'clang'
    deps += cpp.find_library('c++fs')
  endif
endif

brotli_dep = dependency('libbrotlidec', disabler: true, required: false)
if brotli_dep.found()
  deps += brotli_dep
endif

curl_dep = dependency('libcurl', disabler: true, required: get_option('curl'))
if curl_dep.found()
  deps += curl_dep
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
else
  iconv_dep = dependency('', required: false)
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
cdata.set('EXV_USE_CURL', curl_dep.found())
cdata.set('EXV_ENABLE_NLS', intl_dep.found())
cdata.set('EXV_ENABLE_WEBREADY', curl_dep.found())

cfile = configure_file(
  input: 'cmake/config.h.cmake',
  output: 'exv_conf.h',
  format: 'cmake@',
  configuration: cdata,
)

base_lib = files(
  'src/asfvideo.cpp',
  'src/basicio.cpp',
  'src/bmffimage.cpp',
  'src/bmpimage.cpp',
  'src/cr2image.cpp',
  'src/crwimage.cpp',
  'src/datasets.cpp',
  'src/easyaccess.cpp',
  'src/epsimage.cpp',
  'src/error.cpp',
  'src/exif.cpp',
  'src/futils.cpp',
  'src/gifimage.cpp',
  'src/http.cpp',
  'src/image.cpp',
  'src/iptc.cpp',
  'src/jp2image.cpp',
  'src/jpgimage.cpp',
  'src/matroskavideo.cpp',
  'src/metadatum.cpp',
  'src/mrwimage.cpp',
  'src/orfimage.cpp',
  'src/pgfimage.cpp',
  'src/photoshop.cpp',
  'src/preview.cpp',
  'src/properties.cpp',
  'src/psdimage.cpp',
  'src/quicktimevideo.cpp',
  'src/rafimage.cpp',
  'src/riffvideo.cpp',
  'src/rw2image.cpp',
  'src/tags.cpp',
  'src/tgaimage.cpp',
  'src/tiffimage.cpp',
  'src/types.cpp',
  'src/value.cpp',
  'src/version.cpp',
  'src/webpimage.cpp',
  'src/xmp.cpp',
  'src/xmpsidecar.cpp',
)

int_lib = files(
  'src/canonmn_int.cpp',
  'src/casiomn_int.cpp',
  'src/cr2header_int.cpp',
  'src/crwimage_int.cpp',
  'src/fujimn_int.cpp',
  'src/helper_functions.cpp',
  'src/image_int.cpp',
  'src/jp2image_int.cpp',
  'src/makernote_int.cpp',
  'src/minoltamn_int.cpp',
  'src/nikonmn_int.cpp',
  'src/olympusmn_int.cpp',
  'src/orfimage_int.cpp',
  'src/panasonicmn_int.cpp',
  'src/pentaxmn_int.cpp',
  'src/rw2image_int.cpp',
  'src/samsungmn_int.cpp',
  'src/sigmamn_int.cpp',
  'src/sonymn_int.cpp',
  'src/tags_int.cpp',
  'src/tiffcomposite_int.cpp',
  'src/tiffimage_int.cpp',
  'src/tiffvisitor_int.cpp',
  'src/utils.cpp',
)

headers = files(
  'include/exiv2/basicio.hpp',
  'include/exiv2/bmffimage.hpp',
  'include/exiv2/bmpimage.hpp',
  'include/exiv2/config.h',
  'include/exiv2/convert.hpp',
  'include/exiv2/cr2image.hpp',
  'include/exiv2/crwimage.hpp',
  'include/exiv2/datasets.hpp',
  'include/exiv2/easyaccess.hpp',
  'include/exiv2/epsimage.hpp',
  'include/exiv2/error.hpp',
  'include/exiv2/exif.hpp',
  'include/exiv2/exiv2.hpp',
  'include/exiv2/futils.hpp',
  'include/exiv2/gifimage.hpp',
  'include/exiv2/http.hpp',
  'include/exiv2/image.hpp',
  'include/exiv2/image_types.hpp',
  'include/exiv2/iptc.hpp',
  'include/exiv2/jp2image.hpp',
  'include/exiv2/jpgimage.hpp',
  'include/exiv2/metadatum.hpp',
  'include/exiv2/mrwimage.hpp',
  'include/exiv2/orfimage.hpp',
  'include/exiv2/pgfimage.hpp',
  'include/exiv2/photoshop.hpp',
  'include/exiv2/pngimage.hpp',
  'include/exiv2/preview.hpp',
  'include/exiv2/properties.hpp',
  'include/exiv2/psdimage.hpp',
  'include/exiv2/rafimage.hpp',
  'include/exiv2/rw2image.hpp',
  'include/exiv2/slice.hpp',
  'include/exiv2/tags.hpp',
  'include/exiv2/tgaimage.hpp',
  'include/exiv2/tiffimage.hpp',
  'include/exiv2/types.hpp',
  'include/exiv2/value.hpp',
  'include/exiv2/version.hpp',
  'include/exiv2/webpimage.hpp',
  'include/exiv2/xmp_exiv2.hpp',
  'include/exiv2/xmpsidecar.hpp',
)

headers += exiv_conf
headers += cfile

libinc = include_directories('include/exiv2')
xmp_lib = files()
if expat_dep.found()
  libinc = include_directories('include/exiv2', 'xmpsdk/include')
  xmp_lib = files(
    'xmpsdk/src/ExpatAdapter.cpp',
    'xmpsdk/src/MD5.cpp',
    'xmpsdk/src/ParseRDF.cpp',
    'xmpsdk/src/UnicodeConversions.cpp',
    'xmpsdk/src/WXMPIterator.cpp',
    'xmpsdk/src/WXMPMeta.cpp',
    'xmpsdk/src/WXMPUtils.cpp',
    'xmpsdk/src/XML_Node.cpp',
    'xmpsdk/src/XMPCore_Impl.cpp',
    'xmpsdk/src/XMPIterator.cpp',
    'xmpsdk/src/XMPMeta-GetSet.cpp',
    'xmpsdk/src/XMPMeta-Parse.cpp',
    'xmpsdk/src/XMPMeta-Serialize.cpp',
    'xmpsdk/src/XMPMeta.cpp',
    'xmpsdk/src/XMPUtils-FileInfo.cpp',
    'xmpsdk/src/XMPUtils.cpp',
  )
endif

if zlib_dep.found()
  base_lib += files('src/pngimage.cpp')
  int_lib += files('src/pngchunk_int.cpp')
  headers += files('include/exiv2/photoshop.hpp')
endif

install_man('man/man1/exiv2.1')

install_headers(
  headers,
  subdir: 'exiv2',
)

exiv2int = static_library(
  'exiv2int',
  int_lib,
  cpp_args: '-DEXIV2API=',
  dependencies: deps,
  include_directories: libinc,
)

depinc = include_directories('.', 'include')
exiv2int_dep = declare_dependency(
  include_directories: depinc,
  link_with: exiv2int,
)

# This is compiled separately, because there are multiple sources for iconv
# (methods "builtin" and "system") and it's possible to have the include
# directory from one dependency leak over and force the system iconv to be used
# instead of the builtin one. This causes miscompilation.
convertlib = static_library(
  '_convert',
  'src/convert.cpp',
  cpp_args: cargs,
  gnu_symbol_visibility: 'hidden',
  dependencies: iconv_dep,
  include_directories: [depinc, libinc],
  build_by_default: false,
)

exiv2 = library(
  'exiv2',
  base_lib,
  xmp_lib,
  cpp_args: cargs,
  version: meson.project_version(),
  soversion: sover,
  gnu_symbol_visibility: 'hidden',
  link_with: convertlib,
  dependencies: [deps, exiv2int_dep],
  include_directories: libinc,
  install: true,
)

dllapi = '-DEXIV2API='
if host_machine.system() == 'windows' and get_option('default_library') != 'static'
  dllapi = '-DEXIV2API=__declspec(dllimport)'
endif

exiv2_dep = declare_dependency(
  compile_args: dllapi,
  dependencies: intl_dep,
  include_directories: depinc,
  link_with: exiv2,
)

pkg = import('pkgconfig')
pkg.generate(
  exiv2,
  description: 'Exif/IPTC/Xmp C++ metadata library and tools plus ICC Profiles, Previews and more.',
  url: 'https://exiv2.org',
)

exiv2inc = include_directories('include/exiv2', 'src')

exiv2_sources = files(
  'app/actions.cpp',
  'app/app_utils.cpp',
  'app/exiv2.cpp',
  'app/getopt.cpp',
)

if host_machine.system() == 'windows'
  exiv2_sources += files('app/wmain.cpp')
endif

executable(
  'exiv2',
  exiv2_sources,
  include_directories: exiv2inc,
  dependencies: exiv2_dep,
  install: true,
)

samples = {
  'addmoddel': [],
  'conntest': curl_dep,
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

subdir('unitTests')
