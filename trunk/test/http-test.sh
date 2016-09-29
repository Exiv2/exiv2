#!/bin/bash
# Test driver for http

source ./functions.source


(   cd "$testdir"

    host=http://exiv2.dyndns.org:8080/
    dir=userContent/testfiles/webready/
    for host in http://exiv2.dyndns.org:8080 https://exiv2.dyndns.org:8443
    do
    	printf "$host " >&3
		for filename in Reagan.jpg ReaganLargePng.png ReaganLargeTiff.tiff ReaganLargeJpg.jpg
		do
			echo $host/$dir/$filename
			runTest exiv2 -pa --grep Software $host/$dir/$filename
		done
	done

) 3>&1 > $results 2>&1

printf "\n"

# ----------------------------------------------------------------------
# Evaluate results
cat $results | tr -d $'\r' > $results-stripped
mv                           $results-stripped $results
reportTest                                     $results $good

# That's all Folks!
##
