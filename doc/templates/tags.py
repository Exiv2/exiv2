#!/usr/bin/env python3

import sys
import csv

print("""<?xml version = '1.0'?>
<?xml-stylesheet type=\"text/xsl\" href=\"tags.xsl\"?>
<TAGLIST>
<HEADER>
<title>XYZ MakerNote Tags defined in Exiv2</title>
<text>
<p>Tags found in the MakerNote of images taken with XYZ cameras. These tags 
are defined by Exiv2 in accordance with <a href=\"makernote.html#RX\">[X]</a>.</p>
<p>Click on a column header to sort the table.</p>
</text>
</HEADER>
<ROWSET>""")

row=0
data = sys.stdin.readlines()
for line in csv.reader(data,quotechar='"',skipinitialspace=True):
    row=row+1
    print(f"   <ROW num=\"{int(row)}\">")
    print(f"      <tagname>{line[0]}</tagname>")
    print(f"      <tagdec>{line[1]}</tagdec>")
    print(f"      <taghex>{line[2]}</taghex>")
    print(f"      <ifd>{line[3]}</ifd>")
    print(f"      <key>{line[4]}</key>")
    print(f"      <type>{line[5]}</type>")
    print(f"      <tagdesc>{line[6]}</tagdesc>")
    print("   </ROW>")

print("</ROWSET>")
print("</TAGLIST>")

