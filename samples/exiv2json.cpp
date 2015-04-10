// ***************************************************************** -*- C++ -*-
// exiv2json.cpp, $Id: exiv2json.cpp 518 2013-05-10 23:53:00Z robinwmills@gmail.com $
// Sample program to print metadata in JSON format

#include <exiv2/exiv2.hpp>
#include <Jzon.h>

#include <iostream>
#include <iomanip>
#include <cassert>
#include <string>

#if defined(__MINGW32__) || defined(__MINGW64__)
# ifndef  __MINGW__
#  define __MINGW__
# endif
#endif

#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>

#if defined(_MSC_VER) || defined(__MINGW__)
#include <windows.h>
#ifndef  PATH_MAX
# define PATH_MAX 512
#endif
const char* realpath(const char* file,char* path)
{
	GetFullPathName(file,PATH_MAX,path,NULL);
	return path;
}
#else
#include <unistd.h>
#endif

struct Token {
	std::string n; // the name eg "History"
	bool        a; // name is an array eg History[]
	int         i; // index (indexed from 1) eg History[1]/stEvt:action
};
typedef std::vector<Token> Tokens ;

// "XMP.xmp.MP.RegionInfo/MPRI:Regions[1]/MPReg:Rectangle"
bool getToken(std::string& in,Token& token)
{
	bool result = false;

	token.n = ""    ;
	token.a = false ;
	token.i = 0     ;

	while ( !result && in.length() ) {
		std::string c = in.substr(0,1);
		char        C = c[0];
		in            = in.substr(1,std::string::npos);
		if ( in.length() == 0 && C != ']' )	token.n += c;
		if ( C == '/' || C == '[' || C == ':' || C == '.' || C == ']' || in.length() == 0 ) {
			token.a   = C == '[';
			if ( C == ']' ) token.i = std::atoi(token.n.c_str()); // encoded string first index == 1
			result    = token.n.length() > 0 ;
		}  else {
			token.n   += c;
		}
	}
	return result;
}

Jzon::Node& addToTree(Jzon::Node& r1,Token token)
{
	std::string  key    = token.n  ;
	size_t       index  = token.i-1; // array Eg: "History[1]" indexed from 1.  Jzon expects 0 based index.
	bool         bArray = token.a  ;

	Jzon::Object object ;
	Jzon::Array  array  ;

	if (  r1.IsObject() ) {
		Jzon::Object& o1 = r1.AsObject();
		if ( !o1.Has(key) ) {
			if ( bArray ) o1.Add(key,array); else o1.Add(key,object);
		}
		return o1.Get(key);
	} else if ( r1.IsArray() ) {
		Jzon::Array& a1 = r1.AsArray();
		while ( a1.GetCount() <= index ) {
			if ( bArray ) a1.Add(array); else a1.Add(object);
		}
		return a1.Get(index);
	}
	return r1;
}

Jzon::Node& addToTree(Jzon::Node& rt,Tokens& tokens,size_t k)
{
	return --k == 0 ? addToTree(rt,tokens[0]) :  addToTree(   addToTree(rt,tokens,k)     ,tokens[k]);
}

// build the json tree for this key.  return location and discover the name
Jzon::Node& objectForKey(const std::string Key,Jzon::Object& rt,std::string& name)
{
    // Parse the key
    Tokens      tokens ;
    Token       token  ;
    std::string input  = Key ; // Example: "XMP.xmp.MP.RegionInfo/MPRI:Regions[1]/MPReg:Rectangle"
    while ( getToken(input,token) ) {
		tokens.push_back(token);
		name = token.n ;
    }
	size_t  l  = tokens.size()-1; // leave leaf name to push()
	return addToTree(rt,tokens,l);

#if 0
	// The recursion could be expressed these if statements:
	// Go to the root, and climb out, adding objects or arrays to create the tree
	// The leaf is pushed onto the top by the caller of objectForKey!
	if ( l == 1 ) return                                  addToTree(rt,tokens[0]);
	if ( l == 2 ) return                        addToTree(addToTree(rt,tokens[0]),tokens[1]);
	if ( l == 3 ) return              addToTree(addToTree(addToTree(rt,tokens[0]),tokens[1]),tokens[2]);
	if ( l == 4 ) return    addToTree(addToTree(addToTree(addToTree(rt,tokens[0]),tokens[1]),tokens[2]),tokens[3]);
#endif
}

bool isObject(std::string& value)
{
	return value.compare(std::string("type=\"Struct\""))==0;
}

bool isArray(std::string& value)
{
	return value.compare(std::string("type=\"Seq\""))==0
	||     value.compare(std::string("type=\"Bag\""))==0
	||     value.compare(std::string("type=\"Alt\""))==0
	;
}

#define STORE(son,key,value) \
 	if  (json.IsObject()) json.AsObject().Add(key,value);\
 	else                  json.AsArray() .Add(    value)

