# ************************************************************* -*- Makefile -*-
#
# Copyright (C) 2004 Andreas Huggel <ahuggel@gmx.net>
#
# This Makefile is part of the Exiv2 distribution.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
# 02111-1307, USA.
#
# File:      Makefile
# Version:   $Name:  $ $Revision: 1.21 $
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   10-Dec-03, ahu: created
#
# Description:
#  Do NOT change this file! All system specific settings and configs
#  go into config.mk.
#
#  This makefile contains (supposedly) generic build rules to build the
#  library and applications. It includes all system specific settings
#  from config.mk. The idea is that configuring and porting the 
#  software to a new platform should only require changes in config.mk.
#
# Restrictions:
#  Requires GNU make.
#

# Default make target
all: lib bin

# Include system configuration
top_srcdir = ..
include $(top_srcdir)/config.mk

# ******************************************************************************
# Source files

# Add standalone C++ header files to this list
CCHDR = rcsid.hpp error.hpp

# Add library C++ source files to this list
CCSRC = canonmn.cpp exif.cpp fujimn.cpp ifd.cpp image.cpp makernote.cpp \
        sigmamn.cpp tags.cpp types.cpp value.cpp

# Add source files of simple applications to this list
BINSRC = addmoddel.cpp exifprint.cpp makernote-test.cpp taglist.cpp \
         write-test.cpp

# State the main source file of the Exiv2 application here
EXIV2MAIN = exiv2.cpp

# Add additional source files of the real application to this list
EXIV2SRC = actions.cpp utils.cpp

# ******************************************************************************
# Library

LIBNAME = exiv2

# ******************************************************************************
# Defines, Includes and Libraries
CXXDEFS = $(DEFS)
CXXINCS = $(INCS)
LDLIBS  = $(LIBS) -l$(LIBNAME)

# ******************************************************************************
# ==============================================================================
# ******************************************************************************

# Initialisations
SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .cpp .o .so

.PRECIOUS: %.cpp

# Generic variables
CCHDR := $(CCHDR) $(CCSRC:.cpp=.hpp)
CCOBJ = $(CCSRC:.cpp=.o)
CCSOBJ = $(CCSRC:.cpp=.so)

SRC = $(CCSRC)
HDR = $(CCHDR)
OBJ = $(CCOBJ)
SOBJ = $(CCSOBJ)
DEP = $(CCSRC:%.cpp=.%.d) $(BINSRC:%.cpp=.%.d) \
      $(EXIV2MAIN:%.cpp=.%.d) $(EXIV2SRC:%.cpp=.%.d)

BINOBJ = $(BINSRC:.cpp=.o)
BINARY = $(BINSRC:.cpp=)

EXIV2OBJ = $(EXIV2MAIN:.cpp=.o) $(EXIV2SRC:.cpp=.o)
EXIV2BIN = $(EXIV2MAIN:.cpp=)

ARCHIVE = lib$(LIBNAME)$(ARCHIVE_SUFFIX)
SHAREDLIB = lib$(LIBNAME)$(SHAREDLIB_SUFFIX)

# ******************************************************************************
# Assemble the dependencies for the 'lib' target and corresponding 
# (un)install targets. If neither STATIC_LIBS nor SHARED_LIBS is 
# defined, 'lib' does nothing.
ifdef STATIC_LIBS
    LIBRARY = archive
    INSTALL = bin
    INSTALL_LIB = install-archive
    UNINSTALL_LIB = uninstall-archive
    LDLIBS := mn.o $(LDLIBS)
endif
ifdef SHARED_LIBS
    LIBRARY := $(LIBRARY) sharedlib
    INSTALL = bin install-sharedlib
    INSTALL_LIB := $(INSTALL_LIB) install-sharedlib
    UNINSTALL_LIB := $(UNINSTALL_LIB) uninstall-sharedlib
endif

# ******************************************************************************
# Include `.*.d' files, but only if we need them, 
# i.e., if no target was given...
ifeq ($(strip $(MAKECMDGOALS)),)
-include $(DEP)
else
# ...or the target is _not_ one in the list of targets below.
NOINCLUDE = uninstall uninstall-lib check doc ctags mostlyclean clean  \
            install-header uninstall-header distclean maintainer-clean \
            uninstall-archive uninstall-sharedlib
ifneq ($(MAKECMDGOALS), $(filter $(MAKECMDGOALS), $(NOINCLUDE)))
-include $(DEP)
endif
endif

# ******************************************************************************
# Rules
%.o: %.cpp
	$(CXX) $(CXXFLAGS_STATIC) $(CXXDEFS) $(CXXINCS) -c $< -o $@

%.so: %.cpp
	$(CXX) $(CXXFLAGS_SHARED) $(CXXDEFS) $(CXXINCS) -c $< -o $@

%.ii: %.cpp
	set -e; \
	$(CXXCPP) $(CPPFLAGS) $(CXXDEFS) $(CXXINCS) $< \
	| sed '/^[ 	]*$$/d' > $@

