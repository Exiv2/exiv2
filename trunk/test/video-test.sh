#! /bin/sh
# Test driver for video files
#
# video-asf.wmv        http://www.educationalquestions.com/video/DLP_PART_2_768k.wmv
# video-avi.avi        http://redmine.yorba.org/attachments/631/Nikon_Coolpix_S3000.AVI
# video-matroska.mkv   http://www.bunkus.org/videotools/mkvtoolnix/samples/vsshort-vorbis-subs.mkv
# video-quicktime.mp4  http://dev.exiv2.org/attachments/362/20100709_002.mp4

# ----------------------------------------------------------------------
# Setup
export LC_ALL=C
cd tmp/
if [ -z "$EXIV2_BINDIR" ] ; then
    bin="$VALGRIND ../../bin"
else
    bin="$VALGRIND $EXIV2_BINDIR"
fi
diffargs="--strip-trailing-cr"
if ! diff -q $diffargs /dev/null /dev/null 2>/dev/null ; then
    diffargs=""
fi

# ----------------------------------------------------------------------
# Tests
(
    for file in ../data/video/video-*; do
        video="`basename "$file"`"
	if [ $video = "video-test.out" ] ; then
	    continue
	fi

        printf "." >&3

        echo
        echo "-----> $video <-----"

        cp "../data/video/$video" ./

        echo
        echo "Command: exiv2 -u -pa $video"
        $bin/exiv2 -u -pa "$video"
        exitcode="$?"
        echo "Exit code: $exitcode"

        if [ "$exitcode" -ne 0 -a "$exitcode" -ne 253 ] ; then
            continue
        fi

    done
) 3>&1 > "video-test.out" 2>&1

echo "."

# ----------------------------------------------------------------------
# Result
if ! diff -q $diffargs "../data/video/video-test.out" "video-test.out" ; then
    diff -u -a $diffargs "../data/video/video-test.out" "video-test.out"
    exit 1
fi
echo "All testcases passed."
