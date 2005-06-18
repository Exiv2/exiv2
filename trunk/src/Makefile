# ************************************************************* -*- Makefile -*-
#
# Copyright (C) 2004, 2005 Andreas Huggel <ahuggel@gmx.net>
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
# Version:   $Rev$
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
all: bin

# Include system configuration
top_srcdir = ..
include $(top_srcdir)/config/config.mk

# ******************************************************************************
# Source files

# Add standalone C++ header files to this list
CCHDR = exv_conf.h exv_msvc.h mn.hpp rcsid.hpp 

# Add library C++ source files to this list
CCSRC = basicio.cpp canonmn.cpp datasets.cpp error.cpp exif.cpp futils.cpp \
	fujimn.cpp ifd.cpp image.cpp iptc.cpp jpgimage.cpp makernote.cpp \
        metadatum.cpp nikonmn.cpp olympusmn.cpp panasonicmn.cpp sigmamn.cpp \
	sonymn.cpp tags.cpp types.cpp value.cpp 

# Add source files of simple applications to this list
BINSRC = addmoddel.cpp dataarea-test.cpp exifcomment.cpp exifdata-test.cpp \
         exifprint.cpp ifd-test.cpp iotest.cpp iptceasy.cpp iptcprint.cpp \
         iptctest.cpp key-test.cpp makernote-test.cpp taglist.cpp write-test.cpp \
         write2-test.cpp 

# Main source file of the Exiv2 application
EXIV2MAIN = exiv2.cpp

# Add additional source files of the Exiv2 application to this list
EXIV2SRC = actions.cpp utils.cpp 
# C source files of the Exiv2 application
ifndef HAVE_TIMEGM
EXIVCSRC = localtime.c
endif

# State the main source file of the metacopy application here
MCMAIN = metacopy.cpp

# Add additional source files of the metacopy application to this list
MCSRC = utils.cpp

# ******************************************************************************
# Library

LIBRARY = libexiv2.la

# ******************************************************************************
# Defines, Includes and Libraries
CXXDEFS = $(DEFS)
CXXINCS = $(INCS)
LDLIBS  = $(LIBS) $(LIBRARY)

# ******************************************************************************
# ==============================================================================
# ******************************************************************************

# Initialisations
SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .cpp .o .so

.PRECIOUS: %.cpp

# Generic variables
CCHDR := $(CCHDR) $(CCSRC:.cpp=.hpp)
CCOBJ = $(CCSRC:.cpp=.o)
CCLOBJ = $(CCSRC:.cpp=.lo)

SRC = $(CCSRC)
HDR = $(CCHDR)
OBJ = $(CCOBJ)
LOBJ = $(CCLOBJ)

DEPDIR = .deps
DEP = $(CCSRC:%.cpp=$(DEPDIR)/%.d) $(BINSRC:%.cpp=$(DEPDIR)/%.d) \
      $(EXIV2MAIN:%.cpp=$(DEPDIR)/%.d) $(EXIV2SRC:%.cpp=$(DEPDIR)/%.d) \
      $(EXIVCSRC:%.c=$(DEPDIR)/%.d) $(MCMAIN:%.cpp=$(DEPDIR)/%.d) \
      $(MCSRC:%.cpp=$(DEPDIR)/%.d) $(DEPDIR)/path-test.d

BINOBJ = $(BINSRC:.cpp=.o)
BINARY = $(BINSRC:.cpp=)
EXECUTABLE = $(BINSRC:.cpp=$(EXEEXT))

EXIV2OBJ = $(EXIV2MAIN:.cpp=.o) $(EXIV2SRC:.cpp=.o) 
EXIV2COBJ = $(EXIVCSRC:.c=.o)
EXIV2BIN = $(EXIV2MAIN:.cpp=)
EXIV2EXE = $(EXIV2MAIN:.cpp=$(EXEEXT))

MCOBJ = $(MCMAIN:.cpp=.o) $(MCSRC:.cpp=.o)
MCBIN = $(MCMAIN:.cpp=)
MCEXE = $(MCMAIN:.cpp=$(EXEEXT))

# ******************************************************************************
# Include dependency files
-include $(DEP)

# Dependency files post-process commands 
POSTDEPEND = cp $*.d $(DEPDIR)/$*.d; \
	     sed -e 's/^\#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                 -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $(DEPDIR)/$*.d; \
             $(RM) $*.d

# ******************************************************************************
# Rules
$(OBJ): %.o: %.cpp | $(DEPDIR)
	@$(LIBTOOL) --mode=compile $(CXX) $(CXXFLAGS) $(CXXDEFS) $(CXXINCS) -o $@ -c $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

$(sort $(BINOBJ) $(EXIV2OBJ) $(MCOBJ)): %.o: %.cpp | $(DEPDIR)
	$(CXX) $(CXXFLAGS) $(CXXDEFS) $(CXXINCS) -o $@ -c $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

