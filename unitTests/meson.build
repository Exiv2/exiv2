gtest_dep = dependency('gmock_main', required: get_option('unitTests'))
if not gtest_dep.found()
  subdir_done()
endif

test_sources = files(
  'test_DateValue.cpp',
  'test_Error.cpp',
  'test_FileIo.cpp',
  'test_ImageFactory.cpp',
  'test_IptcKey.cpp',
  'test_LangAltValueRead.cpp',
  'test_Photoshop.cpp',
  'test_TimeValue.cpp',
  'test_XmpKey.cpp',
  'test_basicio.cpp',
  'test_bmpimage.cpp',
  'test_cr2header_int.cpp',
  'test_datasets.cpp',
  'test_enforce.cpp',
  'test_futils.cpp',
  'test_helper_functions.cpp',
  'test_image_int.cpp',
  'test_jp2image.cpp',
  'test_jp2image_int.cpp',
  'test_safe_op.cpp',
  'test_slice.cpp',
  'test_tiffheader.cpp',
  'test_types.cpp',
  'test_utils.cpp',
)

if get_option('video')
  test_sources += files(
    'test_asfvideo.cpp',
    'test_matroskavideo.cpp',
    'test_riffVideo.cpp',
  )
endif

if zlib_dep.found()
  test_sources += files(
    'test_pngimage.cpp',
  )
endif

if host_machine.system() == 'windows' and get_option('default_library') != 'static'
  test_sources += int_lib
endif

t_args = ['-UEXIV2API', '-DEXIV2API=', '-DTESTDATA_PATH="@0@"'.format(meson.current_source_dir() / '..' / 'test' / 'data')]

unit_tests = executable(
  'unit_tests',
  test_sources,
  cpp_args: t_args,
  dependencies: [deps, exiv2_dep, gtest_dep],
  include_directories: exiv2inc,
)

test('Unit Tests', unit_tests)
