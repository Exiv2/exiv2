# These flags only applies to exiv2lib, and the applications, but not to the xmp code

include(CheckCXXCompilerFlag)

if (COMPILER_IS_GCC OR COMPILER_IS_CLANG) # MINGW, Linux, APPLE, CYGWIN
    if ( EXIV2_TEAM_WARNINGS_AS_ERRORS )
        add_compile_options(-Werror)
        check_cxx_compiler_flag(-Wdeprecated-copy DEPRECATED_COPY)
        if ( DEPRECATED_COPY) 
            add_compile_options(-Wdeprecated-copy)
        endif ()
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
                    " -Wpointer-arith" # This warning is also enabled by -Wpedantic
                    " -Wformat=2"
                    #" -Wold-style-cast"
                    #" -Wuseless-cast" Disabled mainly because of conversion of socket types (different types on OSs)
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

if (MSVC)
    if ( EXIV2_TEAM_WARNINGS_AS_ERRORS )
        add_compile_options(/WX)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /WX")
        set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /WX")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /WX")
    endif ()
endif ()
