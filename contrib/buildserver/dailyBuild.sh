#!/bin/bash

dow=$(date  '+%w') # 0..6   day of the week
dom=$(date  '+%d') # 1..31  day of the month

a=$(/usr/local/bin/svn info .. | grep '^Last Changed Rev' | cut -f 2 -d':' | tr -d ' ')
b=$(/usr/local/bin/svn update . | grep ^At       | cut '-d ' -f 3 | cut -d. -f 1 | tr -d ' ')


if [ "$a"xx != "$b"yy -o "$dow" == 1 -o "$dom" == 1 ]; then # build if changed or monday or 1st of month 
  b=$(/usr/local/bin/svn info .   | grep ^Revision | cut '-d:' -f 2                | tr -d ' ')
  echo ==================================
  echo 'updated from svn:' $a 'to svn:' $b
  echo ==================================
  ssh rmills@rmillsmm                                            'cd ~/gnu/exiv2/buildserver ; /usr/local/bin/svn update . ; rm -rf build ; contrib/buildserver/dailyCMake.sh'
  ssh rmills@rmillsmm-kubuntu                                    'cd ~/gnu/exiv2/buildserver ; /usr/local/bin/svn update . ; rm -rf build ; contrib/buildserver/dailyCMake.sh'
  ssh rmills@rmillsmm-w7                                         'cd ~/gnu/exiv2/buildserver ; /usr/local/bin/svn update . ; rm -rf build ; contrib/buildserver/dailyCMake.sh'
  ssh rmills@rmillsmm-w7 'export PLATFORM=msvc                   ;cd ~/gnu/exiv2/buildserver ; /usr/local/bin/svn update . ; rm -rf build ; contrib/buildserver/dailyCMake.sh'
  ssh rmills@rmillsmm-w7 'export PLATFORM=mingw;export win32=true;cd ~/gnu/exiv2/buildserver ; /usr/local/bin/svn update . ; rm -rf build ; contrib/buildserver/dailyCMake.sh'

  ##
  # test the delivery
  date=$(date '+%Y-%m-%d+%H-%M-%S')
  svn=$(/usr/local/bin/svn info .. | grep '^Last Changed Rev' | cut -f 2 -d':' | tr -d ' ')
  (  
    ssh rmills@rmillsmm                                            'cd ~/gnu/exiv2/buildserver ; contrib/buildserver/dailyTest.sh'
    ssh rmills@rmillsmm-kubuntu                                    'cd ~/gnu/exiv2/buildserver ; contrib/buildserver/dailyTest.sh'
    ssh rmills@rmillsmm-w7                                         'cd ~/gnu/exiv2/buildserver ; contrib/buildserver/dailyTest.sh'
    ssh rmills@rmillsmm-w7 'export PLATFORM=msvc;                   cd ~/gnu/exiv2/buildserver ; contrib/buildserver/dailyTest.sh'
    ssh rmills@rmillsmm-w7 'export PLATFORM=mingw;export win32=true;cd ~/gnu/exiv2/buildserver ; contrib/buildserver/dailyTest.sh'
  ) | tr -d $'\r' | tee "/mmHD/Users/Shared/Jenkins/Home/userContent/builds/Daily/test-svn-${svn}-date-${date}.txt" 
  ##
  # categorize the builds
  ssh rmills@rmillsmm         '~/gnu/exiv2/buildserver/contrib/buildserver/categorize.sh /mmHD/Users/Shared/Jenkins/Home/userContent/builds'

else 

  echo ==================================
  echo 'no build needed svn = ' $a
  echo ==================================
  
fi

# That's all Folks!
##
