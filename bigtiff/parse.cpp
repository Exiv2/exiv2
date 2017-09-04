#include <iostream>

#define WIDTH 32

enum TypeId {
	unsignedByte       =  1, //!< Exif BYTE type, 8-bit unsigned integer.
	asciiString        =  2, //!< Exif ASCII type, 8-bit byte.
	unsignedShort      =  3, //!< Exif SHORT type, 16-bit (2-byte) unsigned integer.
	unsignedLong       =  4, //!< Exif LONG type, 32-bit (4-byte) unsigned integer.
	unsignedRational   =  5, //!< Exif RATIONAL type, two LONGs: numerator and denumerator of a fraction.
	signedByte         =  6, //!< Exif SBYTE type, an 8-bit signed (twos-complement) integer.
	undefined          =  7, //!< Exif UNDEFINED type, an 8-bit byte that may contain anything.
	signedShort        =  8, //!< Exif SSHORT type, a 16-bit (2-byte) signed (twos-complement) integer.
	signedLong         =  9, //!< Exif SLONG type, a 32-bit (4-byte) signed (twos-complement) integer.
	signedRational     = 10, //!< Exif SRATIONAL type, two SLONGs: numerator and denumerator of a fraction.
	tiffFloat          = 11, //!< TIFF FLOAT type, single precision (4-byte) IEEE format.
	tiffDouble         = 12, //!< TIFF DOUBLE type, double precision (8-byte) IEEE format.
	tiffIfd            = 13, //!< TIFF IFD type, 32-bit (4-byte) unsigned integer.
	iptcString         = 0x10000, //!< IPTC string type.
	iptcDate           = 0x10001, //!< IPTC date type.
	iptcTime           = 0x10002, //!< IPTC time type.
	comment            = 0x10003, //!< %Exiv2 type for the Exif user comment.
	directory          = 0x10004, //!< %Exiv2 type for a CIFF directory.
	xmpText            = 0x10005, //!< XMP text type.
	xmpAlt             = 0x10006, //!< XMP alternative type.
	xmpBag             = 0x10007, //!< XMP bag type.
	xmpSeq             = 0x10008, //!< XMP sequence type.
	langAlt            = 0x10009, //!< XMP language alternative type.
	invalidTypeId      = 0x1fffe, //!< Invalid type id.
	lastTypeId         = 0x1ffff  //!< Last type id.
};

bool isStringType(uint16_t type)
{
	return type == asciiString
		|| type == unsignedByte
		|| type == signedByte
		|| type == undefined
		;
}

bool isShortType(uint16_t type) {
	 return type == unsignedShort
		 || type == signedShort
		 ;
}

bool isLongType(uint16_t type) {
	 return type == unsignedLong
		 || type == signedLong
		 ;
}

bool isRationalType(uint16_t type) {
	 return type == unsignedRational
		 || type == signedRational
		 ;
}

bool is2ByteType(uint16_t type)
{
	return isShortType(type);
}

bool is4ByteType(uint16_t type)
{
	return isLongType(type)
		|| type == tiffFloat
		|| type == tiffIfd
		;
}

bool is8ByteType(uint16_t type)
{
	return isRationalType(type)
		 || type == tiffDouble
		;
}

bool isBigEndianPlatform()
{
	union {
		uint32_t i;
		char c[4];
	} e = { 0x01000000 };

	return e.c[0]?true:false;
}

bool isLittleEndianPlatform() { return !isBigEndianPlatform(); }

uint32_t byteSwap(uint32_t value,bool bSwap)
{
	uint32_t result = 0;
	result |= (value & 0x000000FF) << 24;
	result |= (value & 0x0000FF00) << 8;
	result |= (value & 0x00FF0000) >> 8;
	result |= (value & 0xFF000000) >> 24;
	return bSwap ? result : value;
}

uint16_t byteSwap(uint16_t value,bool bSwap)
{
	uint16_t result = 0;
	result |= (value & 0x00FF) << 8;
	result |= (value & 0xFF00) >> 8;
	return bSwap ? result : value;
}

uint16_t byteSwap2(void* buf,size_t offset,bool bSwap)
{
	uint16_t v;
	char*    p = (char*) &v;
	char*    b = (char*) buf;
	p[0]       = b[offset];
	p[1]       = b[offset+1];
	return byteSwap(v,bSwap);
}

uint32_t byteSwap4(void* buf,size_t offset,bool bSwap)
{
	uint32_t v;
	char*    p = (char*) &v;
	char*    b = (char*) buf;
	p[0]       = b[offset];
	p[1]       = b[offset+1];
	p[2]       = b[offset+2];
	p[3]       = b[offset+3];
	return byteSwap(v,bSwap);
}

std::string indent(int32_t d)
{
	std::string result ;
	if ( d > 0 )
		while ( d--)
			result += "  ";
	return result;
}

