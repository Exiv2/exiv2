# ***************************************************** -*- Makefile -*-
#
# File:      Makefile
# Version:   $Rev$
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   15-Jan-04, ahu: created
#
# Description:
#  This makefile mainly forwards to makefiles in subdirectories.
#
# Restrictions:
#  Requires GNU make.
#

.PHONY: all doc clean distclean maintainer-clean

all:
	cd src && $(MAKE) $(MAKECMDGOALS)
	cd doc && $(MAKE) $(MAKECMDGOALS)

doc:
	cd doc && $(MAKE) $(MAKECMDGOALS)

clean:
	cd src && $(MAKE) $(MAKECMDGOALS)
	cd doc && $(MAKE) $(MAKECMDGOALS)

# `make distclean' also removes files created by configuring 
# the program. Running `make all distclean' prepares the project 
# for packaging.
distclean:
	cd src && $(MAKE) $(MAKECMDGOALS)
	cd doc && $(MAKE) $(MAKECMDGOALS)
	rm -f config.h config.mk config.log config.status
	rm -f configure.scan autoscan.log
	rm -rf autom4te.cache/
	rm -f *~ *.bak *#

# This removes almost everything, including the configure script!
maintainer-clean: distclean
	rm -f configure

# Catch-all
%:
	cd src && $(MAKE) $(MAKECMDGOALS)
