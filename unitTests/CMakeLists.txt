add_executable(unit_tests mainTestRunner.cpp
    test_types.cpp
)

target_link_libraries(unit_tests 
    PRIVATE
        exiv2lib
        GTest::GTest
)

target_compile_definitions(unit_tests
    PRIVATE
        GTEST_LINKED_AS_SHARED_LIBRARY=1
)
