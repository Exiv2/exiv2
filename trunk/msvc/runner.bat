@echo off
rem ---- in the bin directories ----

bin\Debug\addmoddel.exe
bin\Debug\exifcomment.exe
bin\Debug\exifdata-test.exe
bin\Debug\exifprint.exe
bin\Debug\exiv2.exe
bin\Debug\iotest.exe
bin\Debug\iptceasy.exe
bin\Debug\iptcprint.exe
bin\Debug\iptctest.exe
bin\Debug\largeiptc-test.exe
bin\Debug\metacopy.exe
bin\Debug\prevtest.exe
bin\Debug\stringto-test.exe
bin\Debug\taglist.exe
bin\Debug\utiltest.exe
bin\Debug\write-test.exe
bin\Debug\write2-test.exe
bin\Debug\xmpparse.exe
bin\Debug\xmpparser-test.exe
bin\Debug\xmpsample.exe
bin\DebugDLL\addmoddel.exe
bin\DebugDLL\exifcomment.exe
bin\DebugDLL\exifdata-test.exe
bin\DebugDLL\exifprint.exe
bin\DebugDLL\exiv2.exe
bin\DebugDLL\iotest.exe
bin\DebugDLL\iptceasy.exe
bin\DebugDLL\iptcprint.exe
bin\DebugDLL\iptctest.exe
bin\DebugDLL\largeiptc-test.exe
bin\DebugDLL\metacopy.exe
bin\DebugDLL\prevtest.exe
bin\DebugDLL\stringto-test.exe
bin\DebugDLL\taglist.exe
bin\DebugDLL\utiltest.exe
bin\DebugDLL\write-test.exe
bin\DebugDLL\write2-test.exe
bin\DebugDLL\xmpparse.exe
bin\DebugDLL\xmpparser-test.exe
rem *** bin\DebugDLL\xmpsample.exe
bin\Release\addmoddel.exe
bin\Release\exifcomment.exe
bin\Release\exifdata-test.exe
bin\Release\exifprint.exe
bin\Release\exiv2.exe
bin\Release\iotest.exe
bin\Release\iptceasy.exe
bin\Release\iptcprint.exe
bin\Release\iptctest.exe
bin\Release\largeiptc-test.exe
bin\Release\metacopy.exe
bin\Release\prevtest.exe
bin\Release\stringto-test.exe
bin\Release\taglist.exe
bin\Release\utiltest.exe
bin\Release\write-test.exe
bin\Release\write2-test.exe
bin\Release\xmpparse.exe
bin\Release\xmpparser-test.exe
bin\Release\xmpsample.exe
bin\ReleaseDLL\addmoddel.exe
bin\ReleaseDLL\exifcomment.exe
bin\ReleaseDLL\exifdata-test.exe
bin\ReleaseDLL\exifprint.exe
bin\ReleaseDLL\exiv2.exe
bin\ReleaseDLL\iotest.exe
bin\ReleaseDLL\iptceasy.exe
bin\ReleaseDLL\iptcprint.exe
bin\ReleaseDLL\iptctest.exe
bin\ReleaseDLL\largeiptc-test.exe
bin\ReleaseDLL\metacopy.exe
bin\ReleaseDLL\prevtest.exe
rem *** bin\ReleaseDLL\stringto-test.exe
rem *** bin\ReleaseDLL\taglist.exe
bin\ReleaseDLL\utiltest.exe
bin\ReleaseDLL\write-test.exe
bin\ReleaseDLL\write2-test.exe
bin\ReleaseDLL\xmpparse.exe
bin\ReleaseDLL\xmpparser-test.exe
rem *** bin\ReleaseDLL\xmpsample.exe

rem ------- exiv2.exe on a file ----
bin\Debug\exiv2.exe      -pt test.jpg
bin\Debug\exiv2.exe          test.png
rem *** bin\DebugDLL\exiv2.exe   -pt test.jpg
bin\DebugDLL\exiv2.exe       test.png
bin\Release\exiv2.exe    -pt test.jpg
bin\Release\exiv2.exe        test.png
bin\ReleaseDLL\exiv2.exe -pt test.jpg
bin\ReleaseDLL\exiv2.exe     test.png

