#! /bin/sh
# Test driver for tests of MakerNoteFactory::match
results="./tmp/makernote-test.out"
good="./data/makernote-test.out"
(
binpath="../../src"
cd ./tmp
$binpath/makernote-test
) > $results

diff -q $results $good
rc=$?
if [ $rc -eq 0 ] ; then
    echo "All testcases passed."
else
    diff $results $good
fi
