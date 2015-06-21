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
echo
echo ==========================================================================
echo Exporting sources, this may take a while...
echo
path=tags/$rel
if [ $rel = trunk ] ; then
    path=trunk
fi
svn export svn://dev.exiv2.org/svn/$path
mv $rel exiv2-$rel
echo
echo ==========================================================================
echo Preparing the source code
echo
cd exiv2-$rel
make config
./configure --disable-shared
make -j4
sudo make install
make -j4 samples
make doc
cd test
make
cd .. 
rm -f ABOUT-NLS
rm -f .gitignore
rm -rf kdevelop/
sudo make uninstall
make distclean
rm -rf test/
rm -f Makefile
rm -f bootstrap.linux
rm -f msvc64\\runner.txt
# 01-Dec-2013, ahu: Include CMake files in the distribution
#find . -type f -name '*[Cc][Mm][Aa][Kk][Ee]*' | xargs rm -f
rm -rf xmpsdk/src/.libs
rm -f config.log
rm -rf website
rm -f jenkins_build.bat jenkins_build.sh
rm -f fixxml.sh
echo
echo ==========================================================================
echo Creating source and doc packages
echo
cd ..
tar zcvf exiv2-$rel-doc.tar.gz exiv2-$rel/doc/index.html exiv2-$rel/doc/html exiv2-$rel/doc/include
rm -rf exiv2-$rel/doc/html
tar zcvf exiv2-$rel.tar.gz exiv2-$rel
echo
echo ==========================================================================
echo Testing the tarball: unpack, build and run tests
echo
rm -rf exiv2-$rel
tar zxvf exiv2-$rel.tar.gz
cd exiv2-$rel
./configure
make -j4
sudo make install
make -j4 samples
echo Exporting tests, this may take a while...
svn export svn://dev.exiv2.org/svn/tags/$rel/test
du -sk test/
cd test
make
cd ../..
echo
echo ==========================================================================
echo Error-summary
echo
grep 'Error ' exiv2-buildrelease-$rel.out | grep -v -e'BasicError ' -e'Error 1 (ignored)'

) 2>&1 | tee exiv2-buildrelease-$rel.out 2>&1
