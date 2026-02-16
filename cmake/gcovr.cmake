# Intended usage
#   cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_WITH_COVERAGE=yes ../
#   cmake --build . --config Debug 
#   ctest
#   cmake --build . --config Debug --target coverage

if(BUILD_WITH_COVERAGE)
    find_program (GCOVR gcovr)

    if(GCOVR)
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/coverage_output )
        add_custom_command(OUTPUT _run_gcovr_parser
            POST_BUILD
            COMMAND ${GCOVR} --root ${PROJECT_SOURCE_DIR} --object-dir=${CMAKE_BINARY_DIR} --html --html-details -o coverage_output/coverage.html
              --exclude-directories xmpsdk --exclude-directories unitTests --exclude-directories samples
              --exclude '.*xmpsdk.*' --exclude '.*unitTests.*' --exclude '.*samples.*'
              --exclude-unreachable-branches --exclude-throw-branches

            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        add_custom_target (coverage DEPENDS _run_gcovr_parser)
    endif()

endif(BUILD_WITH_COVERAGE)
