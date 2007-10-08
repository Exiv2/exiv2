# ************************************************************* -*- Makefile -*-
#
# Copyright (C) 2004-2007 Andreas Huggel <ahuggel@gmx.net>
#
# This Makefile is part of the Exiv2 distribution.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#    1. Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#    2. Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#    3. The name of the author may not be used to endorse or promote
#       products derived from this software without specific prior
#       written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
# IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# File:      Makefile
# Version:   $Rev$
# Author(s): Andreas Huggel (ahu) <ahuggel@gmx.net>
# History:   28-May-05, ahu: created
#
# Description:
#  Simple Makefile to create html documentation from templates. Requires
#  some special tools (awk, python, xsltproc) but really only needs to 
#  be used to update the documentation after changing Exiv2 tags in the
#  source code.
#
# Restrictions:
#  Only tested with GNU make.
#

TABLES = Exif \
         Canon CanonCs CanonSi CanonCf CanonPi CanonPa \
         Fujifilm \
         Minolta MinoltaCsNew MinoltaCs5D MinoltaCs7D \
         Nikon1 Nikon2 Nikon3 \
         Olympus \
	 Panasonic \
         Pentax \
         Sigma \
         Sony

TAGLIST = ../../samples/taglist

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
	@$(TAGLIST) $@ | sed -e"s/</\&lt;/g" -e"s/>/\&gt;/g" | awk -f tags.awk > $@.xml
	@xsltproc tags.xsl $@.xml > $@.tmp
	@sed "s/report1/$@/" $@.tmp > __$@__
	@touch $@
	@rm -f $@.tmp

Iptc: $(TAGLIST)
	@echo Generating $@ table...
	@$(TAGLIST) $@ | sed -e"s/</\&lt;/g" -e"s/>/\&gt;/g" -e"s/±/\&#177;/g" \
	               | awk -f iptc.awk > $@.xml
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
