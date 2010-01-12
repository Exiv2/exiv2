#! /bin/sh
# Test driver for exiv2 Exif.Photo.UserComment character encoding tests
scriptdir=`dirname $0`
cd $scriptdir
exiv2="../src/exiv2"

# Function takes two parameters
#
# 1. A exiv2 comment spec
# 2. The expected exiv2 hex dump of the UserComment value
#
function writeComment {
    cp ./data/exiv2-bug662.jpg ./tmp/exiv2-bug662.jpg
    $exiv2 mo "-Mset Exif.Photo.UserComment $1" ./tmp/exiv2-bug662.jpg
    res=`$exiv2 pr -PEnh ./tmp/exiv2-bug662.jpg | grep --after-context=200 UserComment`

    # remove newlines and the tag name
    res=`echo $res | colrm 1 12 | sed -e 's/"//g'`
    if [ "$res" != "$2" ] ; then
        echo "Expected:"
        echo $2
        echo "Got:"
        echo $res
        return 1
    fi
}

writeComment "charset=Ascii An ascii comment" "0000 41 53 43 49 49 00 00 00 41 6e 20 61 73 63 69 69 ASCII...An ascii 0010 20 63 6f 6d 6d 65 6e 74 comment"
writeComment "charset=Ascii A\\nnewline" "0000 41 53 43 49 49 00 00 00 41 0a 6e 65 77 6c 69 6e ASCII...A.newlin 0010 65 e"
writeComment "charset=Unicode A Unicode comment" "0000 55 4e 49 43 4f 44 45 00 41 00 20 00 55 00 6e 00 UNICODE.A. .U.n. 0010 69 00 63 00 6f 00 64 00 65 00 20 00 63 00 6f 00 i.c.o.d.e. .c.o. 0020 6d 00 6d 00 65 00 6e 00 74 00 m.m.e.n.t."
writeComment "charset=Unicode \\u01c4" "0000 55 4e 49 43 4f 44 45 00 c4 01 UNICODE..."
writeComment "charset=Unicode A\\u01c4C" "0000 55 4e 49 43 4f 44 45 00 41 00 c4 01 43 00 UNICODE.A...C."
writeComment "charset=Unicode With\\nNewline" "0000 55 4e 49 43 4f 44 45 00 57 00 69 00 74 00 68 00 UNICODE.W.i.t.h. 0010 0a 00 4e 00 65 00 77 00 6c 00 69 00 6e 00 65 00 ..N.e.w.l.i.n.e."
writeComment "charset=Unicode With\\tTab" "0000 55 4e 49 43 4f 44 45 00 57 00 69 00 74 00 68 00 UNICODE.W.i.t.h. 0010 09 00 54 00 61 00 62 00 ..T.a.b."

# Test invalid escape sequences
writeComment "charset=Unicode \\ugggg" "0000 55 4e 49 43 4f 44 45 00 5c 00 75 00 67 00 67 00 UNICODE.\.u.g.g. 0010 67 00 67 00 g.g."
