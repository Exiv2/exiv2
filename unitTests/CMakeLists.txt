# Prefer using Google supplied CMake config as built-in module is buggy before 3.23
find_package(GTest CONFIG)
if(NOT GTest_FOUND)
  find_package(GTest REQUIRED)
endif()

# video support.
if(EXV_ENABLE_VIDEO)
  set(VIDEO_SUPPORT test_asfvideo.cpp test_matroskavideo.cpp test_riffVideo.cpp)
endif()

add_executable(
  unit_tests
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
  ${VIDEO_SUPPORT}
  $<TARGET_OBJECTS:exiv2lib_int>
)

target_compile_definitions(unit_tests PRIVATE exiv2lib_STATIC TESTDATA_PATH="${PROJECT_SOURCE_DIR}/test/data")

target_link_libraries(unit_tests PRIVATE exiv2lib GTest::gmock_main std::filesystem)

if(NOT EXV_HAVE_STD_FORMAT)
  target_link_libraries(unit_tests PRIVATE fmt::fmt)
endif()

if(EXIV2_ENABLE_INIH)
  target_link_libraries(unit_tests PRIVATE inih::libinih inih::inireader)
endif()

# ZLIB is used in exiv2lib_int.
if(EXIV2_ENABLE_PNG)
  target_link_libraries(unit_tests PRIVATE ${ZLIB_LIBRARIES})
endif()

target_include_directories(unit_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)

set_target_properties(unit_tests PROPERTIES COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS})

if(MSVC)
  set_target_properties(unit_tests PROPERTIES LINK_FLAGS "/ignore:4099")
endif()

add_test(NAME unitTests COMMAND unit_tests)
