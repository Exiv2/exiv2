// ***************************************************************** -*- C++ -*-
// geotag.cpp, $Rev: 2286 $
// Sample program to read gpx files and update the images with GPS tags
// Work in progress - doesn't do anything yet!

#include <exiv2/exiv2.hpp>

#include <iostream>
#include <iomanip>
#include <cassert>

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#include "expat.h"

#include <vector>
#include <string>
using namespace std;

#ifndef  lengthof
#define  lengthof(x) (sizeof(*x)/sizeof(x))
#endif
#ifndef nil
#define nil NULL
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 1024
#endif

#ifdef   _MSC_VER
#include <windows.h>
bool realpath(const char* file,char* path,int path_l=_MAX_PATH);
bool realpath(const char* file,char* path,int path_l)
{
	GetFullPathName(file,path_l,path,NULL);
	return true;
}

#if      _MSC_VER < 1400
#define strcpy_s(d,l,s) strcpy(d,s)
#define strcat_s(d,l,s) strcat(d,s)
#endif

#else
#include <dirent.h>
#include <unistd.h>
#include <sys/param.h>
#endif

// prototypes
int getFileType(const char* path );
int getFileType(std::string& path);

string getExifTime(const time_t t);
time_t parseTime(const char* ,bool bAdjust=true);
int    timeZoneAdjust();

// Command-line parser
class Options  {
public:
	bool        verbose;

	Options()
	{
		verbose     = false;
	}

	virtual ~Options() {} ;
} ;

enum
{   resultOK=0
,   resultSyntaxError
,   resultSelectFailed
};

enum                        // keyword indices
{   kwHELP = 0
,   kwVERSION
,   kwVERBOSE
,   kwADJUST
,   kwMAX                   // manages keyword array
,   kwNEEDVALUE             // bogus keywords for error reporting
,   kwSYNTAX                // -- ditto --
,   kwNOVALUE = -kwVERBOSE  // keywords <= kwNOVALUE are flags (no value needed)
};

// file types supported
enum
{   typeUnknown   = 0
,   typeDirectory = 1
,   typeImage     = 2
,   typeXML       = 3
,   typeFile      = 4
,   typeDoc       = 5
,   typeCode      = 6
,   typeMax       = 7
};

// Position (from gpx file)
class Position
{
public:
	         Position(time_t time,double lat,double lon,double ele) : time_(time),lon_(lon),lat_(lat),ele_(ele) {};
			 Position() { time_=0 ; lat_=0.0 ; lon_=0.0 ; ele_=0.0 ; };
    virtual ~Position() {} ;
//  copy constructor
	Position(const Position& o) : time_(o.time_),lon_(o.lon_),lat_(o.lat_),ele_(o.ele_) {};

//  methods
	bool good()                 { return time_ || lon_ || lat_ || ele_ ; }
	std::string getTimeString() { if ( times_.empty() ) times_ = getExifTime(time_) ;  return times_; }
	time_t getTime()            { return time_ ; }

//  data
private:
	time_t      time_;
	double      lon_ ;
	double      lat_ ;
	double      ele_ ;
	std::string times_;
// public static data
public:
	static int    adjust_;
	static time_t timeDiffMax;
} ;

// globals
typedef std::map<time_t,Position>           TimeDict_t;
typedef std::map<time_t,Position>::iterator TimeDict_i;
typedef std::vector<std::string>            strings_t;
TimeDict_t   gTimeDict ;
strings_t    gFiles;

int    Position::adjust_     = timeZoneAdjust();
time_t Position::timeDiffMax = 120 ;

///////////////////////////////////////////////////////////
// UserData - used by XML Parser
class UserData
{
public:
	UserData() : indent(0),count(0),nTrkpt(0),bTime(false),bEle(false)  {};
    virtual ~UserData() {} ;

//  public data members
    int    indent;
    size_t count ;
	Position now ;
	Position prev;
    int    nTrkpt;
    bool   bTime ;
    bool   bEle  ;
    double ele;
    double lat;
    double lon;
	std::string xmlt;
	std::string exift;
    time_t time;
// static public data memembers
};

