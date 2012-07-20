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

//using namespace Exiv2;
using namespace std;

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
{   typeUnknown   = 0
,   typeDirectory = 1
,   typeImage     = 2
,   typeXML       = 3
,   typeFile      = 4
,   typeMax       = 5
};

enum                        // keyword indices
{   kwHELP = 0
,   kwARGS
,   kwVERSION
,   kwNOCR
,   kwUNSPACE
,   kwCLOSE
,   kwOPEN
,   kwEND1
,   kwEND2
,   kwSLEEP
,   kwT1
,   kwT2
,   kwSPEED
,   kwSEND
,   kwOUTPUT
,   kwMAX                   // manages keyword array
,   kwNEEDVALUE             // bogus keywords for error reporting
,   kwSYNTAX                // -- ditto --
,   kwNOVALUE = -kwCLOSE    // keywords <= kwNOVALUE are flags (no value needed)
};

enum
{   resultOK=0
,   resultSyntaxError
,   resultSelectFailed
,   resultOpenFailed
,   resultTimeout
};

time_t parseTime(const char* arg)
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
    time_t now = time(NULL);
#if   defined(__CYGWIN__)
    struct tm loc   ; memcpy(&loc  ,localtime(&now),sizeof(loc  ));
    time_t gmt = timegm(&loc);
    int offset = (int) ( ((long signed int) gmt) - ((long signed int) now) ) ;
#elif defined(_MSC_VER)
    TIME_ZONE_INFORMATION TimeZoneInfo;
    GetTimeZoneInformation( &TimeZoneInfo );
    int offset = - ((int)TimeZoneInfo.Bias + (int)TimeZoneInfo.DaylightBias) * 60;
#else
    int offset = local.tm_gmtoff ;
#endif
//  struct tm local ; memcpy(&local,localtime(&now),sizeof(local));
//  struct tm utc   ; memcpy(&utc  ,gmtime   (&now),sizeof(utc  ));
//  printf("local: offset = %6ld dst = %d time = %s", offset,local.tm_isdst, asctime(&local));
//  printf("utc  : offset = %6ld dst = %d time = %s", 0     ,utc  .tm_isdst, asctime(&utc  ));

//  printf("timeZoneAdjust = %d\n",offset);
    return offset ;
}

class UserData
{
public:
             UserData() : indent(0),count(0),nTrkpt(0),bTime(false),bEle(false)  {};
    virtual ~UserData() {} ;

//  public data members
    int    indent;
    size_t count ;
    int    nTrkpt;
    bool   bTime ;
    bool   bEle  ;
    double ele;
    double lat;
    double lon;
    time_t t;
};

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
        printf("lat,lon = %f,%f ele = %f time = %ld %s",me->lat,me->lon,me->ele,(long int)me->t,"\n"); // asctime(localtime(&me->t)) ) ;
    }
    //for ( int i = 0 ; i < me->indent ; i++ ) printf(" ");
    //printf("end %s\n",name);
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
                me->t = parseTime(b);
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

bool readDir(const char* path,size_t& count)
{
    strings_t files;
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
                    files.push_back( std::string(ffd.cFileName));
                    printf("-> %s\n",ffd.cFileName);
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
            files.push_back(std::string(ent->d_name)) ;
        }
        closedir (dir);
    }
#endif
    count = files.size();
    return bResult ;
}

bool readXML(const char* path,size_t& count)
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

    if ( f      ) fclose(f);
    if ( parser ) XML_ParserFree(parser);

    return bResult ;
}

bool readImage(const char* path,size_t& count)
{
    using namespace Exiv2;
    bool bResult = false ;
    count = 0 ;

    try {
        Image::AutoPtr image = ImageFactory::open(path);
        if ( image.get() ) {
            image->readMetadata();
            ExifData &exifData = image->exifData();
            if ( !exifData.empty() ) {
                bResult = true ;
                count   = exifData.count();
            }

        }
    } catch (Exiv2::Error& ) {};
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
                result = parseTime(exifData[ds-1].toString().c_str());
            }
        } catch ( ... ) {};
    }
    return result ;
}



bool readFile(const char* path,size_t& count)
{
    FILE*       f       = fopen(path,"r");
    bool bResult        = f ? true : false;
    if ( bResult ) {
        fseek(f,0L,SEEK_END);
        count = ftell(f);
    }
    if ( f ) fclose(f) ;

    return bResult ;
}

int getFileType(const char* path,size_t& count)
{
    return readXML  (path,count) ? typeXML
        :  readDir  (path,count) ? typeDirectory
        :  readImage(path,count) ? typeImage
        :  readFile (path,count) ? typeFile
        :  typeUnknown
        ;
}

