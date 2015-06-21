#! /bin/sh
# Cross-compile Exiv2 executable for Windows 
# 17-Mar-07, ahu

if [ $# -eq 0 ] ; then
    echo "Usage: `basename $0` directory"
    cat <<EOF

Cross-compile Exiv2 executable for Windows and build -win package.
EOF
    exit 1
fi

dir=$1
(
if [ ! -e $dir ] ; then
    echo "Error: Directory $dir doesn't exist."
    exit 1
fi
echo
echo ==========================================================================
echo Configure and cross-compile
echo
cd $dir
sudo make maintainer-clean
make config
./configure --disable-shared --disable-visibility --target=i586-mingw32msvc --host=i586-mingw32msvc --build=i586-linux --disable-nls --with-zlib=$HOME/mingw --with-libiconv-prefix=$HOME/mingw --with-expat=$HOME/mingw
make -j4
echo
echo ==========================================================================
echo Prepare package
echo
cd src
rel=`grep EXV_PACKAGE_VERSION ../include/exiv2/exv_msvc.h | sed 's/.*"\(.*\)"/\1/'`
tmpdir=exiv2-$rel-win
rm -rf $tmpdir
mkdir $tmpdir
cp ../bin/exiv2 $tmpdir/exiv2.exe
cd $tmpdir
strip exiv2.exe
cp ../../doc/cmd.txt .
cp ../../doc/cmdxmp.txt .
cp $HOME/mingw/dll/libexpat.dll .
todos cmd.txt cmdxmp.txt
$HOME/src/exiv2/exiv2-htmlman.sh ../exiv2.1
zip exiv2-$rel-win.zip exiv2.exe cmd.txt cmdxmp.txt exiv2-man.html libexpat.dll
echo
echo ==========================================================================
echo Cleanup
echo
mv exiv2-$rel-win.zip ../../..
cd ..
rm -rf $tmpdir
cd ../..
echo
echo ==========================================================================
echo Error-summary
echo
grep 'Error ' exiv2-buildwinexe.out | grep -v -e'BasicError ' -e'Error 1 (ignored)'

) 2>&1 | tee exiv2-buildwinexe.out 2>&1
