<?xml version="1.0" encoding="UTF-8" ?>                        <!--*- sgml -*-->
<!-- Exiv2 *********************************************************************

  File    tags.xsl
  Brief   Create a sortable HTML table from the tags.xml list
  Version $Name:  $ $Revision: 1.1 $
  Author  Andreas Huggel (ahu)
          <a href="mailto:ahuggel@gmx.net">ahuggel@gmx.net</a>
  Date    07-Feb-04, ahu: created

 *************************************************************************** -->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- *********************************************************************** -->
<xsl:template match="ROWSET">
  <html>
    <head>
      <title>Exif Tags defined in Exiv2</title>
      <meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
      <script type="text/javascript" src="include/sortabletable.js"></script>
      <link type="text/css" rel="stylesheet" href="include/sortabletable.css" />
    </head>
    <body>
      <h2>Exif Tags defined in Exiv2</h2>
      <!-- main table -->
      <table class="BoxTable" align="center" cellspacing="0" cellpadding="3">
        <tr>
          <td><xsl:call-template name="report-table" /></td>
        </tr>
      </table>  
    </body>
  </html>
</xsl:template>

<!-- *********************************************************************** -->
<xsl:template name="report-table">
  <table class="ReportTable" id="report-1" cellspacing="0">
    <xsl:call-template name="column-group" />
    <thead>
      <xsl:call-template name="header-row" />
    </thead>
    <tbody>
      <xsl:for-each select="ROW">
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
    <col style="text-align: right" />
    <col style="text-align: center" />
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
    <th>Ifd</th>
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
    ["String", "String", "String", "Number", "String", "String"]
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
