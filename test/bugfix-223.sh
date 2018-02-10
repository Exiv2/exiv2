#!/bin/bash

source ./functions.source
cd "$testdir"

mkdir -p reference
this=$(basename $0)
reference=reference
output=$this.out

##
# run the tests
rm -rf $output
for testfile in 223-Monochrome.jpg 223-Sepia.jpg \
                223-Monochrome+G.jpg 223-Monochrome+R.jpg 223-Monochrome+Y.jpg ; do
	if [ ! -e $reference/$testfile ]; then (
	   cd   $reference
	   curl -O http://exiv2.dyndns.org:8080/userContent/testfiles/223/$testfile
	) fi
	runTest exiv2 -pa --grep Fujifilm.Color $reference/$testfile >>$output
done

##
# Reference output
cat > $reference/$output <<END_OF_FILE
Exif.Fujifilm.Color                          Short       1  Monochrome
Exif.Fujifilm.Color                          Short       1  Sepia
Exif.Fujifilm.Color                          Short       1  Monochrome+G Filter
Exif.Fujifilm.Color                          Short       1  Monochrome+R Filter
Exif.Fujifilm.Color                          Short       1  Monochrome+Ye Filter
END_OF_FILE

##
# report result
cat $output | tr -d $'\r' > $output.stripped
mv  $output.stripped $output
echo -n "$this " ; reportTest $output $reference/$output ; exit $rc

# That's all Folks!
##
