#! /bin/sh
# Create download tables

basedir=.
version=$(cat $basedir/var/__version__)

table=__download_table__
buttons=__download_buttons__

rm -rf $basedir/var/$table
rm -rf $basedir/var/$buttons

for P in trunk macosx linux cygwin mingw msvc; do
	p=exiv2-$version-$P.tar.gz
	size=$(ls -la html/builds/$p | cut -d' ' -f 5)
	date=$(stat -c "%y" html/builds/$p | cut -d' ' -f 1)
	md5=$(md5sum html/builds/$p  | cut -d' ' -f 1)

	echo "<tr>  \
            <td>Exiv2 v$version $P</td> \
            <td><a href=\"builds/${p}\">${p}</a></td> \
            <td>$size</td> \
            <td class=\"text-nowrap\">$date</td> \
            <td>$md5</td> \
          </tr>"  >> $basedir/var/$table

	platform=$P
	config="64 bit shared libraries"
	if [ "$platform" == macosx ]; then platform="MacOSX"       ; fi
	if [ "$platform" == cygwin ]; then platform="Cygwin"       ; fi
	if [ "$platform" == mingw  ]; then platform="MinGW"        ; config="32 bit shared libraries"                     ; fi
	if [ "$platform" == msvc   ]; then platform="Visual Studio"; config="32 & 64 bit DLLs for MSVC 2005/8/10/12/13/15"; fi
	if [ "$platform" == linux  ]; then platform="Linux"        ; fi
	if [ "$platform" != trunk  ]; then
  	  echo "<tr><td>$platform<h3></td><td>$config</td> \
	        <td> \
	          <p3 class=\"text-center\"> \
                <a href=\"builds/$p\" class=\"btn btn-sm btn-success\"> \
	            <span class=\"glyphicon glyphicon-download-alt\" aria-hidden=\"true\"></span>&nbsp;$p \
	            </a> \
	         </p3> \
            </td></tr>" >> $basedir/var/$buttons
    fi
done

# That's all Folks!
##
