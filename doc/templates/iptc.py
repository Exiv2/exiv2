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
    print(f"   <ROW num=\"{int(row)}\">")
    print(f"      <tagname>{line[0]}</tagname>")
    print(f"      <tagdec>{line[1]}</tagdec>")
    print(f"      <taghex>{line[2]}</taghex>")
    print(f"      <recname>{line[3]}</recname>")
    print(f"      <mandatory>{line[4]}</mandatory>")
    print(f"      <repeatable>{line[5]}</repeatable>")
    print(f"      <minbytes>{line[6]}</minbytes>")
    print(f"      <maxbytes>{line[7]}</maxbytes>")
    print(f"      <key>{line[8]}</key>")
    print(f"      <type>{line[9]}</type>")
    print(f"      <tagdesc>{line[10]}</tagdesc>")
    print("   </ROW>")

print("</ROWSET>")
print("</TAGLIST>")

