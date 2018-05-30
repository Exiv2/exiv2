set( EXIV2_SRC
     exiv2.cpp
     exiv2app.hpp
     actions.cpp
     actions.hpp
     utils.cpp
     utils.hpp
     private.h
     tzfile.h
)

add_executable( exiv2 ${EXIV2_SRC} )
set_target_properties( exiv2 PROPERTIES
    COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
)

target_compile_definitions(exiv2 PRIVATE EXV_LOCALEDIR="${CMAKE_INSTALL_LOCALEDIR}" )
target_link_libraries( exiv2 PRIVATE exiv2lib )
if ( BUILD_SHARED_LIBS )
    target_compile_definitions(exiv2 PRIVATE EXV_HAVE_DLL )
endif()
# modify source lists to suit environment

# TODO This should not be needed here! we need to fix the previous TODO
target_include_directories(exiv2 PRIVATE ${CMAKE_SOURCE_DIR}/include/)

if(NOT EXV_HAVE_TIMEGM )
    target_sources(exiv2    PRIVATE localtime.c)
endif()

target_sources(exiv2 PRIVATE getopt_win32.c getopt_win32.h)

# Copy DLLs from conan packages to the bin folder
if (USING_CONAN AND WIN32)
    add_custom_command(TARGET exiv2 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_BINARY_DIR}/conanDlls ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

install(TARGETS exiv2 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
# Man page
install( FILES exiv2.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

