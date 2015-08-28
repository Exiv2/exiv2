<?xml version="1.0" encoding="iso-8859-1" ?>                   <!--*- sgml -*-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" />

<!-- *********************************************************************** -->
<xsl:template match="TAGLIST">
  <!-- content generated from XML -->
  <xsl:call-template name="table" />
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="HEADER/text">
  <xsl:copy-of select="text()|*" />
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="header">
  <h2><xsl:value-of select="HEADER/title" /></h2>
  <xsl:apply-templates select="HEADER/text" />
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="table">
  <table class="table table-striped">
    <thead>
      <xsl:call-template name="header-row" />
    </thead>
    <tbody>
      <xsl:apply-templates select="ROWSET/ROW" />
    </tbody>
  </table>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="header-row">
  <tr>
    <th>Property</th>
    <th>Label</th>
    <th>Value&#160;type</th>
    <th>Exiv2&#160;type</th>
    <th>Category</th>
    <th>Description</th>
  </tr>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="ROWSET/ROW">
  <xsl:call-template name="data-row" />
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="data-row">
  <tr>
    <td><xsl:value-of select="tagname" /></td>
    <td><xsl:value-of select="title" /></td>
    <td><xsl:value-of select="xmpvaltype" /></td>
    <td><xsl:value-of select="type" /></td>
    <td><xsl:value-of select="category" /></td>
    <td><xsl:value-of select="tagdesc" /></td>
  </tr>
</xsl:template>

<!-- *********************************************************************** -->
</xsl:stylesheet>
