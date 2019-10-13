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
    conan_basic_setup(NO_OUTPUT_DIRS KEEP_RPATHS SKIP_STD TARGETS)

    # This is needed to find the 3rd party libraries cross-compiled by conan with cmake
    if (CMAKE_CROSSCOMPILING)
        set(CMAKE_FIND_ROOT_PATH "${CMAKE_FIND_ROOT_PATH};${CMAKE_PREFIX_PATH}")
    endif()
endif()

find_package(Threads REQUIRED)

if( EXIV2_ENABLE_PNG )
    find_package( ZLIB REQUIRED )
endif( )

if( EXIV2_ENABLE_WEBREADY )
    if( EXIV2_ENABLE_CURL )
        find_package(CURL REQUIRED)
    endif()
endif( )

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

if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
  set(CMAKE_REQUIRED_FLAGS "-std=c++11")
  # source: https://stackoverflow.com/questions/12530406/is-gcc-4-8-or-earlier-buggy-about-regular-expressions/41186162#41186162
  check_cxx_source_compiles(
    "#include <regex>
#if __cplusplus >= 201103L &&                             \
    (!defined(__GLIBCXX__) || (__cplusplus >= 201402L) || \
        (defined(_GLIBCXX_REGEX_DFS_QUANTIFIERS_LIMIT) || \
         defined(_GLIBCXX_REGEX_STATE_LIMIT)           || \
             (defined(_GLIBCXX_RELEASE)                && \
             _GLIBCXX_RELEASE > 4)))
int main() { return 0; }
#else
#error \"regex not working\"
#endif
"
    BUILTIN_REGEX_WORKING
    FAIL_REGEX '.*regex not working.*'
    )
  set(CMAKE_REQUIRED_FLAGS "")

  if( NOT BUILTIN_REGEX_WORKING )
    set( EXV_NEED_BOOST_REGEX ON
      CACHE
      BOOL
      "Need Boost::regex as this version of gcc ships a broken implementation of <regex>")
    find_package(Boost REQUIRED COMPONENTS regex)
  endif()
endif()
