# ***************************************************** -*- Makefile -*-
#
# File:      Makefile
# Version:   $Rev$
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   26-Feb-05, ahu: created
#
# Description:
#  Simple makefile to automate config tasks
#
# Restrictions:
#  Requires GNU make.
#

# Default make target
all: config

.PHONY: all config mostlyclean clean distclean maintainer-clean

config:
	autoconf -o ../configure

mostlyclean clean:
	rm -f configure.scan autoscan.log
	rm -rf autom4te.cache/
	rm -f *~ *.bak *#

distclean: clean
	rm -f config.h ../src/exv_conf.h config.mk exiv2-config

# This removes almost everything, including the configure script!
maintainer-clean: distclean