static const char* tagName(uint16_t tag,size_t nMaxLength)
{
	const char* result = NULL;
/*
	// build a static map of tags for fast search
	static std::map<int,std::string> tags;
	static bool init  = true;
	static char buffer[80];

	if ( init ) {
		int idx;
		const TagInfo* ti ;
		for (ti =   mnTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx) tags[ti[idx].tag_] = ti[idx].name_;
		for (ti =  iopTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx) tags[ti[idx].tag_] = ti[idx].name_;
		for (ti =  gpsTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx) tags[ti[idx].tag_] = ti[idx].name_;
		for (ti =  ifdTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx) tags[ti[idx].tag_] = ti[idx].name_;
		for (ti = exifTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx) tags[ti[idx].tag_] = ti[idx].name_;
		for (ti =  mpfTagList(), idx = 0; ti[idx].tag_ != 0xffff; ++idx) tags[ti[idx].tag_] = ti[idx].name_;
		for (ti = Nikon1MakerNote::tagList(), idx = 0
												  ; ti[idx].tag_ != 0xffff; ++idx) tags[ti[idx].tag_] = ti[idx].name_;
	}
	init = false;

	try {
		result = tags[tag].c_str();
		if ( nMaxLength > sizeof(buffer) -2 )
			 nMaxLength = sizeof(buffer) -2;
		strncpy(buffer,result,nMaxLength);
		result = buffer;
	} catch ( ... ) {}
*/
	return result ;
}

static const char* typeName(uint16_t tag)
{
	//! List of TIFF image tags
	const char* result = NULL;
	switch (tag ) {
		case unsignedByte     : result = "BYTE"      ; break;
		case asciiString      : result = "ASCII"     ; break;
		case unsignedShort    : result = "SHORT"     ; break;
		case unsignedLong     : result = "LONG"      ; break;
		case unsignedRational : result = "RATIONAL"  ; break;
		case signedByte       : result = "SBYTE"     ; break;
		case undefined        : result = "UNDEFINED" ; break;
		case signedShort      : result = "SSHORT"    ; break;
		case signedLong       : result = "SLONG"     ; break;
		case signedRational   : result = "SRATIONAL" ; break;
		case tiffFloat        : result = "FLOAT"     ; break;
		case tiffDouble       : result = "DOUBLE"    ; break;
		case tiffIfd          : result = "IFD"       ; break;
		default                      : result = "unknown"   ; break;
	}
	return result;
}

typedef struct {
	uint16_t tagID;
	uint16_t tagType ;
	uint32_t count;
	uint32_t offset;
} field_t;