// XML Parser Callbacks
static void startElement(void* userData, const char* name, const char** atts )
{
    UserData* me = (UserData*) userData;
    //for ( int i = 0 ; i < me->indent ; i++ ) printf(" ");
    //printf("begin %s\n",name);
    me->bTime = strcmp(name,"time")==0;
    me->bEle  = strcmp(name,"ele")==0;

    if ( strcmp(name,"trkpt")==0 ) {
        me->nTrkpt++;
        while ( *atts ) {
            const char* a=atts[0];
            const char* v=atts[1];
            if ( !strcmp(a,"lat") ) me->lon = atof(v);
            if ( !strcmp(a,"lon") ) me->lat = atof(v);
            atts += 2 ;
        }
    }
    me->count++  ;
    me->indent++ ;
}

static void endElement(void* userData, const char* name)
{
    UserData* me = (UserData*) userData;
    me->indent-- ;
    if ( strcmp(name,"trkpt")==0 ) {
        me->nTrkpt--;
		me->now = Position(me->time,me->lat,me->lon,me->ele) ;

		// printf("lat,lon = %f,%f ele = %f xml = %s exif = %s\n",me->lat,me->lon,me->ele,me->xmlt.c_str(),me->exift.c_str());

		// if we have a good previous position
		// add missed entries to timedict
		if ( me->prev.good() && (me->now.getTime() - me->prev.getTime()) < Position::timeDiffMax ) {
			time_t missed = me->prev.getTime() ;
			while ( ++missed < me->now.getTime() )
				gTimeDict[missed] = me->prev ; // Position(missed,me->lat,me->lon,me->ele) ;
		}

		// remember our location and put it in gTimeDict
		gTimeDict[me->time] = me->now ;
		me->prev = me->now ;
    }
}

void charHandler(void* userData,const char* s,int len)
{
    UserData* me = (UserData*) userData;

    if ( me->nTrkpt == 1 ) {
        char buffer[100];
        int  l_max = 98 ; // lengthof(buffer) -2 ;

        if ( me->bTime && len > 5 ) {
            if ( len < l_max ) {
                memcpy(buffer,s,len);
                buffer[len]=0;
                char* b = buffer ;
                while ( *b == ' ' && b < buffer+len ) b++ ;
				me->xmlt  = b ;
                me->time  = parseTime(me->xmlt.c_str());
				me->exift = getExifTime(me->time);
            }
            me->bTime=false;
        }
        if ( me->bEle && len > 5 ) {
            if ( len < l_max ) {
                memcpy(buffer,s,len);
                buffer[len]=0;
                char* b = buffer ;
                while ( *b == ' ' && b < buffer+len ) b++ ;
                me->ele = atof(b);
            }
            me->bEle=false;
        }
    }
}

