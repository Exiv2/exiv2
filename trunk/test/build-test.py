#!/usr/bin/python

import os
import sys
import subprocess

##
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
#
def expect(dict,expects):
	libs=dict['library']
 	E={};
 	for e in expects:
		e=e.split('.')[0]
 		E[e]=0
 	for lib in libs:
 		lib=os.path.basename(lib).split('.')[0]
 		if E.has_key(lib):
 			E[lib]=E[lib]+1
 	for e in E.keys():
 		if E[e]==0:
 			error( "expected library '%s' not found" % e )

## 
#
def apple(dict):
	platform(dict)

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
		expects=[ 'libexiv2.12.dylib'
				];
		expect(dict,expects)

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
#  
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
	error("can't test platform")
## 
def cygwin(dict):
	platform(dict)
	error("can't test platform")
## 
def mingw(dict):
	platform(dict)
	error("can't test platform")
## 
def unknown(dict):
	platform(dict)
	error("can't test platform")

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
	for arg in args:
		if arg == '--verbose':
			dict['verbose'] = True
	
	if os.path.exists(exiv2):
		output = subprocess.check_output( [ exiv2, '-v', '-V' ])
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
		dict['dll']=int(dict['dll'])
		dict['debug']=int(dict['debug'])
		dict['bits']=int(dict['bits'])

		##
		# analyse the version dictionary
		eval(dict['platform']+"(dict)")
	else:
		error("exiv2 not found!")

if __name__ == '__main__':
	main(sys.argv)
	print "build looks good"

# That's all Folks!
##