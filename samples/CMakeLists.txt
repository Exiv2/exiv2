# CMakeLists.txt for exiv2 library sample applications

set(SAMPLES
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
    jpegparsetest.cpp
    key-test.cpp
    largeiptc-test.cpp
    mmap-test.cpp
    mrwthumb.cpp
    prevtest.cpp
    stringto-test.cpp
    taglist.cpp
    tiff-test.cpp
    write-test.cpp
    write2-test.cpp
    xmpparse.cpp
    xmpparser-test.cpp
    xmpprint.cpp
    xmpsample.cpp
    xmpdump.cpp
)

#
# build samples AND add them to the APPLICATIONS list
foreach(entry ${SAMPLES})
  string(REPLACE ".cpp" "" target ${entry})
  add_executable(${target} ${target}.cpp)
  set_target_properties(${target} PROPERTIES COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS})
  list(APPEND APPLICATIONS ${target})
  target_include_directories(${target} PRIVATE ${PROJECT_SOURCE_DIR}/src) # To find enforce.hpp
  if(NOT ${target} MATCHES ".*test.*") # don't install tests
    install(TARGETS ${target} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
  endif()
endforeach()

# ####################################################################################################################################################

if(MSVC)
  link_directories(${CMAKE_INSTALL_PREFIX}/lib)
endif()

add_executable(getopt-test getopt-test.cpp ../app/getopt.cpp)
list(APPEND APPLICATIONS getopt-test)
target_include_directories(getopt-test PRIVATE ${PROJECT_SOURCE_DIR}/app) # To find getopt.hpp

add_executable(metacopy metacopy.cpp metacopy.hpp ../app/getopt.cpp)
list(APPEND APPLICATIONS metacopy)
target_include_directories(metacopy PRIVATE ${PROJECT_SOURCE_DIR}/app) # To find getopt.hpp

add_executable(path-test path-test.cpp ../app/getopt.cpp)
list(APPEND APPLICATIONS path-test)
set_target_properties(path-test PROPERTIES OUTPUT_NAME path-test)
target_include_directories(path-test PRIVATE ${PROJECT_SOURCE_DIR}/app) # To find getopt.hpp

install(TARGETS metacopy RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(EXIV2_ENABLE_INIH)
  target_link_libraries(ini-test PRIVATE inih::libinih)
  target_link_libraries(ini-test PRIVATE inih::inireader)
endif()

if(EXPAT_FOUND)
  add_executable(geotag geotag.cpp)
  list(APPEND APPLICATIONS geotag)
  target_link_libraries(geotag PRIVATE exiv2-xmp ${EXPAT_LIBRARIES})
  target_include_directories(geotag PRIVATE ${CMAKE_BINARY_DIR}) # exv_conf.h
  target_include_directories(geotag PRIVATE ${PROJECT_SOURCE_DIR}/include) # <exiv2/exiv2.hpp>
  target_include_directories(geotag PRIVATE ${EXPAT_INCLUDE_DIR})

  if(WIN32)
    target_compile_definitions(geotag PRIVATE XML_STATIC)
  endif()

  if(MSVC)
    set_target_properties(geotag PROPERTIES LINK_FLAGS "/ignore:4099")
  endif()

  install(TARGETS geotag RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

# ******************************************************************************
# connection test application
if(EXIV2_ENABLE_WEBREADY)
  add_executable(conntest conntest.cpp)
  list(APPEND APPLICATIONS conntest)
  if(EXIV2_ENABLE_CURL)
    target_link_libraries(conntest PRIVATE CURL::libcurl)
  endif()
endif()

# ******************************************************************************
# remotetest application
add_executable(remotetest remotetest.cpp)
list(APPEND APPLICATIONS remotetest)

# ******************************************************************************
foreach(application ${APPLICATIONS})
  # Make app use UTF-8 code page in Windows
  if(WIN32)
    if(MSVC)
      target_sources(${application} PRIVATE ${PROJECT_SOURCE_DIR}/app/utf8.manifest)
    else()
      # Must wrap manifest in .rc w/ other toolchains
      target_sources(${application} PRIVATE ${PROJECT_SOURCE_DIR}/app/utf8.rc)
    endif()
  endif()
  target_link_libraries(${application} PRIVATE exiv2lib std::filesystem)
  if(EXIV2_ENABLE_PNG)
    target_link_libraries(${application} PRIVATE ${ZLIB_LIBRARIES})
    if(MSVC)
      set_target_properties(${application} PROPERTIES LINK_FLAGS "/ignore:4099") # Ignore missing PDBs
    endif()
  endif()
endforeach()

# That's all Folks!
#
