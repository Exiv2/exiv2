#!/usr/bin/python

##
# build-test.py [--verbose]
#
# reads the output of exiv2 -v -V and inspects it for sanity
# inspect run-library libraries platform/compiler/{debug|release}{shared|static}
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
#   run a command and return output string
def runCommand(command):
    ##
    # don't use check_output 
    #           this is 2.7 feature.
    #           not available on cygwin's default python 2.6.8 interpreter
    #           result=subprocess.check_output( command.split(' '))
    ##

    result  =subprocess.Popen(command.split(' '), stdout=subprocess.PIPE).communicate()[0]

    # ensure lines are \n terminated (remove \r bytes)
    return result.replace('\r\n', '\n').replace('\r', '\n')

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
        e=e.lower().replace('-','.')      # cygwin uses '-' in versioning
        e=e.split('.')[0]
        if verbose:
            print 'expect library',e
        E[e]=0
        
    for lib in libs:
        lib=lib.lower().replace('\\','/') # cygwin uses '\' in pathnames
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
    expects= [ 'libSystem.B.dylib'
             , 'libexpat.1.dylib'
             , 'libz.1.dylib'
             , 'libiconv.2.dylib'
             , 'libdyld.dylib'
             , 'libc++.1.dylib'
             # , 'libstdc++.6.dylib' # I suspect this is only with GCC (Xcode4 and earlier)
             ] ;

    # which version of macOS ?
    os_major=int(os.uname()[2].split('.')[0])
    os_minor=int(os.uname()[2].split('.')[1])
    NC=13;ML=12;LION=11;SL=10;LEO=9;

    if dict['dll']:
        expects.append('libexiv2.12.dylib')
    
        ## Mountain Lion
        if os_major==ML and dict['dll']:
            expects.append('libexiv2.12.dylib')
            expects.append('libSystem.B.dylib')
            expects.append('libexpat.1.dylib')
            expects.append('libz.1.dylib')
            expects.append('libiconv.2.dylib')
            expects.append('libstdc++.6.dylib')
            expects.append('libdyld.dylib')
            expects.append('libc++.1.dylib')

    expect(dict,expects)

    expect_bits = 32 if os_major==LEO else 64
    if expect_bits != dict['bits']:
        error('*** expected %d bit build ***' % expect_bits)

##
def linux(dict):
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
    expects = [ 'ntdll.dll'
              , 'kernel32.dll'
              , 'kernelbase.dll'
              , 'psapi.dll'
              ];
    if dict['dll']:
        dll='d.dll' if dict['debug'] else '.dll' 
        expects.append( 'exiv2' + dll )
        expects.append( 'zlib1' + dll ) 
        expects.append('libexpat.dll' )

        # c run time libraries
        # 2003=71, 2005=80, 2008=90, 2010=100
        v=int( float(dict['version'])*10 )
        expects.append('msvcr%d%s' % (v,dll) )
        expects.append('msvcp%d%s' % (v,dll) )

    expect(dict,expects)

## 
def cygwin(dict):
    expects = [ 'ntdll.dll'
              , 'kernel32.dll'
              , 'kernelbase.dll'
              , 'cygexiv2-12.dll'
              , 'cygwin1.dll'
              , 'cyggcc_s-1.dll'
              , 'cygstdc++-6.dll'
              , 'psapi.dll'
              , 'cygexpat-1.dll'
              , 'cygiconv-2.dll'
              , 'cygintl-8.dll'
              ];
    expect(dict,expects)

## 
def mingw(dict):
    error("can't test platform mingw")

## 
def unknown(dict):
    error("can't test platform unknown")

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
        debug='debug'
        dll='dll'
        bits='bits'
        dict[dll  ] = int(dict[dll])
        dict[debug] = int(dict[debug])
        dict[bits ] = int(dict[bits])

        ##
        # analyse the version dictionary
        platform(dict)
        eval(dict['platform']+'(dict)')

        ##
        # report
        v='Release' if dict[debug]==0 else 'Debug'
        d='DLL'     if dict[dll  ]==0 else ''
        print "build %dbit %-8s  %-12s looks good" % (dict[bits],dict['platform'],v+d)
    else:
        error("exiv2 not found!")

if __name__ == '__main__':
    main(sys.argv)

# That's all Folks!
##