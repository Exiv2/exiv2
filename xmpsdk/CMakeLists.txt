# CMakeLists.txt exiv2/xmpsdk

# list files to be use to build XMPsdk
file(GLOB XMPSRC src/*.cpp include/*.h include/*.hpp)

foreach(_currentfile ${XMPSRC})
    if(UNIX AND NOT CYGWIN)
        set_source_files_properties(${_currentfile} PROPERTIES COMPILE_FLAGS "-fPIC")
    endif()
endforeach()

if(NOT MSVC)
  # http://stackoverflow.com/questions/10046114/in-cmake-how-can-i-test-if-the-compiler-is-clang
  if ( NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "AppleClang")
    # 1123 - hide xmpsdk symbols
    add_definitions( -fvisibility=hidden -fvisibility-inlines-hidden )
  endif()
endif()

add_library( xmp STATIC ${XMPSRC} )
target_include_directories(xmp PUBLIC ${CMAKE_SOURCE_DIR}/xmpsdk/include)
target_include_directories(xmp PUBLIC ${EXPAT_INCLUDE_DIR})

if ( NOT MSVC )
    target_link_libraries(xmp ${EXPAT_LIBRARIES})
else()
    target_link_libraries(xmp Threads::Threads ${EXPAT_LIBRARIES})
endif()

# 1119  Install libxmp.a for use by third party applications (Thanks, Emmanuel)
install(TARGETS xmp
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# That's all Folks!
##
