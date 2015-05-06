#!/usr/bin/python3

import os
import sys
import optparse
import xml.dom.minidom
import datetime
import platform
import uuid

global empty		   # set:  empty
global uid			   # dict: UID of every projects
global project		   # dict: dependency sets for every project
global strings		   # dict: Visual Studio Strings
global ignore		   # set:  projects/directories to ignore

empty=set([])
ignore=set(['webready','expat','expat201','expat210','zlib123','zlib125','zlib127','tools','exiv2lib'])

##
# build dict:uid - hunt the tree for .vcproj files
# we should read the .vcproj and build the dependency set
uid = {}
for d in os.listdir('.'):
	if os.path.isdir(d) & (not d in ignore):
		for root, dirs, files in os.walk(d):
			for file in files:
				ext = ".vcproj"
				if file.endswith(ext) & (file.find('configure') < 0):
					uid[d]=str(uuid.uuid1())

##
# define project dependances
project = {}
# no dependancy
for p in ['libexpat','zlib']:
	project[p]=empty

##
# dependancies
project['libcurl'			] = set(['webready'					   ])
project['libssh'			] = set(['webready'					   ])
project['openssl'			] = set(['webready'					   ])
project['xmpparser-test'	] = set(['libexiv2','xmpsdk'		   ])
project['xmpsample'			] = set(['libexiv2','xmpsdk'		   ])
project['xmpsdk'			] = set(['libexpat'					   ])
project['libexiv2'			] = set(['zlib'	   ,'libexpat','xmpsdk'])
project['geotag'			] = set(['libexiv2','libexpat'		   ])
# all others depend on libexiv2
for p in uid:
	if not p in project:
		project[p]=set(['libexiv2'])

##
# Remove feature
remove={}
for p in uid:
	remove[p]=empty
remove['zlib'] = set([ 'pngimage' ])

##
# MSVC strings
strings = {}
strings['UID'] = str(uuid.uuid1())

strings['Begin']='''Microsoft Visual Studio Solution File, Format Version 9.00
# Visual Studio 2005
''' + '# Created by:%s at:%s using:%s on:%s in:%s\n' % (sys.argv[0], datetime.datetime.now().time(), platform.node(), platform.platform(), os.path.abspath('.'))
strings['End'] = ''

strings['globalBegin'] = 'Global'
strings['globalEnd'	 ] = 'EndGlobal'

strings['platforms'] = '''
\tGlobalSection(SolutionConfigurationPlatforms) = preSolution
\t\tDebug|Win32 = Debug|Win32
\t\tDebug|x64 = Debug|x64
\t\tDebugDLL|Win32 = DebugDLL|Win32
\t\tDebugDLL|x64 = DebugDLL|x64
\t\tRelease|Win32 = Release|Win32
\t\tRelease|x64 = Release|x64
\t\tReleaseDLL|Win32 = ReleaseDLL|Win32
\t\tReleaseDLL|x64 = ReleaseDLL|x64
\t\tEndGlobalSection
'''
strings['postSolutionBegin'] = '\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n'
strings['postSolutionEnd'  ] = '\tEndGlobalSection\n'

strings['postProjectBegin' ] = '\tProjectSection(ProjectDependencies) = postProject\n'
strings['postProjectEnd'   ] = '\tProjectSectionEnd\n'

strings['preSolution']		 = '''\tGlobalSection(SolutionProperties) = preSolution
\t\tHideSolutionNode = FALSE
\tEndGlobalSection
'''
##
# {831EF580-92C8-4CA8-B0CE-3D906280A54D}.Debug|Win32.ActiveCfg = Debug|Win32
def compilationForProject(uid):
	result = ''
	for t in [ 'Debug' , 'DebugDLL' , 'Release' , 'ReleaseDLL' ]:
		for p in [ 'Win32','x64']:
			for z in ['ActiveCfg','Build.0']:
				result = result +  '\t\t{%s}.%s|%s.%s = %s|%s\n' % (uid,t,p,z,t,p)
	return result;

