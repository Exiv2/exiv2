# ***************************************************** -*- Makefile -*-
#
# AUTHOR(S): Andreas Huggel (ahu)
#
# RCS information
#  $Name:  $
#  $Revision: 1.1 $
#
# Description:
#  This makefile just forwards to src/Makefile.
#
# Restrictions:
#  Requires GNU make.
#

.PHONY: all maintainer-clean doc

all maintainer-clean:
	cd src && $(MAKE) $(MAKECMDGOALS)
	cd doc && $(MAKE) $(MAKECMDGOALS)

doc:
	cd doc && $(MAKE) $(MAKECMDGOALS)

%:
	cd src && $(MAKE) $(MAKECMDGOALS)
