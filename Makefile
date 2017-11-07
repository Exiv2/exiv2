# ***************************************************** -*- Makefile -*-
#
# Copyright (C) 2004-2017 Andreas Huggel <ahuggel@gmx.net>
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
# File:      Makefile.in
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   15-Jan-04, ahu: created
#
# Description:
#  Simple top-level makefile that mainly forwards to makefiles in
#  subdirectories.
#
# Restrictions:
#  Requires GNU make.
#

########################################################################
# Makefile is a generated file. Do NOT change any settings in this file.
# Run ./configure with the appropriate options to regenerate the file
# and possibly others.
########################################################################

SHELL = /bin/sh
ENABLE_XMP = 1

.PHONY: all doc config samples xmpsdk                \
        mostlyclean clean distclean maintainer-clean \
        install uninstall tests teste testv

all install: config/config.mk xmpsdk
	cd src && $(MAKE) $(MAKECMDGOALS)
	cd po && $(MAKE) $(MAKECMDGOALS)

uninstall: config/config.mk
	cd src && $(MAKE) $(MAKECMDGOALS)
	cd po && $(MAKE) $(MAKECMDGOALS)

doc: config/config.mk
	cd doc && $(MAKE) $(MAKECMDGOALS)

# exiv2 application and programs required by test suite
exiv2:
	cd src && $(MAKE) $0

# sample programs required by test suite
conntest exifprint remotetest:
	cd samples && $(MAKE) $@

samples: config/config.mk
	cd samples && $(MAKE) $(MAKECMDGOALS)

# test suite
tests:
	cd test && $(MAKE) test

# test suite sub groups
teste testx testv:
	cd test && $(MAKE) $@

# convenience for running individual tests
bugfixes-test.sh   crw-test.sh     curliotest.sh       eps-preview-test.sh    eps-test.sh       exifdata-test.sh \
exiv2-test.sh      httpiotest.sh   icc-test.sh         imagetest.sh           iotest.sh         iptctest.sh      \
modify-test.sh     path-test.sh    preview-test.sh     sshiotest.sh           stringto-test.sh  tiff-test.sh     \
video-test.sh      write-test.sh   write-video-test.sh write2-test.sh         xmpparser-test.sh                  \
webp-test.sh       version-test.sh:
	cd test && ./$@

# convenience targets for running selected test scripts
bugtest bugstest testbugs bugfixes:
	cd test && ./bugfixes-test.sh

bugfixes-test icc-test preview-test webp-test stdin-test geotag-test http-test:
	cd test && ./$@.sh

# convenience target for building individual sample programs
addmoddel		exifcomment		exifvalue	httptest	iptctest		mmap-test	stringto-test \
exifdata		iotest			key-test	path-test	taglist			write2-test write-test	\
convert-test	exifdata-test	exiv2json	iptceasy	largeiptc-test	prevtest	tiff-test	\
easyaccess-test	geotag			iptcprint	metacopy	mt-test         toexv       werror-test	\
xmpparser-test 	xmpsample 		xmpparse :
	cd samples && $(MAKE) $(MAKECMDGOALS)

MAJOR=$(shell grep "define.*EXIV2_.*_VERSION .*\\d*" src/version.hpp | grep MAJOR | sed -e 's/EXIV2//g' | tr -dC [:digit:])
MINOR=$(shell grep "define.*EXIV2_.*_VERSION .*\\d*" src/version.hpp | grep MINOR | sed -e 's/EXIV2//g' | tr -dC [:digit:])
VERSION=exiv2-$(MAJOR).$(MINOR)
tarball:
	@rm -rf         /tmp/$(VERSION)
	@rm -rf         /tmp/$(VERSION).tar
	@rm -rf         /tmp/$(VERSION).tar.gz
	@rm -rf              $(VERSION).tar.gz
	svn export -q . /tmp/$(VERSION)
	@cd             /tmp/$(VERSION)    ;\
	make config                        ;\
	cd   -  ; cd /tmp                  ;\
	tar cf  $(VERSION).tar $(VERSION)/ ;\
	gzip    $(VERSION).tar             ;\
	cd   -                             ;\
	mv /tmp/$(VERSION).tar.gz .        ;\
	ls -alt $(VERSION).tar.gz

configure:
	make config

config:
	cd config && $(MAKE) -f config.make $(MAKECMDGOALS)

xmpsdk: config/config.mk
	if test "x$(ENABLE_XMP)" =    "x1"; then cd xmpsdk/src && $(MAKE) $@; fi;
	if test "x$(ENABLE_XMP)" = "x2016" -o "x$(ENABLE_XMP)" = "x2014" -o "x$(ENABLE_XMP)" = "x2013"; then xmpsdk/buildXMPsdk.sh $(ENABLE_XMP) $(MAKECMDGOALS); fi;

mostlyclean clean: config/config.mk
	cd src && $(MAKE) $(MAKECMDGOALS)
	cd doc && $(MAKE) $(MAKECMDGOALS)
	cd samples && $(MAKE) $(MAKECMDGOALS)
	cd test && $(MAKE) $(MAKECMDGOALS)
	cd xmpsdk/src && $(MAKE) $(MAKECMDGOALS)
	cd config && $(MAKE) -f config.make $(MAKECMDGOALS)
	cd po && $(MAKE) $(MAKECMDGOALS)
	rm -f include/exiv2/exv_conf.h


# `make distclean' also removes files created by configuring
# the program. Running `make all distclean' prepares the project
# for packaging.
distclean: clean
	rm -f config.log config.status libtool
	rm -f *~ *.bak
	if [ -e bin ]; then rm -rf bin ; fi
	if [ -e xmpsdk/Adobe ]; then rm -rf xmpsdk/Adobe ; fi
	if [ -e /usr/local/include/exiv2 ]; then rm -rf /usr/local/include/exiv2 ; fi

# This removes almost everything, including the configure script!
maintainer-clean: distclean
	rm -f configure
	-cd test && $(MAKE) $(MAKECMDGOALS)

config/config.mk:
	$(error File config/config.mk does not exist. Did you run ./configure?)

rebuild:
	make distclean
	make config
	./configure
	make -j
	sudo make install
	make -j samples

# That's all Folks!
##
