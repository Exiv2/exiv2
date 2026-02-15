#!/usr/bin/env bash
# Test driver for video files
#
# video-asf.wmv        http://www.educationalquestions.com/video/DLP_PART_2_768k.wmv
# video-avi.avi        http://redmine.yorba.org/attachments/631/Nikon_Coolpix_S3000.AVI
# video-matroska.mkv   http://www.bunkus.org/videotools/mkvtoolnix/samples/vsshort-vorbis-subs.mkv
# video-quicktime.mp4  http://dev.exiv2.org/attachments/362/20100709_002.mp4

source ./functions.source

(   cd "$testdir"

    for file in ../data/video/video-*; do
        video="`basename "$file"`"
        if [ $video != "video-test.out" ] ; then

            printf "." >&3

            echo
            echo "-----> $video <-----"

            copyTestFile "video/$video" "$video"

            echo
            echo "Command: exiv2 -u -pa $video"
            runTest exiv2 -u -pa "$video"
            exitcode="$?"
            echo "Exit code: $exitcode"

            if [ "$exitcode" -ne 0 -a "$exitcode" -ne 253 ] ; then
                continue
            fi
        fi
    done

) 3>&1 > "$testdir/video-test.out" 2>&1

echo "."

# ----------------------------------------------------------------------
# Result
if ! diff   -q $diffargs "$testdir/$datadir/video/video-test.out" "$testdir/video-test.out" ; then
    diff -u -a $diffargs "$testdir/$datadir/video/video-test.out" "$testdir/video-test.out"
    exit 1
fi
echo "All testcases passed."

# That's all Folks!
##