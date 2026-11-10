pytest = find_program('pytest', required: get_option('tests'))
if not pytest.found()
  subdir_done()
endif

# the suites fail without these: XMP (expat), PNG (zlib), the lens config
# file (inih) and the http iotest (webready)
if not (expat_dep.found() and inih_dep.found() and web_dep.found() and zlib_dep.found())
  subdir_done()
endif

# windows does conversion without iconv
if not (host_machine.system() == 'windows' or iconv_dep.found())
  subdir_done()
endif

tests = {
  'bashTests': ['bash_tests'],
  'bugfixTests': ['bugfixes'],
  'lensTests': ['lens_tests'],
  'regressionTests': ['regression_tests'],
  'tiffTests': ['tiff_test'],
  # -s so that the version information ends up in the test log
  'versionTests': ['-s', 'bash_tests/version_test.py'],
}

foreach n, t : tests
  test(
    n,
    pytest,
    env: ['MSYSTEM=', bindir],
    args: ['--verbose'] + t,
    timeout: 1200,
    workdir: meson.current_source_dir(),
  )
endforeach
