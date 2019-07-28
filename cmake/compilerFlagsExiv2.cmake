# These flags only applies to exiv2lib, and the applications, but not to the xmp code

if (COMPILER_IS_GCC OR COMPILER_IS_CLANG) # MINGW, Linux, APPLE, CYGWIN
    if ( EXIV2_TEAM_WARNINGS_AS_ERRORS )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
    endif ()
endif()

if (MSVC)
    if ( EXIV2_TEAM_WARNINGS_AS_ERRORS )
        add_compile_options(/WX)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /WX")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /WX")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /WX")
    endif ()
endif ()
