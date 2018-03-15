# ************************************************************* -*- Makefile -*-
#
# Copyright (C) 2004-2015 Andreas Huggel <ahuggel@gmx.net>
#
# This Makefile is part of the Exiv2 distribution.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    1. Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#    3. The name of the author may not be used to endorse or promote
#       products derived from this software without specific prior
#       written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# File:      Makefile
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

# exiv2 command-line program C++ header files
CCHDR =  exiv2.hpp             \
         config.h              \
         exv_conf.h            \
         xmp_exiv2.hpp         \
         version.hpp

# library C++ source files
CCSRC =  basicio.cpp           \
         bigtiffimage.cpp      \
         bmpimage.cpp          \
         canonmn_int.cpp       \
         casiomn_int.cpp       \
         convert.cpp           \
         cr2image_int.cpp      \
         cr2image.cpp          \
         crwimage_int.cpp      \
         crwimage.cpp          \
         datasets.cpp          \
         easyaccess.cpp        \
         epsimage.cpp          \
         error.cpp             \
         exif.cpp              \
         fujimn_int.cpp        \
         futils.cpp            \
         gifimage.cpp          \
         http.cpp              \
         image.cpp             \
         ini.cpp               \
         iptc.cpp              \
         jp2image.cpp          \
         jpgimage.cpp          \
         makernote_int.cpp     \
         metadatum.cpp         \
         minoltamn_int.cpp     \
         mrwimage.cpp          \
         nikonmn_int.cpp       \
         olympusmn_int.cpp     \
         orfimage_int.cpp      \
         orfimage.cpp          \
         pgfimage.cpp          \
         preview.cpp           \
         panasonicmn_int.cpp   \
         pentaxmn_int.cpp      \
         properties.cpp        \
         psdimage.cpp          \
         rafimage.cpp          \
         rw2image_int.cpp      \
         rw2image.cpp          \
         samsungmn_int.cpp     \
         sigmamn_int.cpp       \
         sonymn_int.cpp        \
         tags.cpp              \
         tgaimage.cpp          \
         tiffcomposite_int.cpp \
         tiffimage_int.cpp     \
         tiffimage.cpp         \
         tiffvisitor_int.cpp   \
         types.cpp             \
         value.cpp             \
         version.cpp           \
         webpimage.cpp         \
         xmp.cpp               \
         xmpsidecar.cpp

ifdef HAVE_LIBZ
CCSRC += pngimage.cpp          \
         pngchunk_int.cpp
endif

ifdef USE_SSH
CCSRC += ssh.cpp
endif

ifdef ENABLE_VIDEO
CCSRC += asfvideo.cpp          \
         matroskavideo.cpp     \
         quicktimevideo.cpp    \
         riffvideo.cpp         \
         utilsvideo.cpp
endif

# C source files
ifndef HAVE_TIMEGM
CSRC =   localtime.c
endif

# Source files for the Exiv2 application
EXIV2MAIN = exiv2.cpp
EXIV2SRC  = actions.cpp \
            utils.cpp
EXIVCSRC  = getopt_win32.c

# ******************************************************************************
# Library
LIBRARY = libexiv2.la

# ******************************************************************************
# Initialisations
SHELL = /bin/bash

.SUFFIXES:
.SUFFIXES: .c .cpp .o .so

.PRECIOUS: %.cpp

CPPFLAGS += $(SSH_CPPFLAGS) $(CURL_CPPFLAGS) $(XMPSDK_CPPFLAGS)
LDFLAGS  += $(SSH_LDFLAGS) $(CURL_LDFLAGS) $(EXPAT_LDFLAGS) $(XMPSDK_LDFLAGS)
LIBS     += $(SSH_LIBS) $(CURL_LIBS) $(EXPAT_LIBS) $(XMPSDK_LIBS)

# Generic variables
CCHDR := $(CCHDR) $(CCSRC:.cpp=.hpp)
CCOBJ  = $(CCSRC:.cpp=.o)
CCLOBJ = $(CCSRC:.cpp=.lo)

COBJ   = $(CSRC:.c=.o)
CLOBJ  = $(CSRC:.c=.lo)

SRC    = $(CCSRC) $(CSRC)
HDR    = $(CCHDR) rwlock.hpp
OBJ    = $(CCOBJ) $(COBJ)
LOBJ   = $(CCLOBJ) $(CLOBJ)

EXIV2OBJ  = $(EXIV2MAIN:.cpp=.o) $(EXIV2SRC:.cpp=.o)
EXIVCOBJ  = $(EXIVCSRC:.c=.o)
EXIV2EXE  = $(EXIV2MAIN:.cpp=$(EXEEXT))

ifdef DEP_TRACKING
DEP = $(CCSRC:%.cpp=$(DEPDIR)/%.d)      \
      $(CSRC:%.c=$(DEPDIR)/%.d)         \
      $(BINSRC:%.cpp=$(DEPDIR)/%.d)     \
      $(EXIV2MAIN:%.cpp=$(DEPDIR)/%.d)  \
      $(EXIV2SRC:%.cpp=$(DEPDIR)/%.d)   \
      $(EXIVCSRC:%.c=$(DEPDIR)/%.d)

endif

# ******************************************************************************
# Rules
$(CCOBJ): %.o: %.cpp
	@$(LIBTOOL) --mode=compile $(COMPILE.cc) -DEXV_BUILDING_LIB=1 -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

