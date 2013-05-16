#! /bin/sh

if [ $# -eq 0 ] ; then
    echo "Usage: `basename $0` manpage"
    cat <<EOF

Convert manpage to html and make external links point to linux.die.net.
EOF
    exit 1
fi

cat $1 | sed '/^JPEG/ i\
' | man2html -Hlinux.die.net -M/man -p | sed -e's,\(HREF=.*\)+,\1/,' > exiv2-man.html
