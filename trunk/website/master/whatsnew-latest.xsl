<?xml version="1.0" encoding="iso-8859-1" ?>                   <!--*- sgml -*-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" />

<!-- *********************************************************************** -->
<xsl:template match="news">
 <div id="latest">
  <h2>Latest News</h2>
  <ul id="news">
   <xsl:apply-templates select="newsitem[position()&lt;5]" />
   <xsl:if test="newsitem[last()>4]">
    <li>
     <span class="newstitle"><a href="whatsnew.html#item5">More news...</a></span>
    </li>
   </xsl:if>
  </ul>
 </div>
 <xsl:comment> closes latest </xsl:comment>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="newsitem">
 <li>
  <a>
   <xsl:attribute name="href">
    <xsl:value-of select="concat('whatsnew.html#item', position())" />
   </xsl:attribute>
    <span class="newstitle">
     <xsl:value-of select="title" />
    </span>
    <br />
    <span class="newsabstract">
    <xsl:value-of select="abstract" />
    </span>
  </a>
 </li>
</xsl:template>

<!-- *********************************************************************** -->
</xsl:stylesheet>
