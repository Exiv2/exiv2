#! /bin/sh
# Fix < and > signs and replace bugnumbers in the file given 
# (ChangeLog) with links to Redmine issues.

if [ $# -ne 1 ] ; then
    echo Usage: buglinks.sh ChangeLog
    exit 1
fi

file=$1

sed -e 's/\&/\&amp;/g' -e 's/</\&lt;/g' -e 's/>/\&gt;/g' $file > $file.tmp
mv $file.tmp $file

sed -e 's,\#\([1-9][0-9][0-9]\),<a title="bug \1" href="http://dev.exiv2.org/issues/\1">#\1</a>,g; s,\([0-9][0-9][0-9][0-9][0-9][0-9][0-9]\),<a title="bug \1" href="http://dev.exiv2.org/issues/\1">\1</a>,g' $file > $file.tmp
mv $file.tmp $file
