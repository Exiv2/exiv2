if ( MINGW OR UNIX ) # MINGW, Linux, APPLE, CYGWIN
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -Wall
                                               -Wcast-align
                                               -Wpointer-arith
                                               -Wformat-security
                                               -Wmissing-format-attribute
                                               -Woverloaded-virtual
                                               -W
            )
    endif()

    if ( CYGWIN OR (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.0))
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=gnu++98 ) # to support snprintf
    else()
        set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++98 )
    endif()
endif ()


##
# msvc tuning macros
macro(msvc_runtime_set_static_ignores bDynamic bDynamicRuntime)
  if(MSVC)
    # don't link msvcrt for .exe which use shared libraries (use default libcmt)
	if ( ${bDynamic} STREQUAL "OFF" AND ${bDynamicRuntime} STREQUAL "OFF")
		set(CMAKE_EXE_LINKER_FLAGS_DEBUG          "/NODEFAULTLIB:MSVCRTD")
		set(CMAKE_EXE_LINKER_FLAGS_RELEASE        "/NODEFAULTLIB:MSVCRT")
		set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL     "/NODEFAULTLIB:MSVCRT")
		set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/NODEFAULTLIB:MSVCRT")
	endif()
  endif()
endmacro()

# http://stackoverflow.com/questions/10113017/setting-the-msvc-runtime-in-cmake
macro(msvc_runtime_configure bDynamic bDynamicRuntime)
  if(MSVC)
    set(variables
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )

    if( ${bDynamic} STREQUAL "ON" OR ${bDynamicRuntime} STREQUAL "ON")
      message(STATUS  "MSVC -> forcing use of dynamically-linked runtime." )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MT")
          string(REGEX REPLACE "/MT" "/MD" ${variable} "${${variable}}")
        endif()
      endforeach()
    else()
      message(STATUS "MSVC -> forcing use of statically-linked runtime." )
      foreach(variable ${variables})
        if(${variable} MATCHES "/MD")
          string(REGEX REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endif()
        set(${variable} "${${variable}} /DXML_STATIC /D_LIB")
      endforeach()
    endif()
    
    # remove /Ob2 and /Ob1 - they cause linker issues
    set(obs /Ob2 /Ob1)
    foreach(ob ${obs})
      foreach(variable ${variables})
        if(${variable} MATCHES ${ob} )
          string(REGEX REPLACE ${ob} "" ${variable} "${${variable}}")
        endif()
      endforeach()
    endforeach()
    msvc_runtime_set_static_ignores(${bDynamic} ${bDynamicRuntime})
  endif()
endmacro()

msvc_runtime_configure(${EXIV2_ENABLE_SHARED} ${EXIV2_ENABLE_DYNAMIC_RUNTIME})
