# ***************************************************** -*- Makefile -*-
#
# AUTHOR(S): Andreas Huggel (ahu)
#
# RCS information
#  $Name:  $
#  $Revision: 1.2 $
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
