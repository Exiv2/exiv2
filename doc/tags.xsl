<?xml version="1.0" encoding="iso-8859-1" ?>                   <!--*- sgml -*-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" version="4.01" encoding="iso-8859-1" 
doctype-public="-//W3C//DTD HTML 4.01 Transitional//EN" />
 
<!-- *********************************************************************** -->
<xsl:template match="TAGLIST">
<html>
<head>
  <title>Exiv2 - Exif metadata manipulation library and tools</title>
  <link type="text/css" rel="stylesheet" href="include/sortabletable.css" />
  <script type="text/javascript" src="include/sortabletable.js"></script>
</head>
<body>

<h1>Exif metadata manipulation library and tools</h1>

<!-- content generated from XML -->
<xsl:call-template name="header" />
<xsl:call-template name="report-table" />

<br />

</body>
</html>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template match="HEADER/text">
  <xsl:copy-of select="text()|*"/>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="header">
  <h2><xsl:value-of select="HEADER/title" /></h2>
  <xsl:apply-templates select="HEADER/text"/>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="report-table">
  <table class="ReportTable" id="report-1" cellspacing="0">
    <xsl:call-template name="column-group" />
    <thead>
      <xsl:call-template name="header-row" />
    </thead>
    <tbody>
      <xsl:for-each select="ROWSET/ROW">
        <xsl:call-template name="data-row" />
      </xsl:for-each>
    </tbody>
  </table>
  <xsl:call-template name="interactivity" />
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="column-group">
  <colgroup>
    <col />
    <col />
    <col align="right" />
    <col align="center" />
    <col />
    <col />
  </colgroup>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="header-row">
  <tr>
    <th>Tag name</th>
    <th>Tag description</th>
    <th>Tag (dec)</th>
    <th>Tag (hex)</th>
    <th>IFD</th>
    <th>Key</th>
  </tr>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="data-row">
  <tr>
    <td><xsl:value-of select="tagname" /></td>
    <td><xsl:value-of select="tagdesc" /></td>
    <td><xsl:value-of select="tagdec" /></td>
    <td><xsl:value-of select="taghex" /></td>
    <td><xsl:value-of select="ifd" /></td>
    <td><xsl:value-of select="key" /></td>
  </tr>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="interactivity">
  <script type="text/javascript">
  //<![CDATA[
  var t1 = new SortableTable(
    document.getElementById("report-1"),
    ["String", "String", "Number", "String", "String", "String"]
  );
  t1.onsort = function () { 
	var rows = t1.tBody.rows;
	var l = rows.length;
	for (var i = 0; i < l; i++) {
		removeClassName(rows[i], i % 2 ? "OddRow" : "EvenRow");
		addClassName(rows[i], i % 2 ? "EvenRow" : "OddRow");
	}
  };
  t1.onsort();
  //]]>
  </script>
</xsl:template>

<!-- *********************************************************************** -->
</xsl:stylesheet>
