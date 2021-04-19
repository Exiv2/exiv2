#!/usr/bin/env bash
# Test driver for exiv2.exe nls support

source ./functions.source

(   cd "$testdir"

    nls=$(runTest exiv2 -vVg nls|tail -1)
    platform=$(${bin}exiv2${exe} -vVg platform|tail -1)
    if [ "$nls" != "enable_nls=1" ]; then
    	echo "exiv2 not bulid with nls"
    	exit 0
    fi
    if [ "$platform" == "platform=windows" ]; then
    	echo "nls_test cannot be run msvc builds" >2
    	exit 0
    fi
    if [ "$platform"  == "platform=linux" ]; then
    	LANG=LANGUAGE
    else 
    	LANG=LANG
    fi
    ##
    # if necessary ditto /usr/local/share/locale -> build/share/locale
    share=${bin}../share
    if [ ! -e $share ]; then
    	mkdir -p $share
    fi
    usr=/usr/local/share/locale
    if [ -e "$usr" -a -e "$share" ]; then
    	cp -r "$usr" "$share"
    else 
    	echo "localisation files are not installed in $usr"
    	exit 0
    fi
    ##
    # test a couple of languages
	for l in fr_FR es_ES; do
		export LC_ALL=$l
		export $LANG=$l 
		runTest exiv2 2>&1 | head -n 1
    done

) 3>&1 > $results 2>&1

reportTest

# That's all Folks!
##
