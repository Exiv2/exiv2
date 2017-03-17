TABLES = Exif \
         Canon \
         CanonCs \
         CanonSi \
         CanonCf \
         CanonPi \
         CanonFi \
         CanonPa \
         Casio \
         Casio2 \
         Fujifilm \
         Minolta \
         MinoltaCsNew \
         MinoltaCs5D \
         MinoltaCs7D \
         Nikon1 \
         Nikon2 \
         Nikon3 \
         NikonVr \
         NikonPc \
         NikonWt \
         NikonIi \
         NikonAf \
         NikonAf2 \
         NikonAFT \
         NikonFi \
         NikonMe \
         NikonFl1 \
         NikonFl2 \
         NikonFl3 \
         NikonSiD80 \
         NikonSiD40 \
         NikonSiD300a \
         NikonSiD300b \
         NikonSi01xx \
         NikonCb1 \
         NikonCb2 \
         NikonCb2a \
         NikonCb2b \
         NikonCb3 \
         NikonCb4 \
         NikonLd1 \
         NikonLd2 \
         NikonLd3 \
         Olympus \
         OlympusCs \
         OlympusEq \
         OlympusRd \
         OlympusRd2 \
         OlympusIp \
         OlympusFi \
         OlympusFe1 \
         OlympusRi \
	 Panasonic \
         PanasonicRaw \
         Pentax \
         Samsung2 \
         Sigma \
         Sony1 \
         SonyMinolta \
         Sony1Cs \
         Sony1Cs2 \
         Sony1MltCs7D \
         Sony1MltCsOld \
         Sony1MltCsA100

SCHEMA = dc \
         xmp \
         xmpRights \
         xmpMM \
         xmpBJ \
         xmpTPg \
         xmpDM \
         pdf \
         photoshop \
         crs \
         crss \
         tiff \
         exif \
         exifEX \
         aux \
         iptc \
         iptcExt \
         plus \
         mwg-rs \
         mwg-kw \
         dwc \
         dcterms \
         digiKam \
         kipi \
         GPano \
         lr \
         MP \
         MPRI \
         MPReg \
         acdsee \
         mediapro \
         expressionmedia \
         MicrosoftPhoto

all: html doc

.PHONY: html doc news clean update \
        update-intro update-sample update-conversion update-changelog update-tags update-makernotes \
        publish really-publish

html: news
	@bin/download.sh
	@bin/gen.py master/*.in
	@echo "Moving files to html/ ..."
	-@mv -f master/*.html html/

news: var/__whatsnew-latest__ var/__whatsnew-table__ html/rss.xml

doc: html
	@echo "Generating API documentation ... "
        # Preprocessing
	@bin/gen.py doxygen/*.in
	@sed -e 's,="dist/,="../dist/,' \
             -e 's,="include/,="../include/,' \
	    doxygen/exiv2-header.html > ttt
	@mv -f ttt doxygen/exiv2-header.html
	@sed -e 's,="dist/,="../dist/,' \
             -e 's,="include/,="../include/,' \
	    doxygen/exiv2-footer.html > ttt
	@mv -f ttt doxygen/exiv2-footer.html
        # API doc generation
	doxygen doxygen/Doxyfile
        # Postprocessing
	-@cp -f doxygen/doxy-boot.js html/doc/
        # Cleanup
	-@rm -f html/doc/namespacemembers*.html
	-@rm -f html/doc/functions*.html

var/__whatsnew-table__: master/news.xml
	@echo "Generating news table ..."
	@xsltproc master/whatsnew-table.xsl $< > $@

var/__whatsnew-latest__: master/news.xml
	@echo "Generating latest news ..."
	@xsltproc master/whatsnew-latest.xsl $< > $@

html/rss.xml: master/news.xml
	@echo "Generating RSS feed ..."
	@xsltproc master/whatsnew-rss.xsl $< \
	| awk '/pubDate/ { gsub("<[/]?pubDate>", ""); printf "      <pubDate>"; system("bin/rssdate " $$0); printf "</pubDate>\n"; next } /__lastBuildDate__/ { printf "    <lastBuildDate>"; system("date -R"); printf "    </lastBuildDate>\n"; next; } {print}' > $@

# Run bin/release_info.sh to update release related variables (incl __version__)
update: update-manpage update-intro update-sample update-conversion update-changelog update-tags update-makernotes

update-manpage:
	@man -t exiv2 | ps2pdf - html/exiv2-manpage.pdf
	@cat ../src/exiv2.1 | sed '/^JPEG/ i\
' | man2html -Hlinux.die.net -M/man -p | sed -e's,\(HREF=.*\)+,\1/,' -e'1,15d' -e'/NAME="index"/,$$d' \
	| sed -e's/ &lt;<A HREF="mailto.*&gt;//' -e'$$d' | sed -e's/<TABLE>/<TABLE class="table table-striped">/' -e's,<TR VALIGN=top><TD><HR></TD><TD><HR></TD><TD><HR></TD><TD><HR></TD><TD><HR></TD></TR>,,' -e's/<TR VALIGN=top>/<TR>/' -e's,<TR><TD>\([A-Z0-9]*\)</TD>,<TR><TH>\1</TH>,' -e's/<H2>/<H2 class="text-capitalize">/' > var/__manpage__

update-intro:
# commented for now, until the doc/ intro is updated accordingly
#	@cp -f ../doc/templates/__intro__ doxygen/draft-intro.html
#	@sed -e 's,="makernote.html,="../makernote.html,' \
#	    doxygen/draft-intro.html > doxygen/intro.html
#	@rm -f doxygen/draft-intro.html

update-sample:
	@./bin/sample.sh

update-conversion:
	@./bin/conversion.sh

update-changelog:
	echo changelog should be updated using team/contrib/redmine/progress.py
	# @cp -f ../doc/ChangeLog var/__ChangeLog__
	# @bin/buglinks.sh var/__ChangeLog__

update-makernotes:
	@cp -f ../doc/templates/__makernotes__ var/__makernotes__

$(TABLES):
	@cp -f ../doc/templates/$@.xml var/
	@xsltproc var/exif.xsl var/$@.xml > var/__$@__

Iptc:
	@cp -f ../doc/templates/$@.xml var/
	@xsltproc var/iptc.xsl var/$@.xml > var/__$@__

$(SCHEMA):
	@cp -f ../doc/templates/xmp_$@.xml var/
	@xsltproc var/xmp.xsl var/xmp_$@.xml > var/__xmp_$@__

update-tags: $(TABLES) Iptc $(SCHEMA)

clean:
	@rm -vf html/*.html
	@rm -vf html/rss.xml
	@rm -rf html/doc
	@rm -vf var/__whatsnew-latest__
	@rm -vf var/__whatsnew-table__
	@rm -vf doxygen/mainpage.hpp
	@rm -vf doxygen/exiv2-header.html
	@rm -vf doxygen/exiv2-footer.html

publish:
	rsync -vlDtrz --delete --exclude='.svn' --include="exiv2-`cat var/__version__`*" --exclude='exiv2-*tar.gz' --exclude='exiv2-*.zip' -e ssh html/ `whoami`@localhost:rs

really-publish:
	rsync -vlDtrz --delete --exclude='misc/' --exclude='.svn' --include="exiv2-`cat var/__version__`*" --exclude='exiv2-*tar.gz' --exclude='exiv2-*.zip' -e "ssh -2 -i $$HOME/.ssh/root@exiv2.org" html/ root@exiv2.org:/var/www/exiv2/
