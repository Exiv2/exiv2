# Minimum version imposed by Ubuntu:20.04
cmake_minimum_required(VERSION 3.16.3)

# use TWEAK to categorize the build:
#
# * 1.00.0    = GM               (tagged and released)
# * 1.00.0.9  = 1.00.0 Development
# * 1.00.0.00 = GM Preview
# * 1.00.0.2  = RC2              (tagged and released)
# * 1.00.0.20 = RC2 Preview
# * 1.00.0.29 = RC2 Development
#
project(
  exiv2
  VERSION 1.00.0.9
  DESCRIPTION "Exif/IPTC/Xmp C++ metadata library and tools plus ICC Profiles, Previews and more."
  LANGUAGES C CXX
)

# Shared Object versioning (SemVer-like: must bump major on API breakage)
if(PROJECT_VERSION_MAJOR EQUAL 0)
  # support legacy scheme (e.g. 0.27.x -> 27)
  set(EXIV2LIB_SOVERSION ${PROJECT_VERSION_MINOR})
else()
  # restart from 30
  math(EXPR EXIV2LIB_SOVERSION "30 + (${PROJECT_VERSION_MAJOR} - 1)")
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(cmake/mainSetup.cmake)

# options and their default values
option(BUILD_SHARED_LIBS "Build exiv2lib as a shared library" ON)
option(EXIV2_ENABLE_XMP "Build with XMP metadata support" ON)
option(EXIV2_ENABLE_EXTERNAL_XMP "Use external version of XMP" OFF)
option(EXIV2_ENABLE_PNG "Build with PNG support (requires zlib)" ON)
option(EXIV2_ENABLE_NLS "Build native language support (requires gettext)" OFF)
option(EXIV2_ENABLE_LENSDATA "Build including Nikon lens data" ON)
option(EXIV2_ENABLE_DYNAMIC_RUNTIME "Use dynamic runtime (used for static libs)" ON)
option(EXIV2_ENABLE_WEBREADY "Build webready support into library" OFF)
option(EXIV2_ENABLE_CURL "Use libcurl for HttpIo (WEBREADY)" OFF)
option(EXIV2_ENABLE_BMFF "Build with BMFF support" ON)
option(EXIV2_ENABLE_BROTLI "Use Brotli for JPEG XL compressed boxes (BMFF)" ON)
option(EXIV2_ENABLE_VIDEO "Build with video support" ON)
option(EXIV2_ENABLE_INIH "Use inih library" ON)
option(EXIV2_ENABLE_FILESYSTEM_ACCESS "Build with filesystem access" ON)

option(EXIV2_BUILD_SAMPLES "Build sample applications" OFF)
option(EXIV2_BUILD_EXIV2_COMMAND "Build exiv2 command-line executable" ON)
option(EXIV2_BUILD_UNIT_TESTS "Build unit tests" OFF)
option(EXIV2_BUILD_FUZZ_TESTS "Build fuzz tests (libFuzzer)" OFF)
option(EXIV2_BUILD_DOC "Add 'doc' target to generate documentation" OFF)

