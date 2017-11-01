if ( MINGW OR UNIX ) # MINGW, Linux, APPLE, CYGWIN
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        set(COMPILER_IS_GCC ON)
    elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        set(COMPILER_IS_CLANG ON)
    endif()

    if (COMPILER_IS_GCC OR COMPILER_IS_CLANG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wcast-align -Wpointer-arith -Wformat-security -Wmissing-format-attribute -Woverloaded-virtual -W")

        if ( CYGWIN OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0))
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++98") # to support snprintf
        else()
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++98")
        endif()

        if ( EXIV2_TEAM_WARNINGS_AS_ERRORS )
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")
        endif ()

        if ( EXIV2_TEAM_EXTRA_WARNINGS )
            # Note that this is intended to be used only by Exiv2 developers/contributors.

            if ( COMPILER_IS_GCC )
                if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.0 )
                    string(CONCAT EXTRA_COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
                        " -Wextra"
                        " -Wlogical-op"
                        " -Wdouble-promotion"
                        " -Wshadow"
                        " -Wuseless-cast"
                        #" -Wold-style-cast"
                    )
                endif ()

                if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 6.0 )
                    string(CONCAT EXTRA_COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
                        " -Wduplicated-cond"
                    )
                endif ()

                if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 7.0 )
                    string(CONCAT EXTRA_COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
                        " -Wduplicated-branches"
                        " -Wrestrict"
                    )
                endif ()
            endif ()

            if ( COMPILER_IS_CLANG )
                # https://clang.llvm.org/docs/DiagnosticsReference.html
                # These variables are at least available since clang 3.9.1
                string(CONCAT EXTRA_COMPILE_FLAGS "-Wextra" 
                    " -Wdouble-promotion"
                    " -Wshadow"
                    " -Wassign-enum"
                    " -Wmicrosoft"
                    " -Wcomma"
                    " -Wcomments"
                    " -Wconditional-uninitialized"
                    " -Wdirect-ivar-access"
                    " -Weffc++"
                    # These two raises lot of warnings. Use them wisely
                    #" -Wconversion"
                    #" -Wold-style-cast"
                )
            endif ()


        endif ()
    endif()

endif ()

# http://stackoverflow.com/questions/10113017/setting-the-msvc-runtime-in-cmake
if(MSVC)
    set(variables
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

    if ( EXIV2_WARNINGS_AS_ERRORS )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /WX")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /WX")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /WX")
    endif ()

    if ( EXIV2_EXTRA_WARNINGS )
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif ()

endif()
