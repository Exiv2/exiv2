# In this file we configure some CMake settings we do not want to make visible directly in the main
# CMakeLists.txt file.

include(GNUInstallDirs)
include(CheckFunctionExists)
include(GenerateExportHeader)
include(CMakeDependentOption)
include(cmake/JoinPaths.cmake)
include(CTest)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
endif()

if (UNIX)
    if (APPLE)
        set(CMAKE_MACOSX_RPATH ON)
        set(CMAKE_INSTALL_RPATH "@loader_path")
    else()
        join_paths(CMAKE_INSTALL_RPATH "$ORIGIN" ".." "${CMAKE_INSTALL_LIBDIR}")
    endif()
endif()

# Prevent conflicts when exiv2 is consumed in multiple-subdirectory projects.
if (NOT TARGET uninstall)
  configure_file(cmake/exiv2_uninstall.cmake ${CMAKE_BINARY_DIR}/cmake_uninstall.cmake COPYONLY)
  add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake")
endif()
