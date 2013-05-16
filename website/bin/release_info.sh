#! /bin/sh
# Create variables for a new release

basedir=.

if [ $# -ne 1 ] ; then
    echo Usage: release_info.sh version
    exit 1
fi

version=$1

echo -n $version > $basedir/var/__version__

date +'%d-%b-%Y' | tr -d '\n' > $basedir/var/__rel_date_src_pkg__
#date +'%d-%b-%Y' | tr -d '\n' > $basedir/var/__rel_date_doc_pkg__
date +'%d-%b-%Y' | tr -d '\n' > $basedir/var/__rel_date_win_pkg__

ls -la html/exiv2-${version}.tar.gz | awk '{printf "%s", $5}' > $basedir/var/__size_src_pkg__
#ls -la html/exiv2-${version}-doc.tar.gz | awk '{printf "%s", $5}' > $basedir/var/__size_doc_pkg__
ls -la html/exiv2-${version}-win.zip | awk '{printf "%s", $5}' > $basedir/var/__size_win_pkg__

md5sum html/exiv2-${version}.tar.gz | awk '{printf "%s", $1}' > $basedir/var/__md5sum_src_pkg__
#md5sum html/exiv2-${version}-doc.tar.gz | awk '{printf "%s", $1}' > $basedir/var/__md5sum_doc_pkg__
md5sum html/exiv2-${version}-win.zip | awk '{printf "%s", $1}' > $basedir/var/__md5sum_win_pkg__