# generate a makefile with the prerequisites for each source file
# (see `info make' for details)
.%.d: %.cpp
	@echo generating $@
	@set -e; \
	$(CXXDEP) $(CPPFLAGS) $(CXXDEFS) $(CXXINCS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o \1.so $@ : /' > $@; \
	[ -s $@ ] || rm -f $@

$(BINARY): %: %.o
	$(CXX) $(CXXFLAGS) $< $(LDLIBS) $(LDFLAGS_BIN) -o $@

$(EXIV2BIN): %: %.o
	$(CXX) $(CXXFLAGS) $(EXIV2OBJ) $(LDLIBS) $(LDFLAGS_BIN) -o $@

mn.cpp: ./mn.sh
	./mn.sh

# ******************************************************************************
# Targets
.PHONY: all archive sharedlib bin check ctags doc                      \
        clean mostlyclean distclean maintainer-clean                   \
        install install-archive install-header                         \
        install-sharedlib install-lib                                  \
        uninstall uninstall-archive uninstall-header                   \
        uninstall-sharedlib uninstall-lib                              \

$(ARCHIVE)($(OBJ)): $(OBJ)
	@$(AR) $(ARFLAGS) $@ $%

$(ARCHIVE): $(ARCHIVE)($(OBJ))
	@if test -n "$(CXX_REPOSITORY)"; then \
	    find $(CXX_REPOSITORY) -name "*.o" -type f | \
	    xargs $(AR) $(ARFLAGS) $@; \
	fi
	$(RANLIB) $@

archive: $(ARCHIVE)

$(SHAREDLIB): $(SOBJ)
	$(CXX) $^ $(LDFLAGS_SHARED) -o $@

sharedlib: $(SHAREDLIB)

lib: $(LIBRARY)

# Re-link executables whenever the static library changes
ifdef STATIC_LIBS
$(BINARY): $(ARCHIVE) mn.o
endif

$(EXIV2BIN): lib $(EXIV2OBJ)

bin: lib $(BINARY) $(EXIV2BIN)

install install-bin: $(INSTALL)
	mkinstalldirs $(bindir) 
	@list='$(BINARY) $(EXIV2BIN)'; for p in $$list; do \
	    if test -f $$p; then \
	        echo "$(INSTALL_PROGRAM) $$p $(bindir)/$$p"; \
	        $(INSTALL_PROGRAM) $$p $(bindir)/$$p; \
	    else :; fi; \
	done

install-header:
	mkinstalldirs $(incdir)
	@list='$(HDR)'; for p in $$list; do \
	    if test -f $$p; then \
	        echo "$(INSTALL_DATA) $$p $(incdir)/$$p"; \
	        $(INSTALL_DATA) $$p $(incdir)/$$p; \
	    else :; fi; \
	done

install-archive: archive
	mkinstalldirs $(libdir)
	$(INSTALL_DATA) $(ARCHIVE) $(libdir)/$(ARCHIVE)

install-sharedlib: sharedlib
	mkinstalldirs $(libdir)
	$(INSTALL_DATA) $(SHAREDLIB) $(libdir)/$(SHAREDLIB)

install-lib: $(INSTALL_LIB) install-header

uninstall:
	@list='$(BINARY) $(EXIV2BIN)'; for p in $$list; do \
	    echo "rm -f $(bindir)/$$p"; \
	    rm -f $(bindir)/$$p; \
	done

uninstall-header:
	@list='$(HDR)'; for p in $$list; do \
	    echo "rm -f $(incdir)/$$p"; \
	    rm -f $(incdir)/$$p; \
	done

uninstall-archive:
	$(RM) $(libdir)/$(ARCHIVE)

uninstall-sharedlib:
	$(RM) $(libdir)/$(SHAREDLIB)

uninstall-lib: $(UNINSTALL_LIB) uninstall-header

ctags:
	ebrowse $(CCHDR) $(CCSRC)
#	ctags-exuberant --extra=+q -e *
#	ctags-exuberant --extra=+q *

check:
	@echo "No checks available for this library."

mostlyclean:
	$(RM) core
	$(RM) $(CCSRC:.cpp=.ii)
	$(RM) $(OBJ) $(SOBJ) $(BINOBJ) $(EXIV2OBJ)
	$(RM) mn.o
	@if test -n "$(CXX_REPOSITORY)"; then \
	    echo "rm -rf $(CXX_REPOSITORY)"; \
	    rm -rf $(CXX_REPOSITORY); \
	fi

clean: mostlyclean
	$(RM) $(ARCHIVE) $(SHAREDLIB)
	$(RM) $(BINARY) $(EXIV2BIN)

# Run `make distclean' from the top source directory to also remove
# files created by configuring the program.
distclean: clean
	$(RM) tags TAGS
	$(RM) $(DEP)
	$(RM) .mn.d
	$(RM) *~ *#

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: uninstall uninstall-lib distclean
	$(RM) mn.cpp
