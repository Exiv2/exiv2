# ************************************************************* -*- Makefile -*-
#
# Copyright (C) 2004-2007 Andreas Huggel <ahuggel@gmx.net>
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
# Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
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
CCHDR =  exv_conf.h           \
         exv_msvc.h           \
         mn.hpp               \
         rcsid.hpp            \
         tifffwd.hpp          \
         version.hpp

# Add library C++ source files to this list
CCSRC =  basicio.cpp          \
	 canonmn.cpp          \
	 cr2image.cpp         \
	 crwimage.cpp         \
	 datasets.cpp         \
	 error.cpp            \
	 exif.cpp             \
	 futils.cpp           \
	 fujimn.cpp           \
	 ifd.cpp              \
	 image.cpp            \
	 iptc.cpp             \
	 jpgimage.cpp         \
	 makernote.cpp        \
	 makernote2.cpp       \
	 metadatum.cpp        \
	 minoltamn.cpp        \
	 mrwimage.cpp         \
	 nikonmn.cpp          \
	 olympusmn.cpp        \
	 panasonicmn.cpp
ifdef HAVE_LIBZ
CCSRC += pngimage.cpp         \
	 pngchunk.cpp
endif
CCSRC += rafimage.cpp         \
	 sigmamn.cpp          \
	 sonymn.cpp           \
	 tags.cpp             \
	 tiffcomposite.cpp    \
	 tiffimage.cpp        \
	 tiffparser.cpp       \
	 tiffvisitor.cpp      \
	 types.cpp            \
	 value.cpp            \
	 version.cpp

# Add library C source files to this list
ifndef HAVE_TIMEGM
CSRC =   localtime.c
endif

# Add source files of simple applications to this list
BINSRC = addmoddel.cpp        \
         crwedit.cpp          \
         crwparse.cpp         \
         dataarea-test.cpp    \
         exifcomment.cpp      \
         exifdata-test.cpp    \
         exifprint.cpp        \
         ifd-test.cpp         \
         iotest.cpp           \
         iptceasy.cpp         \
         iptcprint.cpp        \
         iptctest.cpp         \
         key-test.cpp         \
         largeiptc-test.cpp   \
         makernote-test.cpp   \
         taglist.cpp          \
         write-test.cpp       \
         write2-test.cpp      \
         tiffparse.cpp

# Main source file of the Exiv2 application
EXIV2MAIN = exiv2.cpp
# Add additional source files of the Exiv2 application to this list
EXIV2SRC = actions.cpp        \
           utils.cpp
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
# Initialisations
SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .cpp .o .so

.PRECIOUS: %.cpp

# Generic variables
CCHDR := $(CCHDR) $(CCSRC:.cpp=.hpp)
CCOBJ = $(CCSRC:.cpp=.o)
CCLOBJ = $(CCSRC:.cpp=.lo)

COBJ = $(CSRC:.c=.o)
CLOBJ = $(CSRC:.c=.lo)

SRC = $(CCSRC) $(CSRC)
HDR = $(CCHDR)
OBJ = $(CCOBJ) $(COBJ)
LOBJ = $(CCLOBJ) $(CLOBJ)

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

ifdef DEP_TRACKING
DEPDIR = .deps
DEP = $(CCSRC:%.cpp=$(DEPDIR)/%.d) $(CSRC:%.c=$(DEPDIR)/%.d) \
      $(BINSRC:%.cpp=$(DEPDIR)/%.d) \
      $(EXIV2MAIN:%.cpp=$(DEPDIR)/%.d) $(EXIV2SRC:%.cpp=$(DEPDIR)/%.d) \
      $(EXIVCSRC:%.c=$(DEPDIR)/%.d) $(MCMAIN:%.cpp=$(DEPDIR)/%.d) \
      $(MCSRC:%.cpp=$(DEPDIR)/%.d) $(DEPDIR)/path-test.d

# Dependency files post-process commands
POSTDEPEND = if test ! -d $(DEPDIR); then mkdir $(DEPDIR); fi; \
	     if test -e $*.d; then cp $*.d $(DEPDIR)/$*.d; \
	     sed -e 's/^\#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
                 -e '/^$$/ d' -e 's/$$/ :/' < $*.d >> $(DEPDIR)/$*.d; \
             $(RM) $*.d; fi
endif

# Compilation shortcuts
COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c
COMPILE.c = $(CC) $(CFLAGS) $(CPPFLAGS) -c
# LINK.cc does not need $(LIBS), libtool's dark magic takes care of that
LINK.cc = $(CXX) $(CXXFLAGS) $(LIBRARY) $(LDFLAGS) -rpath $(libdir)

# ******************************************************************************
# Rules
$(CCOBJ): %.o: %.cpp
	@$(LIBTOOL) --mode=compile $(COMPILE.cc) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

$(COBJ): %.o: %.c
	@$(LIBTOOL) --mode=compile $(COMPILE.c) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

$(sort $(BINOBJ) $(EXIV2OBJ) $(MCOBJ) path-test.o): %.o: %.cpp
	$(COMPILE.cc) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

%.o: %.c
	$(COMPILE.c) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

%.ii: %.cpp
	set -e; \
	$(CXXCPP) $(CPPFLAGS) $< | sed '/^[ 	]*$$/d' > $@

# ******************************************************************************
# Targets
.PHONY: all bin check ctags doc                                        \
        clean mostlyclean distclean maintainer-clean                   \
        install install-header install-lib                             \
        uninstall uninstall-header uninstall-lib

