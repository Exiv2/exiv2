
macro(fuzzer name)
    add_executable(${name} ${name}.cpp)

    # The EXIV2_TEAM_OSS_FUZZ option is used by the OSS-Fuzz build script:
    # https://github.com/google/oss-fuzz/tree/master/projects/exiv2/build.sh
    # OSS-Fuzz wants full control of the sanitizer flags, and it uses
    # the LIB_FUZZING_ENGINE option to supply its own main function.
    if(EXIV2_TEAM_OSS_FUZZ)
        set_target_properties(${name}
                PROPERTIES
                    LINK_FLAGS "${LIB_FUZZING_ENGINE}")
    else()
        set_target_properties(${name}
                PROPERTIES
                    COMPILE_FLAGS "-fsanitize=fuzzer"
                    LINK_FLAGS "-fsanitize=fuzzer")
    endif()
    target_link_libraries(${name}
            PRIVATE
            exiv2lib
            )
endmacro()

fuzzer(fuzz-read-print-write)
