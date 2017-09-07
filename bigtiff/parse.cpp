
#include <cstring>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <iostream>

#include "basicio.hpp"
#include "iptc.hpp"
#include "image_int.hpp"
#include "image.hpp"
#include "types.hpp"

#define WIDTH 32

// helpful links:
// http://www.awaresystems.be/imaging/tiff/bigtiff.html
// http://bigtiff.org/
// https://stackoverflow.com/questions/105252/how-do-i-convert-between-big-endian-and-little-endian-values-in-c

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

constexpr bool isBigEndianPlatform()
{
	union {
		uint32_t i;
		char c[4];
	} e = { 0x01000000 };

	return e.c[0]?true:false;
}

constexpr bool isLittleEndianPlatform() { return !isBigEndianPlatform(); }

template<int>
struct TypeForSize {};

template<>
struct TypeForSize<16>
{
    typedef int16_t Type;
};

template<>
struct TypeForSize<32>
{
    typedef int32_t Type;
};

template<>
struct TypeForSize<64>
{
    typedef int64_t Type;
};

template<int size>
typename TypeForSize<size>::Type byte_swap(const typename TypeForSize<size>::Type& v)
{
    static_assert(size == 16 || size == 32 || size == 64);

    typename TypeForSize<size>::Type result = 0;
    if (size == 16)
        result = __builtin_bswap16(v);
    else if (size == 32)
        result = __builtin_bswap32(v);
    else if (size == 64)
        result = __builtin_bswap64(v);

    return result;
}

template<int size>
typename TypeForSize<size>::Type conditional_byte_swap(const typename TypeForSize<size>::Type& v, bool swap)
{
    const typename TypeForSize<size>::Type result = swap? byte_swap<size>(v): v;

    return result;
}


