set(inih_LIBRARY_NAMES "inih" "libinih")
set(inih_inireader_LIBRARY_NAMES "INIReader" "libINIReader")

find_path(inih_INCLUDE_DIR
  NAMES "ini.h"
  DOC "inih include directory")

find_path(inih_inireader_INCLUDE_DIR
  NAMES "INIReader.h"
  DOC "INIReader include directory")

find_library(inih_LIBRARY
  NAMES ${inih_LIBRARY_NAMES}
  DOC "inih library")

find_library(inih_inireader_LIBRARY
  NAMES ${inih_inireader_LIBRARY_NAMES}
  DOC "inih library")

mark_as_advanced(inih_INCLUDE_DIR)
mark_as_advanced(inih_LIBRARY)
mark_as_advanced(inih_inireader_INCLUDE_DIR)
mark_as_advanced(inih_inireader_LIBRARY)

find_package_handle_standard_args(inih REQUIRED_VARS inih_LIBRARY inih_INCLUDE_DIR inih_inireader_INCLUDE_DIR inih_inireader_LIBRARY)

if(NOT inih_FOUND)
  message(FATAL_ERROR "inih library not found")
endif()

set(inih_INCLUDE_DIRS "${inih_INCLUDE_DIR}")
set(inih_LIBRARIES "${inih_LIBRARY}")
if(NOT TARGET inih::libinih)
  add_library(inih::libinih INTERFACE IMPORTED)
endif()
set_property(TARGET inih::libinih PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${inih_INCLUDE_DIRS}")
set_property(TARGET inih::libinih PROPERTY INTERFACE_LINK_LIBRARIES "${inih_LIBRARIES}")

set(inih_inireader_INCLUDE_DIRS "${inih_inireader_INCLUDE_DIR}")
set(inih_inireader_LIBRARIES "${inih_inireader_LIBRARY}")
if(NOT TARGET inih::inireader)
  add_library(inih::inireader INTERFACE IMPORTED)
endif()
set_property(TARGET inih::inireader PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${inih_inireader_INCLUDE_DIRS}")
set_property(TARGET inih::inireader PROPERTY INTERFACE_LINK_LIBRARIES "${inih_inireader_LIBRARIES}")
