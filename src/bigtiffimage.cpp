
#include "bigtiffimage.hpp"

#include <cassert>

#include "exif.hpp"
#include "image_int.hpp"


namespace Exiv2
{

    namespace
    {
        struct Header
        {
            enum Format
            {
                Tiff,
                BigTiff,
            };

            Header(): byteOrder_(invalidByteOrder), version_(-1), data_size_(0), dir_offset_(0) {}
            Header(const ByteOrder& order, int v, int size, uint64_t offset):
                byteOrder_(order),
                version_(v),
                data_size_(size),
                dir_offset_(offset)
            {

            }

            bool isValid() const
            {
                return version_ != -1;
            }

            ByteOrder byteOrder() const
            {
                assert(isValid());
                return byteOrder_;
            }

            int version() const
            {
                assert(isValid());
                return version_;
            }

            Format format() const
            {
                assert(isValid());
                return version_ == 0x2A? Tiff: BigTiff;
            }

            int dataSize() const
            {
                assert(isValid());
                return data_size_;
            }

            uint64_t dirOffset() const
            {
                assert(isValid());
                return dir_offset_;
            }

            private:
                ByteOrder byteOrder_;
                int version_;         // 42 or 43     - regular tiff or big tiff
                int data_size_;       // 4 or 8
                uint64_t dir_offset_;
        };

        struct DirEntry4StandardTiff
        {
            uint16_t tagID;
            uint16_t tagType;
            uint32_t count;
            uint32_t data;
        } __attribute__((packed));

        struct DirEntry4BigTiff
        {
            uint16_t tagID;
            uint16_t tagType;
            uint64_t count;
            uint64_t data;
        } __attribute__((packed));

        union DirEntry
        {
            DirEntry4StandardTiff standardTiff;
            DirEntry4BigTiff      bigTiff;
        };

        std::string indent(int32_t d)
        {
            std::string result ;
            if ( d > 0 )
                while ( d--)
                    result += "  ";

            return result;
        }

        Header readHeader(BasicIo& io)
        {
            byte header[2];
            io.read(header, 2);

            ByteOrder byteOrder = invalidByteOrder;
            if (header[0] == 'I' && header[1] == 'I')
                byteOrder = littleEndian;
            else if (header[0] == 'M' && header[1] == 'M')
                byteOrder = bigEndian;

            if (byteOrder == invalidByteOrder)
                return Header();

            byte version[2];
            io.read(version, 2);

            const uint16_t magic = getUShort(version, byteOrder);

            if (magic != 0x2A && magic != 0x2B)
                return Header();

            Header result;

            if (magic == 0x2A)
            {
                byte buffer[4];
                io.read(buffer, 4);

                const uint32_t offset = getULong(buffer, byteOrder);
                result = Header(byteOrder, magic, 4, offset);
            }
            else
            {
                byte buffer[8];
                io.read(buffer, 2);
                const int size = getUShort(buffer, byteOrder);
                assert(size == 8);

                io.read(buffer, 2); // null

                io.read(buffer, 8);
                const uint64_t offset = getULongLong(buffer, byteOrder);

                result = Header(byteOrder, magic, size, offset);
            }

            return result;
        }

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
        typename TypeForSize<size>::Type conditional_byte_swap_4_array(void* buf, uint64_t offset, bool swap)
        {
            typedef typename TypeForSize<size>::Type Type;

            const uint8_t* bytes_buf = static_cast<uint8_t*>(buf);
            const Type* value = reinterpret_cast<const Type *>(&bytes_buf[offset]);

            return conditional_byte_swap<size>(*value, swap);
        }

        class BigTiffImage: public Image
        {
            public:
                BigTiffImage(BasicIo::AutoPtr io):
                    Image(ImageType::bigtiff, mdExif, io),
                    header_(),
                    dataSize_(0),
                    doSwap_(false)
                {
                    header_ = readHeader(Image::io());
                    assert(header_.isValid());

                    doSwap_ =  (isLittleEndianPlatform() && header_.byteOrder() == bigEndian)
                          ||   (isBigEndianPlatform()    && header_.byteOrder() == littleEndian);

                    dataSize_ = header_.format() == Header::Tiff? 4 : 8;
                }

                virtual ~BigTiffImage() {}

                // overrides
                void readMetadata()
                {

                }

                void writeMetadata()
                {

                }

                std::string mimeType() const
                {

                }

                void printStructure(std::ostream& os, PrintStructureOption option, int depth)
                {
                    printIFD(os, option, header_.dirOffset(), depth);
                }

