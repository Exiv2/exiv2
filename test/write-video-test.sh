#!/usr/bin/env bash
# Test driver for write video

source ./functions.source

##
# set up output and reference file
out=$(real_path "$testdir/$this.out")
copyTestFile       "video/$this.out"

(   cd "$testdir"

    videos=($(copyVideoFiles))

    # write metadata to videos
    runTest exiv2 -M "set Xmp.video.MicroSecPerFrame 64"      ${videos[*]}
    runTest exiv2 -M "set Xmp.video.MaxDataRate 4096"         ${videos[*]}
    runTest exiv2 -M "set Xmp.video.FrameCount 2048"          ${videos[*]}
    runTest exiv2 -M "set Xmp.video.InitialFrames 4"          ${videos[*]}
    runTest exiv2 -M "set Xmp.video.StreamCount 2"            ${videos[*]}
    runTest exiv2 -M "set Xmp.video.SuggestedBufferSize 1024" ${videos[*]}
    runTest exiv2 -M "set Xmp.video.Width 240"                ${videos[*]}
    runTest exiv2 -M "set Xmp.video.Height 320"               ${videos[*]}
    runTest exiv2 -M "set Xmp.video.FileDataRate 128"         ${videos[*]}
    runTest exiv2 -M "set Xmp.video.Duration 2048"            ${videos[*]}
    runTest exiv2 -M "set Xmp.video.Codec mjpg"               ${videos[*]}
    runTest exiv2 -M "set Xmp.video.FrameRate 1024"           ${videos[*]}
    runTest exiv2 -M "set Xmp.video.VideoQuality 128"         ${videos[*]}
    runTest exiv2 -M "set Xmp.video.VideoSampleSize 256"      ${videos[*]}
    runTest exiv2 -M "set Xmp.audio.Codec mpv4"               ${videos[*]}
    runTest exiv2 -M "set Xmp.audio.SampleRate 32"            ${videos[*]}
    runTest exiv2 -M "set Xmp.audio.SampleCount 32"           ${videos[*]}
    runTest exiv2 -M "set Xmp.video.DateUT $date"             ${videos[*]}
    runTest exiv2 -M "set Xmp.video.Comment Metadata was Edited Using Exiv2" ${videos[*]}
    runTest exiv2 -M "set Xmp.video.Language Kannada"         ${videos[*]}
    runTest exiv2 -M "set Xmp.video.Country India"            ${videos[*]}
    runTest exiv2 -M "set Xmp.video.Copyright Photographer"   ${videos[*]}
    runTest exiv2 -M "set Xmp.video.Genre Sample Test Video"  ${videos[*]}
    runTest exiv2 -M "set Xmp.video.Software Exiv2 0.25"      ${videos[*]}
    runTest exiv2 -M "set Xmp.video.Junk Its a junk Data"     ${videos[*]}
    runTest exiv2 -M "set Xmp.video.MediaLanguage English"    ${videos[*]}

    for video in ${videos[*]}; do
        printf "." >&3
        echo
        echo "-----> $video <-----"
        echo
        echo "Command: exiv2 -u -pa $video"
        # run command                 | ignore binary and no Date nor NumOfColours tags
        runTest exiv2 -u -pa "$video" | sed -E -e 's/\d128-\d255/_/g' | grep -a -v -e Date
    done

) 3>&1 2>&1 > "$out"

echo "."

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
