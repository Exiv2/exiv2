set(CMAKE_SYSTEM_NAME Windows)
#set(CMAKE_SYSTEM_PROCESSOR AMD64)
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
#set(CMAKE_SYSROOT /usr/x86_64-w64-mingw32/)
set(CMAKE_SYSROOT /usr/lib/gcc/x86_64-w64-mingw32/9.3-posix)

# cross compilers to use for C and C++
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_Fortran_COMPILER ${TOOLCHAIN_PREFIX}-gfortran)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# target environment on the build host system
#   set 1st to dir with the cross compiler's C/C++ headers/libs
#set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})
#set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})

# modify default behavior of FIND_XXX() commands to
# search for headers/libs in the target environment and
# search for programs in the build host environment
#set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
#set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
#set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
