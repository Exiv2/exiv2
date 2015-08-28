<?xml version="1.0" encoding="iso-8859-1" ?>                   <!--*- sgml -*-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" />

<!-- *********************************************************************** -->
<xsl:template match="news">
 <table class="table table-striped">
   <xsl:apply-templates select="newsitem" />
 </table>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="newsitem">
 <tr>
  <td class="text-nowrap"><xsl:value-of select="date" /></td>
  <td>
   <b>
    <a><xsl:attribute name="name">item<xsl:value-of select="position()" /></xsl:attribute>
     <xsl:value-of select="title" />
    </a>
   </b>
   <br />
   <xsl:apply-templates select="desc" />
  </td>
 </tr>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="desc">
  <xsl:copy-of select="text()|*" />
</xsl:template>

<!-- *********************************************************************** -->
</xsl:stylesheet>
