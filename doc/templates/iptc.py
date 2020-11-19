#!/usr/bin/env python3

import sys
import csv

print("""<?xml version = '1.0'?>
<?xml-stylesheet type=\"text/xsl\" href=\"tags.xsl\"?>
<TAGLIST>
<HEADER>
<title>Iptc datasets defined in Exiv2</title>
<text>
<p>Datasets are defined according to the specification of the Iptc 
<a href=\"http://www.iptc.org/IIM/\">Information Interchange Model (IIM)</a>.</p>
<p>Click on a column header to sort the table.</p>
</text>
</HEADER>
<ROWSET>""")

row=0
data = sys.stdin.readlines()
for line in csv.reader(data,quotechar='"',skipinitialspace=True):
    row=row+1
    print("   <ROW num=\"%d\">" % row)
    print("      <tagname>"    + line[ 0] + "</tagname>")
    print("      <tagdec>"     + line[ 1] + "</tagdec>")
    print("      <taghex>"     + line[ 2] + "</taghex>")
    print("      <recname>"    + line[ 3] + "</recname>")
    print("      <mandatory>"  + line[ 4] + "</mandatory>")
    print("      <repeatable>" + line[ 5] + "</repeatable>")
    print("      <minbytes>"   + line[ 6] + "</minbytes>")
    print("      <maxbytes>"   + line[ 7] + "</maxbytes>")
    print("      <key>"        + line[ 8] + "</key>")
    print("      <type>"       + line[ 9] + "</type>")
    print("      <tagdesc>"    + line[10] + "</tagdesc>")
    print("   </ROW>")

print("</ROWSET>")
print("</TAGLIST>")

