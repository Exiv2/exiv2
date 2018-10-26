#!/usr/local/bin/python3

##
# Syntax: categorize.py builds-directory
# Rebuild directories Latest, Date, Platform in builds-directory
# Caution: builds are expected in builds/all
##

import os
import sys
import shutil
import time
import datetime
import re

global TSformat
TSformat  = '%Y:%m:%d_%H:%M:%S'
##
# get difference in seconds between two time stamps
def tsdiff(ts1,ts2):
    global TSformat
    # print(time.mktime(time.strptime(re.sub('\.[0-9]*','',ts1),TSformat)))
    return abs( time.mktime(time.strptime(re.sub('\.[0-9]*','',ts1),TSformat)) \
              - time.mktime(time.strptime(re.sub('\.[0-9]*','',ts2),TSformat)) \
              )

def year_month(ts):
    global TSformat
    t=time.strptime(re.sub('\.[0-9]*','',ts),TSformat)
    return str(t.tm_year)+':'+str(t.tm_mon)

def year_week(ts):
    global TSformat
    t=time.strptime(re.sub('\.[0-9]*','',ts),TSformat)
    w=datetime.date(t.tm_year, t.tm_mon, t.tm_mday).isocalendar()[1]
    return str(t.tm_year)+':'+str(w)

##
#
def mkdir(P):
    if not os.path.exists(P):
        os.makedirs(P)
##

##
#
def rmdir_mkdir(P):
    if os.path.exists(P):
        shutil.rmtree(P,ignore_errors=True)
    mkdir(P)
##

##
#
def visitfile(file,path,myData):
    """visitfile - use by the directory walker"""
    # print("file,path = " , file , " -> ",path)
    global latest,sweep
    global Platform, Version, Date, Monthly, Weekly

    home = myData['home' ]

    if os.path.isfile(path) and file.find('-') != -1:
        # parse exiv2-0.27.0.1-CYGWIN-2018:10:19_01:13:00.tar.gz
        # print('file = ' + file)
        splits = file.split('-')
        if file.find('MinGW') != -1:
            platform = splits[2]+'-'+splits[3]
            date     = splits[4].split('.')[0]
            version  = splits[1]
        else:
            platform = splits[2]
            date     = splits[3].split('.')[0]
            version  = splits[1]
        monthly= year_month(date)
        weekly = year_week(date)
        # print( "%s -> platform = %s version = %s date = %s " % (file , platform,version,date) )

        myData ['platform'].add(platform)
        myData ['date'    ].add(date    )
        myData ['version' ].add(version )
        myData ['weekly'  ].add(monthly )
        myData ['monthly' ].add(weekly  )

        if sweep==1:
            Links = { Platform:platform, Date:date, Version:version, Monthly:monthly, Weekly:weekly }
            for L in Links:
                # path    : all/exiv2-0.27.0.1-MSVC-2018:10:20_06:33:13.zip
                # file    : exiv2-0.27.0.1-MSVC-2018:10:20_06:33:13.zip
                # L       : /Users/rmills/Jenkins/builds/Platform
                # Links[L]: MSVC
                # Link    : /Users/rmills/Jenkins/builds/Platform/MSVC
                # file    : exiv2-0.27.0.1-MSVC-2018:10:20_06:33:13.zip
                # ls     -s all/exiv2-0.27.0.1-MSVC-2018:10:20_06:33:13.zip \
                #           Platform/MSVC/exiv2-0.27.0.1-MSVC-2018:10:20_06:33:13.zip
                Link=os.path.join(L,Links[L])
                mkdir(Link)
                os.symlink(path,os.path.join(Link,file))
                if ( 1 == 2 ):
                    print('path    : %s' % path    )
                    print('file    : %s' % file    )
                    print('L       : %s' % L       )
                    print('Links[L]: %s' % Links[L])
                    print('Link    : %s' % Link    )
                    print('file    : %s' % file    )
                    print('ls -s %s %s'  % (path,os.path.join(Link,file))   )
                    exit(0)

            # print('latest? file = %s date = %s latest = %s DIFF = %d' % (file,date,latest_date,0)) # tsdiff(date,latest) ))
            if tsdiff(date,latest) < 5:
                # print('linking file = %s path = %s Latest = %s' % (file,path,Latest ))
                os.symlink(path,os.path.join(Latest,file))
##

##
#
def searcher(home):
    """searcher - walk the tree"""
    global latest,sweep
    global Platform, Version, Latest, Date, Monthly, Weekly

    print("home = " , home)

    # recursive search
    myData = {}
    myData ['home'    ]=os.path.abspath(home)
    myData ['platform']=set()
    myData ['version' ]=set()
    myData ['date'    ]=set()
    myData ['monthly' ]=set()
    myData ['weekly'  ]=set()

    for sweep in range(0,1+1):
        # sweep == 0 : ignore links and populate myData
        # sweep == 1 : create the new links
        myData ['sweep'   ]=sweep

        for dir, subdirs, files in os.walk(os.path.join(home,'all')):
            for file in files:
                path = os.path.join(dir, file)
                visitfile(file,path,myData)

        platform = list(myData ['platform'])
        platform.sort()
        # print(platform)

        date = list(myData ['date'])
        date.sort()
        date.reverse()
        latest=date[0]
        # print(date)
        # print("setting latest = ",latest)

        version = list(myData ['version'])
        version.sort()
        # print(version)

        weekly = list(myData['weekly'])
        weekly.sort()
        # print(weekly)

        monthly = list(myData['monthly'])
        monthly.sort()

        if sweep==0:
            Platform = os.path.join(home,'Platform')
            Date     = os.path.join(home,'Date'    )
            Latest   = os.path.join(home,'Latest'  )
            Version  = os.path.join(home,"Version" )
            Weekly   = os.path.join(home,"Weekly"  )
            Monthly  = os.path.join(home,"Monthly" )
            Source   = os.path.join(home,"Source"  )

            rmdir_mkdir(Platform)
            rmdir_mkdir(Date    )
            rmdir_mkdir(Latest  )
            rmdir_mkdir(Version )
            rmdir_mkdir(Weekly  )
            rmdir_mkdir(Monthly )

    print("Latest: ",latest)
    # ln -s Platform/Source Source
    if ( os.path.exists(Source) ):
        os.unlink (Source)
    os.symlink(os.path.join(Platform,'Source'),Source )

    # print(myData ['date'    ])
##

def syntax(program):
    print('syntax: %s path-to-builds' % (program) )

##
#
def main(argv):
    """main - main program of course"""

    argc = len(argv)
    if argc < 2:
        syntax(argv[0])
        return

    searcher(argv[1])

##
#
if __name__ == '__main__':
    main(sys.argv)
##

# That's all Folks!
##
