#!/bin/bash

##
# jenkins_daily.sh
#
#  environment variables (all optional)
#    JENKINS   : URL of jenkins server. Default http://exiv2.dyndns.org:8080
##
if [ -z "$JENKINS" ]; then export JENKINS=http://exiv2.dyndns.org:8080; fi

export EXIV2=$(cygpath -aw .)
rm -rf $PWD/../build
mkdir  $PWD/../build
PATH=$PATH:/cygdrive/c/Windows/System32:/cygdrive/c/gnu/exiv2/trunk/contrib/
cmd.exe /c "cd $(cygpath -aw ./../build) && vcvars 2015 64 && set && cmakeBuild --rebuild --test"
result=$?

set -v
# That's all Folks!
##
exit $result
