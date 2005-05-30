# ************************************************************* -*- Makefile -*-
#
# File:      Makefile
# Version:   $Rev$
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   28-May-05, ahu: created
#
# Description:
#  Simple Makefile to create html documentation from templates. Requires some
#  special tools (awk, python, xsltproc) but really only needs to be used to 
#  update the documentation after changing Exiv2 tags in the source code.
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

$(TABLES): $(TAGLIST)
	@echo Generating $@ table...
	@$(TAGLIST) $@ | awk -f tags.awk > $@.xml
	@xsltproc tags.xsl $@.xml > $@.tmp
	@sed "s/report1/$@/" $@.tmp > __$@__
	@touch $@
	@rm -f $@.tmp

Iptc: $(TAGLIST)
	@echo Generating $@ table...
	@$(TAGLIST) $@ | awk -f iptc.awk > $@.xml
	@xsltproc iptc.xsl $@.xml > $@.tmp
	@sed "s/report1/$@/g" $@.tmp > __$@__
	@touch $@
	@rm -f $@.tmp

$(TAGLIST): 
	$(error File $(TAGLIST) does not exist. Did you build the library and examples?)

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