cd bin\Debug
echo -------Debug---------------
..\..\depends1.exe addmoddel.exe         | sort
..\..\depends1.exe exifcomment.exe       | sort
..\..\depends1.exe exifdata-test.exe     | sort
..\..\depends1.exe exifprint.exe         | sort
..\..\depends1.exe exiv2.exe             | sort
..\..\depends1.exe iotest.exe            | sort
..\..\depends1.exe iptceasy.exe          | sort
..\..\depends1.exe iptcprint.exe         | sort
..\..\depends1.exe iptctest.exe          | sort
..\..\depends1.exe largeiptc-test.exe    | sort
..\..\depends1.exe metacopy.exe          | sort
..\..\depends1.exe prevtest.exe          | sort
..\..\depends1.exe stringto-test.exe     | sort
..\..\depends1.exe taglist.exe           | sort
..\..\depends1.exe utiltest.exe          | sort
..\..\depends1.exe write-test.exe        | sort
..\..\depends1.exe write2-test.exe       | sort
..\..\depends1.exe xmpparse.exe          | sort
..\..\depends1.exe xmpparser-test.exe    | sort
..\..\depends1.exe xmpsample.exe         | sort
..\..\depends1.exe exivsimple.dll        | sort

cd ..\DebugDLL

echo -------DebugDLL---------------
..\..\depends1.exe addmoddel.exe         | sort
..\..\depends1.exe exifcomment.exe       | sort
..\..\depends1.exe exifdata-test.exe     | sort
..\..\depends1.exe exifprint.exe         | sort
..\..\depends1.exe exiv2.exe             | sort
..\..\depends1.exe iotest.exe            | sort
..\..\depends1.exe iptceasy.exe          | sort
..\..\depends1.exe iptcprint.exe         | sort
..\..\depends1.exe iptctest.exe          | sort
..\..\depends1.exe largeiptc-test.exe    | sort
..\..\depends1.exe metacopy.exe          | sort
..\..\depends1.exe prevtest.exe          | sort
..\..\depends1.exe stringto-test.exe     | sort
..\..\depends1.exe taglist.exe           | sort
..\..\depends1.exe utiltest.exe          | sort
..\..\depends1.exe write-test.exe        | sort
..\..\depends1.exe write2-test.exe       | sort
..\..\depends1.exe xmpparse.exe          | sort
..\..\depends1.exe xmpparser-test.exe    | sort
..\..\depends1.exe xmpsample.exe         | sort
..\..\depends1.exe exivsimple.dll        | sort

cd ..\Release

echo -------Release---------------
..\..\depends1.exe addmoddel.exe         | sort
..\..\depends1.exe exifcomment.exe       | sort
..\..\depends1.exe exifdata-test.exe     | sort
..\..\depends1.exe exifprint.exe         | sort
..\..\depends1.exe exiv2.exe             | sort
..\..\depends1.exe iotest.exe            | sort
..\..\depends1.exe iptceasy.exe          | sort
..\..\depends1.exe iptcprint.exe         | sort
..\..\depends1.exe iptctest.exe          | sort
..\..\depends1.exe largeiptc-test.exe    | sort
..\..\depends1.exe metacopy.exe          | sort
..\..\depends1.exe prevtest.exe          | sort
..\..\depends1.exe stringto-test.exe     | sort
..\..\depends1.exe taglist.exe           | sort
..\..\depends1.exe utiltest.exe          | sort
..\..\depends1.exe write-test.exe        | sort
..\..\depends1.exe write2-test.exe       | sort
..\..\depends1.exe xmpparse.exe          | sort
..\..\depends1.exe xmpparser-test.exe    | sort
..\..\depends1.exe xmpsample.exe         | sort
..\..\depends1.exe exivsimple.dll        | sort

cd ..\ReleaseDLL

echo -------ReleaseDLL---------------
..\..\depends1.exe addmoddel.exe         | sort
..\..\depends1.exe exifcomment.exe       | sort
..\..\depends1.exe exifdata-test.exe     | sort
..\..\depends1.exe exifprint.exe         | sort
..\..\depends1.exe exiv2.exe             | sort
..\..\depends1.exe iotest.exe            | sort
..\..\depends1.exe iptceasy.exe          | sort
..\..\depends1.exe iptcprint.exe         | sort
..\..\depends1.exe iptctest.exe          | sort
..\..\depends1.exe largeiptc-test.exe    | sort
..\..\depends1.exe metacopy.exe          | sort
..\..\depends1.exe prevtest.exe          | sort
..\..\depends1.exe stringto-test.exe     | sort
..\..\depends1.exe taglist.exe           | sort
..\..\depends1.exe utiltest.exe          | sort
..\..\depends1.exe write-test.exe        | sort
..\..\depends1.exe write2-test.exe       | sort
..\..\depends1.exe xmpparse.exe          | sort
..\..\depends1.exe xmpparser-test.exe    | sort
..\..\depends1.exe xmpsample.exe         | sort
..\..\depends1.exe exivsimple.dll        | sort

cd ..\..
