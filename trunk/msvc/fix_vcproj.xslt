<xsl:stylesheet version="1.0"
 xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
 <xsl:output omit-xml-declaration="yes" indent="yes"/>
 <xsl:strip-space elements="*"/>

 <!--
   This script is used to fix <File>white space</File> to <File></File> in Visual Studio project files
   for i in $(finder "*.vcproj"); do xsltproc foo.xslt $i > /tmp/foo.vcproj ; cp /tmp/foo.vcproj $i; done
 -->

 <xsl:template match="node()|@*">
     <xsl:copy>
       <xsl:apply-templates select="node()|@*"/>
     </xsl:copy>
 </xsl:template>

 <xsl:template match=
    "*[not(@*|*|comment()|processing-instruction()) 
     and normalize-space()=''
      ]"/>
</xsl:stylesheet>