template <class T>
void push(Jzon::Node& json,const std::string& key,T i)
{
    std::string value = i->value().toString();
	if ( !json.IsArray() && !json.IsObject() ) return ;

    switch ( i->typeId() ) {
        case Exiv2::xmpText:
			 if ( ::isObject(value) ) {
				 Jzon::Object   v;
				 STORE(json,key,v);
			 } else if ( isArray(value) ) {
				 Jzon::Array    v;
				 STORE(json,key,v);
			 } else {
				 STORE(json,key,value);
			 }
    	break;

        case Exiv2::unsignedByte:
        case Exiv2::unsignedShort:
        case Exiv2::unsignedLong:
        case Exiv2::signedByte:
        case Exiv2::signedShort:
        case Exiv2::signedLong:
			 STORE(json,key,(int)i->value().toLong());
	    break;

        case Exiv2::tiffFloat:
        case Exiv2::tiffDouble:
			 STORE(json,key,i->value().toFloat());
        break;

        case Exiv2::unsignedRational:
        case Exiv2::signedRational: {
             Jzon::Array     arr;
             Exiv2::Rational rat = i->value().toRational();
             arr.Add(rat.first );
             arr.Add(rat.second);
			 STORE(json,key,arr);
        } break;

        default:
        case Exiv2::date:
        case Exiv2::time:
        case Exiv2::asciiString :
        case Exiv2::string:
        case Exiv2::comment:
        case Exiv2::undefined:
        case Exiv2::tiffIfd:
        case Exiv2::directory:
        case Exiv2::xmpAlt:
        case Exiv2::xmpBag:
        case Exiv2::xmpSeq:
        case Exiv2::langAlt:
             // http://dev.exiv2.org/boards/3/topics/1367#message-1373
             if ( key == "UserComment" ) {
                size_t pos  = value.find('\0') ;
             	if (   pos != std::string::npos )
             	    value = value.substr(0,pos);
             }
             if ( key == "MakerNote") return;
       	     STORE(json,key,value);
        break;
    }
}

void fileSystemPush(const char* path,Jzon::Node& nfs)
{
    Jzon::Object& fs = (Jzon::Object&) nfs;
    fs.Add("path",path);
    char resolved_path[2000]; // PATH_MAX];
    fs.Add("realpath",realpath(path,resolved_path));

	struct stat buf;
    memset(&buf,0,sizeof(buf));
    stat(path,&buf);

    fs.Add("st_dev"    ,(int) buf.st_dev    ); /* ID of device containing file    */
    fs.Add("st_ino"    ,(int) buf.st_ino    ); /* inode number                    */
    fs.Add("st_mode"   ,(int) buf.st_mode   ); /* protection                      */
    fs.Add("st_nlink"  ,(int) buf.st_nlink  ); /* number of hard links            */
    fs.Add("st_uid"    ,(int) buf.st_uid    ); /* user ID of owner                */
    fs.Add("st_gid"    ,(int) buf.st_gid    ); /* group ID of owner               */
    fs.Add("st_rdev"   ,(int) buf.st_rdev   ); /* device ID (if special file)     */
    fs.Add("st_size"   ,(int) buf.st_size   ); /* total size, in bytes            */
    fs.Add("st_atime"  ,(int) buf.st_atime  ); /* time of last access             */
    fs.Add("st_mtime"  ,(int) buf.st_mtime  ); /* time of last modification       */
    fs.Add("st_ctime"  ,(int) buf.st_ctime  ); /* time of last status change      */

#if defined(_MSC_VER) || defined(__MINGW__)
	size_t blksize     = 1024;
	size_t blocks      = (buf.st_size+blksize-1)/blksize;
#else
    size_t blksize     = buf.st_blksize;
    size_t blocks      = buf.st_blocks ;
#endif
    fs.Add("st_blksize",(int) blksize       ); /* blocksize for file system I/O   */
    fs.Add("st_blocks" ,(int) blocks        ); /* number of 512B blocks allocated */
}

std::string escape(Exiv2::XmpData::const_iterator it,bool bValue)
{
	std::string   result ;

	std::ostringstream os;
	if ( bValue ) os << it->value() ; else os << it->key() ;

	return os.str();
}

int main(int argc, char* const argv[])
try {
    if (argc < 2 || argc > 3) {
        std::cout << "Usage: " << argv[0] << " [-option] file\n";
        std::cout << "Option: all | exif | iptc | xmp | filesystem" << argv[0] << " [option] file\n";
        return 1;
    }
    const char* path   = argv[argc-1];
    const char* opt    = argc == 3 ? argv[1] : "-all" ;
    while      (opt[0] == '-') opt++ ; // skip past leading -'s
    char        option = opt[0];

    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(path);
    assert(image.get() != 0);
    image->readMetadata();

    Jzon::Object   root;

	if ( option == 'a' || option == 'f' ) {
		const char*    FS="FS";
		Jzon::Object      fs  ;
		root.Add(FS,fs) ;
		fileSystemPush(path,root.Get(FS));
	}

	if ( option == 'a' || option == 'e' ) {
		Exiv2::ExifData &exifData = image->exifData();
		for ( Exiv2::ExifData::const_iterator i = exifData.begin(); i != exifData.end() ; ++i ) {
        	std::string name   ;
			Jzon::Node& object = objectForKey(i->key(),root,name);
        	push(object,name,i);
    	}
	}

	if ( option == 'a' || option == 'i' ) {
		Exiv2::IptcData &iptcData = image->iptcData();
		for (Exiv2::IptcData::const_iterator i = iptcData.begin(); i != iptcData.end(); ++i) {
			std::string name   ;
			Jzon::Node& object = objectForKey(i->key(),root,name);
			push(object,name,i);
		}
	}

	if ( option == 'a' || option == 'x' ) {
		Exiv2::XmpData  &xmpData  = image->xmpData();
    	for (Exiv2::XmpData::const_iterator i = xmpData.begin(); i != xmpData.end(); ++i) {
        	std::string name   ;
			Jzon::Node& object = objectForKey(i->key(),root,name);
        	push(object,name,i);
    	}
	}

    Jzon::Writer writer(root,Jzon::StandardFormat);
    writer.Write();
    std::cout << writer.GetResult() << std::endl;
    return 0;
}

//catch (std::exception& e) {
//catch (Exiv2::AnyError& e) {
catch (Exiv2::Error& e) {
    std::cout << "Caught Exiv2 exception '" << e.what() << "'\n";
    return -1;
}
