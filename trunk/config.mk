# ***************************************************** -*- Makefile -*-
#
# Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
#
# This Makefile is part of the Exiv2 distribution.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
# 02111-1307, USA.
#
# File:      config.mk
# Version:   $Name:  $ $Revision: 1.4 $
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   10-Dec-03, ahu: created
#
# Description: 
#  System configuration file for GNU/Linux: gcc 3.x on Debian GNU/Linux
#  running Linux kernel 2.2.x or higher
#

# **********************************************************************
# Define which libraries (shared and/or static) to build
SHARED_LIBS = 1
STATIC_LIBS = 

# **********************************************************************
# C++ Compiler and precompiler
CXX = g++

# Common compiler flags (warnings, symbols [-ggdb], optimization [-O2], etc)
CXXFLAGS := -Wall -Woverloaded-virtual -Wsign-promo -O2
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
LDFLAGS_BIN = $(LDFLAGS)
# Linker flags used to link shared libraries
LDFLAGS_SHARED = $(LDFLAGS) -shared -fPIC

# Repository for object files of templates (e.g., Compaq Tru64 cxx)
CXX_REPOSITORY = 

# Library filename suffixes
SHAREDLIB_SUFFIX = .so
ARCHIVE_SUFFIX = .a

# **********************************************************************
# Global includes, libraries and defines
INCS = -I. -I$(top_srcdir) -I$(incdir)
LIBS = -L. -L$(libdir) -L/usr/local/lib
DEFS = 

# **********************************************************************
# Archive management
RANLIB = ranlib
AR = ar
ARFLAGS = rcuv

# **********************************************************************
# Installation programs
INSTALL_EXE = $(top_srcdir)/install-sh
INSTALL_PROGRAM = $(INSTALL_EXE) -c
INSTALL_DATA = $(INSTALL_EXE) -c -m 644
INSTALL_DIRS = $(top_srcdir)/mkinstalldirs

# **********************************************************************
# Other programs
RM = @rm -vf

# **********************************************************************
# Directories
prefix = ..
exec_prefix = /usr/local

# Source directory
srcdir = .

# Installation directories
bindir = ${exec_prefix}/bin
incdir = ${exec_prefix}/include
libdir = ${exec_prefix}/lib
