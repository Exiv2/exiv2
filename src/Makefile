# ***************************************************** -*- Makefile -*-
#
# Copyright (c) 2004 Andreas Huggel
# 
# Todo: Insert the license blabla here
# 
# Author(s): Andreas Huggel (ahu)
#
# RCS information
#  $Name:  $
#  $Revision: 1.1 $
#
# Description:
#  Do NOT change this file! All system specific settings and configs
#  go into config.mk.
#
#  This makefile contains (supposedly) generic build rules to build a
#  library and applications. It includes all system specific settings
#  from config.mk. The idea is that configuring and porting the 
#  software to a new platform should only require changes in config.mk.
#
#  Adding new source files or applications: Just add an entry with the
#  name of the new file in the section 'Source files' below.
#
# Restrictions:
#  Requires GNU make.
#

# Default make target
all: lib bin

# Include system configuration
top_srcdir = ..
include $(top_srcdir)/config.mk

# **********************************************************************
# Source files

# Add standalone C++ header files to this list
CCHDR = 

# Add library C++ source files to this list
CCSRC = exif.cc utils.cc

# Add source files of applications to this list
BINSRC = exiftest.cc

# **********************************************************************
# Library

LIBNAME = exiv2

# **********************************************************************
# Defines, Includes and Libraries
CXXDEFS = $(DEFS)
CXXINCS = $(INCS)
LDLIBS  = $(LIBS) -l$(LIBNAME)

# **********************************************************************
# ======================================================================
# **********************************************************************

# Initialisations
SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .cc .o .so

.PRECIOUS: %.cc

# Generic variables
CCHDR := $(CCHDR) $(CCSRC:.cc=.h)
CCOBJ = $(CCSRC:.cc=.o)
CCSOBJ = $(CCSRC:.cc=.so)

SRC = $(CCSRC)
HDR = $(CCHDR)
OBJ = $(CCOBJ)
SOBJ = $(CCSOBJ)
DEP = $(CCSRC:%.cc=.%.d) $(BINSRC:%.cc=.%.d)

BINOBJ = $(BINSRC:.cc=.o)
BINARY = $(BINSRC:.cc=)

ARCHIVE = lib$(LIBNAME)$(ARCHIVE_SUFFIX)
SHAREDLIB = lib$(LIBNAME)$(SHAREDLIB_SUFFIX)

# **********************************************************************
# Assemble the dependencies for the 'lib' target and corresponding 
# (un)install targets. If neither STATIC_LIBS nor SHARED_LIBS is 
# defined, 'lib' does nothing.
ifdef STATIC_LIBS
    LIBRARY = archive
    INSTALL = bin
    INSTALL_LIB = install-archive
    UNINSTALL_LIB = uninstall-archive
endif
ifdef SHARED_LIBS
    LIBRARY := $(LIBRARY) sharedlib
    INSTALL = bin install-sharedlib
    INSTALL_LIB := $(INSTALL_LIB) install-sharedlib
    UNINSTALL_LIB := $(UNINSTALL_LIB) uninstall-sharedlib
endif

# **********************************************************************
# Include `.*.d' files, but only if we need them, 
# i.e., if no target was given...
ifeq ($(strip $(MAKECMDGOALS)),)
-include $(DEP)
else
# ...or the target is _not_ one in the list of targets below.
NOINCLUDE = uninstall uninstall-lib check doc mostlyclean clean        \
            install-header uninstall-header distclean maintainer-clean \
            uninstall-archive uninstall-sharedlib
ifneq ($(MAKECMDGOALS), $(filter $(MAKECMDGOALS), $(NOINCLUDE)))
-include $(DEP)
endif
endif

# **********************************************************************
# Rules
%.o: %.cc
	$(CXX) $(CXXFLAGS_STATIC) $(CXXDEFS) $(CXXINCS) -c $< -o $@

%.so: %.cc
	$(CXX) $(CXXFLAGS_SHARED) $(CXXDEFS) $(CXXINCS) -c $< -o $@

%.ii: %.cc
	set -e; \
	$(CXXCPP) $(CPPFLAGS) $(CXXDEFS) $(CXXINCS) $< \
	| sed '/^[ 	]*$$/d' > $@

# generate a makefile with the prerequisites for each source file
# (see `info make' for details)
.%.d: %.cc
	@echo generating $@
	@set -e; \
	$(CXXDEP) $(CPPFLAGS) $(CXXDEFS) $(CXXINCS) $< \
	| sed 's/\($*\)\.o[ :]*/\1.o \1.so $@ : /' > $@; \
	[ -s $@ ] || rm -f $@

$(BINARY): %: %.o
	$(CXX) $(CXXFLAGS) $< $(LDLIBS) $(LDFLAGS_BIN) -o $@

# **********************************************************************
# Targets
.PHONY: all archive sharedlib bin check doc                            \
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
	$(CXX) $^ $(PMTLIBS) $(LDFLAGS_SHARED) -o $@

sharedlib: $(SHAREDLIB)

lib: $(LIBRARY)

# Re-link executables whenever the static library changes
ifdef STATIC_LIBS
$(BINARY): $(ARCHIVE)
endif

bin: lib $(BINARY)

install install-bin: $(INSTALL)
	mkinstalldirs $(bindir) 
	@list='$(BINARY)'; for p in $$list; do \
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
	@list='$(BINARY)'; for p in $$list; do \
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

check:
	@echo "No checks available for this library."

mostlyclean:
	$(RM) core
	$(RM) $(CCSRC:.cc=.ii)
	$(RM) $(OBJ) $(SOBJ) $(BINOBJ)
	@if test -n "$(CXX_REPOSITORY)"; then \
	    echo "rm -rf $(CXX_REPOSITORY)"; \
	    rm -rf $(CXX_REPOSITORY); \
	fi

clean: mostlyclean
	$(RM) $(ARCHIVE) $(SHAREDLIB)
	$(RM) $(BINARY)

# Run `make distclean' from the top source directory to also remove
# files created by configuring the program.
distclean: clean
	$(RM) tags TAGS
	$(RM) $(DEP)
	$(RM) *~ *#

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: uninstall uninstall-lib distclean
	rm -rf $(top_srcdir)/html/*
