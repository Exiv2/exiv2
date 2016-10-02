#!/usr/bin/env python3

##
# categorize.py directory
# Rebuild directories Latest, SVN, Date, Platform in builds directory
##

import os
import sys
import shutil

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
	sweep= myData['sweep']
	home = myData['home' ]
	
	if not os.path.islink(path):
		try:
			platform = file.index('-')
			svn		 = file.index('-svn-')
			date	 = file.index('-date-')
			plus	 = file.index('+')
			platform = file[	 0:platform]
			svn		 = file[ svn+5:date]
			date	 = file[date+6:plus]
			# print( "%s -> %s %s %s " % (filen , platform,svn,date) )

			myData ['platform'].add(platform)
			myData ['date'	  ].add(date	)
			myData ['svn'	  ].add(svn		)

			if sweep==1:
				Platform = os.path.join(home,'Platform')
				Date	 = os.path.join(home,'Date'	   )
				SVN		 = os.path.join(home,'SVN'	   )
				Latest	 = os.path.join(home,'Latest'  )

				P=os.path.join(Platform,platform)
				mkdir(P)
				os.symlink(path,os.path.join(P,file))

				D=os.path.join(Date,date)
				mkdir(D)
				os.symlink(path,os.path.join(D,file))

				S=os.path.join(SVN,svn)
				mkdir(S)
				os.symlink(path,os.path.join(S,file))
				
				if svn==myData['latest_svn'] and date == myData['latest_date']:
					os.symlink(path,os.path.join(Latest,file))

		except:
			pass
##

##
#
def searcher(home):
	"""searcher - walk the tree"""
	print("home = " , home)

	# recursive search
	myData = {}
	myData ['home'	  ]=os.path.abspath(home)
	myData ['platform']=set()
	myData ['svn'	  ]=set()
	myData ['date'	  ]=set()

	for sweep in range(0,1+1):
		# sweep == 0 : ignore links and populate myData
		# sweep == 1 : create the new links
		myData ['sweep'	  ]=sweep

		for dir, subdirs, files in os.walk(home):
			for file in files:
				path = os.path.join(dir, file)
				visitfile(file,path,myData)

		platform = list(myData ['platform'])
		platform.sort()
		# print(platform)

		date = list(myData ['date'])
		date.sort()
		# print(date)

		svn = list(myData ['svn'])
		svn.sort()
		# print(svn)

		myData['latest_svn' ]= svn[len(svn )-1]
		myData['latest_date']=date[len(date)-1]

		if sweep==0:
			Platform = os.path.join(home,'Platform')
			Date	 = os.path.join(home,'Date'	   )
			SVN		 = os.path.join(home,'SVN'	   )
			Latest	 = os.path.join(home,'Latest'  )

			remake(Platform)
			remake(Date)
			remake(SVN)
			remake(Latest)


	# print("Latest: ",myData['latest_svn' ],myData['latest_date' ])

	# print(myData ['date'	  ])
	# print(myData ['svn'	  ])
##

##
#
def main(argv):
	"""main - main program of course"""

	argc = len(argv)
	if argc < 2:
		syntax()
		return

	searcher(argv[1])
	
##
#
if __name__ == '__main__':
	main(sys.argv)
##

# That's all Folks!
##
