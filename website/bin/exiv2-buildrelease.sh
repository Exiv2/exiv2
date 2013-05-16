#! /bin/sh
# Export and build an exiv2 release, 26-Jan-06, ahu

if [ $# -eq 0 ] ; then
    echo "Usage: `basename $0` <tagname>|trunk"
    cat <<EOF

Export and build an Exiv2 release.
EOF
    exit 1
fi

rel=$1
(
if [ -e $rel ] ; then
    echo File $rel already exists, exiting...
    exit 1
fi
if [ -e exiv2-$rel ] ; then
    echo File exiv2-$rel already exists, exiting...
    exit 1
fi

echo Exporting sources, this may take a while...

path=tags/$rel
if [ $rel = trunk ] ; then
    path=trunk
fi
svn export svn://dev.exiv2.org/svn/$path
mv $rel exiv2-$rel

# Prepare the source code
cd exiv2-$rel
make config
./configure --disable-shared
make -j3
make install
make samples
make doc
cd test
make
cd .. 
rm -f ABOUT-NLS
rm -f .gitignore
rm -rf test/
rm -rf kdevelop/
make distclean
rm -f Makefile
rm -f bootstrap.linux
rm -f msvc64\\runner.txt
find . -type f -name '*[Cc][Mm][Aa][Kk][Ee]*' | xargs rm -f
rm -rf xmpsdk/src/.libs

# Create source and doc packages
cd ..
tar zcvf exiv2-$rel-doc.tar.gz exiv2-$rel/doc/index.html exiv2-$rel/doc/html exiv2-$rel/doc/include
rm -rf exiv2-$rel/doc/html
tar zcvf exiv2-$rel.tar.gz exiv2-$rel

# Test the tarball: unpack, build and run tests
rm -rf exiv2-$rel
tar zxvf exiv2-$rel.tar.gz
cd exiv2-$rel
./configure
make -j3
make install
make samples
echo Exporting tests, this may take a while...
svn export svn://dev.exiv2.org/svn/tags/$rel/test
du -sk test/
cd test
make
cd ../..

) 2>&1 | tee exiv2-buildrelease-$rel.out 2>&1
