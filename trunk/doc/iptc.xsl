<?xml version="1.0" encoding="iso-8859-1" ?>                   <!--*- sgml -*-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" version="4.01" encoding="iso-8859-1" 
doctype-public="-//W3C//DTD HTML 4.01 Transitional//EN" />

<!-- *********************************************************************** -->
<xsl:template match="TAGLIST">
<html>
<head>
  <title>Exiv2 - Exif and Iptc metadata manipulation library and tools</title>
  <link type="text/css" rel="stylesheet" href="include/sortabletable.css" />
  <script type="text/javascript" src="include/sortabletable.js"></script>
</head>
<body>

<h1>Exif and Iptc metadata manipulation library and tools</h1>

<!-- content generated from XML -->
<xsl:call-template name="header" />
<xsl:call-template name="report-table" />

<br />

</body>
</html>
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
<xsl:template name="report-table">
  <table class="ReportTable" id="report-1" cellspacing="0">
    <xsl:call-template name="column-group" />
    <thead>
      <xsl:call-template name="header-row" />
    </thead>
    <tbody>
      <xsl:apply-templates select="ROWSET/ROW" />
    </tbody>
  </table>
  <xsl:call-template name="interactivity" />
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="column-group">
  <colgroup>
    <col align="center" />
    <col align="right" />
    <col />
    <col />
    <col />
    <col align="center" />
    <col align="center" />
    <col align="right" />
    <col align="right" />
    <col />
  </colgroup>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="header-row">
  <tr>
    <th>Tag (hex)</th>
    <th>Tag (dec)</th>
    <th>Record</th>
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
<xsl:template match="ROWSET/ROW[position() mod 2 = 0]">
  <xsl:call-template name="data-row">
    <xsl:with-param name="rowClass" select="'EvenRow'" />
  </xsl:call-template>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="ROWSET/ROW[position() mod 2 = 1]">
  <xsl:call-template name="data-row">
    <xsl:with-param name="rowClass" select="'OddRow'" />
  </xsl:call-template>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="data-row">
  <xsl:param name="rowClass" />
  <tr><xsl:attribute name="class"><xsl:value-of select="$rowClass" /></xsl:attribute>
    <td><xsl:value-of select="taghex" /></td>
    <td><xsl:value-of select="tagdec" /></td>
    <td><xsl:value-of select="recname" /></td>
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
<xsl:template name="interactivity">
  <script type="text/javascript">
  //<![CDATA[
  var t1 = new SortableTable(
    document.getElementById("report-1"),
    ["String", "Number", "String", "String", "String", "String", "String", "Number", "Number", "String"]
  );
  t1.onsort = function () { 
	var rows = t1.tBody.rows;
	var l = rows.length;
	for (var i = 0; i < l; i++) {
		removeClassName(rows[i], i % 2 ? "OddRow" : "EvenRow");
		addClassName(rows[i], i % 2 ? "EvenRow" : "OddRow");
	}
  };
  //]]>
  </script>
</xsl:template>

<!-- *********************************************************************** -->
</xsl:stylesheet>
