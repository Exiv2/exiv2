# ************************************************************* -*- Makefile -*-
#
# File:      Makefile
# Version:   $Rev$
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   28-May-05, ahu: created
#
# Description:
#  Simple Makefile to create html documentation from templates. Requires a
#  number of special tools (java, xalan, awk, python) but really only needs 
#  to be used to update the documentation after changing Exiv2 tags in the 
#  source code.
#
# Restrictions:
#  Only tested with GNU make.
#

TABLES = Exif Canon CanonCs1 CanonCs2 CanonCf Fujifilm Nikon1 Nikon2 Nikon3 \
         Olympus Sigma Sony

TAGLIST = ../../src/taglist

# **********************************************************************
# ======================================================================
# **********************************************************************

# Initialisations
SHELL = /bin/sh

.SUFFIXES:

# Default make target
all: tags

.PHONY: tags mostlyclean clean distclean maintainer-clean

tags: $(TABLES) Iptc
	@./gen.py *.html.in

$(TABLES):
	@echo Generating $@ table...
	@$(TAGLIST) $@ | awk -f tags.awk > $@.xml
	@java org.apache.xalan.xslt.Process -IN $@.xml -XSL tags.xsl -OUT $@.tmp
	@sed "s/report1/$@/" $@.tmp > __$@__
	@touch $@
	@rm -f $@.tmp

Iptc:
	@echo Generating $@ table...
	@$(TAGLIST) $@ | awk -f iptc.awk > $@.xml
	@java org.apache.xalan.xslt.Process -IN $@.xml -XSL iptc.xsl -OUT $@.tmp
	@sed "s/report1/$@/g" $@.tmp > __$@__
	@touch $@
	@rm -f $@.tmp

mostlyclean:
	rm -f *.xml
	rm -f *.tmp

clean: mostlyclean
	rm -f $(TABLES:%=__%__) __Iptc__
	rm -f $(TABLES) Iptc
	rm -f *.html

distclean: clean
	rm -f *~

maintainer-clean: distclean
