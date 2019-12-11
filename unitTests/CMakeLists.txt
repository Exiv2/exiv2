find_package(GTest REQUIRED)
enable_testing()

add_executable(unit_tests mainTestRunner.cpp
    test_DateValue.cpp
    test_FileIo.cpp
    test_ImageFactory.cpp
    test_ImageJpeg.cpp
    test_MemIo.cpp
    test_PngChunks.cpp
    test_TimeValue.cpp
    test_XmpKey.cpp
    test_cr2header_int.cpp
    test_enforce.cpp
    test_futils.cpp
    test_helper_functions.cpp
    test_image_int.cpp
    test_safe_op.cpp
    test_slice.cpp
    test_tiffheader.cpp
    test_types.cpp
    $<TARGET_OBJECTS:exiv2lib_int>
)

target_compile_definitions(unit_tests
    PRIVATE
        TESTDATA_PATH="${PROJECT_SOURCE_DIR}/test/data"
)

target_link_libraries(unit_tests
    PRIVATE
        exiv2lib
        GTest::GTest
)

# ZLIB is used in exiv2lib_int.
if( EXIV2_ENABLE_PNG )
    target_link_libraries(unit_tests PRIVATE ${ZLIB_LIBRARIES} )
endif()

# To test exiv2lib_int
target_include_directories(unit_tests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src
)

set_target_properties(unit_tests PROPERTIES
    COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
)

if (MSVC)	
    # Fix linking issue about missing 3rd party PDB
    set_target_properties(unit_tests PROPERTIES LINK_FLAGS "/ignore:4099")	
endif()

add_test(NAME unit_tests COMMAND unit_tests)
