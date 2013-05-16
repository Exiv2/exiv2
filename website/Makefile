TABLES = Exif \
         Canon \
         CanonCs \
         CanonSi \
         CanonCf \
         CanonPi \
         CanonFi \
         CanonPa \
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
         Sony1MltCsA100 \
         Iptc

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
         tiff \
         exif \
         aux \
         iptc \
         iptcExt \
         MicrosoftPhoto \
         digiKam \
         kipi \
         plus \
         mediapro \
         expressionmedia \
         MP \
         MPRI \
         MPReg \
         mwg-rs

all: html doc

.PHONY: html doc news clean update update-intro update-sample update-changelog \
        update-makernotes publish really-publish

html: news
	@bin/gen.py master/*.in
	@echo "Moving files to html/ ..."
	-@mv -f master/*.html master/*.php html/

news: var/__whatsnew-latest__ var/__whatsnew-table__ html/rss.xml

doc: html
	@echo "Generating API documentation ... "
        # Preprocessing
	@sed -e 's,="include/,="../include/,' \
	    -e 's,="whatsnew.html#,="../whatsnew.html#,' \
	    html/draft-exiv2-header.html > html/exiv2-header.html
	@sed 's,="include/,="../include/,' \
	    html/draft-exiv2-footer.html > html/exiv2-footer.html
	@bin/gen.py doxygen/mainpage.hpp.in
        # API doc generation
	doxygen-1.5.4 doxygen/Doxyfile
        # Postprocessing
	@sed 's,<h1>Exiv2 Class Index</h1>,<h1>Image metadata library and tools</h1><h2>Exiv2 Class index</h2>,' html/doc/classes.html > html/doc/ttt
	@mv -f html/doc/ttt html/doc/classes.html
	@sed 's,<h1>Exiv2 Namespace List</h1>,<h1>Image metadata library and tools</h1><h2>Exiv2 Namespaces</h2>,' html/doc/namespaces.html > html/doc/ttt
	@mv -f html/doc/ttt html/doc/namespaces.html
	@sed 's,<h1>Exiv2 File List</h1>,<h1>Image metadata library and tools</h1><h2>Exiv2 File list</h2>,' html/doc/files.html > html/doc/ttt
	@mv -f html/doc/ttt html/doc/files.html
	@sed 's,<h1>Exiv2 Examples</h1>,<h1>Image metadata library and tools</h1><h2>Exiv2 Examples</h2>,' html/doc/examples.html > html/doc/ttt
	@mv -f html/doc/ttt html/doc/examples.html
	-@cp -f doxygen/doxygen.css html/doc
        # Cleanup
	-@rm -f html/exiv2-header.html
	-@rm -f html/exiv2-footer.html
	-@rm -f html/draft-exiv2-header.html
	-@rm -f html/draft-exiv2-footer.html
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
update: update-manpage update-intro update-sample update-changelog update-tags update-makernotes

update-manpage:
	@man -t exiv2 | ps2pdf - html/exiv2-manpage.pdf
	@cat ../src/exiv2.1 | sed '/^JPEG/ i\
' | man2html -Hlinux.die.net -M/man -p | sed -e's,\(HREF=.*\)+,\1/,' -e'1,15d' -e'/NAME="index"/,$$d' \
	| sed -e's/ &lt;<A HREF="mailto.*&gt;//' -e'$$d' > var/__manpage__

update-intro:
	@cp -f ../doc/templates/__intro__ doxygen/draft-intro.html
	@sed -e 's,="makernote.html,="../makernote.html,' \
	     -e 's,<a href="mailto:ahuggel@gmx.net"> contact me</a>,contact me,' \
	    doxygen/draft-intro.html > doxygen/intro.html
	@rm -f doxygen/draft-intro.html

update-sample:
	@./bin/sample.sh

update-changelog:
	@cp -f ../doc/ChangeLog var/__ChangeLog__
	@bin/buglinks.sh var/__ChangeLog__

update-makernotes:
	@cp -f ../doc/templates/__makernotes__ var/__makernotes__

$(TABLES):
	@cp -f ../doc/templates/__$@__ var/__$@__

$(SCHEMA):
	@cp -f ../doc/templates/__xmp_$@__ var/__xmp_$@__

update-tags: $(TABLES) $(SCHEMA)

clean:
	@rm -vf html/*.html
	@rm -vf html/rss.xml
	@rm -rf html/doc
	@rm -vf var/__whatsnew-latest__
	@rm -vf var/__whatsnew-table__
	@rm -vf doxygen/mainpage.hpp

publish:
	rsync -vlDtrz --delete --exclude='.svn' -e ssh html/ andreas@localhost:rs

really-publish:
	rsync -vlDtrz --delete --exclude='misc/' --exclude='.svn' -e "ssh -2 -i /home/andreas/.ssh/ahuggel.key" html/ ahuggel@exiv2.org:
