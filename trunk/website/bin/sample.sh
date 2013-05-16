#! /bin/sh
# Generate variables used for the sample.html page
# The appropriate exiv2 must be built

exiv2=$EXIV2HOME/bin/exiv2
vp=../../var

cd html/include/ >/dev/null 2>&1
$exiv2 img_1771.jpg > $vp/__sample-p__
$exiv2 -pt img_1771.jpg > $vp/__sample-pt__
$exiv2 -h > $vp/sample-h.tmp
sed -e 's/</\&lt;/g' -e 's/>/\&gt;/g' $vp/sample-h.tmp > $vp/__sample-h__
rm -f $vp/sample-h.tmp
