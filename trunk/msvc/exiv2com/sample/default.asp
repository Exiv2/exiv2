<html>
<body>
<img src="sample.jpg" width=600><p>
<%
		'object initialisation
		Dim oExiv2
		Set oExiv2 = Server.CreateObject("exiv2com.Exif")

		'filename
		Dim strFileName
		strFileName = Server.MapPath("./") & "\sample.jpg"

		'GetExif method
		Response.Write "<b>GetExif method</b><br>"
		Response.Write oExiv2.GetExif(strFileName, "Exif.Image.Model") & "<p>"

		'GetExifInterpreted method
		Response.Write "<b>GetExifInterpreted method</b><br>"
		Response.Write Replace(oExiv2.GetExifInterpreted(strFileName), vbLf, "<br>")
%>
</body>
</html>
