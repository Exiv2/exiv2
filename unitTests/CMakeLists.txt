add_executable(unit_tests mainTestRunner.cpp
    gtestwrapper.h
    test_types.cpp
    test_tiffheader.cpp
    test_futils.cpp
    test_enforce.cpp
    test_safe_op.cpp
    test_XmpKey.cpp
    test_DateValue.cpp
)

#TODO Use GTest::GTest once we upgrade the minimum CMake version required
target_link_libraries(unit_tests
    PRIVATE
        exiv2lib
        ${GTEST_BOTH_LIBRARIES}
        Threads::Threads
)

target_include_directories(unit_tests
    PRIVATE
        ${GTEST_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/src
)

set_target_properties( unit_tests PROPERTIES
    COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
)
