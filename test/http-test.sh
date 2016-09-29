#!/bin/bash
# Test driver for http

source ./functions.source
cd "$testdir"

server=exiv2.dyndns.org
dir=userContent/testfiles/webready
hosts="http://$server:8080"

if [ $(runTest exiv2 -vVg curlproto | grep -q https ; echo $?) == "0" ]; then
    hosts="https://$server:8443"
else
    echo "https is not a supported protocol"
fi

(   (   for host in $hosts ; do
            printf "$host " >&3
            for filename in Reagan.jpg ReaganLargePng.png ReaganLargeTiff.tiff ReaganLargeJpg.jpg; do
                echo $filename
                runTest exiv2 -pa --grep Software $host/$dir/$filename
            done
        done
    ) 2>&1 > $results

    # ----------------------------------------------------------------------
    # Evaluate results
    cat $results | tr -d $'\r' > $results-stripped
    mv                           $results-stripped $results
    reportTest                                     $results $good
) 3>&1
printf "\n"


# That's all Folks!
##
