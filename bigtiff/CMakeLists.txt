
add_executable(big_tiff_parser
                parse.cpp

                #
                ${CMAKE_SOURCE_DIR}/src/basicio.cpp
                ${CMAKE_SOURCE_DIR}/src/image.cpp
                ${CMAKE_SOURCE_DIR}/src/types.cpp
)

target_include_directories(big_tiff_parser PRIVATE
                                           ${CMAKE_SOURCE_DIR}/include/exiv2
                                           ${CMAKE_SOURCE_DIR}/src
)
