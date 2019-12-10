if(USE_GCOV)
 
    SET(GCOVR gcovr)
 
    SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")
    SET(GCC_COVERAGE_LINK_FLAGS "-lgcov")
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}" )
             
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/coverage_output )
    add_custom_command(OUTPUT _run_gcovr_parser
    POST_BUILD
    COMMAND ${GCOVR} -r ${CMAKE_SOURCE_DIR}/src --object-dir=${CMAKE_BINARY_DIR} --html --html-details -o coverage_output/coverage.html
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    add_custom_target (coverage DEPENDS _run_gcovr_parser)
endif(USE_GCOV)