int main(int argc, char* const argv[])
{
    if ( argc < 2 ) {
        std::cout << "Usage: " << argv[0] << " arg+\n";
        return 1;
    }

#if 0
    char const* keywords[kwMAX];
    memset(keywords,0,sizeof(keywords));
    keywords[kwHELP]    = "help";
    keywords[kwVERSION] = "version";
    keywords[kwARGS]    = "args";
    keywords[kwNOCR]    = "nocr";
    keywords[kwUNSPACE] = "unspace";
    keywords[kwCLOSE]   = "close";
    keywords[kwOPEN]    = "open";
    keywords[kwEND1]    = "end1";
    keywords[kwEND2]    = "end2";
    keywords[kwSLEEP]   = "sleep";
    keywords[kwT1]      = "t1";
    keywords[kwT2]      = "t2";
    keywords[kwSPEED]   = "speed";
    keywords[kwSEND]    = "send";
    keywords[kwOUTPUT]  = "output";

    if ( argc < 2 ) help(program,keywords,kwMAX) ;

    for ( int a = 0 ; !result && a < 2 ; a++ ) { // a = 0 is a dry run
        options options ;
        for ( int i = 1 ; !result && i < argc ; i++ ) {
            const char* arg   = argv[i++];
            const char* value = argv[i  ];
            int        ivalue = atoi(value?value:"0");
            int         key   = find(arg,keywords,kwMAX);
            int         needv = key < kwMAX && key > (-kwNOVALUE);

            if (!needv ) i--;
            if ( needv && !value ) key = kwNEEDVALUE;
            if ( arg[0] != '-'   ) key = kwSYNTAX;

            switch ( key ) {
                case kwHELP     : if ( a ) { /* help(program,keywords,kwMAX)               ; */ } break;
                case kwARGS     : if ( a ) { /* args(argv,argc)                            ; */ } break;
                case kwVERSION  : if ( a ) { /* version(program)                           ; */ } break;
                case kwNOCR     : if ( a ) { /* options.bNOCR=true                         ; */ } break;
                case kwUNSPACE  : if ( a ) { /* options.bUnspace=true                      ; */ } break;
                case kwCLOSE    : if ( a ) { /* modem_close(options)                       ; */ } break;
                case kwOPEN     : if ( a ) { /* options.open = value                       ; }*/  break ;
                case kwEND1     : if ( a ) { /* options.end1=value                         ; */ } break;
                case kwEND2     : if ( a ) { /* options.end2=value                         ; */ } break;
                case kwSLEEP    : if ( a ) { /* sleep(ivalue)                              ; }*/  break ;
                case kwT1       : if ( a ) { /* options.t1=ivalue                          ; }*/  break ;
                case kwT2       : if ( a ) { /* options.t2=ivalue                          ; */ } break;
                case kwSPEED    : if ( a ) { /* options.speed=ivalue                       ; }*/  break ;
/*
                case kwSEND     : if ( a )
                {
                    result=modem_send(value,options);
                    if ( result == resultTimeout && options.t2 > 0 )
                        result = modem_send(value,options,true);
                } break ;
                case kwOUTPUT   : if ( a ) { printf("%s",value) ;                       ; } break ;
*/
                case kwNEEDVALUE: fprintf(stderr,"error: %s requires a value\n",arg); result = resultSyntaxError ; break ;
                default         : fprintf(stderr,"error: illegal syntax %s\n",arg)  ; result = resultSyntaxError ; break ;
            }
            if ( a ) fflush(stdout);
            // if ( options.fd < -1 ) result = resultOpenFailed ;
        }
    }
#endif

    const char* types[typeMax];
    types[typeUnknown  ] = "unknown";
    types[typeDirectory] = "directory";
    types[typeImage    ] = "image";
    types[typeXML      ] = "xml";
    types[typeFile     ] = "file";

    int tzadjust = timeZoneAdjust();
    printf("tzadjust seconds = %d HH:MM = %c%02d:%02d [-/+ = West/East of UTC]\n",tzadjust,tzadjust<0?'-':'+',abs(tzadjust/3600),tzadjust%3600);

    for ( int i = 1 ; i < argc ; i++ ) {
        char* arg = argv[i];
        size_t count = 0   ;
        time_t t     = 0   ;
        int   type   = getFileType(arg,count) ;
        printf("%s %s %d",arg,types[type],count) ;
        if ( type == typeImage ) {
            t = readImageTime(arg) ;
            if ( t ) printf(" %ld ",(long int)t);
        }
        puts(t ? asctime(localtime(&t)) : "" );
    }

    return 0;
}