template<int size>
typename TypeForSize<size>::Type conditional_byte_swap_4_array(void* buf, int offset, bool swap)
{
    typedef typename TypeForSize<size>::Type Type;

    const uint8_t* bytes_buf = static_cast<uint8_t*>(buf);
    const Type* value = reinterpret_cast<const Type *>(&bytes_buf[offset]);

    return conditional_byte_swap<size>(*value, swap);
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

void printIFD(Exiv2::BasicIo& io, std::ostream& out, Exiv2::PrintStructureOption option, uint32_t offset, bool bSwap, int depth)
{
	depth++;
	bool bFirst  = true;

	// buffer
	bool bPrint = true;

        int start = 0;         // TODO: just a buld fix

	do {
		// Read top of directory
                io.seek(offset, Exiv2::BasicIo::beg);

		uint16_t dir;
                io.read(reinterpret_cast<Exiv2::byte *>(&dir), 2);

		uint16_t dirLength = conditional_byte_swap_4_array<16>(&dir, 0, bSwap);

		bool tooBig = dirLength > 500;

		if ( bFirst && bPrint ) {
			out << indent(depth) << Exiv2::Internal::stringFormat("STRUCTURE OF TIFF FILE") << io.path() << std::endl;
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

                        io.read(reinterpret_cast<Exiv2::byte*>(&field), sizeof(field));
			uint16_t tag    = conditional_byte_swap_4_array<16>(&field.tagID,   0, bSwap);
			uint16_t type   = conditional_byte_swap_4_array<16>(&field.tagType, 2, bSwap);
			uint32_t count  = conditional_byte_swap_4_array<32>(&field.count,   4, bSwap);
			uint32_t offset = conditional_byte_swap_4_array<32>(&field.offset,  8, bSwap);

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

			Exiv2::DataBuf  buf(size*count + pad);  // allocate a buffer
                        Exiv2::DataBuf  dir(size*count + pad);  // TODO: fix me, I'm object out of nowhere
			std::memcpy(buf.pData_,dir.pData_+8,4);  // copy dir[8:11] into buffer (short strings)
			if ( count*size > 4 ) {            // read into buffer
				size_t   restore = io.tell();  // save
				io.seek(offset, Exiv2::BasicIo::beg);  // position
				io.read(buf.pData_,count*size);// read
				io.seek(restore, Exiv2::BasicIo::beg); // restore
			}

			if ( bPrint ) {
				uint32_t address = start + 2 + i*12 ;
				out << indent(depth)
						<< Exiv2::Internal::stringFormat("%8u | %#06x %-25s |%10s |%9u |%10u | "
							,address,tag,tagName(tag,25),typeName(type),count,offset);
				if ( isShortType(type) ){
					for ( size_t k = 0 ; k < kount ; k++ ) {
						out << sp << conditional_byte_swap_4_array<16>(&buf, k*size, bSwap);
						sp = " ";
					}
				} else if ( isLongType(type) ){
					for ( size_t k = 0 ; k < kount ; k++ ) {
						out << sp << conditional_byte_swap_4_array<32>(&buf, k*size, bSwap);
						sp = " ";
					}

				} else if ( isRationalType(type) ){
					for ( size_t k = 0 ; k < kount ; k++ ) {
						uint32_t a = conditional_byte_swap_4_array<32>(&buf, k*size+0, bSwap);
						uint32_t b = conditional_byte_swap_4_array<32>(&buf, k*size+4, bSwap);
						out << sp << a << "/" << b;
						sp = " ";
					}
				} else if ( isStringType(type) ) {
					out << sp << Exiv2::Internal::binaryToString(buf, kount);
				}

				sp = kount == count ? "" : " ...";
				out << sp << std::endl;

				if ( option == Exiv2::kpsRecursive && (tag == 0x8769 /* ExifTag */ || tag == 0x014a/*SubIFDs*/  || type == tiffIfd) ) {
					for ( size_t k = 0 ; k < count ; k++ ) {
						size_t   restore = io.tell();
						uint32_t offset = conditional_byte_swap_4_array<32>(&buf, k*size, bSwap);
						std::cerr << "tag = " << Exiv2::Internal::stringFormat("%#x",tag) << std::endl;
						//Exiv2::RiffVideo(io).printIFDStructure(io,out,option,offset,bSwap,c,depth);   // TODO: blind fix
						io.seek(restore, Exiv2::BasicIo::beg);
					}
				} else if ( option == Exiv2::kpsRecursive && tag == 0x83bb /* IPTCNAA */ ) {
					size_t   restore = io.tell();  // save
					io.seek(offset, Exiv2::BasicIo::beg);  // position
					Exiv2::byte* bytes=new Exiv2::byte[count] ;  // allocate memory
					io.read(bytes,count)        ;  // read
					io.seek(restore, Exiv2::BasicIo::beg); // restore
					Exiv2::IptcData::printStructure(out,bytes,count,depth);
					delete[] bytes;                // free
				}  else if ( option == Exiv2::kpsRecursive && tag == 0x927c /* MakerNote */ && count > 10) {
					size_t   restore = io.tell();  // save

					uint32_t jump= 10           ;
					Exiv2::byte     bytes[20]          ;
					const char* chars = (const char*) &bytes[0] ;
					io.seek(offset, Exiv2::BasicIo::beg);  // position
					io.read(bytes,jump    )     ;  // read
					bytes[jump]=0               ;
					if ( ::strcmp("Nikon",chars) == 0 ) {
						// tag is an embedded tiff
						Exiv2::byte* bytes=new Exiv2::byte[count-jump] ;  // allocate memory
						io.read(bytes,count-jump)        ;  // read
						Exiv2::MemIo memIo(bytes,count-jump)    ;  // create a file
						std::cerr << "Nikon makernote" << std::endl;
						// printTiffStructure(memIo,out,option,depth);  TODO: fix it
						delete[] bytes                   ;  // free
					} else {
						// tag is an IFD
						io.seek(0, Exiv2::BasicIo::beg);  // position
						std::cerr << "makernote" << std::endl;
						//printIFDStructure(io,out,option,offset,bSwap,c,depth);  // TODO: fix me
					}

					io.seek(restore,Exiv2::BasicIo::beg); // restore
				}
			}
		}
		//io.read(&dir.pData_, 4);   TODO: fix me
		start = tooBig ? 0 : conditional_byte_swap_4_array<32>(&dir, 0, bSwap);
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
	bool     bSwap  = false;
	uint32_t offset = 0;

	if ( argc > 0 ) {
		FILE* f = fopen(argv[1],"rb");
		if ( f ) {
			char buff[2*WIDTH];
			fread(buff,1,sizeof buff,f);

			bSwap =  (isLittleEndianPlatform() && buff[0] == 'M')
			    ||   (isBigEndianPlatform()    && buff[0] == 'I')
			    ;

			uint16_t magic    = conditional_byte_swap_4_array<16>(buff, 2, bSwap);
                        uint16_t byteSize = conditional_byte_swap_4_array<16>(buff, 4, bSwap);
                        uint16_t zeroByte = conditional_byte_swap_4_array<16>(buff, 6, bSwap);
			         offset   = conditional_byte_swap_4_array<64>(buff, 8, bSwap);

			if ( buff[0] != buff[1]                ||
                            (buff[0] != 'I' && buff[0] != 'M') ||
                             magic != 43                       ||
                             byteSize != 8                     ||
                             zeroByte != 0
                           )
                        {
				std::cerr << "bSwap     = " << bSwap    << std::endl;
				std::cerr << "magic     = " << magic    << std::endl;
                                std::cerr << "byteSize  = " << byteSize << std::endl;
                                std::cerr << "zeroByte  = " << zeroByte << std::endl;
				std::cerr << "offset    = " << offset   << std::endl;
				std::cerr << argv[1] << " is not a BigTIFF file" << std::endl;
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
                Exiv2::FileIo file(argv[1]);
                file.open("rb");
		printIFD(file, std::cout, Exiv2::kpsRecursive, offset, bSwap, depth);
	}

	return result;
}
