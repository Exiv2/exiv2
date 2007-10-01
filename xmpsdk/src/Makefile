# ************************************************************* -*- Makefile -*-
#
# Copyright (C) 2007 Andreas Huggel <ahuggel@gmx.net>
#
# This Makefile is part of the Exiv2 distribution.
#
# NOTICE: You are permitted to use, modify, and distribute this file in
# accordance with the terms of the accompanying license agreement valid for 
# the XMP Toolkit.
#
# File:      Makefile
# Version:   $Rev$
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   30-Sep-07, ahu: created
#
# Description:
#  Makefile to build the Adobe XMP Toolkit integrated and distributed with
#  Exiv2. Adapted from the main Exiv2 src/Makefile.
#
# Restrictions:
#  Requires GNU make.
#

# Default make target
all: install

# Include system configuration
top_srcdir = ../..
include $(top_srcdir)/config/config.mk

# ******************************************************************************
# Source files

# Add library C++ source files to this list
CCSRC = XMPMeta.cpp            \
        XMPMeta-GetSet.cpp     \
        XMPMeta-Parse.cpp      \
        XMPMeta-Serialize.cpp  \
        XMPIterator.cpp        \
        XMPUtils.cpp           \
        XMPUtils-FileInfo.cpp  \
        XMPCore_Impl.cpp       \
        ExpatAdapter.cpp       \
        ParseRDF.cpp           \
        UnicodeConversions.cpp \
        MD5.cpp                \
        WXMPMeta.cpp           \
        WXMPIterator.cpp       \
        WXMPUtils.cpp

# ******************************************************************************
# Library
LIBRARY = libxmpsdk.a

# ******************************************************************************
# Initialisations
SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .cpp .o .so

.PRECIOUS: %.cpp

# For a debug build, replace -DNDEBUG=1 with -DDEBUG=1 -D_DEBUG=1
CPPFLAGS += -I../include -funsigned-char -DUNIX_ENV=1 -DXMP_IMPL=1 -DXMP_ClientBuild=0 -D_FILE_OFFSET_BITS=64 -DHAVE_EXPAT_CONFIG_H=1 -DXML_STATIC=1 -DNDEBUG=1 $(EXPAT_CPPFLAGS)
LDFLAGS += $(EXPAT_LDFLAGS)
LIBS += $(EXPAT_LIBS)

# Generic variables
CCOBJ = $(CCSRC:.cpp=.o)
CCLOBJ = $(CCSRC:.cpp=.lo)

# Compilation shortcuts
COMPILE.cc = $(CXX) $(CXXFLAGS) $(CPPFLAGS) -c
# LINK.cc does not need $(LIBS), libtool's dark magic takes care of that
LINK.cc = $(CXX) $(CXXFLAGS) $(LIBRARY) $(LDFLAGS)

# ******************************************************************************
# Rules
$(CCOBJ): %.o: %.cpp
	@$(LIBTOOL) --mode=compile $(COMPILE.cc) -o $@ $< -static -prefer-non-pic

%.o: %.c
	$(COMPILE.c) -o $@ $<

%.ii: %.cpp
	set -e; \
	$(CXXCPP) $(CPPFLAGS) $< | sed '/^[ 	]*$$/d' > $@

# ******************************************************************************
# Targets
.PHONY: all mostlyclean clean distclean maintainer-clean install uninstall

$(LIBTOOL): $(LIBTOOL_DEPS)
	$(SHELL) $(top_srcdir)/config.status --recheck

xmpsdk: install

lib: $(CCOBJ)
	$(LIBTOOL) --mode=link $(CXX) $(LDFLAGS) $(CCLOBJ) -o $(LIBRARY) -static
	@touch lib

install: lib
	$(INSTALL_DATA) $(LIBRARY) ../lib/$(LIBRARY)

uninstall:
	$(RM) ../lib/$(LIBRARY)

mostlyclean:
	$(RM) core
	$(RM) $(CCSRC:.cpp=.ii)
	$(RM) lib
	$(RM) $(CCSRC:%.cpp=.libs/%.d)
	@$(LIBTOOL) --mode=clean $(RM) $(CCLOBJ)

clean: uninstall mostlyclean
	@$(LIBTOOL) --mode=clean $(RM) $(LIBRARY)

# Run `make distclean' from the top source directory to also remove
# files created by configuring the program.
distclean: clean
	$(RM) *~ *.bak *#

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: distclean
