 #!/bin/bash

dest=html/builds
if [ ! -e $dest ]; then
	mkdir -p $dest
fi
if [ ! -d $dest ]; then
	echo '***' $dest 'does not exist ***'
	exiv 1
fi

version=$(cat var/__version__)
url=http://exiv2.dyndns.org:8080/userContent/builds/Latest
builds=$(curl --silent $url/ | xmllint --html --format - 2>&1 | grep tar | grep -v -ve view -ve job -ve bread | cut -d= -f 2 | cut -d'"' -f 2)

for build in $builds; do
	plat=$(echo $build | cut -d- -f 1)
	plat="exiv2-$version-$plat.tar.gz"
	echo curl --silent -O $url/$build '>' $dest/$plat
	 	 curl --silent $url/$build     >  $dest/$plat
done

plat=trunk
orig=exiv2-trunk.tar.gz
plat="exiv2-$version-$plat.tar.gz"
if [ -e $orig ]; then
	cp $orig $dest/$plat
fi

echo ---- $dest ----
ls -lt $dest

 # That's all Folks!
 ##
