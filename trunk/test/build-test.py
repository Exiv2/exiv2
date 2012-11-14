#!/usr/bin/python

import os
import sys
import subprocess

##
#   process the version information dictionary
def platform(dict):
	for k in sorted(dict.keys()):
		v=dict[k]
		if type(v)==type([]):
			v='['+str(len(v))+']'
		print "%-20s -> %s" % (k,v)
## 
def apple(dict):
	platform(dict)
## 
def linux(dict):
	platform(dict)
## 
def windows(dict):
	platform(dict)
## 
def cygwin(dict):
	platform(dict)
## 
def mingw(dict):
	platform(dict)
## 
def unknown(dict):
	platform(dict)

##
# collect the version dictionary from exiv2
dict={}
output = subprocess.check_output( [ '../bin/exiv2', '-v', '-V' ])
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
	if len(dict[k])==1:
		dict[k]=dict[k][0]

##
# analyse the version dictionary
eval(dict['platform']+"(dict)")

# That's all Folks!
##