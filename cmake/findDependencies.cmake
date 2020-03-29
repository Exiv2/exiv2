# set include path for FindXXX.cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# don't use Frameworks on the Mac (#966)
if (APPLE)
     set(CMAKE_FIND_FRAMEWORK NEVER)
endif()

# Check if the conan file exist to find the dependencies
if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    set(USING_CONAN ON)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup(NO_OUTPUT_DIRS KEEP_RPATHS TARGETS)
endif()

find_package(Threads REQUIRED)

if( EXIV2_ENABLE_PNG )
    find_package( ZLIB REQUIRED )
endif( )

if( EXIV2_ENABLE_WEBREADY )
    if( EXIV2_ENABLE_CURL )
        find_package(CURL REQUIRED)
    endif()

    if( EXIV2_ENABLE_SSH )
        find_package(libssh CONFIG REQUIRED)
        # Define an imported target to have compatibility with <=libssh-0.9.0
        # libssh-0.9.1 is broken regardless.
        if(NOT TARGET ssh)
            add_library(ssh SHARED IMPORTED)
            set_target_properties(ssh PROPERTIES
                IMPORTED_LOCATION "${LIBSSH_LIBRARIES}"
                INTERFACE_INCLUDE_DIRECTORIES "${LIBSSH_INCLUDE_DIR}"
            )
        endif()
    endif()
endif()

if (EXIV2_ENABLE_XMP AND EXIV2_ENABLE_EXTERNAL_XMP)
    message(FATAL_ERROR "EXIV2_ENABLE_XMP AND EXIV2_ENABLE_EXTERNAL_XMP are mutually exclusive.  You can only choose one of them")
else()
    if (EXIV2_ENABLE_XMP)
        find_package(EXPAT REQUIRED)
    elseif (EXIV2_ENABLE_EXTERNAL_XMP)
        find_package(XmpSdk REQUIRED)
    endif ()
endif()

if (EXIV2_ENABLE_NLS)
    find_package(Intl REQUIRED)
endif( )

find_package(Iconv)
if( ICONV_FOUND )
    message ( "-- ICONV_INCLUDE_DIR : " ${Iconv_INCLUDE_DIR} )
    message ( "-- ICONV_LIBRARIES : " ${Iconv_LIBRARY} )
endif()

if( BUILD_WITH_CCACHE )
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        message(STATUS "Program ccache found")
        set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
        set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
    endif()
endif()

