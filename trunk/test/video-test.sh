#!/bin/bash
# Test driver for video files
#
# video-asf.wmv        http://www.educationalquestions.com/video/DLP_PART_2_768k.wmv
# video-avi.avi        http://redmine.yorba.org/attachments/631/Nikon_Coolpix_S3000.AVI
# video-matroska.mkv   http://www.bunkus.org/videotools/mkvtoolnix/samples/vsshort-vorbis-subs.mkv
# video-quicktime.mp4  http://dev.exiv2.org/attachments/362/20100709_002.mp4

source ./functions.source

##
# set up output and reference file
out=$(real_path "$testdir/$this.out")
copyTestFile       "video/$this.out"

(	cd "$testdir"

	videos=($(copyVideoFiles))
	for video in ${videos[*]}; do
	    printf "." >&3
    	echo
        echo "-----> $video <-----"
    	echo
        echo "Command: exiv2 -u -pa $video"
        # run command                 | ignore binary and no Date nor NumOfColours tags
	    runTest exiv2 -u -pa "$video" | sed -E -e 's/\d128-\d255/_/g' | grep -a -v -e Date -v -e NumOfC
    done

) 3>&1 2>&1 > "$out" 

# ----------------------------------------------------------------------
# Result
diffCheck "$out" "$testdir/$datadir/video/$this.out" 

if [ $errors ]; then
	echo -e $errors 'test case(s) failed!'
else
	echo -e "all testcases passed."
fi

# That's all Folks!
##
