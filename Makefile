# ***************************************************** -*- Makefile -*-
#
# File:      Makefile
# Version:   $Rev$
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   15-Jan-04, ahu: created
#
# Description:
#  Simple makefile that mainly forwards to makefiles in subdirectories.
#
# Restrictions:
#  Requires GNU make.
#

.PHONY: all install uninstall doc config mostlyclean clean distclean maintainer-clean

all install uninstall: config/config.mk
	cd src && $(MAKE) $(MAKECMDGOALS)

doc: config/config.mk
	cd doc && $(MAKE) $(MAKECMDGOALS)

config:
	cd config && $(MAKE) $(MAKECMDGOALS)

mostlyclean clean: config/config.mk
	cd src && $(MAKE) $(MAKECMDGOALS)
	cd doc && $(MAKE) $(MAKECMDGOALS)
	cd config && $(MAKE) $(MAKECMDGOALS)

# `make distclean' also removes files created by configuring 
# the program. Running `make all distclean' prepares the project 
# for packaging.
distclean: clean
	rm -f config.log config.status libtool
	rm -f *~ *.bak *#

# This removes almost everything, including the configure script!
maintainer-clean: distclean
	rm -f configure

config/config.mk: 
	$(error File config/config.mk does not exist. Did you run ./configure?)
