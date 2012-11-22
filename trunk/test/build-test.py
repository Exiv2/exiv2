#!/usr/bin/python

##
# build-test.py [--verbose]
#
# reads the output of exiv2 -v -V and inspects it for sanity
# primary test is to inspect the loaded libraries
# TODO: 1 report loaded libraries that we didn't expect
#       2 mingw support
##

import os
import sys
import subprocess

##
#   print error msg and terminate
def error(msg):
    print '***',msg,'***'
    sys.exit(1)

##
#   process the version information dictionary
def platform(dict):
    verbose=dict.has_key('verbose')
    for k in sorted(dict.keys()):
        v=dict[k]
        if type(v)==type([]):
            v='['+str(len(v))+']'
        if verbose:
            print "%-20s -> %s" % (k,v)

##
#   search dict for an array of libraries
def expect(dict,expects):
    verbose=dict.has_key('verbose')
    libs=dict['library']
    E={};
    for e in expects:
        e=e.lower().replace('-','.') # cygwin uses - in versioning
        e=e.split('.')[0]
        if verbose:
            print 'expect library',e
        E[e]=0
        
    for lib in libs:
        lib=lib.lower().replace('\\','/') # cygwin uses \ in pathnames
        lib=lib.replace('-','.')
        lib=os.path.basename(lib).split('.')[0]
        if E.has_key(lib):
            E[lib]=E[lib]+1
            if verbose:
                print 'found  library',lib
    for e in E.keys():
        if E[e]==0:
            error( "expected library '%s' not found" % e )

## 
def apple(dict):
    platform(dict)

    # which version of MacOS-X ?
    os_major=int(os.uname()[2].split('.')[0])
    os_minor=int(os.uname()[2].split('.')[1])
    NC=13;ML=12;LION=11;SL=10;LEO=9;

    if dict['bits'] != 64:
        print '*** expected 64 bit build ***'

    expects= [ 'libSystem.B.dylib'
             , 'libexpat.1.dylib'            
             , 'libz.1.dylib'
             , 'libiconv.2.dylib'
             , 'libstdc++.6.dylib'
             , 'libdyld.dylib'
             , 'libc++.1.dylib'
             ] ;
    if dict['dll']:
        expects.append('libexiv2.12.dylib')
    
    expect(dict,expects)

    ## Mountain lion dll build
    if os_major == ML and dict['dll']==1:
        expects= [ 'libexiv2.12.dylib'
                 , 'libSystem.B.dylib'
                 , 'libexpat.1.dylib'
                 , 'libz.1.dylib'
                 , 'libiconv.2.dylib'
                 , 'libstdc++.6.dylib'
                 , 'libdyld.dylib'
                 , 'libc++.1.dylib'
                 ] ;
        expect(dict,expects)

##
def linux(dict):
    platform(dict)
    expects = [ 'libdl.so.2'
              , 'libexiv2.so.12'
              , 'libstdc++.so.6'
              , 'libm.so.6'
              , 'libgcc_s.so.1'
              , 'libc.so.6'
              , 'libz.so.1'
              , 'libexpat.so.1'
              ]
    expect(dict,expects)
    
## 
def windows(dict):
    platform(dict)
    expects = [ 'ntdll.dll'
              , 'kernel32.dll'
              , 'KERNELBASE.dll'
              , 'PSAPI.DLL'
              ];
    expect(dict,expects)
    
    if dict['dll']==1:
        dll='d.dll' if dict['debug']==1 else '.dll' 
        expects = [ 'exiv2' + dll 
                  , 'zlib1' + dll 
                  , 'libexpat.dll'
                  ]
        # c run time libraries
        v=int(float(dict['version'])) # 7,8,9,10 etc
        if v in range(8,10):
            expects.append('msvcr%d0%s' % (v,dll) )
            expects.append('msvcp%d0%s' % (v,dll) )

        expect(dict,expects)

## 
def cygwin(dict):
    platform(dict)
    expects = [ 'ntdll.dll'
              , 'kernel32.dll'
              , 'KERNELBASE.dll'
              , 'cygexiv2-12.dll'
              , 'cygwin1.dll'
              , 'cyggcc_s-1.dll'
              , 'cygstdc++-6.dll'
              , 'PSAPI.DLL'
              , 'cygexpat-1.dll'
              , 'cygiconv-2.dll'
              , 'cygintl-8.dll'
              ];
    expect(dict,expects)

## 
def mingw(dict):
    platform(dict)
    error("can't test platform mingw")

## 
def unknown(dict):
    platform(dict)
    error("can't test platform unknown")

##
def runCommand(command):
    ##
    # don't use       check_output 
    # this is 2.7 feature.
    # Not available cygwin's default python 2.6.8 interpreter
    # result=subprocess.check_output( command.split(' '))
    result  =subprocess.Popen(command.split(' '), stdout=subprocess.PIPE).communicate()[0]
    # ensure lines are \n terminated
    return result.replace('\r\n', '\n').replace('\r', '\n')

##
def main(args):
    this=os.path.abspath(args[0])

    ##
    # find the exiv2 executable
    exiv2='exiv2'
    if os.environ.has_key('EXIV2_BINDIR'):
        exiv2=os.path.abspath(os.path.join(os.environ['EXIV2_BINDIR'],exiv2))
    else:
        exiv2=os.path.join(os.path.dirname(this),'../bin',exiv2)
        
    ##
    # collect the version dictionary from exiv2
    dict={}
    verbose=False
    for arg in args:
        if arg == '--verbose':
            verbose=True
            dict['verbose']=True
    
    if os.path.exists(exiv2):
        output = runCommand(exiv2 + ' -v -V')
        if verbose: 
            print output
        lines = output.split('\n')
        for l in lines:
            kv=l.split('=')
            if len(kv)==2:
                k=kv[0]
                v=kv[1]
                if not dict.has_key(k):
                    dict[k]=[]
                dict[k].append(v)

        # flatten value arrays of length 1
        for k in dict.keys():
            if type(dict[k])==type([]): 
                if len(dict[k])==1:
                    dict[k]=dict[k][0]
        # convert numeric strings to ints
        dict['dll'  ] = int(dict['dll'])
        dict['debug'] = int(dict['debug'])
        dict['bits' ] = int(dict['bits'])

        ##
        # analyse the version dictionary
        eval(dict['platform']+'(dict)')

        ##
        # report success!
        debug='debug';dll='dll';platform='platform';bits='bits'
        v='Release' if dict[debug]==0 else 'Debug  '
        d='DLL'     if dict[dll  ]==0 else '   '
        print "build %s %dbit %s%s looks good" % (dict[platform],dict[bits],v,d)
    else:
        error("exiv2 not found!")

if __name__ == '__main__':
    main(sys.argv)

# That's all Folks!
##
