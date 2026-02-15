#!/usr/bin/env bash
# Test driver for http/https

source ./functions.source
pushd "$testdir" 2>/dev/null

recursive=$1

server=exiv2.dyndns.org
dir=userContent/testfiles/webready
host="http://$server:8080"

if [ ! -z "$recursive" ]; then
	if [ $(runTest exiv2 -vVg curlproto | grep -q https ; echo $?) == "0" ]; then
    	host="https://$server:8443"
	else
    	echo "*** https is not a supported protocol - test skipped ***"
    	exit 0
	fi
fi

echo 'server: ' $host

(
    for filename in Reagan.jpg ReaganLargePng.png ReaganLargeTiff.tiff ReaganLargeJpg.jpg Reagan.tiff DSC_0002.NEF ; do
        echo $filename
        runTest exiv2 -pa --grep Software $host/$dir/$filename
    done
) 2>&1 > $results

# ----------------------------------------------------------------------
# Evaluate results
cat $results | tr -d $'\r' > $results-stripped
mv                           $results-stripped $results
reportTest                                     $results $good

popd 2>/dev/null

if [ -z "$recursive" ]; then
	$0 1
fi



# That's all Folks!
##