$(COBJ): %.o: %.c
	@$(LIBTOOL) --mode=compile $(COMPILE.c) -DEXV_BUILDING_LIB=1 -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

$(EXIV2OBJ): %.o: %.cpp
	$(COMPILE.cc) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

%.o: %.c
	$(COMPILE.c) -o $@ $<
	@$(MAKEDEPEND)
	@$(POSTDEPEND)

%.ii: %.cpp
	set -e; \
	$(CXXCPP) $(CPPFLAGS) -DEXV_BUILDING_LIB=1 $< | LC_ALL=C sed '/^[ 	]*$$/d' > $@

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

# Be sure to rewrite exv_conf.h before compiling anything
$(SRC): ../include/exiv2/exv_conf.h

../include/exiv2/exv_conf.h: $(top_srcdir)/config/config.h
	LC_ALL=C sed 's/#define \([A-Z]\)/#define EXV_\1/; s/#undef \([A-Z]\)/#undef EXV_\1/' < $< > $@

$(LIBTOOL): $(LIBTOOL_DEPS)
	$(SHELL) $(top_srcdir)/config.status --recheck

bin: lib $(EXIV2EXE)

lib: $(OBJ)
	$(LIBTOOL) --mode=link $(LINK.cc) -o $(LIBRARY) $(LOBJ) -rpath $(libdir) -version-info $(EXIV2_LTVERSION) $(LIBS)
	@touch lib

$(BINARY): %: %.o lib
	@$(LIBTOOL) --mode=link $(LINK.cc) -o $@ $(LIBRARY) $@.o -rpath $(libdir)

$(EXIV2EXE): lib $(EXIV2OBJ) $(EXIVCOBJ)
	mkdir -pv ../bin 2>&1 > /dev/null
	@$(LIBTOOL) --mode=link $(LINK.cc) -o ../bin/$@ $(LIBRARY) $(EXIV2OBJ) $(EXIVCOBJ) -rpath $(libdir)

install-header:
	$(INSTALL_DIRS) $(DESTDIR)$(incdir)
	@list='$(HDR)'; for p in $$list; do \
	    if test -f ../include/exiv2/$$p; then \
	        echo "$(INSTALL_DATA) ../include/exiv2/$$p $(DESTDIR)$(incdir)/$$p"; \
	              $(INSTALL_DATA) ../include/exiv2/$$p $(DESTDIR)$(incdir)/$$p; \
	    else :; fi; \
	done ; \

install-lib: lib install-header
	$(INSTALL_DIRS) $(DESTDIR)$(libdir)
	@$(LIBTOOL) --mode=install $(INSTALL_DATA) $(LIBRARY) $(DESTDIR)$(libdir)/$(LIBRARY)
	$(INSTALL_DIRS) $(DESTDIR)$(bindir)
	$(INSTALL_DIRS) $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL_DATA) $(top_srcdir)/config/exiv2.pc $(DESTDIR)$(libdir)/pkgconfig/exiv2.pc

install: $(EXIV2EXE) install-lib
	$(INSTALL_DIRS) $(DESTDIR)$(bindir)
	@$(LIBTOOL) --mode=install $(INSTALL_PROGRAM) ../bin/$(EXIV2EXE) $(DESTDIR)$(bindir)/$(EXIV2EXE)
	$(INSTALL_DIRS) $(DESTDIR)$(mandir)
	$(INSTALL_DIRS) $(DESTDIR)$(man1dir)
	$(INSTALL_DATA) exiv2.1 $(DESTDIR)$(man1dir)/exiv2.1
	$(INSTALL_DATA) $(top_srcdir)/samples/exiv2samples.1 $(DESTDIR)$(man1dir)/exiv2samples.1

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
	$(RM) $(DESTDIR)$(man1dir)/exiv2.1 $(DESTDIR)$(man1dir)/exiv2samples.1
	-rmdir $(DESTDIR)$(man1dir)
	-rmdir $(DESTDIR)$(mandir)
	@$(LIBTOOL) --mode=uninstall $(RM) $(DESTDIR)$(bindir)/$(EXIV2EXE)
	-rmdir $(DESTDIR)$(bindir)

ctags:
	ebrowse $(HDR) $(SRC)
#	ctags-exuberant --extra=+q -e *
#	ctags-exuberant --extra=+q *

check:
	@echo "No checks available for this library - or $ make tests"

mostlyclean:
	$(RM) core
	$(RM) $(CCSRC:.cpp=.ii)
	$(RM) lib
	$(RM) -rf $(CCSRC:%.cpp=.libs/%.d) $(CSRC:%.c=.libs/%.d) .libs/*.dSYM
	@$(LIBTOOL) --mode=clean $(RM) $(LOBJ) $(sort $(EXIV2OBJ) $(EXIV2COBJ))

clean: mostlyclean
	@$(LIBTOOL) --mode=clean $(RM) $(LIBRARY)
	@$(LIBTOOL) --mode=clean $(RM) $(EXECUTABLE) $(EXIV2EXE)

# Run `make distclean' from the top source directory to also remove
# files created by configuring the program.
distclean: clean
	$(RM) doxygen.hpp
ifdef DEP_TRACKING
	$(RM) $(DEP)
	-rmdir $(DEPDIR)
endif
	$(RM) tags TAGS
	$(RM) *~ *.bak *#

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: uninstall distclean
	-$(RM) -r .deps
	-$(RM) -r .libs

# That's all Folks!
##
