runner = find_program('runner.py', required: get_option('tests'))
if not runner.found()
  subdir_done()
endif

# these are all needed. Not sure if conditional testing is desired.
if not (brotli_dep.found() and expat_dep.found() and inih_dep.found() and web_dep.found() and zlib_dep.found())
  subdir_done()
endif

# windows does conversion without iconv
if not (host_machine.system() == 'windows' or iconv_dep.found())
  subdir_done()
endif

tests = {
  'bashTests': 'bash_tests',
  'bugfixTests': 'bugfixes',
  'lensTests': 'lens_tests',
  'tiffTests': 'tiff_test',
  'versionTests': 'bash_tests/version_test.py',
}

foreach n, t : tests
  test(
    n,
    runner,
    env: bindir,
    args: ['--verbose', t],
    timeout: 1200,
    workdir: meson.current_source_dir(),
  )
endforeach

test(
  'regressionTests',
  runner,
  env: bindir,
  args: ['--verbose', 'regression_tests'],
  timeout: 1200,
  is_parallel: false,
  workdir: meson.current_source_dir(),
)