///////////////////////////////////////////////////////////
// Time Functions
time_t parseTime(const char* arg,bool bAdjust)
{
    time_t result = 0 ;
    try {
        //559 rmills@rmills-imac:~/bin $ exiv2 -pa ~/R.jpg | grep -i date
        //Exif.Image.DateTime                          Ascii      20  2009:08:03 08:58:57
        //Exif.Photo.DateTimeOriginal                  Ascii      20  2009:08:03 08:58:57
        //Exif.Photo.DateTimeDigitized                 Ascii      20  2009:08:03 08:58:57
        //Exif.GPSInfo.GPSDateStamp                    Ascii      21  2009-08-03T15:58:57Z

        // <time>2012-07-14T17:33:16Z</time>

        if ( strstr(arg,":") || strstr(arg,"-") ) {
            int  YY,MM,DD,HH,mm,SS ;
            char a,b,c,d,e   ;
            sscanf(arg,"%d%c%d%c%d%c%d%c%d%c%d",&YY,&a,&MM,&b,&DD,&c,&HH,&d,&mm,&e,&SS);

            struct tm T;
    #if 0
            int tm_sec;     /* seconds (0 - 60) */
            int tm_min;     /* minutes (0 - 59) */
            int tm_hour;    /* hours (0 - 23) */
            int tm_mday;    /* day of month (1 - 31) */
            int tm_mon;     /* month of year (0 - 11) */
            int tm_year;    /* year - 1900 */
            int tm_wday;    /* day of week (Sunday = 0) */
            int tm_yday;    /* day of year (0 - 365) */
            int tm_isdst;   /* is summer time in effect? */
            char *tm_zone;  /* abbreviation of timezone name */
            long tm_gmtoff; /* offset from UTC in seconds */
    #endif
            memset(&T,sizeof(T),0);
			T.tm_min  = mm  ;
            T.tm_hour = HH  ;
            T.tm_sec  = SS  ;
			if ( bAdjust ) T.tm_sec += Position::adjust_ ;
            T.tm_year = YY -1900 ;
            T.tm_mon  = MM -1    ;
            T.tm_mday = DD  ;
            result = mktime(&T);
        }
    } catch ( ... ) {};
    return result ;
}

// West of GMT is negative (PDT = Pacific Daylight = -07:00 == -25200 seconds
int timeZoneAdjust()
{
    time_t    now   = time(NULL);
    struct tm local = *localtime(&now) ;

#if   defined(_MSC_VER)
    TIME_ZONE_INFORMATION TimeZoneInfo;
    GetTimeZoneInformation( &TimeZoneInfo );
    int offset = - (((int)TimeZoneInfo.Bias + (int)TimeZoneInfo.DaylightBias) * 60);
#elif defined(__CYGWIN__)
    struct tm lcopy  = *localtime(&now);
    time_t    gmt    =  timegm(&lcopy) ; // timegm modifies lcopy, so don't use local
    int       offset = (int) ( ((long signed int) gmt) - ((long signed int) now) ) ;
#else
    int offset = local.tm_gmtoff ;
#endif

#if 0
	// debugging code
	struct tm utc = *gmtime(&now);
	printf("utc  :  offset = %6d dst = %d time = %s", 0     ,utc  .tm_isdst, asctime(&utc  ));
	printf("local:  offset = %6d dst = %d time = %s", offset,local.tm_isdst, asctime(&local));
	printf("timeZoneAdjust = %6d\n",offset);
#endif
    return offset ;
}

string getExifTime(const time_t t)
{
	static char result[100];
	strftime(result,sizeof(result),"%Y-%m-%d %H:%M:%S",localtime(&t));
	return result ;
}

std::string makePath(std::string dir,std::string file)
{
	return dir + std::string(EXV_SEPERATOR_STR) + file ;
}

const char* makePath(const char* dir,const char* file)
{
	static char result[_MAX_PATH] ;
	std::string r = makePath(std::string(dir),std::string(file));
	strcpy(result,r.c_str());
	return result;
}

// file utilities
bool readDir(const char* path)
{
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
					std::string pathName = makePath(path,std::string(ffd.cFileName));
					if ( getFileType(pathName) == typeImage ) {
						gFiles.push_back( pathName );
					}
                }
                bGo = FindNextFile(hFind, &ffd) != 0;
            }
            CloseHandle(hFind);
        }
    }
#else
    DIR*    dir = opendir (path);
    if (dir != NULL)
    {
        bResult = true;
        struct dirent*  ent;

        // print all the files and directories within directory
        while ((ent = readdir (dir)) != NULL)
        {
            printf ("%s\n", ent->d_name);
			gFiles.push_back( makePath(path,ent->d_name) ) ;
        }
        closedir (dir);
    }
#endif
    return bResult ;
}

inline size_t sip(FILE* f,char* buffer,size_t max_len,size_t len)
{
	while ( !feof(f) && len < max_len && buffer[len-1] != '>')
		buffer[len++] = fgetc(f);
	return len;
}

