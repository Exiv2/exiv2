add_executable(unit_tests mainTestRunner.cpp
    test_types.cpp
)

target_link_libraries(unit_tests 
    PRIVATE
        exiv2lib
        GTest::GTest
)
