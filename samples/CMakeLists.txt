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
    target_link_libraries( ${target} PRIVATE exiv2lib)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src) # To find unused.h
    install( TARGETS ${target} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endforeach()

###################################

if (MSVC)
    link_directories(${CMAKE_INSTALL_PREFIX}/lib)
endif()

add_executable( metacopy metacopy.cpp ../src/utils.cpp)
list(APPEND APPLICATIONS metacopy)
target_link_libraries( metacopy PRIVATE exiv2lib)
target_include_directories(metacopy PRIVATE ${CMAKE_SOURCE_DIR}/src) # To find utils.hpp

add_executable(          pathtest  path-test.cpp ../src/utils.cpp)
list(APPEND APPLICATIONS pathtest)
set_target_properties(   pathtest  PROPERTIES OUTPUT_NAME path-test )
target_link_libraries(   pathtest  PRIVATE exiv2lib)
target_include_directories(pathtest PRIVATE ${CMAKE_SOURCE_DIR}/src) # To find utils.hpp

if(NOT EXV_HAVE_TIMEGM )
    target_sources(pathtest PRIVATE ../src/localtime.c)
endif()

add_executable(          exiv2json exiv2json.cpp Jzon.cpp)
list(APPEND APPLICATIONS exiv2json)
target_link_libraries(   exiv2json PRIVATE exiv2lib)

install( TARGETS metacopy pathtest exiv2json RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if (MSVC)
    target_sources(pathtest PRIVATE ../src/getopt_win32.c)
    target_sources(metacopy PRIVATE ../src/getopt_win32.c)
endif()

if( EXPAT_FOUND )
    add_executable(        geotag    geotag.cpp)
    list(APPEND APPLICATIONS geotag)
    target_link_libraries( geotag    PRIVATE exiv2lib ${EXPAT_LIBRARIES})
    target_include_directories(geotag PRIVATE ${EXPAT_INCLUDE_DIR})
    target_include_directories(geotag PRIVATE ${CMAKE_SOURCE_DIR}/src) # To find unused.h

    if (WIN32)
        target_compile_definitions(geotag PRIVATE XML_STATIC)
    endif()

    install( TARGETS       geotag    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# ******************************************************************************
# connection test application
add_executable(conntest conntest.cpp)
target_link_libraries(conntest PRIVATE  exiv2lib)
list(APPEND APPLICATIONS conntest)

# ******************************************************************************
# remotetest application
add_executable(remotetest remotetest.cpp)
target_link_libraries(remotetest PRIVATE exiv2lib)
list(APPEND APPLICATIONS remotetest)

# ******************************************************************************
# set compiler -DEXV_HAVE_DLL when linking exiv2 dll
foreach(application ${APPLICATIONS})
    message(STATUS "sample: ${application}")
    if ( BUILD_SHARED_LIBS )
        target_compile_definitions(${application} PRIVATE EXV_HAVE_DLL )
    endif()
    if( EXIV2_ENABLE_PNG )
        target_link_libraries( ${application} PRIVATE ${ZLIB_LIBRARIES} )
    endif()
endforeach()

# ******************************************************************************
# Man page
install( FILES exiv2samples.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1 )

# That's all Folks!
##