%.o: %.c | $(DEPDIR)
	$(CC) $(CFLAGS) $(DEFS) $(INCS) -o $@ -c $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

%.ii: %.cpp
	set -e; \
	$(CXXCPP) $(CPPFLAGS) $(CXXDEFS) $(CXXINCS) $< \
	| sed '/^[ 	]*$$/d' > $@

# ******************************************************************************
# Targets
.PHONY: all bin check ctags doc                                        \
        clean mostlyclean distclean maintainer-clean                   \
        install install-header install-lib                             \
        uninstall uninstall-header uninstall-lib

actions.cpp basicio.cpp exif.cpp exiv2.cpp futils.cpp image.cpp jpgimage.cpp utils.cpp: exv_conf.h

exv_conf.h: $(top_srcdir)/config/config.h
	sed 's/#define \([A-Z]\)/#define EXV_\1/; s/#undef \([A-Z]\)/#undef EXV_\1/' < $< > $@

$(DEPDIR):
	mkdir $(DEPDIR)

$(LIBTOOL): $(LIBTOOL_DEPS)
	$(SHELL) $(top_srcdir)/config.status --recheck

bin: lib $(BINARY) $(EXIV2BIN) $(MCBIN) path-test

lib: $(OBJ)
	$(LIBTOOL) --mode=link $(CXX) $(LDFLAGS) $(LOBJ) -o $(LIBRARY) -rpath $(libdir) -release $(EXIV2_VERSION) 
	@touch lib

path-test: path-test.o utils.o
	$(CXX) $(CXXFLAGS) path-test.o utils.o -o $@

$(BINARY): %: %.o lib
	@$(LIBTOOL) --mode=link $(CXX) $(CXXFLAGS) $(LDLIBS) $@.o $(LDFLAGS) -o $@ -rpath $(libdir)

$(EXIV2BIN): lib $(EXIV2OBJ) $(EXIV2COBJ)
	@$(LIBTOOL) --mode=link $(CXX) $(CXXFLAGS) $(EXIV2OBJ) $(EXIV2COBJ) $(LDLIBS) $(LDFLAGS) -o $@ -rpath $(libdir)

$(MCBIN): lib $(MCOBJ)
	@$(LIBTOOL) --mode=link $(CXX) $(CXXFLAGS) $(MCOBJ) $(LDLIBS) $(LDFLAGS) -o $@ -rpath $(libdir)

install-header:
	$(INSTALL_DIRS) $(incdir)
	@list='$(HDR)'; for p in $$list; do \
	    if test -f $$p; then \
	        echo "$(INSTALL_DATA) $$p $(incdir)/$$p"; \
	        $(INSTALL_DATA) $$p $(incdir)/$$p; \
	    else :; fi; \
	done

install-lib: lib install-header
	$(INSTALL_DIRS) $(libdir)
	@$(LIBTOOL) --mode=install $(INSTALL_DATA) $(LIBRARY) $(libdir)/$(LIBRARY)

install: $(EXIV2BIN) install-lib
	$(INSTALL_DIRS) $(bindir) 
	@$(LIBTOOL) --mode=install $(INSTALL_PROGRAM) $(EXIV2EXE) $(bindir)/$(EXIV2EXE)

uninstall-header:
	@list='$(HDR)'; for p in $$list; do \
	    echo "$(RM) $(incdir)/$$p"; \
	    $(RM) $(incdir)/$$p; \
	done
	-rmdir $(incdir)

uninstall-lib: uninstall-header
	@$(LIBTOOL) --mode=uninstall $(RM) $(libdir)/$(LIBRARY)
	-rmdir $(libdir)

uninstall: uninstall-lib
	@$(LIBTOOL) --mode=uninstall $(RM) $(bindir)/$(EXIV2EXE)
	-rmdir $(bindir)

ctags:
	ebrowse $(CCHDR) $(CCSRC)
#	ctags-exuberant --extra=+q -e *
#	ctags-exuberant --extra=+q *

check:
	@echo "No checks available for this library."

mostlyclean:
	$(RM) core
	$(RM) $(CCSRC:.cpp=.ii)
	$(RM) lib
	$(RM) path-test.o
	@$(LIBTOOL) --mode=clean $(RM) $(LOBJ) $(sort $(BINOBJ) $(EXIV2OBJ) $(EXIV2COBJ) $(MCOBJ))

clean: mostlyclean
	@$(LIBTOOL) --mode=clean $(RM) $(LIBRARY)
	@$(LIBTOOL) --mode=clean $(RM) $(EXECUTABLE) $(EXIV2EXE) $(MCEXE)
	@$(LIBTOOL) --mode=clean $(RM) path-test$(EXEEXT)

# Run `make distclean' from the top source directory to also remove
# files created by configuring the program.
distclean: clean
	$(RM) exv_conf.h
	$(RM) $(DEP)
	rmdir $(DEPDIR)
	$(RM) tags TAGS
	$(RM) *~ *.bak *#

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: uninstall distclean
