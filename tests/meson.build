pytest = find_program('pytest', required: get_option('tests'))
if not pytest.found()
  subdir_done()
endif

# most suites check XMP output. Tests for other optional features skip
# themselves when exiv2 -vV says the feature is missing
if not expat_dep.found()
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
