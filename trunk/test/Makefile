# ************************************************************* -*- Makefile -*-

# Default make target
all: test

top_srcdir = ..

# Initialisations
SHELL = /bin/sh

.SUFFIXES:

# **********************************************************************
# Targets
.PHONY: all test clean distclean maintainer-clean

test:
	@echo Running addmoddel...
	@./addmoddel.sh
	@echo Running exiv2-test...
	@./exiv2-test.sh
	@echo Running ifd-test...
	@./ifd-test.sh
	@echo Running imagetest...
	@./imagetest.sh
	@echo Running iotest...
	@./iotest.sh
	@echo Running iptctest...
	@./iptctest.sh
	@echo Running makernote-test...
	@./makernote-test.sh
	@echo Running write-test...
	@./write-test.sh
	@echo Running write2-test...
	@./write2-test.sh

clean:
	rm -rf $(top_srcdir)/test/tmp/*

distclean: clean
	$(RM) *~ *.bak *#

# This command is intended for maintainers to use; it deletes files
# that may need special tools to rebuild.
maintainer-clean: distclean
