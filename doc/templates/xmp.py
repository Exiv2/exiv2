#!/usr/bin/env python3

import sys
import csv

print("""<?xml version = '1.0'?>
<?xml-stylesheet type=\"text/xsl\" href=\"tags.xsl\"?>
<TAGLIST>
<HEADER>
<title>XMP tags defined in Exiv2</title>
<text>
<p>Some description</p>
<p>Click on a column header to sort the table.</p>
</text>
</HEADER>
<ROWSET>""")

row=0
data = sys.stdin.readlines()
print(data)
for line in csv.reader(data,quotechar='"',skipinitialspace=True):
    row=row+1
    print("   <ROW num=\"%d\">" % row) 
    print("      <tagname>"      + line[0] + "</tagname>")
    print("      <title>"        + line[1] + "</title>")
    print("      <xmpvaltype>"   + line[2] + "</xmpvaltype>")
    print("      <type>"         + line[3] + "</type>")
    print("      <category>"     + line[4] + "</category>")
    print("      <tagdesc>"      + line[5] + "</tagdesc>")
    print("   </ROW>")

print("</ROWSET>")
print("</TAGLIST>")

