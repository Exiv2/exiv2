if(BUILD_WITH_COVERAGE)

    find_program (GCOVR gcovr)

    if(GCOVR)
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/coverage_output )
        add_custom_command(OUTPUT _run_gcovr_parser
            POST_BUILD
            COMMAND ${GCOVR} --root ${CMAKE_SOURCE_DIR} --object-dir=${CMAKE_BINARY_DIR} --html --html-details -o coverage_output/coverage.html
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
        add_custom_target (coverage DEPENDS _run_gcovr_parser)
    endif()
             
endif(BUILD_WITH_COVERAGE)
