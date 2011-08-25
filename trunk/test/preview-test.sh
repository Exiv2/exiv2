#! /bin/sh
# Test driver for previews

# ----------------------------------------------------------------------
# Setup
export LC_ALL=C
cd tmp/
if [ -z "$EXIV2_BINDIR" ] ; then
    bin="$VALGRIND ../../src"
    samples="$VALGRIND ../../samples"
else
    bin="$VALGRIND $EXIV2_BINDIR"
    samples="$VALGRIND $EXIV2_BINDIR"
fi
diffargs="--strip-trailing-cr"
if ! diff -q $diffargs /dev/null /dev/null 2>/dev/null ; then
    diffargs=""
fi

# ----------------------------------------------------------------------
# Tests
images="eps/eps-flat_inkscape-epsi.eps \
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
        exiv2-bug443.jpg \
        exiv2-bug444.jpg \
        exiv2-bug445.jpg \
        exiv2-bug447.jpg \
        exiv2-bug501.jpg \
        exiv2-bug528.jpg \
        exiv2-canon-eos-20d.jpg \
        exiv2-canon-eos-300d.jpg \
        exiv2-canon-eos-d30.jpg \
        exiv2-canon-powershot-a520.jpg \
        exiv2-canon-powershot-s40.crw \
        exiv2-fujifilm-finepix-s2pro.jpg \
        exiv2-gc.jpg \
        exiv2-kodak-dc210.jpg \
        exiv2-nikon-d70.jpg \
        exiv2-nikon-e950.jpg \
        exiv2-nikon-e990.jpg \
        exiv2-olympus-c8080wz.jpg \
        exiv2-panasonic-dmc-fz5.jpg \
        exiv2-photoshop.psd \
        exiv2-pre-in-xmp.xmp \
        exiv2-sigma-d10.jpg \
        exiv2-sony-dsc-w7.jpg \
        glider.exv \
        imagemagick.pgf \
        iptc-psAPP13-noIPTC-psAPP13-wIPTC.jpg \
        iptc-psAPP13-noIPTC.jpg \
        iptc-psAPP13-wIPTC-psAPP13-noIPTC.jpg \
        iptc-psAPP13-wIPTC1-psAPP13-wIPTC2.jpg \
        iptc-psAPP13-wIPTCbeg.jpg \
        iptc-psAPP13-wIPTCempty-psAPP13-wIPTC.jpg \
        iptc-psAPP13-wIPTCempty.jpg \
        iptc-psAPP13-wIPTCend.jpg \
        iptc-psAPP13-wIPTCmid.jpg \
        iptc-psAPP13-wIPTCmid1-wIPTCempty-wIPTCmid2.jpg \
        smiley2.jpg"
(
    for filepath in $images; do
        filename=`basename "$filepath"`
        image=`echo "$filename" | sed 's,\.[^.]*$,,'`

        printf "." >&3

        echo
        echo "-----> $filename <-----"

        cp "../data/$filepath" ./

        echo
        echo "Command: exiv2 -pp $filename"
        $bin/exiv2 -pp "$filename"
        exitcode=$?
        echo "Exit code: $exitcode"

        rm -f "$image-preview"*

        echo
        echo "Command: exiv2 -f -ep $filename"
        $bin/exiv2 -f -ep "$filename"
        echo "Exit code: $?"

        $bin/exiv2 -pp "$filename" 2>/dev/null | sed -n 's,^Preview \([0-9]\+\):.*,\1,p' | while read preview; do
            diff -q "../data/preview/$image-preview$preview."* "$image-preview$preview."*
        done
    done
) 3>&1 > "preview-test.out" 2>&1

echo "."

# ----------------------------------------------------------------------
# Result
if ! diff -q $diffargs "../data/preview/preview-test.out" "preview-test.out" ; then
    diff -u $diffargs "../data/preview/preview-test.out" "preview-test.out"
    exit 1
fi
echo "All testcases passed."
