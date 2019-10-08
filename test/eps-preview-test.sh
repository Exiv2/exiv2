#!/usr/bin/env bash
# Test driver for previews

source ./functions.source

(   cd "$testdir"

    images="eps/eps-flat_coreldraw-x3-lev2.eps \
        eps/eps-flat_coreldraw-x5-lev2.eps \
        eps/eps-flat_inkscape-epsi.eps \
        eps/eps-flat_oodraw_ai-10-lev2.eps \
        eps/eps-flat_oodraw_ai-8-lev2.eps \
        eps/eps-flat_oodraw_ai-8-lev3.eps \
        eps/eps-flat_oodraw_ai-9-lev2.eps \
        eps/eps-flat_oodraw_ai-cs-lev2.eps \
        eps/eps-flat_oodraw_ai-cs2-lev2.eps \
        eps/eps-flat_oodraw_ai-cs3-lev2.eps \
        eps/eps-flat_oodraw_ai-cs4-lev2.eps \
        eps/eps-flat_oodraw_ai-cs5-lev2.eps \
        eps/eps-flat_oodraw_ai-cs5-lev3-nodocthumb.eps \
        eps/eps-flat_oodraw_ai-cs5-lev3-pre.eps \
        eps/eps-flat_oodraw_ai-cs5-lev3.eps \
        eps/eps-flat_oodraw_ai-cs5-lev3_exiftool-8.56.eps \
        eps/eps-flat_oodraw_ai-cs5-lev3_ns-xapGImg.eps \
        eps/eps-flat_photoshop-cs5-ascii-colorprofile.eps \
        eps/eps-flat_photoshop-cs5-ascii-pre.eps \
        eps/eps-flat_photoshop-cs5-ascii.eps \
        eps/eps-flat_photoshop-cs5-ascii85-pre.eps \
        eps/eps-flat_photoshop-cs5-ascii85.eps \
        eps/eps-flat_photoshop-cs5-ascii_failure-bigxmp.eps \
        eps/eps-flat_photoshop-cs5-binary-pre.eps \
        eps/eps-flat_photoshop-cs5-binary.eps \
        eps/eps-flat_photoshop-cs5-binary_exiftool-8.56.eps \
        eps/eps-flat_photoshop-cs5-binary_exiv2-bigxmp.eps \
        eps/eps-flat_photoshop-cs5-binary_exiv2.eps \
        eps/eps-flat_photoshop-cs5-binary_exiv2_missing-begin-xml-packet.eps \
        eps/eps-flat_photoshop-cs5-binary_no-adocontainsxmp.eps \
        eps/eps-flat_photoshop-e9-win-doseps.eps \
        eps/eps-flat_photoshop-e9-win.eps \
        eps/eps-flat_photoshop-e9-win_exiv2.eps \
        eps/eps-nested_noxmp_ai-8-lev2.eps \
        eps/eps-nested_noxmp_ai-8-lev3.eps \
        eps/eps-nested_noxmp_ai-cs5-lev2.eps \
        eps/eps-nested_noxmp_ai-cs5-lev3.eps \
        eps/eps-nested_noxmp_indesign-cs5-lev2-bin.eps \
        eps/eps-nested_noxmp_indesign-cs5-lev2.eps \
        eps/eps-nested_noxmp_indesign-cs5-lev3-bin.eps \
        eps/eps-nested_noxmp_indesign-cs5-lev3.eps \
        eps/eps-nested_noxmp_oodraw-lev2-epsi.eps \
        eps/eps-nested_noxmp_oodraw-lev2-pre.eps \
        eps/eps-nested_xmp_ai-3-lev3.eps \
        eps/eps-nested_xmp_ai-8-lev3.eps \
        eps/eps-nested_xmp_ai-cs5-lev3.eps \
        eps/eps-nested_xmp_indesign-cs5-lev3-bin.eps \
        eps/eps-nested_xmp_indesign-cs5-lev3.eps \
        eps/eps-nested_xmp_indesign-cs5-lev3_failure-exiftool-8.56.eps \
        eps/eps-nested_xmp_oodraw-lev2-epsi.eps \
        eps/eps-nested_xmp_oodraw-lev2-pre.eps \
        eps/eps-old-r2872_01.eps \
        eps/eps-old-r2872_02.eps \
        eps/eps-old-r2872_03.eps \
        eps/eps-old-r2872_04.eps"

    for filepath in $images; do
        filename=`basename "$filepath"`
        image=`echo "$filename" | sed 's,\.[^.]*$,,'`

        printf "." >&3

        echo
        echo "-----> $filename <-----"

        cp "../data/$filepath" ./

        echo
        echo "Command: exiv2 -pp $filename"
        runTest        exiv2 -pp "$filename"
        exitcode=$?
        echo "Exit code: $exitcode"

        rm -f "$image-preview"*

        echo
        echo "Command: exiv2 -f -ep $filename"
        runTest        exiv2 -f -ep "$filename"
        echo "Exit code: $?"

        runTest exiv2 -pp "$filename" 2>/dev/null | sed -n 's,^Preview \([0-9]\+\):.*,\1,p' | while read preview; do
            diff $diffargs -q "../data/preview/$image-preview$preview."* "$image-preview$preview."*
        done
    done

) 3>&1 > "$testdir/eps-preview-test.out" 2>&1

echo "."

# ----------------------------------------------------------------------
# Result
if ! diff -q $diffargs  "$testdir/$datadir/preview/eps-preview-test.out" "$testdir/eps-preview-test.out" ; then
    diff  -u $diffargs  "$testdir/$datadir/preview/eps-preview-test.out" "$testdir/eps-preview-test.out"
    exit 1
fi
echo "All testcases passed."

# That's all Folks!
##