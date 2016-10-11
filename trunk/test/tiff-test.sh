#!/usr/bin/env bash
# TIFF parser test driver

if [ "${BASH_VERSION:0:1}" -lt "4" ]; then
    echo "$0 requires bash v4 or greater.  Running $BASH_VERSION.  $0 skipped."
    exit 0
fi

exifprobe()
{
    f=$1

    # References
    # process pipe : http://stackoverflow.com/questions/7612320/bash-weird-variable-scope-when-populating-array-with-results
    # to uppercase : http://unix.stackexchange.com/questions/51983/bash-how-to-uppercase-the-command-line-argument
    # ---------------------------------------
    # Parse -pa output
    # Exif.Image.ImageDescription                  Ascii      18  Created with GIMP
    tags=()
    typs=()
    lens=()
    vals=()
    while read line; do
        tag=$(echo $line|cut -d.   -f 3  | cut -d' ' -f 1)                                    ; tags+=($tag)
        typ=$(echo $line|cut -d' ' -f 2- | sed -E -e 's/ +/ /g' -e 's/^ //' | cut -d' ' -f 1 ); typs+=($typ)
        len=$(echo $line|cut -d' ' -f 2- | sed -E -e 's/ +/ /g' -e 's/^ //' | cut -d' ' -f 2 ); lens+=($len)
        val=$(echo $line|cut -d' ' -f 2- | sed -E -e 's/ +/ /g' -e 's/^ //' | cut -d' ' -f 3-); vals+=("$val")
    done < <( runTest exiv2 -pa $f  2>/dev/null ) # process pipe
    count=${#tags[@]}

    echo exiv2 -pa output
    index=0
    while [ $index -lt $count ]; do
        tag=${tags[$index]}
        typ=${typs[$index]}
        len=${lens[$index]}
        val=${vals[$index]}
        echo ${tag} $'\t' ${typ} $'\t' ${len} $'\t' '"'${val}'"'
        index=$((index+1))
    done

    # Parse -pS output
    # address |    tag                           |      type |    count |   offset | value
    #     254 | 0x00fe NewSubfileType            |      LONG |        1 |        0 | 0
    TAGS=()
    TYPS=()
    LENS=()
    OFFS=()
    VALS=()
    k=0
    while read line; do
        k=$((k+1)) # skip the first couple of lines
        if [ $k -gt 2 ]; then
            TAG=$(echo $line| cut -d'|' -f 2  | cut -d' ' -f 3    ); TAGS+=($TAG);
            TYP=$(echo $line| cut -d'|' -f 3  | sed -E -e's/ +//g'); TYPS+=($TYP)
            LEN=$(echo $line| cut -d'|' -f 4  | sed -E -e's/ +//g'); LENS+=($LEN)
            OFF=$(echo $line| cut -d'|' -f 5  | sed -E -e's/ +//g'); OFFS+=($OFF)
            VAL=$(echo $line| cut -d'|' -f 6- | sed -e's/^ //'    ); VALS+=($"$VAL")
        fi
    done < <( runTest exiv2 -pS $f | grep -v -e '^END' 2>/dev/null )
    COUNT=${#TAGS[@]}

    echo ''
    echo exiv2 -pS output
    INDEX=0
    while [ $INDEX -lt $COUNT ]; do
        TAG=${TAGS[$INDEX]}
        TYP=${TYPS[$INDEX]}
        LEN=${LENS[$INDEX]}
        OFF=${OFFS[$INDEX]}
        VAL=${VALS[$INDEX]}
        echo ${TAG} $'\t' ${TYP} $'\t' ${LEN} $'\t' '"'${VAL}'"'
        INDEX=$((INDEX+1))
    done

    # ---------------------------------------
    # Compare
    echo ''
    echo "Analysis"
    echo "count = " $count "COUNT = " $COUNT
    index=0
    while [ $index -lt $count ]; do
        TAG=${TAGS[$index]}
        tag=${tags[$index]}
        if [ "$TAG" != "$tag" ]; then echo TAG $TAG $tag mismatch ; fi

        TYP=${TYPS[$index]^^} # to uppercase
        typ=${typs[$index]^^}
        if [ "$TYP" != "$typ" ]; then echo TYPE $TYP $typ mismatch ; fi

        LEN=${LENS[$index]}
        len=${lens[$index]}
        if [ "$LEN" != "$len" ]; then echo Length $LEN $len mismatch ; fi

        VAL=${VALS[$index]}
        val=${vals[$index]}
        if [[ "${typ}" == "ASCII" && "$VAL" != "$val" ]]; then echo Value $VAL $val mismatch ; fi

        index=$((index+1))
    done
}

# ----------------------------------------------------------------------
# Setup
source ./functions.source

(   cd "$testdir"

    testfile=mini9.tif
    copyTestFile ${testfile}
    exifprobe ${testfile}

    runTest tiff-test ${testfile}
    exifprobe ${testfile}

) > $results

# ----------------------------------------------------------------------
# Evaluate results
cat $results | tr -d $'\r' > $results-stripped
mv                           $results-stripped $results
reportTest                                     $results $good

# That's all Folks!
##