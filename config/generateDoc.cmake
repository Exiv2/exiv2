# -helper macro to add a "doc" target with CMake build system.
# and configure doxy.config.in to doxy.config
#
# target "doc" allows building the documentation with doxygen/dot on WIN32, Linux and Mac
#

macro(generate_documentation DOX_CONFIG_FILE)
    if(NOT EXISTS "${DOX_CONFIG_FILE}")
        message(FATAL_ERROR "Configuration file for doxygen not found")
    endif()

    #Define variables
    set(INCDIR  "${PROJECT_SOURCE_DIR}/include/exiv2")
    set(SRCDIR  "${PROJECT_SOURCE_DIR}/src")
    set(ROOTDIR "${PROJECT_SOURCE_DIR}")
    #set(TESTSDIR "${PROJECT_SOURCE_DIR}/tests")

    configure_file(${DOX_CONFIG_FILE} ${CMAKE_CURRENT_BINARY_DIR}/doxy.config @ONLY) #OUT-OF-PLACE LOCATION
    set(DOXY_CONFIG "${CMAKE_CURRENT_BINARY_DIR}/doxy.config")
    add_custom_target(doc ${DOXYGEN_EXECUTABLE} ${DOXY_CONFIG})

    install(DIRECTORY "${PROJECT_BINARY_DIR}/doc/html/" DESTINATION "share/doc/lib${PROJECT_NAME}")

    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES doc)
endmacro()

find_package(Doxygen)

if(DOXYGEN_FOUND AND DOXYGEN_DOT_EXECUTABLE)
    message(STATUS "Doxygen and dot found. Adding 'doc' target for generating the documentation")
    generate_documentation("${PROJECT_SOURCE_DIR}/config/Doxyfile.in")
else()
    message(STATUS "doxygen or dot not found. Both commands are needed for the adding the 'doc' target")
endif()

