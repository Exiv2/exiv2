# ***************************************************** -*- Makefile -*-
#
# Copyright (c) 2003, 2004 Andreas Huggel
# All rights reserved. This program and the accompanying materials 
# are made available under the terms of the Common Public License v1.0
# which accompanies this distribution, and is available at
# http://www.ibm.com/developerworks/oss/CPLv1.0.htm
#
# Author(s): Andreas Huggel (ahu)
#
# RCS information
#  $Name:  $
#  $Revision: 1.1 $
#
# Description: System configuration file for GNU/Linux:
#  gcc 3.x on Debian GNU/Linux running Linux kernel 2.2.x or higher
#

# **********************************************************************
# Define which libraries (shared and/or static) to build
SHARED_LIBS = 
STATIC_LIBS = 1

# **********************************************************************
# C++ Compiler and precompiler
CXX = g++

# Common compiler flags (warnings, symbols, optimization, etc)
CXXFLAGS := -Wall -g
# Compiler flags to compile static objects
CXXFLAGS_STATIC := $(CXXFLAGS)
# Compiler flags for shared objects
CXXFLAGS_SHARED := $(CXXFLAGS) -fPIC

# Command to run only the preprocessor
CXXCPP = $(CXX) -E -dD

# Preprocessor flags
CPPFLAGS =

# Command to run the compiler or preprocessor to produce dependencies
CXXDEP = $(CXX) -MM
# Flag to $(CXXDEP) to treat all subsequent file names as C++ source 
# files, regardless of their suffixes
CXXLANGCXX = -x c++

# Linker flags
LDFLAGS = 
# Linker flags to link applications
LDFLAGS_APP = $(LDFLAGS)
# Linker flags used to link shared libraries
LDFLAGS_SHARED = $(LDFLAGS) -shared -fPIC

# Repository for object files of templates (e.g., Compaq Tru64 cxx)
CXX_REPOSITORY = 

# Library filename suffixes
SHAREDLIB_SUFFIX = .so
ARCHIVE_SUFFIX = .a

# **********************************************************************
# Global includes, libraries and defines
INCS = -I. -I$(incdir) -I/usr/local/include
LIBS = -L. -L$(libdir) -L/usr/local/lib
DEFS = 

# **********************************************************************
# Archive management
RANLIB = ranlib
AR = ar
ARFLAGS = rcuv

# **********************************************************************
# Installation programs
INSTALL_EXE = /usr/bin/install
INSTALL_PROGRAM = $(INSTALL_EXE) -c -p
INSTALL_DATA = $(INSTALL_EXE) -c -m 644 -p

# **********************************************************************
# Other programs
RM = @rm -vf

# **********************************************************************
# Directories
prefix = ..
#exec_prefix = ${prefix}
exec_prefix = ~

# Source directory
srcdir = .

# Installation directories
bindir = ${exec_prefix}/bin
incdir = ${exec_prefix}/include
idldir = ${exec_prefix}/idl
libdir = ${exec_prefix}/lib
mandir = ${prefix}/man
man1dir = $(mandir)/man1
man1ext = 1
man3dir = $(mandir)/man3
man3ext = 3
datadir = ${prefix}/share