            private:
                Header header_;
                int dataSize_;
                bool doSwap_;

                void printIFD(std::ostream& out, PrintStructureOption option, uint64_t dir_offset, int depth)
                {
                    BasicIo& io = Image::io();

                    depth++;
                    bool bFirst  = true;

                    // buffer
                    bool bPrint = true;

                    do
                    {
                        // Read top of directory
                        io.seek(dir_offset, BasicIo::beg);

                        const uint64_t entries = readData(header_.format() == Header::Tiff? 2: 8);
                        const bool tooBig = entries > 500;

                        if ( bFirst && bPrint )
                        {
                            out << indent(depth) << Internal::stringFormat("STRUCTURE OF TIFF FILE ") << io.path() << std::endl;
                            if (tooBig)
                                out << indent(depth) << "entries = " << entries << std::endl;
                        }

                        if (tooBig)
                            break;

                        // Read the dictionary
                        for ( uint64_t i = 0; i < entries; i ++ )
                        {
                            if ( bFirst && bPrint )
                                out << indent(depth)
                                    << " address |    tag                           |     "
                                    << " type |    count |    offset | value\n";

                            bFirst = false;

                            const uint16_t tag   = readData(2);
                            const uint16_t type  = readData(2);
                            const uint64_t count = readData(dataSize_);
                            const DataBuf  data  = io.read(dataSize_);        // Read data as raw value. what should be done about it will be decided depending on type

                            std::string sp = "" ; // output spacer

                            //prepare to print the value
                            // TODO: figure out what's going on with kount
                            const uint64_t kount  = isStringType(type)? (count > 32 ? 32 : count) // restrict long arrays
                                                            : count > 5              ? 5
                                                            : count
                                                            ;
                            const uint32_t pad    = isStringType(type) ? 1 : 0;
                            const uint32_t size   = isStringType(type) ? 1
                                                  : is2ByteType(type)  ? 2
                                                  : is4ByteType(type)  ? 4
                                                  : is8ByteType(type)  ? 8
                                                  : 1;

                            DataBuf buf(size * count + pad);

                            const uint64_t offset = header_.format() == Header::Tiff?
                                    conditional_byte_swap_4_array<32>(data.pData_, 0, doSwap_):
                                    conditional_byte_swap_4_array<64>(data.pData_, 0, doSwap_);

                            // big data? Use 'data' as pointer to real data
                            if ( count*size > 8 )                      // read into buffer
                            {
                                size_t   restore = io.tell();          // save
                                io.seek(offset, BasicIo::beg);         // position
                                io.read(buf.pData_, count * size);     // read
                                io.seek(restore, BasicIo::beg);        // restore
                            }
                            else  // use 'data' as data :)
                                std::memcpy(buf.pData_, data.pData_, count * size);     // copy data

                            if ( bPrint )
                            {
                                const int entrySize = header_.format() == Header::Tiff? 12: 20;
                                const uint64_t address = dir_offset + 2 + i * entrySize;
                                out << indent(depth)
                                    << Internal::stringFormat("%8u | %#06x %-25s |%10s |%9u |%10u | ",
                                        address, tag, tagName(tag).c_str(), typeName(type), count, offset);

                                if ( isShortType(type) )
                                {
                                    for ( size_t k = 0 ; k < kount ; k++ )
                                    {
                                        out << sp << conditional_byte_swap_4_array<16>(buf.pData_, k*size, doSwap_);
                                        sp = " ";
                                    }
                                }
                                else if ( isLongType(type) )
                                {
                                    for ( size_t k = 0 ; k < kount ; k++ )
                                    {
                                        out << sp << conditional_byte_swap_4_array<32>(buf.pData_, k*size, doSwap_);
                                        sp = " ";
                                    }
                                }
                                else if ( isLongLongType(type) )
                                {
                                    for ( size_t k = 0 ; k < kount ; k++ )
                                    {
                                        out << sp << conditional_byte_swap_4_array<64>(buf.pData_, k*size, doSwap_);
                                        sp = " ";
                                    }
                                }
                                else if ( isRationalType(type) )
                                {
                                    for ( size_t k = 0 ; k < kount ; k++ )
                                    {
                                        uint32_t a = conditional_byte_swap_4_array<32>(buf.pData_, k*size+0, doSwap_);
                                        uint32_t b = conditional_byte_swap_4_array<32>(buf.pData_, k*size+4, doSwap_);
                                        out << sp << a << "/" << b;
                                        sp = " ";
                                    }
                                }
                                else if ( isStringType(type) )
                                    out << sp << Internal::binaryToString(buf, kount);

                                sp = kount == count ? "" : " ...";
                                out << sp << std::endl;

                                if ( option == kpsRecursive &&
                                        (tag == 0x8769 /* ExifTag */ || tag == 0x014a/*SubIFDs*/ || type == tiffIfd || type == tiffIfd8) )
                                {
                                    for ( size_t k = 0 ; k < count ; k++ )
                                    {
                                        const size_t restore = io.tell();
                                        const uint64_t offset = type == tiffIfd8?
                                            conditional_byte_swap_4_array<64>(buf.pData_, k*size, doSwap_):
                                            conditional_byte_swap_4_array<32>(buf.pData_, k*size, doSwap_);

                                        std::cerr << "tag = " << Internal::stringFormat("%#x",tag) << std::endl;
                                        printIFD(out, option, offset, depth);
                                        io.seek(restore, BasicIo::beg);
                                    }
                                }
                                else if ( option == kpsRecursive && tag == 0x83bb /* IPTCNAA */ )
                                {
                                    size_t   restore = io.tell();  // save
                                    io.seek(dir_offset, BasicIo::beg);  // position
                                    byte* bytes=new byte[count] ;  // allocate memory
                                    io.read(bytes,count)        ;  // read
                                    io.seek(restore, BasicIo::beg); // restore
                                    IptcData::printStructure(out,bytes,count,depth);
                                    delete[] bytes;                // free
                                }
                                else if ( option == kpsRecursive && tag == 0x927c /* MakerNote */ && count > 10)
                                {
                                    size_t   restore = io.tell();  // save

                                    uint32_t jump= 10           ;
                                    byte     bytes[20]          ;
                                    const char* chars = (const char*) &bytes[0] ;
                                    io.seek(dir_offset, BasicIo::beg);  // position
                                    io.read(bytes,jump    )     ;  // read
                                    bytes[jump]=0               ;
                                    if ( ::strcmp("Nikon",chars) == 0 )
                                    {
                                        // tag is an embedded tiff
                                        byte* bytes=new byte[count-jump] ;  // allocate memory
                                        io.read(bytes,count-jump)        ;  // read
                                        MemIo memIo(bytes,count-jump)    ;  // create a file
                                        std::cerr << "Nikon makernote" << std::endl;
                                        // printTiffStructure(memIo,out,option,depth);  TODO: fix it
                                        delete[] bytes                   ;  // free
                                    }
                                    else
                                    {
                                        // tag is an IFD
                                        io.seek(0, BasicIo::beg);  // position
                                        std::cerr << "makernote" << std::endl;
                                        //printIFDStructure(io,out,option,offset,doSwap_,c,depth);  // TODO: fix me
                                    }

                                    io.seek(restore,BasicIo::beg); // restore
                                }
                            }
                        }

                        uint64_t next_dir_offset_raw;
                        io.read(reinterpret_cast<byte*>(&next_dir_offset_raw), dataSize_);
                        dir_offset = tooBig ? 0 : conditional_byte_swap_4_array<64>(&next_dir_offset_raw, 0, doSwap_);
                        out.flush();
                    } while (dir_offset != 0);

                    if ( bPrint )
                        out << indent(depth) << "END " << io.path() << std::endl;

                    depth--;
                }

                uint64_t readData(int size) const
                {
                    uint64_t data = 0;

                    const int read = Image::io().read(reinterpret_cast<byte *>(&data), size);
                    assert(read == size);

                    if (size == 1)
                    {}
                    else if (size == 2)
                        data = conditional_byte_swap<16>(data, doSwap_);
                    else if (size == 4)
                        data = conditional_byte_swap<32>(data, doSwap_);
                    else if (size == 8)
                        data = conditional_byte_swap<64>(data, doSwap_);
                    else
                        assert(!"unexpected size");

                    return data;
                }
        };
    }


    Image::AutoPtr newBigTiffInstance(BasicIo::AutoPtr io, bool)
    {
        return Image::AutoPtr(new BigTiffImage(io));
    }


    bool isBigTiffType(BasicIo& io, bool advance)
    {
        const long pos = io.tell();
        const Header header = readHeader(io);
        const bool valid = header.isValid();

        if (valid == false || advance == false)
            io.seek(pos, BasicIo::beg);

        return valid;
    }

}
