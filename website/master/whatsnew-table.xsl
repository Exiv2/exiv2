<?xml version="1.0" encoding="iso-8859-1" ?>                   <!--*- sgml -*-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" />

<!-- *********************************************************************** -->
<xsl:template match="news">
 <table width="100%" border="0" cellpadding="4" cellspacing="0">
  <colgroup>
   <col width="90" />
   <col />
  </colgroup>
  <tbody>
   <xsl:apply-templates select="newsitem" />
  </tbody>
 </table>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="newsitem[position() mod 2 = 0]">
 <xsl:call-template name="data-row">
  <xsl:with-param name="rowClass" select="'evencolor'" />
 </xsl:call-template>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="newsitem[position() mod 2 = 1]">
 <xsl:call-template name="data-row">
  <xsl:with-param name="rowClass" select="'oddcolor'" />
 </xsl:call-template>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="desc">
  <xsl:copy-of select="text()|*" />
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="data-row">
 <xsl:param name="rowClass" />
 <tr><xsl:attribute name="class"><xsl:value-of select="$rowClass" /></xsl:attribute>
  <td valign="top"><xsl:value-of select="date" /></td>
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
</xsl:stylesheet>
