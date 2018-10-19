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

##
# get difference in seconds between two time stamps
def tsdiff(ts1,ts2):
    TSformat  = '%Y:%m:%d_%H:%M:%S' #
    # print(time.mktime(time.strptime(re.sub('\.[0-9]*','',ts1),TSformat)))
    return abs( time.mktime(time.strptime(re.sub('\.[0-9]*','',ts1),TSformat)) \
              - time.mktime(time.strptime(re.sub('\.[0-9]*','',ts2),TSformat)) \
              )

##
#
def mkdir(P):
    if not os.path.exists(P):
        os.makedirs(P)
##

##
#
def remake(P):
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
    home = myData['home' ]

    if os.path.isfile(path):
        try:
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
            # print( "%s -> platform = %s version = %s date = %s " % (file , platform,version,date) )

            myData ['platform'].add(platform)
            myData ['date'    ].add(date    )
            myData ['version' ].add(version )

            if sweep==1:
                Platform = os.path.join(home,'Platform')
                Date     = os.path.join(home,'Date'    )
                Version  = os.path.join(home,'Version' )
                Latest   = os.path.join(home,'Latest'  )

                P=os.path.join(Platform,platform)
                mkdir(P)
                os.symlink(path,os.path.join(P,file))

                D=os.path.join(Date,date)
                mkdir(D)
                os.symlink(path,os.path.join(D,file))

                V=os.path.join(Version,version)
                mkdir(V)
                os.symlink(path,os.path.join(V,file))

                # print('latest? file = %s date = %s latest = %s DIFF = %d' % (file,date,latest_date,0)) # tsdiff(date,latest) ))
                if tsdiff(date,latest) < 5:
                    # print('linking file = %s path = %s Latest = %s' % (file,path,Latest ))
                    os.symlink(path,os.path.join(Latest,file))

        except:
            pass
##

##
#
def searcher(home):
    """searcher - walk the tree"""
    global latest,sweep

    print("home = " , home)

    # recursive search
    myData = {}
    myData ['home'    ]=os.path.abspath(home)
    myData ['platform']=set()
    myData ['version' ]=set()
    myData ['date'    ]=set()

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

        if sweep==0:
            Platform = os.path.join(home,'Platform')
            Date     = os.path.join(home,'Date'    )
            Latest   = os.path.join(home,'Latest'  )
            Version  = os.path.join(home,"Version" )

            remake(Platform)
            remake(Date)
            remake(Version)
            remake(Latest)

    print("Latest: ",latest)

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
