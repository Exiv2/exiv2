#! /bin/sh

wget http://www.rottmerhusen.com/objektives/lensid/files/c-header/fmountlens4t.h

grep 'List of AF F-Mount lenses - Version .*' fmountlens4t.h > /tmp/newversion
grep 'List of AF F-Mount lenses - Version .*' nikonmn.cpp > /tmp/oldversion
diff /tmp/oldversion /tmp/newversion
if [ $? -eq 0 ] ; then
    echo "We already have the latest version."
    exit
fi

cat nikonmn.cpp | awk '
/8< - - - 8< do not remove this line >8 - - - >8/ {
    if (!s) {
        s = 1
        print
        system("cat fmountlens4t.h")
    }
    else {
        s = 0
    }
}
{
    if (s) next;
    print
}' > new-nikonmn.cpp

sed 's/struct {unsigned char lid,stps,focs,focl,aps,apl,lfw/static const struct {unsigned char lid,stps,focs,focl,aps,apl,lfw/' new-nikonmn.cpp > nikonmn.cpp

ver=`cat /tmp/newversion | awk '{print $9}'`
echo
echo "Updated Nikon Lens lookup table to v$ver of Robert Rottmerhusen's fmountlens list."
rm -f fmountlens4t.h /tmp/newversion /tmp/oldversion new-nikonmn.cpp
