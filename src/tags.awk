################################################################################
#  File    tags.awk
#  Brief   Awk script to convert the taglist from ExifTags::taglist to XML 
#          format used in the documentation.
#          $ taglist | awk -f tags.awk > tags.xml
#  Version $Name:  $ $Revision: 1.1 $
#  Author  Andreas Huggel (ahu)
#          <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
#  Date    07-Feb-04, ahu: created
################################################################################

BEGIN {
   FS = ", "
   print "<?xml version = '1.0'?>";   
   print "<?xml-stylesheet type=\"text/xsl\" href=\"tags.xsl\"?>";
   print "<ROWSET>"
}

{
   print "   <ROW num=\"" ++row "\">";
   print "      <tagname>" $1 "</tagname>"
   print "      <tagdec>" $2 "</tagdec>"
   print "      <taghex>" $3 "</taghex>"
   print "      <ifd>" $4 "</ifd>"
   print "      <key>" $5 "</key>"
   print "      <tagdesc>" $6 "</tagdesc>"
   print "   </ROW>";
}

END {
   print "</ROWSET>"   
}
