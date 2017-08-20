if ( MINGW OR UNIX ) # MINGW, Linux, APPLE, CYGWIN
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wall
                                               -Wcast-align
                                               -Wpointer-arith
                                               -Wformat-security
                                               -Wmissing-format-attribute
                                               -Woverloaded-virtual
                                               -W
            )
    endif()

    if ( CYGWIN OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0))
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=gnu++98 ) # to support snprintf
    else()
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++98 )
    endif()
endif ()
