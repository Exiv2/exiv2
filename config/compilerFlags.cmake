if ( MINGW OR UNIX ) # MINGW, Linux, APPLE, CYGWIN
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wcast-align -Wpointer-arith -Wformat-security -Wmissing-format-attribute -Woverloaded-virtual -W")
    endif()

    if ( CYGWIN OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0))
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++98") # to support snprintf
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
    endif()

endif ()

# http://stackoverflow.com/questions/10113017/setting-the-msvc-runtime-in-cmake
if(MSVC)
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    if ( ${BUILD_SHARED_LIBS} OR ${EXIV2_ENABLE_DYNAMIC_RUNTIME} )
        message(STATUS  "MSVC -> forcing use of dynamically-linked runtime." )
        foreach(variable ${variables})
            if(${variable} MATCHES "/MT")
                string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
            endif()
        endforeach()
    else()
        message(STATUS "MSVC -> forcing use of statically-linked runtime." )
        foreach(variable ${variables})
            if(${variable} MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
            endif()
            set(${variable} "${${variable}} /DXML_STATIC /D_LIB")
        endforeach()
    endif()

    # remove /Ob2 and /Ob1 - they cause linker issues
    set(obs /Ob2 /Ob1)
    foreach(ob ${obs})
        foreach(variable ${variables})
            if(${variable} MATCHES ${ob} )
                string(REGEX REPLACE ${ob} "" ${variable} "${${variable}}")
            endif()
      endforeach()
    endforeach()

    # don't link msvcrt for .exe which use shared libraries (use default libcmt)
    if ( NOT ${BUILD_SHARED_LIBS} AND NOT ${EXIV2_ENABLE_DYNAMIC_RUNTIME})
        set(CMAKE_EXE_LINKER_FLAGS_DEBUG          "/NODEFAULTLIB:MSVCRTD")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE        "/NODEFAULTLIB:MSVCRT")
        set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL     "/NODEFAULTLIB:MSVCRT")
        set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB:MSVCRT")
    endif()

    # Resolving Redefinition Errors Betwen ws2def.h and winsock.h:
    #  - http://www.zachburlingame.com/2011/05/resolving-redefinition-errors-betwen-ws2def-h-and-winsock-h/
    #  - https://stackoverflow.com/questions/11040133/what-does-defining-win32-lean-and-mean-exclude-exactly
    add_definitions(-DWIN32_LEAN_AND_MEAN)

endif()
