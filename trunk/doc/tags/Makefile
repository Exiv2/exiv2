# ************************************************************* -*- Makefile -*-
#
# File:      Makefile
# Version:   $Rev$
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   28-May-05, ahu: created
#
# Description:
#  Simple Makefile to create the tag tables
#
# Restrictions:
#  Only tested with GNU make.
#

TABLES = Exif Canon CanonCs1 CanonCs2 CanonCf Fujifilm Nikon1 Nikon2 Nikon3 \
         Olympus Sigma Sony

TAGLIST = ../../src/taglist

# Default make target
all: tags

.PHONY: clean $(TABLES) Iptc

tags: $(TABLES) Iptc
	@./gen.py tags-*.html.in

$(TABLES):
	@echo Generating $@ table...
	@$(TAGLIST) $@ | awk -f tags.awk > $@.xml
	@java org.apache.xalan.xslt.Process -IN $@.xml -XSL tags.xsl -OUT $@.tmp
	@sed "s/report-1/$@/" $@.tmp > __$@__
	@rm -f $@.tmp

Iptc:
	@echo Generating $@ table...
	@$(TAGLIST) $@ | awk -f iptc.awk > $@.xml
	@java org.apache.xalan.xslt.Process -IN $@.xml -XSL iptc.xsl -OUT $@.tmp
	@sed "s/report-1/$@/" $@.tmp > __$@__
	@rm -f $@.tmp

mostlyclean:
	rm -f *.xml
	rm -f *.tmp

clean: mostlyclean
	rm -f $(TABLES:%=__%__) __Iptc__
	rm -f tags-*.html

distclean: clean
	rm -f *~