def compilationTable():
	result = strings['postSolutionBegin']
	for p in project:
		if type(project[p]) == type(empty):
			result = result + compilationForProject(uid[p])
	return result + strings['postSolutionEnd']

##
# Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "libexiv2", "libexiv2\libexiv2_configure.vcproj", "{831EF580-92C8-4CA8-B0CE-3D906280A54D}"
#	ProjectSection(ProjectDependencies) = postProject
#		{09877CF4-83B6-44FE-A2E2-629AA5C8093E} = {09877CF4-83B6-44FE-A2E2-629AA5C8093E}
#		 ...
#	EndProjectSection
# EndProject
##
def projectRecord(project,projects):
	print( 'Project %-18s uid = %s' % (project, uid[project]) )

	UID	   = strings['UID']
	vcnew  = "%s\%s_configure.vcproj" % (project,project)  # write in DOS notation for Visual Studio
	result = 'Project("{%s}") = "%s", "%s", "{%s}"\n' % (UID,project,vcnew,uid[project])

	count  = 0
	out	   = strings['postProjectBegin']
	for p in projects:
		if not p in ignore:
			count=count+1
			out = out + '\t\t{%s} = {%s}\n' % (uid[p],uid[p])
	out = out + strings['postProjectEnd']
	if count > 0:
		result = result + out

	result = result + 'EndProject\n'
	return result

##
# Filter proj\proj.vcproj -> proj\proj_configure.vcproj
def writeVCproj(project,projects):
	vcold	 = os.path.join(project,("%s.vcproj"			% project) )  # path to old file
	vcnew	 = os.path.join(project,("%s_configure.vcproj"	% project) )  # path to new file
	xmllines = xml.dom.minidom.parse(vcold).toprettyxml().split('\n')
	out		 = ""
	for line in xmllines:
		# 1) Update the project GUID
		projectGUID='ProjectGUID="{'
		if line.find( projectGUID) > 0:
			start  = line.find(projectGUID) + len(projectGUID)
			olduid=line[start:line.find('}',start)-1]
			line=line.replace(olduid,uid[project]);
		# 2) Remove unwanted files (using remove[project] set)
		if line.find( 'File RelativePath=' ) >= 0:
			for r in remove[project]:
				for stub in r:
					if ( line.find(stub) > 0 ):
						line =''
		# 3) Add a preprocessor symbol to ask config.h to read exv_msvc_configure.h
		ppold=		'PreprocessorDefinitions="'
		ppnew=ppold+'EXV_MSVC_CONFIGURE;'
		if line.find( ppold ) > 0:
			line=line.replace(ppold,ppnew)
		if len(line)>0:
			out = out + line + '\n'
	open(vcnew,'w').write(out)

##
# not assert!
def cantHappen(condition,message):
	if condition:
		print(message)
		exit(1)

##
# if opt is true,  make remove set empty
# if opt is false, remove lib from all projects
def cleanProjectSet(opt,lib):
	if opt:
		remove[lib]=empty
	else:
		ignore.add(lib);
		for p in project:
			project[p].add(lib) # make sure we have something to remove!
			project[p].remove(lib)

