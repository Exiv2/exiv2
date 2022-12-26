find_package(GTest REQUIRED)

add_executable(unit_tests
    mainTestRunner.cpp
    test_basicio.cpp
    test_bmpimage.cpp
    test_cr2header_int.cpp
    test_datasets.cpp
    test_Error.cpp
    test_DateValue.cpp
    test_enforce.cpp
    test_FileIo.cpp
    test_futils.cpp
    test_helper_functions.cpp
    test_image_int.cpp
    test_ImageFactory.cpp
    test_jp2image.cpp
    test_jp2image_int.cpp
    test_IptcKey.cpp
    test_LangAltValueRead.cpp
    test_Photoshop.cpp
    test_pngimage.cpp
    test_safe_op.cpp
    test_slice.cpp
    test_tiffheader.cpp
    test_types.cpp
    test_TimeValue.cpp
    test_utils.cpp
    test_XmpKey.cpp
    $<TARGET_OBJECTS:exiv2lib_int>
)

target_compile_definitions(unit_tests
  PRIVATE
    exiv2lib_STATIC
    TESTDATA_PATH="${PROJECT_SOURCE_DIR}/test/data"
)

target_link_libraries(unit_tests
    PRIVATE
        exiv2lib
        exiv2lib_int
        GTest::gtest
        GTest::gtest_main
        std::filesystem
        inih::libinih
        inih::inireader
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

if (MSVC)
    set_target_properties(unit_tests PROPERTIES LINK_FLAGS "/ignore:4099")
endif()

add_test(NAME unitTests COMMAND unit_tests)
