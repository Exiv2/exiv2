#!/bin/bash

source ./functions.source
cd "$testdir"

mkdir -p reference
this=$(basename $0)
reference=reference
output=$this.out
rm -rf $output $reference/$output

##
# run the tests
for testfile in 227-Sports.jpg
do
	if [ ! -e $reference/$testfile ]; then (
	   cd   $reference
	   curl -O http://exiv2.dyndns.org:8080/userContent/testfiles/227/$testfile
	) fi
	runTest exiv2 -pa  --grep Lens/i  $reference/$testfile >>$output
done

##
# Reference output
cat >>  $reference/$output <<END_OF_FILE
Exif.Nikon3.LensType                         Byte        1  D G VR
Exif.Nikon3.Lens                             Rational    4  120-300mm F2.8
Exif.Nikon3.LensFStops                       Undefined   4  6
Exif.NikonLd3.LensIDNumber                   Byte        1  Sigma 120-300mm F2.8 DG OS HSM | S
Exif.NikonLd3.LensFStops                     Byte        1  F6.0
END_OF_FILE

##
# report result
cat $output | tr -d $'\r' > $output.stripped
mv  $output.stripped $output
echo -n "$this " ; reportTest $output $reference/$output ; exit $rc

# That's all Folks!
##
