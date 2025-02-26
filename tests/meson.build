if not expat_dep.found()
  subdir_done()
endif

tests = {
  'bashTests': 'bash_tests',
  'bugfixTests': 'bugfixes',
  'lensTests': 'lens_tests',
  'regressionTests': 'regression_tests',
  'tiffTests': 'tiff_test',
  'versionTests': 'bash_tests/version_test.py',
}

runner = find_program('runner.py')

foreach n, t : tests
  test(
    n,
    runner,
    env: bindir,
    args: ['--verbose', t],
    timeout: 1200,
    is_parallel: false,
    workdir: meson.current_source_dir(),
  )
endforeach
