<?xml version="1.0" encoding="iso-8859-1" ?>                   <!--*- sgml -*-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="xml" indent="yes" />

<!-- *********************************************************************** -->
<xsl:template match="news">
  <rss version="2.0" xmlns:atom="http://www.w3.org/2005/Atom">
  <channel>
   <title>What's new in Exiv2</title>
   <link>http://www.exiv2.org/</link>
   <description>News from the Exiv2 project.</description>
   <lastBuildDate>__lastBuildDate__</lastBuildDate>
   <language>en-us</language>
   <atom:link href="http://www.exiv2.org/rss.xml" rel="self" type="application/rss+xml" />
   <xsl:apply-templates select="newsitem" />
  </channel>
 </rss>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="newsitem">
 <item>
  <title><xsl:value-of select="title" /></title>
  <link>http://exiv2.org/whatsnew.html#item<xsl:value-of select="position()" /></link>
  <guid>http://exiv2.org/whatsnew.html#item<xsl:value-of select="position()" /></guid>
  <pubDate><xsl:value-of select="date" /></pubDate>
  <description><xsl:value-of select="abstract" /></description>
 </item>
</xsl:template>

<!-- *********************************************************************** -->
</xsl:stylesheet>
