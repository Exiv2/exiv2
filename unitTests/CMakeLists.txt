find_package(GTest REQUIRED)

add_executable(unit_tests mainTestRunner.cpp
    gtestwrapper.h
    test_types.cpp
    test_tiffheader.cpp
    test_futils.cpp
    test_enforce.cpp
    test_safe_op.cpp
    test_XmpKey.cpp
    test_DateValue.cpp
    test_TimeValue.cpp
    test_cr2header_int.cpp
    test_helper_functions.cpp
    test_slice.cpp
    test_image_int.cpp
    $<TARGET_OBJECTS:exiv2lib_int>
)

#TODO Use GTest::GTest once we upgrade the minimum CMake version required
target_link_libraries(unit_tests
    PRIVATE
        exiv2lib
        GTest::GTest
)

# ZLIB is used in exiv2lib_int.
if( EXIV2_ENABLE_PNG )
    target_link_libraries(unit_tests PRIVATE ${ZLIB_LIBRARIES} )
endif()

target_include_directories(unit_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(unit_tests PROPERTIES
    COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
)

if (USING_CONAN)
    target_compile_definitions(unit_tests PRIVATE ${CONAN_COMPILE_DEFINITIONS_GTEST})
endif()

if (MSVC)
    set_target_properties(unit_tests PROPERTIES LINK_FLAGS "/ignore:4099")
endif()
