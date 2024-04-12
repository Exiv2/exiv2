set(APP_SOURCES
    exiv2.cpp
    actions.cpp
    getopt.cpp
    app_utils.cpp
)

add_executable(exiv2 ${APP_SOURCES})

# Make app use UTF-8 code page in Windows
if(WIN32)
  if(MSVC)
    target_sources(exiv2 PRIVATE utf8.manifest)
  else()
    # Must wrap manifest in .rc w/ other toolchains
    target_sources(exiv2 PRIVATE utf8.rc)
  endif()
endif()

target_include_directories(exiv2 PRIVATE ${PROJECT_SOURCE_DIR}/src) # To find i18n.hpp

set_target_properties(exiv2 PROPERTIES COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS} XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] "YES")
if(MSVC)
  set_target_properties(exiv2 PROPERTIES LINK_FLAGS "/ignore:4099") # Ignore missing PDBs
endif()

target_link_libraries(exiv2 PRIVATE exiv2lib)

if(EXIV2_ENABLE_NLS)
  target_link_libraries(exiv2 PRIVATE ${Intl_LIBRARIES})
  target_include_directories(exiv2 PRIVATE ${Intl_INCLUDE_DIRS})
endif()

target_link_libraries(exiv2 PRIVATE std::filesystem)

if(USING_CONAN
   AND WIN32
   AND EXISTS ${PROJECT_BINARY_DIR}/conanDlls
)
  # In case of using conan recipes with their 'shared' option turned on, we will have dlls of the 3rd party dependencies in the conanDlls folder.

  add_custom_command(
    TARGET exiv2
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_BINARY_DIR}/conanDlls ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Copy 3rd party DLLs the bin folder"
  )

  # Copy 3rd party DLLs the bin folder. [install step]
  install(DIRECTORY ${PROJECT_BINARY_DIR}/conanDlls/ DESTINATION bin)
endif()

install(TARGETS exiv2 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