bool readXML(const char* path)
{
    FILE*       f       = fopen(path,"r");
    XML_Parser  parser  = XML_ParserCreate(NULL);
    bool bResult        = f && parser ;
    if ( bResult ) {
        char   buffer[8*1024];
        UserData me ;

        XML_SetUserData            (parser, &me);
        XML_SetElementHandler      (parser, startElement, endElement);
        XML_SetCharacterDataHandler(parser,charHandler);

        // a little sip at the data
        size_t len = fread(buffer,1,sizeof(buffer)-100,f);
        const char* lead   = "<?xml" ;
        bResult = strncmp(lead,buffer,strlen(lead))==0;

        // swallow it
        if ( bResult ) {
			len = sip(f,buffer,sizeof buffer,len);
            bResult = XML_Parse(parser, buffer,(int)len, len == 0 ) == XML_STATUS_OK;
        }

        // drink the rest of the file
        while ( bResult && len != 0 ) {
            len = fread(buffer,1,sizeof(buffer)-100,f);
			len = sip(f,buffer,sizeof buffer,len);
            bResult = XML_Parse(parser, buffer,(int)len, len == 0 ) == XML_STATUS_OK;
        };
    }

    if ( f      ) fclose(f);
    if ( parser ) XML_ParserFree(parser);

    return bResult ;
}

bool readImage(const char* path)
{
    using namespace Exiv2;
    bool bResult = false ;

    try {
        Image::AutoPtr image = ImageFactory::open(path);
        if ( image.get() ) {
            image->readMetadata();
            ExifData &exifData = image->exifData();
            bResult = !exifData.empty();
        }
    } catch ( ... ) {};
    return bResult ;
}

time_t readImageTime(const char* path)
{
    using namespace Exiv2;

    time_t       result       = 0 ;

    const char* dateStrings[] =
    { "Exif.Photo.DateTimeOriginal"
    , "Exif.Photo.DateTimeDigitized"
    , "Exif.Image.DateTime"
    , NULL
    };
    const char* ds            = dateStrings[0] ;

    while ( !result && ds++  ) {
        try {
            Image::AutoPtr image = ImageFactory::open(path);
            if ( image.get() ) {
                image->readMetadata();
                ExifData &exifData = image->exifData();
            //  printf("%s => %s\n",(ds-1), exifData[ds-1].toString().c_str());
				result = parseTime(exifData[ds-1].toString().c_str(),false);
            }
        } catch ( ... ) {};
    }
    return result ;
}

bool sin(const char* s,const char** a)
{
	bool bResult = false ;
	int i = 0 ;
	while ( !bResult && a[i]) {
		bResult = stricmp(s,a[i])==0;
		i++;
	}
	return bResult;
}

int readFile(const char* path)
{
    FILE* f     = fopen(path,"r");
    int nResult = f ? typeFile : typeUnknown;
    if (  f ) {
		const char* docs[] = { ".doc",".txt", nil };
		const char* code[] = { ".cpp",".h"  ,".pl" ,".py" ,".pyc", nil };
		const char*  ext   = strstr(path,".");
		if  ( ext ) {
			if ( sin(ext,docs) ) nResult = typeDoc;
			if ( sin(ext,code) ) nResult = typeCode;
		}
    //    fseek(f,0L,SEEK_END);
    //    count = ftell(f);
    }
    if ( f ) fclose(f) ;

    return nResult ;
}

int getFileType(std::string& path) { return getFileType(path.c_str()); }
int getFileType(const char* path)
{
    return readXML  (path) ? typeXML
        :  readDir  (path) ? typeDirectory
        :  readImage(path) ? typeImage
        :  readFile (path)
        ;
}


int version(const char* program)
{
	printf("%s: %s %s\n",program,__DATE__,__TIME__);
	return 0;
}

int help(const char* program,char const* words[],int nWords)
{
	printf("usage: %s",program);
	for ( int i = 0 ; i < nWords ; i++ ) {
		if ( words[i] )
			printf(" -%s%s",words[i],i>(-kwNOVALUE)?" value":"");
	}
	printf(" path+\n");
	return 0;
}