def main():
	##
	# set up argument parser
	usage = "usage: %prog [options]+"
	parser = optparse.OptionParser(usage)
	parser = optparse.OptionParser()
	parser.add_option('-W', '--enable-webready' , action='store_true' , dest='webready',help='enable webready' ,default=False)
	parser.add_option('-w', '--disable-webready', action='store_false', dest='webready',help='disable webready')
	parser.add_option('-V', '--enable-video'	, action='store_true' , dest='video'   ,help='enable video'	   ,default=False)
	parser.add_option('-v', '--disable-video'	, action='store_false', dest='video'   ,help='disable video'   )
	parser.add_option('-X', '--enable-xmp'		, action='store_true' , dest='xmp'	   ,help='enable xmp'	   ,default=True)
	parser.add_option('-x', '--disable-xmp'		, action='store_false', dest='xmp'	   ,help='disable xmp'	   )
	parser.add_option('-Z', '--enable-zlib'		, action='store_true' , dest='zlib'	   ,help='enable zlib/png' ,default=True)
	parser.add_option('-z', '--disable-zlib'	, action='store_false', dest='zlib'	   ,help='disable zlib/png')
	parser.add_option('-C', '--enable-curl'		, action='store_true' , dest='curl'	   ,help='enable curl'	   ,default=False)
	parser.add_option('-c', '--disable-curl'	, action='store_false', dest='curl'	   ,help='disable curl'	   )
	parser.add_option('-S', '--enable-ssh'		, action='store_true' , dest='ssh'	   ,help='enable ssh'	   ,default=False)
	parser.add_option('-s', '--disable-ssh'		, action='store_false', dest='ssh'	   ,help='disable ssh'	   )
	parser.add_option('-O', '--enable-openssl'	, action='store_true' , dest='openssl' ,help='enable openssl'  ,default=False)
	parser.add_option('-o', '--disable-openssl' , action='store_false', dest='openssl' ,help='disable openssl' )
	parser.add_option('-E', '--enable-expat'	, action='store_true' , dest='expat'   ,help='enable expat'	   ,default=True)
	parser.add_option('-e', '--disable-expat'	, action='store_false', dest='expat'   ,help='disable expat'   )
	parser.add_option('-d', '--default'			, action='store_true' , dest='defolt'  ,help='default'		   ,default=False)

	##
	# no arguments, report and quit
	if len(sys.argv) == 1:
		parser.print_help()
		return

	##
	# parse and test for errors
	(options, args) = parser.parse_args()
	cantHappen(options.curl	   & (not options.webready),'cannot use curl without webready'	 );
	cantHappen(options.openssl & (not options.webready),'cannot use openssl without webready');
	cantHappen(options.ssh	   & (not options.webready),'cannot use libssh	without webready');
	cantHappen(options.xmp	   & (not options.expat	  ),'cannot use xmp without expat'		 );

	##
	# print options
	fmt='%-20s\t->\t%s'
	print(fmt % ('Option' , 'Value'))
	print(fmt % ('------' , '-----'))
	for o, v in sorted(options.__dict__.items()):
		if o != 'defolt':
			print(fmt % (o, v))
	print()

	cleanProjectSet(options.curl   ,'libcurl' )
	cleanProjectSet(options.openssl,'openssl' )
	cleanProjectSet(options.openssl,'libeay32')
	cleanProjectSet(options.openssl,'ssleay32')
	cleanProjectSet(options.xmp	   ,'xmpsdk'  )
	cleanProjectSet(options.expat  ,'libexpat')
	cleanProjectSet(options.zlib   ,'zlib'	  )

	##
	# write solution file
	sln='exiv2-configure.sln'
	s = open(sln,'w')
	s.write(strings['Begin'])

	# write projects
	for p in sorted(project):
		projects=project[p]

		zap = False
		zap = zap | ((not options.webready) & ('webready' in projects))
		zap = zap | ((not options.zlib	  ) & ('zlib'	  in projects))
		zap = zap | ((not options.xmp	  ) & ('xmp'	  in projects))
		if zap:
			project.pop(p)
		else:
			s.write(projectRecord(p,projects))

	# write compilation table
	s.write(strings['globalBegin'])
	s.write(strings['platforms'	 ])
	s.write(compilationTable())
	s.write(strings['preSolution'])
	s.write(strings['globalEnd'	 ])

	# finish
	s.write(strings['End'])
	s.close()

	##
	# write project files
	for p in sorted(project):
		writeVCproj(p,project[p])

	print()
	print('MSVC 2005 Solution file created: ' + sln)
	print()

if __name__ == "__main__":
	main()

# That's all Folks
##
