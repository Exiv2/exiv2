# These flags applies to exiv2lib, the applications, and to the xmp code

if ( MINGW OR UNIX OR MSYS ) # MINGW, Linux, APPLE, CYGWIN
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        set(COMPILER_IS_GCC ON)
    elseif (${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
        set(COMPILER_IS_CLANG ON)
    endif()

    set (CMAKE_CXX_FLAGS_DEBUG "-g3 -gstrict-dwarf -O0")

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

    if (COMPILER_IS_GCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 8.0)
        add_compile_options(-fstack-clash-protection -fcf-protection)
    endif()

    if (COMPILER_IS_GCC OR COMPILER_IS_CLANG)
        add_compile_options(-Wp,-D_GLIBCXX_ASSERTIONS)

        if (CMAKE_BUILD_TYPE STREQUAL Release)
            add_compile_options(-Wp,-D_FORTIFY_SOURCE=2) # Requires to compile with -O2
        endif()

        if(BUILD_WITH_COVERAGE)
            # Note: We tried to use here add_compile_options but we got linker errors on Travis-CI
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
        endif()

        add_compile_options(-Wall -Wcast-align -Wpointer-arith -Wformat-security -Wmissing-format-attribute -Woverloaded-virtual -W)
        add_compile_options(-fstack-protector-strong)
        add_compile_options(-fasynchronous-unwind-tables)


        if ( EXIV2_TEAM_USE_SANITIZERS )
            # ASAN is available in gcc from 4.8 and UBSAN from 4.9
            # ASAN is available in clang from 3.1 and UBSAN from 3.3
            # UBSAN is not fatal by default, instead it only prints runtime errors to stderr
            # => make it fatal with -fno-sanitize-recover (gcc) or -fno-sanitize-recover=all (clang)
            # add -fno-omit-frame-pointer for better stack traces
            if ( COMPILER_IS_GCC )
                if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9 )
                    set(SANITIZER_FLAGS "-fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover")
                elseif( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.8 )
                    set(SANITIZER_FLAGS "-fno-omit-frame-pointer -fsanitize=address")
                endif()
            elseif( COMPILER_IS_CLANG )
                if ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9 )
                    set(SANITIZER_FLAGS "-fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all")
                elseif ( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.4 )
                    set(SANITIZER_FLAGS "-fno-omit-frame-pointer -fsanitize=address,undefined")
                elseif( CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.1 )
                    set(SANITIZER_FLAGS "-fno-omit-frame-pointer -fsanitize=address")
                endif()
            endif()

            # sorry, ASAN does not work on Windows
            if ( NOT CYGWIN AND NOT MINGW AND NOT MSYS )
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
                set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SANITIZER_FLAGS}")
                set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
                set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
            endif()
        endif()
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

    if (NOT BUILD_SHARED_LIBS AND NOT EXIV2_ENABLE_DYNAMIC_RUNTIME)
         message(STATUS "MSVC -> forcing use of statically-linked runtime." )
         foreach(variable ${variables})
             if(${variable} MATCHES "/MD")
                 string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
             endif()
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

    if ( EXIV2_EXTRA_WARNINGS )
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif ()

    # Object Level Parallelism
    add_compile_options(/MP)
    add_definitions(-DNOMINMAX -DWIN32_LEAN_AND_MEAN)
endif()
