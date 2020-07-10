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


    if (COMPILER_IS_GCC OR COMPILER_IS_CLANG)

        check_c_compiler_flag(-fstack-protector-strong   FSTACK_PROTECTOR_STRONG         )
        check_c_compiler_flag(-fstack-clash-protection   FSTACK_CLASH_PROTECTION         )
        check_c_compiler_flag(-fcf-protection            FCF-PROTECTION                  )
        check_c_compiler_flag(-fcf-protection            FCF-PROTECTION                  )
        check_c_compiler_flag(-D_GLIBCXX_ASSERTIONS      D_GLIBCXX_ASSERTIONS            )
        check_c_compiler_flag(-D_FORTIFY_SOURCE=2        D_FORTIFY_SOURCE                )
        check_c_compiler_flag(-Wp                        COMPILER_FLAG_WP                )
        check_c_compiler_flag(-Wall                      COMPILER_FLAG_WALL              )
        check_c_compiler_flag(-Wcast-align               COMPILER_FLAG_CAST_ALIGN        )
        check_c_compiler_flag(-Wpointer-arith            COMPILER_FLAG_POINTER_ARITH     )
        check_c_compiler_flag(-Wformat-security          COMPILER_FLAG_FORMAT_SECURITY   )
        check_c_compiler_flag(-Wmissing-format-attribute COMPILER_FLAG_FORMAT_ATTTIBUTE  )
        check_c_compiler_flag(-Woverloaded-virtual       COMPILER_FLAG_OVERLOADED_VIRTUAL)
        check_c_compiler_flag(-W                         COMPILER_FLAG_W                 )
        check_c_compiler_flag(--coverage                 COMPILER_FLAG_COVERAGE          )

        if ( FSTACK_PROTECTOR_STRONG ) 
            add_compile_options(-fstack-protector-strong)
        endif()
        if ( FSTACK_CLASH_PROTECTION )
            add_compile_options(-fstack-clash-protection)
        endif()
        if ( FCF-PROTECTION )
            add_compile_options(-fcf-protection)
        endif()
        if ( COMPILER_FLAG_WP )
            add_compile_options(-Wp)
        endif()
        if ( D_GLIBCXX_ASSERTIONS )
            add_compile_options(-D_GLIBCXX_ASSERTIONS)
        endif()
        if ( COMPILER_FLAG_WALL )
            add_compile_options(-Wall)
        endif()
        if ( COMPILER_FLAG_CAST_ALIGN )
            add_compile_options(-Wcast-align)
        endif()
        if ( COMPILER_FLAG_POINTER_ARITH )
            add_compile_options(-Wpointer-arith)
        endif()
        if ( COMPILER_FLAG_FORMAT_SECURITY )
            add_compile_options(-Wformat-security)
        endif()
        if ( COMPILER_FLAG_FORMAT_ATTTIBUTE )
            add_compile_options(-Wmissing-format-attribute)
        endif()
        if ( COMPILER_FLAG_OVERLOADED_VIRTUAL )
            add_compile_options(-Woverloaded-virtual)
        endif()
        if ( COMPILER_FLAG_W )
            add_compile_options(-W)
        endif()

        if (CMAKE_BUILD_TYPE STREQUAL Release AND D_FORTIFY_SOURCE)
            add_compile_options(-D_FORTIFY_SOURCE=2) # Requires to compile with -O2
        endif()

        if(BUILD_WITH_COVERAGE AND COMPILER_FLAG_COVERAGE)
            add_compile_options(--coverage)
            # TODO: From CMake 3.13 we could use add_link_options instead these 2 lines
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --coverage")
        endif()


        if ( EXIV2_TEAM_USE_SANITIZERS )
			check_c_compiler_flag(-fno-omit-frame-pointer         COMPILER_NO_OMIT_FRAME_POINTER     )
			check_c_compiler_flag(-fsanitize=address,undefined    COMPILER_SANITIZE_ADDRESS_UNDEFINED)
			check_c_compiler_flag(-fno-sanitize-recover=all       COMPILER_SANITIZE_ADDRESS_ALL      )
			check_c_compiler_flag(--fsanitize=address             COMPILER_SANITIZE_ADDRESS          )
			check_c_compiler_flag(-fno-sanitize-recover=all       COMPILER_NO_SANITIZE_RECOVER       )

            # UBSAN is not fatal by default, instead it only prints runtime errors to stderr
            # => make it fatal with -fno-sanitize-recover (gcc) or -fno-sanitize-recover=all (clang)
            # add -fno-omit-frame-pointer for better stack traces
            if ( COMPILER_IS_GCC )
                if ( COMPILER_NO_OMIT_FRAME_POINTER AND COMPILER_SANITIZE_ADDRESS_UNDEFINED AND COMPILER_NO_SANITIZE_RECOVER )
                    set(SANITIZER_FLAGS "-fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover")
                elseif ( COMPILER_NO_OMIT_FRAME_POINTER AND COMPILER_SANITIZE_ADDRESS )
                    set(SANITIZER_FLAGS "-fno-omit-frame-pointer -fsanitize=address")
                endif()
            elseif( COMPILER_IS_CLANG )
                if ( COMPILER_NO_OMIT_FRAME_POINTER AND COMPILER_SANITIZE_ADDRESS_UNDEFINED AND COMPILER_SANITIZE_ADDRESS_ALL )
                    set(SANITIZER_FLAGS "-fno-omit-frame-pointer -fsanitize=address,undefined -fno-sanitize-recover=all")
                elseif ( COMPILER_NO_OMIT_FRAME_POINTER AND COMPILER_SANITIZE_ADDRESS_UNDEFINED  )
                    set(SANITIZER_FLAGS "-fno-omit-frame-pointer -fsanitize=address,undefined")
                elseif( COMPILER_NO_OMIT_FRAME_POINTER AND COMPILER_SANITIZE_ADDRESS )
                    set(SANITIZER_FLAGS "-fno-omit-frame-pointer -fsanitize=address")
                endif()
            endif()

            # ASAN does not work on Windows
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

    # https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
    if (MSVC_VERSION GREATER_EQUAL "1910") # VS2017 and up
        add_compile_options("/Zc:__cplusplus")
    endif()

endif()
