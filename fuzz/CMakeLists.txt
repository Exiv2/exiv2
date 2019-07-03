
macro(fuzzer name)
    add_executable(${name} ${name}.cpp)
    set_target_properties(${name}
            PROPERTIES
                COMPILE_FLAGS "-fsanitize=fuzzer"
                LINK_FLAGS "-fsanitize=fuzzer")
    target_link_libraries(${name}
            PRIVATE
            exiv2lib
            )
endmacro()

fuzzer(read-metadata)