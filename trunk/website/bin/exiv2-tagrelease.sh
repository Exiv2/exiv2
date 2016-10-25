#! /bin/sh
# Tag an exiv2 release, 26-Jan-06, ahu

# Steps to create a package for release
# =====================================

# Manual: Update ChangeLog

# Manual: Update release number. The files that need changes are:
# - config/configure.ac
# - msvc2003/include/exv_msvc.h
# - msvc/include/exv_msvc.h
# - src/version.hpp
# - test/data/exiv2-test.out
# - CMakeLists.txt

if [ $# -eq 0 ] ; then
    echo "Usage: `basename $0` tagname [branch]"
    cat <<EOF

Tag the Exiv2 repository.
EOF
    exit 1
fi

rel=$1
bra=trunk
pre=
if [ $# -eq 2 ] ; then
    bra=branches/$2
    pre="[$2] "
fi

echo "You're about to tag $bra as release $rel, using the following command"
echo "svn copy svn://dev.exiv2.org/svn/$bra svn://dev.exiv2.org/svn/tags/$rel -m\"${pre}Tagged release $rel\""
echo -n "Continue? [n] : "
read var

if [ x$var = xy ] ; then
   # Tag the release
   echo Tagging $bra as release $rel, this may take a while...
   svn copy svn://dev.exiv2.org/svn/$bra svn://dev.exiv2.org/svn/tags/$rel -m"${pre}Tagged release $rel"
fi