void printIFD(FILE* f, std::ostream& out, uint32_t offset,bool bSwap,int depth)
{
	depth++;
	bool bFirst  = true;

	// buffer
	bool bPrint = true;

	do {
		// Read top of directory
		fseek(f,offset,beg);
		uint16_t dir;
		fread(&dir,1,2,f);
		uint16_t dirLength = byteSwap2(dir,0,bSwap);

		bool tooBig = dirLength > 500;

		if ( bFirst && bPrint ) {
			out << indent(depth) << stringFormat("STRUCTURE OF TIFF FILE << path << std::endl;
			if ( tooBig ) out << indent(depth) << "dirLength = " << dirLength << std::endl;
		}
		if  (tooBig) break;

		// Read the dictionary
		for ( int i = 0 ; i < dirLength ; i ++ ) {
			if ( bFirst && bPrint ) {
				out << indent(depth)
					<< " address |    tag                           |     "
					<< " type |    count |    offset | value\n";
			}
			bFirst = false;
			field_t  field;

			fread(&field,sizeof(field_t),1,f);
			uint16_t tag    = byteSwap2(&field.tagID  ,0,bSwap);
			uint16_t type   = byteSwap2(&field.tagType,2,bSwap);
			uint32_t count  = byteSwap4(&field.count  ,4,bSwap);
			uint32_t offset = byteSwap4(&field.offset ,8,bSwap);

			std::string sp  = "" ; // output spacer

			//prepare to print the value
			uint32_t kount  = isStringType(type)     ? (count > 32 ? 32 : count) // restrict long arrays
							: count > 5              ? 5
							: count
							;
			uint32_t pad    = isStringType(type) ? 1 : 0;
			uint32_t size   = isStringType(type) ? 1
							: is2ByteType(type)  ? 2
							: is4ByteType(type)  ? 4
							: is8ByteType(type)  ? 8
							: 1
							;

			DataBuf  buf(size*count + pad);  // allocate a buffer
			std::memcpy(buf.pData_,dir.pData_+8,4);  // copy dir[8:11] into buffer (short strings)
			if ( count*size > 4 ) {            // read into buffer
				size_t   restore = io.tell();  // save
				io.seek(offset,BasicIo::beg);  // position
				io.read(buf.pData_,count*size);// read
				io.seek(restore,BasicIo::beg); // restore
			}

			if ( bPrint ) {
				uint32_t address = start + 2 + i*12 ;
				out << indent(depth)
						<< stringFormat("%8u | %#06x %-25s |%10s |%9u |%10u | "
							,address,tag,tagName(tag,25),typeName(type),count,offset);
				if ( isShortType(type) ){
					for ( size_t k = 0 ; k < kount ; k++ ) {
						out << sp << byteSwap2(buf,k*size,bSwap);
						sp = " ";
					}
				} else if ( isLongType(type) ){
					for ( size_t k = 0 ; k < kount ; k++ ) {
						out << sp << byteSwap4(buf,k*size,bSwap);
						sp = " ";
					}

				} else if ( isRationalType(type) ){
					for ( size_t k = 0 ; k < kount ; k++ ) {
						uint32_t a = byteSwap4(buf,k*size+0,bSwap);
						uint32_t b = byteSwap4(buf,k*size+4,bSwap);
						out << sp << a << "/" << b;
						sp = " ";
					}
				} else if ( isStringType(type) ) {
					out << sp << binaryToString(buf, kount);
				}

				sp = kount == count ? "" : " ...";
				out << sp << std::endl;

				if ( option == kpsRecursive && (tag == 0x8769 /* ExifTag */ || tag == 0x014a/*SubIFDs*/  || type == tiffIfd) ) {
					for ( size_t k = 0 ; k < count ; k++ ) {
						size_t   restore = io.tell();
						uint32_t offset = byteSwap4(buf,k*size,bSwap);
						std::cerr << "tag = " << stringFormat("%#x",tag) << std::endl;
						printIFDStructure(io,out,option,offset,bSwap,c,depth);
						io.seek(restore,BasicIo::beg);
					}
				} else if ( option == kpsRecursive && tag == 0x83bb /* IPTCNAA */ ) {
					size_t   restore = io.tell();  // save
					io.seek(offset,BasicIo::beg);  // position
					byte* bytes=new byte[count] ;  // allocate memory
					io.read(bytes,count)        ;  // read
					io.seek(restore,BasicIo::beg); // restore
					IptcData::printStructure(out,bytes,count,depth);
					delete[] bytes;                // free
				}  else if ( option == kpsRecursive && tag == 0x927c /* MakerNote */ && count > 10) {
					size_t   restore = io.tell();  // save

					uint32_t jump= 10           ;
					byte     bytes[20]          ;
					const char* chars = (const char*) &bytes[0] ;
					io.seek(offset,BasicIo::beg);  // position
					io.read(bytes,jump    )     ;  // read
					bytes[jump]=0               ;
					if ( ::strcmp("Nikon",chars) == 0 ) {
						// tag is an embedded tiff
						byte* bytes=new byte[count-jump] ;  // allocate memory
						io.read(bytes,count-jump)        ;  // read
						MemIo memIo(bytes,count-jump)    ;  // create a file
						std::cerr << "Nikon makernote" << std::endl;
						printTiffStructure(memIo,out,option,depth);
						delete[] bytes                   ;  // free
					} else {
						// tag is an IFD
						io.seek(0,BasicIo::beg);  // position
						std::cerr << "makernote" << std::endl;
						printIFDStructure(io,out,option,offset,bSwap,c,depth);
					}

					io.seek(restore,BasicIo::beg); // restore
				}
			}
		}
		io.read(dir.pData_, 4);
		start = tooBig ? 0 : byteSwap4(dir,0,bSwap);
		out.flush();
	} while (start) ;

	if ( bPrint ) {
		out << indent(depth) << "END " << io.path() << std::endl;
	}
	depth--;
}

int main(int argc,const char* argv[])
{
	int      result = 0;
	int      bSwap  = false;
	uint32_t offset = 0;

	if ( argc > 0 ) {
		FILE* f = fopen(argv[1],"rb");
		if ( f ) {
			char buff[2*WIDTH];
			fread(buff,1,sizeof buff,f);
			bSwap =  (isLittleEndianPlatform() && buff[0] == 'M')
			    ||   (isBigEndianPlatform()    && buff[0] == 'I')
			    ;
			uint16_t magic  = byteSwap2(buff,2,bSwap);
			         offset = byteSwap4(buff,4,bSwap);
			if ( buff[0] != buff[1] || (buff[0] != 'I' && buff[0] != 'M') || magic != 42 ) {
				std::cerr << "bSwap = "  << bSwap  << std::endl;
				std::cerr << "magic  = " << magic  << std::endl;
				std::cerr << "offset = " << offset << std::endl;
				std::cerr << argv[1] << " is not a TIFF file" << std::endl;
				result = 3 ;
			}
			fclose(f);
		} else {
			std::cerr << argv[0] << " unable to open " << argv[1] << std::endl;
			result = 2 ;
		}

	} else {
		std::cerr << argv[0] << " path" << std::endl;
		result = 1 ;
	}

	if ( result == 0 ) {
		std::cout << "Congrats swap = " << (bSwap?"true":"false") << " offset = " << offset << std::endl;
		int depth = 0 ;
		FILE* f = fopen(path,"rb");
		printIFD(f,std::cout,offset,bSwap,depth);
		fclose(f);
	}

	return result;
}
