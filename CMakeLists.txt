# CMakeLists.txt for exiv2 library

cmake_minimum_required( VERSION 3.3.2 )
project( exiv2 )

include(GNUInstallDirs)
include(CheckFunctionExists)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_MACOSX_RPATH ON)

if (APPLE)
    set(CMAKE_INSTALL_RPATH "@loader_path")
endif()

set( PACKAGE_COPYRIGHT      "Andreas Huggel" )
set( PACKAGE_BUGREPORT      "http://github.com/exiv2/exiv2" )
set( PACKAGE                "exiv2" )
set( PACKAGE_NAME           "exiv2" )
set( PACKAGE_VERSION        "0.26" )
set( PACKAGE_URL            "http://www.exiv2.org" )
set( PACKAGE_STRING         "${PACKAGE_NAME} ${PACKAGE_VERSION}" )
set( GENERIC_LIB_VERSION    "26.0.0" )
set( GENERIC_LIB_SOVERSION  "26" )

# options and their default values
option( BUILD_SHARED_LIBS             "Build exiv2lib as a shared library"                    ON  )
option( EXIV2_ENABLE_XMP              "Build with XMP metadata support"                       ON  )
option( EXIV2_ENABLE_EXTERNAL_XMP     "Use external version of XMP"                           OFF )
option( EXIV2_ENABLE_PNG              "Build with png support (requires libz)"                ON  )
option( EXIV2_ENABLE_NLS              "Build native language support (requires gettext)"      ON  )
option( EXIV2_ENABLE_PRINTUCS2        "Build with Printucs2"                                  ON  )
option( EXIV2_ENABLE_LENSDATA         "Build including lens data"                             ON  )
option( EXIV2_ENABLE_COMMERCIAL       "Build with the EXV_COMMERCIAL_VERSION symbol set"      OFF )
option( EXIV2_ENABLE_VIDEO            "Build video support into library"                      OFF )
option( EXIV2_ENABLE_WEBREADY         "Build webready support into library"                   OFF )
option( EXIV2_ENABLE_DYNAMIC_RUNTIME  "Use dynamic runtime (used for static libs)"            OFF )
option( EXIV2_ENABLE_WIN_UNICODE      "Use Unicode paths (wstring) on Windows"                OFF )
option( EXIV2_ENABLE_CURL             "USE Libcurl for HttpIo"                                OFF )
option( EXIV2_ENABLE_SSH              "USE Libssh for SshIo"                                  OFF )

option( EXIV2_BUILD_SAMPLES           "Build sample applications"                             ON  )
option( EXIV2_BUILD_PO                "Build translations files"                              OFF )
option( EXIV2_BUILD_EXIV2_COMMAND     "Build exiv2 command-line executable"                   ON  )
option( EXIV2_BUILD_UNIT_TESTS        "Build unit tests"                                      OFF )
option( EXIV2_BUILD_DOC               "Add 'doc' target to generate documentation"            OFF )

# Only intended to be used by Exiv2 developers/contributors
option( EXIV2_TEAM_EXTRA_WARNINGS     "Add more sanity checks using compiler flags"           OFF )
option( EXIV2_TEAM_WARNINGS_AS_ERRORS "Treat warnings as errors"                              OFF )
set(EXTRA_COMPILE_FLAGS " ")

mark_as_advanced(
    EXIV2_TEAM_EXTRA_WARNINGS 
    EXIV2_TEAM_WARNINGS_AS_ERRORS
    EXIV2_ENABLE_EXTERNAL_XMP
    EXTRA_COMPILE_FLAGS
)

option( BUILD_WITH_CCACHE             "Use ccache to speed up compilations"                   OFF )

if ( EXIV2_ENABLE_WEBREADY )
    set ( EXIV2_ENABLE_CURL ON )
    set ( EXIV2_ENABLE_CURL ON )
    if ( UNIX )
        set ( EXIV2_ENABLE_SSH  ON )
    endif ()
endif()

if( EXIV2_ENABLE_COMMERCIAL )
    set (EXIV2_ENABLE_LENSDATA OFF)
    set (EXIV2_ENABLE_NLS OFF)
endif()

if ( EXIV2_ENABLE_EXTERNAL_XMP )
    set(EXIV2_ENABLE_XMP OFF)
endif()

if( EXIV2_BUILD_UNIT_TESTS )
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON) # Requires CMake 3.3.3
endif()

include(cmake/findDependencies.cmake   REQUIRED)
include(cmake/compilerFlags.cmake      REQUIRED)
include(cmake/generateConfigFile.cmake REQUIRED)

if (EXIV2_BUILD_DOC)
    include(cmake/generateDoc.cmake        REQUIRED)
    generate_documentation("${PROJECT_SOURCE_DIR}/cmake/Doxyfile.in")
endif()


include_directories(${CMAKE_BINARY_DIR})  # Make the exv_conf.h file visible for the full project

include(cmake/CMakeChecks.txt)

if( EXIV2_ENABLE_XMP )
    add_subdirectory( xmpsdk )
endif()

add_subdirectory( src )

if( EXIV2_BUILD_UNIT_TESTS )
    add_subdirectory ( unitTests )
endif()

if( EXIV2_BUILD_SAMPLES )
    add_subdirectory( samples )
endif()

if( EXIV2_BUILD_PO )
    add_subdirectory( po )
endif()

##
# tests
add_custom_target(tests
    COMMAND env EXIV2_BUILDDIR="${CMAKE_BINARY_DIR}" make test
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/test"
)

get_directory_property(SAMPLES DIRECTORY samples DEFINITION APPLICATIONS)
add_dependencies(tests exiv2lib exiv2 ${SAMPLES})

include(cmake/printSummary.cmake)

# That's all Folks!
##