int compare(const char* a,const char* b)
{
	int result=*a && *b;
	while ( result && *a && *b) {
		char A=*a++;
		char B=*b++;
		result=tolower(A)==tolower(B);
	}
	return result;
}

int find(const char* arg,char const* words[],int nWords)
{
	int result=0;
    int count =0;

	for ( int i = 0 ; i < nWords ; i++) {
		int j = 0 ;
		while ( arg[j] == '-' ) j++;
		if ( ::compare(arg+j,words[i]) ) {
			result = i ;
			count++;
		}
	}

	return count==1?result:kwSYNTAX;
}


int main(int argc, char* const argv[])
{
	int result=0;
	const char* program = argv[0];

	const char* types[typeMax];
    types[typeUnknown  ] = "unknown";
    types[typeDirectory] = "directory";
    types[typeImage    ] = "image";
    types[typeXML      ] = "xml";
	types[typeDoc      ] = "doc";
	types[typeCode     ] = "code";
    types[typeFile     ] = "file";

	char const* keywords[kwMAX];
	memset(keywords,0,sizeof(keywords));
	keywords[kwHELP] 	= "help";
	keywords[kwVERSION] = "version";
	keywords[kwVERBOSE] = "verbose";
	keywords[kwADJUST] 	= "adjust" ;

    Options options ;

	if ( argc < 2 ) {
		::help(program,keywords,kwMAX);
		return result ;
    }

    for ( int a = 0 ; !result && a < 2 ; a++ ) { // a = 0 is a dry run
		if ( a && options.verbose ) {
			int tzadjust = Position::adjust_;
			printf("tzadjust seconds = %d HH:MM = %c%02d:%02d [-/+ = West/East of UTC]\n",tzadjust,tzadjust<0?'-':'+',abs(tzadjust/3600),tzadjust%3600);

			for ( TimeDict_i it = gTimeDict.begin() ; it != gTimeDict.end() ; it++) {
			//	printf("time = %s\n",it->second.getTimeString().c_str()); // ,gTimeDict[*it].sTime
			}
			for ( unsigned i = 0 ; i < gFiles.size() ; i++ ) {
				printf("image[%s%d] = %s\n",i>9?"":" ",i,gFiles[i].c_str());
			}
		}

        for ( int i = 1 ; !result && i < argc ; i++ ) {
            const char* arg   = argv[i++];
            const char* value = argv[i  ];
			int        ivalue = ::atoi(value?value:"0");
			int         key   = ::find(arg,keywords,kwMAX);
            int         needv = key < kwMAX && key > (-kwNOVALUE);

            if (!needv ) i--;
            if ( needv && !value ) key = kwNEEDVALUE;

            switch ( key ) {
				case kwHELP     : if ( a ) { ::help(program,keywords,kwMAX)             ; } break;
				case kwVERSION  : if ( a ) { ::version(program)                         ; } break;
				case kwADJUST   : if ( a ) { Position::adjust_=ivalue                   ; } break;
				case kwVERBOSE  : options.verbose = true                                ;   break;
                case kwNEEDVALUE: fprintf(stderr,"error: %s requires a value\n",arg); result = resultSyntaxError ; break ;
                default         :
				{
					int   type   = getFileType(arg) ;
					if ( !a && options.verbose ) printf("%s %s",arg,types[type]) ;
					if ( type == typeImage ) {
						time_t t = readImageTime(arg) ;
						if ( t ) printf(" %ld %s",(long int)t,asctime(localtime(&t)));
						char path[_MAX_PATH];
						realpath(arg,path);
						gFiles.push_back(path);
					}

					if ( type == typeUnknown ) {
						fprintf(stderr,"error: illegal syntax %s\n",arg)  ; result = resultSyntaxError ; break ;
					}
				}
            }
        }
    }

    return result ;
}
