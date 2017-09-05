# CMakeLists.txt exiv2/xmpsdk

# list files to be use to build XMPsdk
file(GLOB XMPSRC src/*.cpp include/*.h include/*.hpp)

foreach(_currentfile ${XMPSRC})
    if(UNIX AND NOT CYGWIN)
        set_source_files_properties(${_currentfile} PROPERTIES COMPILE_FLAGS "-fPIC")
    endif()
endforeach()

# CMake OBJECT LIBRARY (collection of object files)
add_library( xmp_object OBJECT ${XMPSRC} )

target_include_directories(xmp_object PRIVATE ${EXPAT_INCLUDE_DIR})
target_include_directories(xmp_object PRIVATE ${CMAKE_SOURCE_DIR}/xmpsdk/include)

check_include_file( "stdint.h"  EXV_HAVE_STDINT_H )
if (EXV_HAVE_STDINT_H)
    target_compile_definitions(xmp_object PUBLIC EXV_HAVE_STDINT_H)
endif()

if( EXIV2_ENABLE_LIBXMP )
    add_library(xmp STATIC $<TARGET_OBJECTS:xmp_object>)
    target_link_libraries(xmp PUBLIC ${EXPAT_LIBRARIES})
    target_include_directories(xmp PUBLIC ${EXPAT_INCLUDE_DIR})
    target_include_directories(xmp PUBLIC ${CMAKE_SOURCE_DIR}/xmpsdk/include)

    # http://stackoverflow.com/questions/10046114/in-cmake-how-can-i-test-if-the-compiler-is-clang
    if ( NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang")
        # 1123 - hide xmpsdk symbols
        target_compile_definitions(xmp PRIVATE -fvisibility=hidden -fvisibility-inlines-hidden )
    endif()

    # 1119  Install libxmp.a for use by third party applications (Thanks, Emmanuel)
    install(TARGETS xmp
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

# That's all Folks!
##
