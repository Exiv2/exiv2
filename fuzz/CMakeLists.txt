
macro(fuzzer name)
    add_executable(${name} ${name}.cpp)
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
