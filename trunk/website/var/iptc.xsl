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
    <th>Tag (hex)</th>
    <th>Tag (dec)</th>
    <th>Key</th>
    <th>Type</th>
    <th><abbr title="Mandatory">M.</abbr></th>
    <th><abbr title="Repeatable">R.</abbr></th>
    <th>Min. bytes</th>
    <th>Max. bytes</th>
    <th>Tag description</th>
  </tr>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="ROWSET/ROW">
  <xsl:call-template name="data-row" />
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="data-row">
  <tr>
    <td><xsl:value-of select="taghex" /></td>
    <td><xsl:value-of select="tagdec" /></td>
    <td><xsl:value-of select="key" /></td>
    <td><xsl:value-of select="type" /></td>
    <td>
      <xsl:choose>
        <xsl:when test="mandatory = 'true'">
          <xsl:text>Yes</xsl:text>
        </xsl:when>
        <xsl:when test="mandatory = 'false'">
          <xsl:text>No</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="mandatory" />
        </xsl:otherwise>
      </xsl:choose>
    </td>
    <td>
      <xsl:choose>
        <xsl:when test="repeatable = 'true'">
          <xsl:text>Yes</xsl:text>
        </xsl:when>
        <xsl:when test="repeatable = 'false'">
          <xsl:text>No</xsl:text>
        </xsl:when>
        <xsl:otherwise>
          <xsl:value-of select="repeatable" />
        </xsl:otherwise>
      </xsl:choose>
    </td>
    <td><xsl:value-of select="minbytes" /></td>
    <td><xsl:value-of select="maxbytes" /></td>
    <td><xsl:value-of select="tagdesc" /></td>
  </tr>
</xsl:template>

<!-- *********************************************************************** -->
</xsl:stylesheet>
