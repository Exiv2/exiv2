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
for testfile in 226-logo.tif
do
	if [ ! -e $reference/$testfile ]; then (
	   cd   $reference
	   curl -O http://exiv2.dyndns.org:8080/userContent/testfiles/226/$testfile
	) fi
	runTest exiv2 -pX  $reference/$testfile | xmllint -format - >>$output
	runTest exiv2 -pXX $reference/$testfile | xmllint -format - >>$output
	runTest xmpdump    $reference/$testfile | xmllint -format - >>$output
done

##
# Reference output
for i in 1 2 3
do cat >>  $reference/$output <<END_OF_FILE
<?xml version="1.0"?>
<?xpacket begin="﻿" id="W5M0MpCehiHzreSzNTczkc9d"?>
<x:xmpmeta xmlns:x="adobe:ns:meta/" x:xmptk="XMP Core 4.4.0-Exiv2">
  <rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
    <rdf:Description xmlns:dc="http://purl.org/dc/elements/1.1/" rdf:about="">
      <dc:rights>
        <rdf:Alt>
          <rdf:li xml:lang="x-default">foo is alive</rdf:li>
        </rdf:Alt>
      </dc:rights>
    </rdf:Description>
  </rdf:RDF>
</x:xmpmeta>
<?xpacket end="w"?>
END_OF_FILE
done

##
# report result
cat $output | tr -d $'\r' > $output.stripped
mv  $output.stripped $output
echo -n "$this " ; reportTest $output $reference/$output ; exit $rc

# That's all Folks!
##
