#!/usr/bin/python3

import os
import sys
import optparse

global empty		   # set:  empty
global strings		   # dict: Visual Studio Strings
global uid			   # dict: UID of every projects
global project		   # dict: dependency sets for every project
global ignore		   # set: projects to ignore

empty=set([])
ignore=set(['webready'])

strings = {}
strings['Begin']='''Microsoft Visual Studio Solution File, Format Version 9.00
# Visual Studio 2005
'''
strings['End'] = ''

strings['globalBegin'] = 'Global'
strings['globalEnd'	 ] = 'EndGlobal'

strings['platforms'] = '''
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Win32 = Debug|Win32
		Debug|x64 = Debug|x64
		DebugDLL|Win32 = DebugDLL|Win32
		DebugDLL|x64 = DebugDLL|x64
		Release|Win32 = Release|Win32
		Release|x64 = Release|x64
		ReleaseDLL|Win32 = ReleaseDLL|Win32
		ReleaseDLL|x64 = ReleaseDLL|x64
	EndGlobalSection
'''
strings['postSolutionBegin'] =	 '	  GlobalSection(ProjectConfigurationPlatforms) = postSolution\n'
strings['postSolutionEnd'  ] =	 '	  EndGlobalSection\n'

strings['preSolution']		 = '''	  GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
'''

##
# Remove feature (not implemented yet)
# import sys
# import xml.dom.minidom
# global remove			 # dict: files to removed for an option (not implemented yet)
# remove={}
# remove['zlib'] = set([ 'pngimage.cpp' ])
#
# modify writeProject():
#	# Filter every .vcproj	foo\foo.vcproj -> foo\foo_configure.vcproj
#	xml	   = xml.dom.minidom.parse('foo\foo.vcproj')
#	pretty = xml.toprettyxml()
#	lines  = pretty.split('\n')
#	out	   = ""
#	for line in lines:
#		if not line.match( '<File RelativePath="..\..\src\pngimage.cpp"></File>' ):
#			out = out + line.remove('\n')
#		if not line.match( 'PreprocessorDefinitions=.*"' ):
#			line=replace('"$','EXV_MSVC_CONFIGURE"')
#			out = out + line.remove('\n')
#
#	open('foo\foo_configure.vcproj').write(out)
##

# for l in $(find . -name "*" -type d -maxdepth 1); do l=$(basename $l) ; if [ -e $l/$l.vcproj ]; then xmllint $l/$l.vcproj -pretty 1 | grep RootNamespace ; fi; done
uid = {}
uid['addmoddel'				] = '83914D93-57B3-4718-8A50-662C17C4AE8F'
uid['conntest'				] = 'E015DB8C-C463-4A6B-88EA-AFC671D84B5B'
uid['convert-test'			] = 'D802FE1E-7868-4034-92B8-00865E1CABAB'
uid['easyaccess-test'		] = '30E18D87-0147-4601-9ED2-4D5291645DB3'
uid['exifcomment'			] = 'D291B6EF-986B-4222-ADA6-38A2EC2A56CE'
uid['exifdata'				] = '1A091C40-C0F3-4405-B99E-CA60B9855D77'
uid['exifdata-test'			] = 'FEF9C19E-F774-4D8A-991C-A566C1B2E8B6'
uid['exifprint'				] = 'BFB98A96-7ABD-4F78-BA8B-2C2C257D74D8'
uid['exifvalue'				] = 'B85BDC90-013A-4D6B-B774-F2D7E0067DA6'
uid['exiv2'					] = '07293CAC-00DA-493E-90C9-5D010C2B1B53'
uid['exiv2json'				] = '4171BC51-2FDD-4BF5-BB80-1D9B2ACB03B9'
uid['geotag'				] = 'E3073076-4837-4DDB-89E5-5AC297C7481D'
uid['httptest'				] = '4AC6B957-1506-4EDB-BF6A-CF7CCE86EC1F'
uid['iotest'				] = '21F0CEB1-D850-4C29-88BF-1CE4171824E6'
uid['iptceasy'				] = '6860BB9B-2053-46CD-9E2D-EEC199D68982'
uid['iptcprint'				] = 'A7D22798-1262-4194-94A5-C636BCB68329'
uid['iptctest'				] = 'AD41F87C-242B-4B61-B767-A9879F0D5C04'
uid['key-test'				] = 'F11358FA-AA36-46E1-BA80-A17B8042BF9B'
uid['largeiptc-test'		] = '953404C9-B20A-4D17-8262-9D9AD1CDC5C1'
uid['libcurl'				] = 'F36F075A-880D-47BA-805F-C47850062121'
uid['libeay32'				] = '2C117585-9BA4-4BFE-8335-E3E9D51D4DA7'
uid['libexiv2'				] = '831EF580-92C8-4CA8-B0CE-3D906280A54D'
uid['libexpat'				] = '6C4C06A3-6F8F-4067-AA4C-D5F41E1FFF9A'
uid['libssh'				] = '729E4E11-3BBA-4306-B53C-8AEC45E70E10'
uid['metacopy'				] = 'AD231915-942F-4083-9671-85E26A5798B0'
uid['mmap-test'				] = '556CB4FC-33BB-4E67-AB0E-1865E67176A5'
uid['openssl'				] = '3D77E4F8-02EE-491F-B01C-EE8012CABA18'
uid['path-test'				] = 'E04D48BF-F529-4267-9311-908E94DF5A49'
uid['prevtest'				] = 'FC7120EC-BEB2-4CC3-9B90-B022F560E584'
uid['remotetest'			] = 'B25A3F96-68E8-4FD4-860E-2C33E539B892'
uid['ssleay32'				] = '6ABBF8BA-0A23-4C5C-8AEB-1B2577625DFA'
uid['stringto-test'			] = '9DCEE051-A07B-4C6B-B2BC-0814F0C323AA'
uid['taglist'				] = 'DBD630FC-0DA9-41EB-925D-70654D01F6FA'
uid['tiff-test'				] = 'C34B11A1-B707-46B2-8053-2FA236B369CF'
uid['utiltest'				] = '495BC686-DF50-4250-B469-9C6B7B33A4B8'
uid['werror-test'			] = '430C4512-CC73-4943-8CDF-71DEA573BD47'
uid['write-test'			] = '98A9F59D-FDFE-4B27-88FE-2625F1E7597F'
uid['write2-test'			] = 'E796088F-0EE6-4EC7-ABA0-8A18F54A4DD7'
uid['xmpparse'				] = '1708EFC1-414E-4712-80A5-813A6F38814C'
uid['xmpparser-test'		] = '85121FD0-01A1-49BA-B168-CC8D90F91A6F'
uid['xmpsample'				] = '45CA5427-4260-4F4A-86B0-FB7AE233D76B'
uid['xmpsdk'				] = '09877CF4-83B6-44FE-A2E2-629AA5C8093E'
uid['zlib'					] = '8308C68D-E12B-4C71-96F4-7137F6BEB654'

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
project['libexiv2'			] = set(['zlib'	   ,'libexpat','xmpsdk','libcurl','libeay32','ssleay32','openssl','libssh'])
project['geotag'			] = set(['libexiv2','libexpat'		   ])
# all others depend on libexiv2
for p in uid:
	if not p in project:
		project[p]=set(['libexiv2'])

