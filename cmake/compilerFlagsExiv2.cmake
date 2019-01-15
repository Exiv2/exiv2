# These flags only applies to exiv2lib, and the applications, but not to the xmp code

if (MINGW OR UNIX) # MINGW, Linux, APPLE, CYGWIN
    if ( EXIV2_TEAM_WARNINGS_AS_ERRORS )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations")
    endif ()
endif()

if (MSVC)
    if ( EXIV2_TEAM_WARNINGS_AS_ERRORS )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /WX")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /WX")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /WX")
    endif ()
endif ()
