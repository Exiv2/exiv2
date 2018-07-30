if ( MINGW OR UNIX ) # MINGW, Linux, APPLE, CYGWIN
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        set(COMPILER_IS_GCC ON)
    elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        set(COMPILER_IS_CLANG ON)
    endif()

    set (CMAKE_CXX_FLAGS_DEBUG      "-g3 -gstrict-dwarf -O0")

    if (CMAKE_GENERATOR MATCHES "Xcode")
        set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
        if (EXIV2_ENABLE_EXTERNAL_XMP)
            # XMP SDK 2016 uses libstdc++ even when it is deprecated in modern versions of the OSX SDK.
            # The only way to make Exiv2 work with the external XMP SDK is to use the same standard library.
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libstdc++")
        else()
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
        endif()
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
                        " -Wpointer-arith" # This warning is also enabled by -Wpedantic
                        " -Wformat=2"
                        #" -Wold-style-cast"
                    )
                endif ()

                if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0 )
                  string(CONCAT EXTRA_COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
                    " -Warray-bounds=2"
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
                    " -Wshadow"
                    " -Wassign-enum"
                    " -Wmicrosoft"
                    " -Wcomments"
                    " -Wconditional-uninitialized"
                    " -Wdirect-ivar-access"
                    " -Weffc++"
                    " -Wpointer-arith"
                    " -Wformat=2"
                    #" -Warray-bounds" # Enabled by default
                    # These two raises lot of warnings. Use them wisely
                    #" -Wconversion"
                    #" -Wold-style-cast"
                )
                # -Wdouble-promotion flag is not available in clang 3.4.2
                if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.4.2 )
                    string(CONCAT EXTRA_COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
                        " -Wdouble-promotion"
                    )
                endif ()
                # -Wcomma flag is not available in clang 3.8.1
                if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.8.1 )
                    string(CONCAT EXTRA_COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
                        " -Wcomma"
                    )
                endif ()
            endif ()


        endif ()
    endif()

endif ()

# http://stackoverflow.com/questions/10113017/setting-the-msvc-runtime-in-cmake
if(MSVC)
    find_program(CLCACHE name clcache.exe
        PATHS ENV CLCACHE_PATH
        PATH_SUFFIXES Scripts clcache-4.1.0
    )
    if (CLCACHE)
        message(STATUS "clcache found in ${CLCACHE}")
        if (CMAKE_BUILD_TYPE STREQUAL "Debug")
            message(WARNING "clcache only works for Release builds")
        else()
            set(CMAKE_CXX_COMPILER ${CLCACHE})
        endif()
    endif()
    
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

    # Object Level Parallelism
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

endif()
