# ************************************************************* -*- Makefile -*-
#
# Copyright (C) 2004-2008 Andreas Huggel <ahuggel@gmx.net>
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
# Version:   $Rev$
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   26-Dec-04, ahu: created
#
# Description:
#  Simple makefile to run the Exiv2 unit tests
#
# Restrictions:
#  Only tested with GNU make.
#

# Default make target
all: test

top_srcdir = ..

# Initialisations
SHELL = /bin/sh

.SUFFIXES:

# ******************************************************************************
# Targets
.PHONY: all test clean distclean maintainer-clean

# Add test drivers to this list
TESTS = addmoddel.sh bugfixes-test.sh crw-test.sh exifdata-test.sh \
	exiv2-test.sh ifd-test.sh imagetest.sh iotest.sh iptctest.sh \
	makernote-test.sh modify-test.sh path-test.sh stringto-test.sh \
        write-test.sh write2-test.sh xmpparser-test.sh conversions.sh

test:
	@list='$(TESTS)'; for p in $$list; do \
	    echo Running $$p ...; \
	    ./$$p; \
	done

clean:
	rm -rf $(top_srcdir)/test/tmp/*

distclean: clean
	$(RM) *~ *.bak *#

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: distclean