ifdef DEP_TRACKING
# Include targets from dependency files
-include $(DEP)
endif

actions.cpp basicio.cpp exif.cpp exiv2.cpp futils.cpp image.cpp jpgimage.cpp utils.cpp: exv_conf.h

exv_conf.h: $(top_srcdir)/config/config.h
	sed 's/#define \([A-Z]\)/#define EXV_\1/; s/#undef \([A-Z]\)/#undef EXV_\1/' < $< > $@

$(LIBTOOL): $(LIBTOOL_DEPS)
	$(SHELL) $(top_srcdir)/config.status --recheck

bin: lib $(BINARY) $(EXIV2BIN) $(MCBIN) path-test

lib: $(OBJ)
	$(LIBTOOL) --mode=link $(CXX) $(LDFLAGS) $(LOBJ) -o $(LIBRARY) -rpath $(libdir) -release $(EXIV2_VERSION) $(LIBS)
	@touch lib

path-test: path-test.o utils.o
	$(CXX) $(CXXFLAGS) path-test.o utils.o -o $@

$(BINARY): %: %.o lib
	@$(LIBTOOL) --mode=link $(LINK.cc) -o $@ $@.o

$(EXIV2BIN): lib $(EXIV2OBJ) $(EXIV2COBJ)
	@$(LIBTOOL) --mode=link $(LINK.cc) -o $@ $(EXIV2OBJ) $(EXIV2COBJ)

$(MCBIN): lib $(MCOBJ)
	@$(LIBTOOL) --mode=link $(LINK.cc) -o $@ $(MCOBJ)

install-header:
	$(INSTALL_DIRS) $(DESTDIR)$(incdir)
	@list='$(HDR)'; for p in $$list; do \
	    if test -f $$p; then \
	        echo "$(INSTALL_DATA) $$p $(DESTDIR)$(incdir)/$$p"; \
	        $(INSTALL_DATA) $$p $(DESTDIR)$(incdir)/$$p; \
	    else :; fi; \
	done

install-lib: lib install-header
	$(INSTALL_DIRS) $(DESTDIR)$(libdir)
	@$(LIBTOOL) --mode=install $(INSTALL_DATA) $(LIBRARY) $(DESTDIR)$(libdir)/$(LIBRARY)
	$(INSTALL_DIRS) $(DESTDIR)$(bindir)
	$(INSTALL_DIRS) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) $(top_srcdir)/config/exiv2.pc $(DESTDIR)$(libdir)/pkgconfig/exiv2.pc

install: $(EXIV2BIN) install-lib
	$(INSTALL_DIRS) $(DESTDIR)$(bindir)
	@$(LIBTOOL) --mode=install $(INSTALL_PROGRAM) $(EXIV2EXE) $(DESTDIR)$(bindir)/$(EXIV2EXE)
	$(INSTALL_DIRS) $(DESTDIR)$(mandir)
	$(INSTALL_DIRS) $(DESTDIR)$(man1dir)
	$(INSTALL_DATA) exiv2.1 $(DESTDIR)$(man1dir)/exiv2.1

uninstall-header:
	@list='$(HDR)'; for p in $$list; do \
	    echo "$(RM) $(DESTDIR)$(incdir)/$$p"; \
	    $(RM) $(DESTDIR)$(incdir)/$$p; \
	done
	-rmdir $(DESTDIR)$(incdir)

uninstall-lib: uninstall-header
	$(RM) $(DESTDIR)$(libdir)/pkgconfig/exiv2.pc
	-rmdir $(DESTDIR)$(libdir)/pkgconfig
	@$(LIBTOOL) --mode=uninstall $(RM) $(DESTDIR)$(libdir)/$(LIBRARY)
	-rmdir $(DESTDIR)$(libdir)

uninstall: uninstall-lib
	$(RM) $(DESTDIR)$(man1dir)/exiv2.1
	-rmdir $(DESTDIR)$(man1dir)
	-rmdir $(DESTDIR)$(mandir)
	@$(LIBTOOL) --mode=uninstall $(RM) $(DESTDIR)$(bindir)/$(EXIV2EXE)
	-rmdir $(DESTDIR)$(bindir)

ctags:
	ebrowse $(HDR) $(SRC)
#	ctags-exuberant --extra=+q -e *
#	ctags-exuberant --extra=+q *

check:
	@echo "No checks available for this library."

mostlyclean:
	$(RM) core
	$(RM) $(CCSRC:.cpp=.ii)
	$(RM) lib
	$(RM) path-test.o
	$(RM) $(CCSRC:%.cpp=.libs/%.d) $(CSRC:%.c=.libs/%.d)
	@$(LIBTOOL) --mode=clean $(RM) $(LOBJ) $(sort $(BINOBJ) $(EXIV2OBJ) $(EXIV2COBJ) $(MCOBJ))

clean: mostlyclean
	@$(LIBTOOL) --mode=clean $(RM) $(LIBRARY)
	@$(LIBTOOL) --mode=clean $(RM) $(EXECUTABLE) $(EXIV2EXE) $(MCEXE)
	@$(LIBTOOL) --mode=clean $(RM) path-test$(EXEEXT)

# Run `make distclean' from the top source directory to also remove
# files created by configuring the program.
distclean: clean
	$(RM) exv_conf.h
ifdef DEP_TRACKING
	$(RM) $(DEP)
	-rmdir $(DEPDIR)
endif
	$(RM) tags TAGS
	$(RM) *~ *.bak *#

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: uninstall distclean
