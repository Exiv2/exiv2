<?xml version="1.0" encoding="iso-8859-1" ?>                   <!--*- sgml -*-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" />

<!-- *********************************************************************** -->
<xsl:template match="news">
  <div class="list-group">
    <xsl:apply-templates select="newsitem[position()&lt;5]" />
  </div>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="newsitem">
 <a>
  <xsl:attribute name="href">
   <xsl:value-of select="concat('whatsnew.html#item', position())" />
  </xsl:attribute>
  <xsl:attribute name="class">
  <xsl:choose>
   <xsl:when test="position() &lt; 2">
    <xsl:value-of select="'list-group-item active'" />
   </xsl:when>
   <xsl:otherwise>
    <xsl:value-of select="'list-group-item'" />
   </xsl:otherwise>
  </xsl:choose>
  </xsl:attribute>
  <h4 class="list-group-item-heading"> 
   <xsl:value-of select="concat(title, ' ')" />
   <small>
    <xsl:value-of select="date" />
   </small>
  </h4>
  <p class="list-group-item-text">
   <xsl:value-of select="abstract" />
  </p>
 </a>
</xsl:template>

<!-- *********************************************************************** -->
</xsl:stylesheet>