# Only intended to be used by Exiv2 developers/contributors
option(EXIV2_TEAM_EXTRA_WARNINGS "Add more sanity checks using compiler flags" OFF)
option(EXIV2_TEAM_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(EXIV2_TEAM_USE_SANITIZERS "Enable ASAN and UBSAN when available" OFF)

# The EXIV2_TEAM_OSS_FUZZ option is used by the OSS-Fuzz build script:
#
# * https://github.com/google/oss-fuzz/tree/master/projects/exiv2/build.sh
#
option(EXIV2_TEAM_OSS_FUZZ "Build config for OSS-Fuzz" OFF)

option(EXIV2_TEAM_PACKAGING "Additional stuff for generating packages" OFF)
set(EXTRA_COMPILE_FLAGS " ")

mark_as_advanced(EXIV2_TEAM_EXTRA_WARNINGS EXIV2_TEAM_WARNINGS_AS_ERRORS EXIV2_ENABLE_EXTERNAL_XMP EXTRA_COMPILE_FLAGS EXIV2_TEAM_USE_SANITIZERS)

option(BUILD_WITH_STACK_PROTECTOR "Build with stack protector" ON)
option(BUILD_WITH_CCACHE "Use ccache to speed up compilations" OFF)
option(BUILD_WITH_COVERAGE "Add compiler flags to generate coverage stats" OFF)
include(cmake/gcovr.cmake REQUIRED)

set(PACKAGE_URL "https://exiv2.org")

if(EXIV2_ENABLE_EXTERNAL_XMP)
  set(EXIV2_ENABLE_XMP OFF)
endif()

include(cmake/findDependencies.cmake REQUIRED)
include(cmake/compilerFlags.cmake REQUIRED)
include(cmake/generateConfigFile.cmake REQUIRED)

if(EXIV2_BUILD_DOC)
  include(cmake/generateDoc.cmake REQUIRED)
  generate_documentation("${PROJECT_SOURCE_DIR}/cmake/Doxyfile.in")
endif()

include_directories(${CMAKE_BINARY_DIR}) # Make the exv_conf.h file visible for the full project

if(NOT EXV_HAVE_STD_FORMAT)
  find_package(fmt REQUIRED)
endif()

if(EXIV2_ENABLE_XMP)
  add_subdirectory(xmpsdk)
endif()

include(cmake/compilerFlagsExiv2.cmake REQUIRED)

add_subdirectory(src)

if(BUILD_TESTING AND EXIV2_BUILD_UNIT_TESTS)
  set(EXIV2_ENABLE_FILESYSTEM_ACCESS ON)
  add_subdirectory(unitTests)
endif()

if(EXIV2_BUILD_FUZZ_TESTS)
  set(EXIV2_ENABLE_FILESYSTEM_ACCESS ON)
  add_subdirectory(fuzz)
endif()

if(EXIV2_BUILD_EXIV2_COMMAND)
  add_subdirectory(app)
  set(EXIV2_ENABLE_FILESYSTEM_ACCESS ON)

  if(EXIV2_BUILD_SAMPLES)
    add_subdirectory(samples)
    set(EXIV2_ENABLE_FILESYSTEM_ACCESS ON)
    get_directory_property(SAMPLES DIRECTORY samples DEFINITION APPLICATIONS)

    if(BUILD_TESTING AND Python3_Interpreter_FOUND)
      add_test(
        NAME bashTests
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
        COMMAND cmake -E env EXIV2_BINDIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${Python3_EXECUTABLE} runner.py --verbose bash_tests
      )
    endif()
  endif()

  if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(
      NAME bugfixTests
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
      COMMAND cmake -E env EXIV2_BINDIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${Python3_EXECUTABLE} runner.py --verbose bugfixes
    )
    add_test(
      NAME lensTests
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
      COMMAND cmake -E env EXIV2_BINDIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${Python3_EXECUTABLE} runner.py --verbose lens_tests
    )
    # Repeat lens test with a DE locale that uses "," as float delimiter
    # to check that all float conversions are done independently of locale
    # See GitHub issue #2746
    add_test(
      NAME lensTestsLocaleDE
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
      COMMAND cmake -E env LC_ALL=de_DE.UTF-8 EXIV2_BINDIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${Python3_EXECUTABLE} runner.py --verbose lens_tests
    )
    add_test(
      NAME tiffTests
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
      COMMAND cmake -E env EXIV2_BINDIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${Python3_EXECUTABLE} runner.py --verbose tiff_test
    )
    add_test(
      NAME versionTests
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
      COMMAND cmake -E env EXIV2_BINDIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${Python3_EXECUTABLE} runner.py --verbose bash_tests/version_test.py
    )
    add_test(
      NAME regressionTests
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
      COMMAND cmake -E env EXIV2_BINDIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY} ${Python3_EXECUTABLE} runner.py --verbose regression_tests
    )
  endif()
endif()

if(EXIV2_ENABLE_NLS)
  add_subdirectory(po)
endif()

if(EXIV2_TEAM_PACKAGING)
  include(cmake/packaging.cmake)
endif()

# Handle both relative and absolute paths (e.g. NixOS) for a relocatable package
if(IS_ABSOLUTE "${CMAKE_INSTALL_LIBDIR}")
  set(EXIV2_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
else()
  join_paths(EXIV2_LIBDIR "\${exec_prefix}" "${CMAKE_INSTALL_LIBDIR}")
endif()
if(IS_ABSOLUTE "${CMAKE_INSTALL_INCLUDEDIR}")
  set(EXIV2_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
else()
  join_paths(EXIV2_INCLUDEDIR "\${prefix}" "${CMAKE_INSTALL_INCLUDEDIR}")
endif()
configure_file(cmake/exiv2.pc.in exiv2.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/exiv2.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# ******************************************************************************
# Man page
install(FILES ${PROJECT_SOURCE_DIR}/man/man1/exiv2.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)

include(cmake/printSummary.cmake)

# That's all Folks!
#
