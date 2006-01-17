# ************************************************************* -*- Makefile -*-

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
	makernote-test.sh modify-test.sh path-test.sh write-test.sh \
	write2-test.sh 

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
