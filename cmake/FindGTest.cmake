# Use pkg-config to find GTest as older CMake does not include gmock properly
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTEST REQUIRED gtest_main)

# Set GTest variables
set(GTEST_INCLUDE_DIRS ${GTEST_INCLUDE_DIRS} PARENT_SCOPE)
set(GTEST_LIBRARIES ${GTEST_LIBRARIES} PARENT_SCOPE)
set(GTEST_FOUND TRUE PARENT_SCOPE)