##
# {831EF580-92C8-4CA8-B0CE-3D906280A54D}.Debug|Win32.ActiveCfg = Debug|Win32
def writeCompilationTable(s,uid):
	for t in [ 'Debug' , 'DebugDLL' , 'Release' , 'ReleaseDLL' ]:
		for p in [ 'Win32','x64']:
			for z in ['ActiveCfg','Build.0']:
				s.write(   '\t\t{%s}.%s|%s.%s = %s|%s\n' % (uid,t,p,z,t,p)	 );

##
# Project("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}") = "libexiv2", "libexiv2\libexiv2.vcproj", "{831EF580-92C8-4CA8-B0CE-3D906280A54D}"
#	ProjectSection(ProjectDependencies) = postProject
#		{09877CF4-83B6-44FE-A2E2-629AA5C8093E} = {09877CF4-83B6-44FE-A2E2-629AA5C8093E}
#		 ...
#	EndProjectSection
# EndProject
# '''
def writeProject(s,project,projects):
	UID='8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942' # grep 8BC9 exiv2.sln | head -1
	s.write( 'Project("{%s}") = "%s", "%s\%s.vcproj", "{%s}"\n' % (UID,project,project,project,uid[project]) )
	count=0
	out='\tProjectSection(ProjectDependencies) = postProject\n'
	for p in projects:
		if not p in ignore:
			count=count+1
			out = out + '\t\t{%s} = {%s}\n' % (uid[p],uid[p])
	out = out + '\tEndProjectSection\n'
	if count > 0:
		s.write(out)
	s.write('EndProject\n')

##
# not assert!
def cantHappen(condition,message):
	if condition:
		print(message)
		exit(1)

##
# if opt is false, remove lib from all projects
def cleanProjectSet(opt,lib):
	if not opt:
		ignore.add(lib);
		for p in project:
			project[p].add(lib)
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
	parser.add_option('-d', '--default'			, action='store_true' , dest='defolt'  ,help='default'		   ,default=True)

	##
	# no arguments, report and quit
	if len(sys.argv) == 1:
		parser.print_help()
		return

	##
	# parse and test for errors
	(options, args) = parser.parse_args()
	if options.defolt == True:
		options.xmp=True
		options.zlib=True
		options.expat=True
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

	cleanProjectSet(options.curl   ,'libcurl' )
	cleanProjectSet(options.openssl,'openssl' )
	cleanProjectSet(options.openssl,'libeay32')
	cleanProjectSet(options.openssl,'ssleay32')
	cleanProjectSet(options.xmp	   ,'xmpsdk'  )
	cleanProjectSet(options.expat  ,'libexpat')
	cleanProjectSet(options.zlib   ,'zlib'	  )

	##
	# open the strings file
	sln='exiv2-configure.sln'
	s = open(sln,'w') # 'a+')
	s.write(strings['Begin'])

	##
	# write projects
	print('Projects:',end=' ')
	for p in sorted(project):
		if not p in ignore:
			print(p,end=' ')
			projects=project[p]

			zap = False
			zap = zap | ((not options.webready) & ('webready' in projects))
			zap = zap | ((not options.zlib	  ) & ('zlib'	  in projects))
			zap = zap | ((not options.xmp	  ) & ('xmp'	  in projects))
			if zap:
				project[p]=0
			else:
				writeProject(s,p,projects)
	print('')

	##
	# write compilation table
	s.write(strings['globalBegin'])
	s.write(strings['platforms'	 ])
	s.write(strings['postSolutionBegin'	 ])
	for p in project:
		if type(project[p]) == type(empty):
			writeCompilationTable(s,uid[p])
	s.write(strings['postSolutionEnd'  ])
	s.write(strings['preSolution'	   ])
	s.write(strings['globalEnd'])

	##
	# finish
	s.write(strings['End'])
	s.close()
	print()
	print('MSVC 2005 Solution file created: ' + sln)
	print()

if __name__ == "__main__":
	main()

# That's all Folks
##
