# ***************************************************** -*- Makefile -*-
#
# AUTHOR(S): Andreas Huggel (ahu)
#
# RCS information
#  $Name:  $
#  $Revision: 1.3 $
#
# Description:
#  This makefile just forwards to src/Makefile.
#
# Restrictions:
#  Requires GNU make.
#

.PHONY: all maintainer-clean doc

all %:
	cd src && $(MAKE) $(MAKECMDGOALS)

doc:
	cd doc && $(MAKE) $(MAKECMDGOALS)

maintainer-clean:
	cd src && $(MAKE) $(MAKECMDGOALS)
	cd doc && $(MAKE) $(MAKECMDGOALS)
	rm -f *~ *#
	rm -f config.h config.mk config.log configure
	rm -rf autom4te.cache/
