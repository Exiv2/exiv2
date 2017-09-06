
add_executable(big_tiff_parser
                parse.cpp
)

target_include_directories(big_tiff_parser PRIVATE
                                           ${CMAKE_SOURCE_DIR}/include/exiv2
                                           ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(big_tiff_parser PRIVATE exiv2lib)
