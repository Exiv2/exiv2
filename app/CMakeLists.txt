add_executable( exiv2
     exiv2.cpp
     exiv2app.hpp
     actions.cpp actions.hpp
     getopt.cpp getopt.hpp
     app_utils.cpp app_utils.hpp
)

target_include_directories(exiv2 PRIVATE ${PROJECT_SOURCE_DIR}/src) # To find i18n.hpp

set_target_properties( exiv2 PROPERTIES
    COMPILE_FLAGS ${EXTRA_COMPILE_FLAGS}
    XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] "YES"
)
if (MSVC)
    set_target_properties(exiv2 PROPERTIES LINK_FLAGS "/ignore:4099") # Ignore missing PDBs
endif()

target_link_libraries( exiv2 PRIVATE exiv2lib )

if( EXIV2_ENABLE_NLS )
    target_link_libraries(exiv2 PRIVATE ${Intl_LIBRARIES})
    target_include_directories(exiv2 PRIVATE ${Intl_INCLUDE_DIRS})
endif()

target_link_libraries(exiv2 PRIVATE std::filesystem)

if(MSVC OR MINGW)
    # Trick to get properly UTF-8 encoded argv 
    # More info at: https://github.com/huangqinjin/wmain
    add_library(wmain STATIC wmain.cpp)
    target_link_libraries(exiv2 PRIVATE wmain)
endif()

if (MSVC)
    target_link_options(wmain INTERFACE /WHOLEARCHIVE:$<TARGET_FILE:wmain>)
    target_link_options(exiv2 PRIVATE "/ENTRY:wWinMainCRTStartup")
endif()

if (MINGW)
    target_compile_options(exiv2 PRIVATE -municode)
    target_link_options(exiv2 PRIVATE -municode)
endif()

if (USING_CONAN AND WIN32 AND EXISTS ${PROJECT_BINARY_DIR}/conanDlls)
    # In case of using conan recipes with their 'shared' option turned on, we will have dlls of
    # the 3rd party dependencies in the conanDlls folder.

    # Copy 3rd party DLLs the bin folder. [build step]
    add_custom_command(TARGET exiv2 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_BINARY_DIR}/conanDlls ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

    # Copy 3rd party DLLs the bin folder. [install step]
    install(DIRECTORY ${PROJECT_BINARY_DIR}/conanDlls/ DESTINATION bin)
endif()

install(TARGETS exiv2 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
