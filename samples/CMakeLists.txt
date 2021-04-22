# CMakeLists.txt for exiv2 library sample applications

set( SAMPLES
     addmoddel.cpp
     convert-test.cpp
     easyaccess-test.cpp
     exifcomment.cpp
     exifdata-test.cpp
     exifdata.cpp
     exifprint.cpp
     exifvalue.cpp
     ini-test.cpp
     iotest.cpp
     iptceasy.cpp
     iptcprint.cpp
     iptctest.cpp
     key-test.cpp
     largeiptc-test.cpp
     mmap-test.cpp
     mrwthumb.cpp
     prevtest.cpp
     stringto-test.cpp
     taglist.cpp
     tiff-test.cpp
     werror-test.cpp
     write-test.cpp
     write2-test.cpp
     xmpparse.cpp
     xmpparser-test.cpp
     xmpprint.cpp
     xmpsample.cpp
     xmpdump.cpp
)

##
# build samples AND add them to the APPLICATIONS list
foreach(entry ${SAMPLES})
    string( REPLACE ".cpp" "" target ${entry})
    add_executable( ${target} ${target}.cpp )
    set_target_properties(${target} PROPERTIES
      COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS})
    list(APPEND APPLICATIONS ${target})
    add_test( ${target}_test ${target} )
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src) # To find unused.h
    if ( NOT ${target} MATCHES ".*test.*")                                # don't install tests
        install( TARGETS ${target} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
endforeach()

###################################

if (MSVC)
    link_directories(${CMAKE_INSTALL_PREFIX}/lib)
endif()

add_executable( getopt-test getopt-test.cpp ../src/utils.cpp ../src/getopt.cpp)
list(APPEND APPLICATIONS getopt-test)
target_include_directories(getopt-test PRIVATE ${CMAKE_SOURCE_DIR}/src) # To find utils.hpp

add_executable( metacopy metacopy.cpp ../src/utils.cpp ../src/getopt.cpp)
list(APPEND APPLICATIONS metacopy)
target_include_directories(metacopy PRIVATE ${CMAKE_SOURCE_DIR}/src) # To find utils.hpp

add_executable(          path-test  path-test.cpp ../src/utils.cpp ../src/getopt.cpp)
list(APPEND APPLICATIONS path-test)
set_target_properties(   path-test  PROPERTIES OUTPUT_NAME path-test )
target_include_directories(path-test PRIVATE ${CMAKE_SOURCE_DIR}/src) # To find utils.hpp

add_executable(          exiv2json exiv2json.cpp Jzon.cpp Jzon.h)
list(APPEND APPLICATIONS exiv2json)

install( TARGETS metacopy exiv2json RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if( EXPAT_FOUND )
    add_executable(        geotag    geotag.cpp)
    list(APPEND APPLICATIONS geotag)
    target_link_libraries(geotag 
        PRIVATE
            exiv2-xmp
            ${EXPAT_LIBRARIES}
    )
    target_include_directories(geotag PRIVATE ${EXPAT_INCLUDE_DIR})
    target_include_directories(geotag PRIVATE ${CMAKE_SOURCE_DIR}/src) # To find unused.h

    if (WIN32)
        target_compile_definitions(geotag PRIVATE XML_STATIC)
    endif()

    if (MSVC)
        set_target_properties(geotag PROPERTIES LINK_FLAGS "/ignore:4099")
    endif()

    install( TARGETS       geotag    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# ******************************************************************************
# connection test application
add_executable(conntest conntest.cpp)
list(APPEND APPLICATIONS conntest)

if (EXIV2_ENABLE_WEBREADY)
    if( EXIV2_ENABLE_CURL )
        target_include_directories(conntest SYSTEM PRIVATE ${CURL_INCLUDE_DIR} )
        target_link_libraries(conntest PRIVATE ${CURL_LIBRARIES})
        if (USING_CONAN)
            target_compile_definitions(conntest PRIVATE ${CONAN_COMPILE_DEFINITIONS_LIBCURL})
            target_link_libraries(conntest PRIVATE ${CONAN_EXE_LINKER_FLAGS_LIBCURL})
            if ( MSVC ) 
			    target_link_libraries(conntest PRIVATE Crypt32 Ws2_32 ${CURL_LIBRARIES})
            endif()
        endif()
    endif()
endif()

# ******************************************************************************
# remotetest application
add_executable(remotetest remotetest.cpp)
list(APPEND APPLICATIONS remotetest)

# ******************************************************************************
foreach(application ${APPLICATIONS})
    target_link_libraries(${application} PRIVATE exiv2lib)
    if( EXIV2_ENABLE_PNG )
        target_link_libraries( ${application} PRIVATE ${ZLIB_LIBRARIES} )
    endif()
endforeach()

# That's all Folks!
##
