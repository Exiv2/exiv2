base_lib = files(
  'basicio.cpp',
  'bmffimage.cpp',
  'bmpimage.cpp',
  'cr2image.cpp',
  'crwimage.cpp',
  'datasets.cpp',
  'easyaccess.cpp',
  'epsimage.cpp',
  'error.cpp',
  'exif.cpp',
  'futils.cpp',
  'gifimage.cpp',
  'http.cpp',
  'image.cpp',
  'iptc.cpp',
  'jp2image.cpp',
  'jpgimage.cpp',
  'metadatum.cpp',
  'mrwimage.cpp',
  'orfimage.cpp',
  'pgfimage.cpp',
  'photoshop.cpp',
  'pngimage.cpp',
  'preview.cpp',
  'properties.cpp',
  'psdimage.cpp',
  'rafimage.cpp',
  'rw2image.cpp',
  'tags.cpp',
  'tgaimage.cpp',
  'tiffimage.cpp',
  'types.cpp',
  'value.cpp',
  'version.cpp',
  'webpimage.cpp',
  'xmp.cpp',
  'xmpsidecar.cpp',
)

if get_option('video')
  base_lib += files('asfvideo.cpp', 'matroskavideo.cpp', 'quicktimevideo.cpp', 'riffvideo.cpp')
endif

int_lib = files(
  'canonmn_int.cpp',
  'casiomn_int.cpp',
  'cr2header_int.cpp',
  'crwimage_int.cpp',
  'fujimn_int.cpp',
  'helper_functions.cpp',
  'image_int.cpp',
  'jp2image_int.cpp',
  'makernote_int.cpp',
  'minoltamn_int.cpp',
  'nikonmn_int.cpp',
  'olympusmn_int.cpp',
  'orfimage_int.cpp',
  'panasonicmn_int.cpp',
  'pentaxmn_int.cpp',
  'pngchunk_int.cpp',
  'rw2image_int.cpp',
  'samsungmn_int.cpp',
  'sigmamn_int.cpp',
  'sonymn_int.cpp',
  'tags_int.cpp',
  'tiffcomposite_int.cpp',
  'tiffimage_int.cpp',
  'tiffvisitor_int.cpp',
  'utils.cpp',
)

exiv2int = static_library(
  'exiv2int',
  int_lib,
  cpp_args: '-DEXIV2API=',
  dependencies: deps,
  include_directories: libinc,
)

exiv2int_dep = declare_dependency(
  include_directories: libinc,
  link_with: exiv2int,
)

# This is compiled separately, because there are multiple sources for iconv
# (methods "builtin" and "system") and it's possible to have the include
# directory from one dependency leak over and force the system iconv to be used
# instead of the builtin one. This causes miscompilation.
convertlib = static_library(
  '_convert',
  'convert.cpp',
  cpp_args: cargs,
  gnu_symbol_visibility: 'hidden',
  dependencies: iconv_dep,
  include_directories: libinc,
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
  install: true,
)

dllapi = '-DEXIV2API='
if host_machine.system() == 'windows' and get_option('default_library') != 'static'
  dllapi = '-DEXIV2API=__declspec(dllimport)'
endif

cmake = import('cmake')
cmake.write_basic_package_version_file(
  name: meson.project_name(),
  version: meson.project_version(),
)

pkg = import('pkgconfig')
pkg.generate(
  exiv2,
  description: 'Exif/IPTC/Xmp C++ metadata library and tools plus ICC Profiles, Previews and more.',
  url: 'https://exiv2.org',
)

exiv2_dep = declare_dependency(
  compile_args: dllapi,
  dependencies: intl_dep,
  include_directories: depinc,
  link_with: exiv2,
)
