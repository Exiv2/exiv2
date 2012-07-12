// ***************************************************************** -*- C++ -*-
// geotag.cpp, $Rev: 2286 $
// Sample program to read gpx files and update the images with GPS tags
// Work in progress - doesn't do anything yet!

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

#include <sys/types.h>
#include <stdio.h>
#include "expat.h"

#include <vector>
#include <string>

typedef std::vector<std::string> strings_t ;

#ifndef  lengthof
#define  lengthof(x) (sizeof(*x)/sizeof(x))
#endif

#ifdef  _MSC_VER

#include <windows.h>
#if     _MSC_VER < 1400
#define strcpy_s(d,l,s) strcpy(d,s)
#define strcat_s(d,l,s) strcat(d,s)
#endif

#else
#include <dirent.h>
#endif

enum
{	typeUnknown   = 0
,	typeDirectory = 1
,	typeImage	  = 2
,	typeXML		  = 3
,	typeFile      = 4
};
static const char* types[]  = { "unknown" , "directory" ,  "image" , "xml" ,"file" };

class UserData
{
public:
	         UserData() : indent(0),count(0) {};
	virtual ~UserData() {} ;

//  public data members
	int    indent;
	size_t count ;
};
 	
static void startElement(void* userData, const char* name, const char** /*atts*/ )
{
	UserData* me = (UserData*) userData;
	for ( int i = 0 ; i < me->indent ; i++ ) printf(" ");
	printf("begin %s\n",name);
	me->count++  ;
	me->indent++ ;
}
static void endElement(void* userData, const char* name)
{
	UserData* me = (UserData*) userData;
	me->indent-- ;
	for ( int i = 0 ; i < me->indent ; i++ ) printf(" ");
	printf("end %s\n",name);
}


bool readDir(char* path,strings_t& paths)
{
	paths.empty();
	bool bResult = false;
#ifdef _MSC_VER
	DWORD attrs    =  GetFileAttributes(path);
	bool  bOKAttrs =  attrs != INVALID_FILE_ATTRIBUTES;
	bool  bIsDir   = (attrs  & FILE_ATTRIBUTE_DIRECTORY) ? true : false ;

	if( bOKAttrs && bIsDir ) {
		bResult = true ;

		char     search[_MAX_PATH+10];
		strcpy_s(search,_MAX_PATH,path);
		strcat_s(search,_MAX_PATH,"\\*");

		WIN32_FIND_DATA ffd;
		HANDLE  hFind = FindFirstFile(search, &ffd);
		BOOL    bGo = hFind != INVALID_HANDLE_VALUE;

		if ( bGo ) {
			while ( bGo ) {
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					// _tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
				}
				else
				{
					paths.push_back( std::string(ffd.cFileName));
					printf("-> %s\n",ffd.cFileName);
				}
				bGo = FindNextFile(hFind, &ffd) != 0;
			} 
			CloseHandle(hFind);
		}
	}
#else
	DIR*	dir = opendir (path);
	if (dir != NULL)
	{
		bResult = true;
		struct dirent*	ent;

		// print all the files and directories within directory
		while ((ent = readdir (dir)) != NULL)
		{
			printf ("%s\n", ent->d_name);
			paths.push_back(std::string(ent->d_name)) ;
		}
		closedir (dir);
	}
#endif
	return bResult ;
}

bool readXML(char* path,size_t& count)
{
	FILE*		f		= fopen(path,"r");
	XML_Parser	parser	= XML_ParserCreate(NULL);
	bool bResult		= f && parser ;
	if ( bResult ) {
		char   buffer[8*1024];
		UserData me ;

		XML_SetUserData(parser, &me);
		XML_SetElementHandler(parser, startElement, endElement);
		
		// a little sip at the data
		size_t len = fread(buffer,1,sizeof(buffer),f);
		const char* lead   = "<?xml" ;
		bResult = strncmp(lead,buffer,strlen(lead))==0;

		// swallow it
		if ( bResult ) {
			bResult = XML_Parse(parser, buffer,(int)len, len == 0 ) == XML_STATUS_OK; 
		}

		// drink the rest of the file
		while ( bResult && !feof(f) ) {
			len = fread(buffer,1,sizeof(buffer),f);
			bResult = XML_Parse(parser, buffer,(int)len, len == 0 ) == XML_STATUS_OK; 
		};
		count = me.count ;
	}

	if ( f		) fclose(f);
	if ( parser ) XML_ParserFree(parser);

	return bResult ;
}

bool readFile(char* path,size_t& count)
{
	FILE*		f		= fopen(path,"r");
	bool bResult		= f ? true : false;
	if ( bResult ) {
		fseek(f,0L,SEEK_END);
		count = ftell(f);
	}
	if ( f ) fclose(f) ;

	return bResult ;
}


int main(int argc, char* const argv[])
{
    if ( argc < 2 ) {
        std::cout << "Usage: " << argv[0] << " arg+\n";
        return 1;
    }
	
	for ( int i = 1 ; i < argc ; i++ ) {
		char* arg = argv[i];
		size_t count = 0 ;
		int fileType = typeUnknown ;

		if ( fileType == typeUnknown ) {
			if ( readXML(arg,count) ) {
				if ( count ) fileType = typeXML ;
			}
		}

		if ( fileType == typeUnknown ) {
			strings_t files ;
			if ( readDir(arg,files) ) {
				fileType = typeDirectory ;
				count = files.size();
			}
		}
		
		if ( fileType == typeUnknown ) try {
			Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(arg);
			if ( image.get() ) {
				Exiv2::ExifData &exifData = image->exifData();
				image->readMetadata();
				if ( !exifData.empty() ) {
					fileType = typeImage ;
					count = exifData.count();
				}
			} 
		} catch (Exiv2::Error& ) {};

		if ( fileType == typeUnknown ) {
			if ( readFile(arg,count) ) {
				if ( count ) fileType = typeFile ;
			}
		} 

		printf("arg:%s type:%s count:%d\n",arg,types[fileType],count); ;
	}

    return 0;
}
