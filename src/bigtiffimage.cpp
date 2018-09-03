
#include "bigtiffimage.hpp"

#include <cassert>
#include <limits>

#include "exif.hpp"
#include "error.hpp"
#include "image_int.hpp"
#include "enforce.hpp"


namespace Exiv2
{

    namespace
    {
        struct Header
        {
            enum Format
            {
                StandardTiff,
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
                return version_ == 0x2A? StandardTiff: BigTiff;
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

        Header readHeader(BasicIo& io)
        {
            byte header[2] = {0, 0};
            io.read(header, 2);

            ByteOrder byteOrder = invalidByteOrder;
            if (header[0] == 'I' && header[1] == 'I')
                byteOrder = littleEndian;
            else if (header[0] == 'M' && header[1] == 'M')
                byteOrder = bigEndian;

            if (byteOrder == invalidByteOrder)
                return Header();

            byte version[2] = {0, 0};
            io.read(version, 2);

            const uint16_t magic = getUShort(version, byteOrder);

            if (magic != 0x2A && magic != 0x2B)
                return Header();

            Header result;

            if (magic == 0x2A)
            {
                byte buffer[4];
                int read = io.read(buffer, 4);

                if (read < 4)
                    throw Exiv2::Error(kerCorruptedMetadata);

                const uint32_t offset = getULong(buffer, byteOrder);
                result = Header(byteOrder, magic, 4, offset);
            }
            else
            {
                byte buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};
                int read = io.read(buffer, 2);
                if (read < 2)
                    throw Exiv2::Error(kerCorruptedMetadata);

                const int size = getUShort(buffer, byteOrder);

                if (size == 8)
                {
                    read = io.read(buffer, 2); // null
                    if (read < 2)
                        throw Exiv2::Error(kerCorruptedMetadata);

                    read = io.read(buffer, 8);
                    if (read < 8)
                        throw Exiv2::Error(kerCorruptedMetadata);

                    const uint64_t offset = getULongLong(buffer, byteOrder);

                    if (offset >= io.size())
                        throw Exiv2::Error(kerCorruptedMetadata);

                    result = Header(byteOrder, magic, size, offset);
                }
                else
                    throw Exiv2::Error(kerCorruptedMetadata);
            }

            return result;
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

                    dataSize_ = header_.format() == Header::StandardTiff? 4 : 8;
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
                    return std::string();
                }

                void printStructure(std::ostream& os, PrintStructureOption option, int depth)
                {
                    printIFD(os, option, header_.dirOffset(), depth - 1);
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

                        const uint64_t entries = readData(header_.format() == Header::StandardTiff? 2: 8);
                        const bool tooBig = entries > 500;

                        if ( bFirst && bPrint )
                        {
                            out << Internal::indent(depth) << Internal::stringFormat("STRUCTURE OF BIGTIFF FILE ") << io.path() << std::endl;
                            if (tooBig)
                                out << Internal::indent(depth) << "entries = " << entries << std::endl;
                        }

                        if (tooBig)
                            break;

                        // Read the dictionary
                        for ( uint64_t i = 0; i < entries; i ++ )
                        {
                            if ( bFirst && bPrint )
                                out << Internal::indent(depth)
                                    << " address |    tag                           |     "
                                    << " type |    count |    offset | value\n";

                            bFirst = false;

                            const uint16_t tag   = (uint16_t) readData(2);
                            const uint16_t type  = (uint16_t) readData(2);
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

                            // #55 and #56 memory allocation crash test/data/POC8

                            // size * count > std::numeric_limits<uint64_t>::max()
                            // =>
                            // size > std::numeric_limits<uint64_t>::max() / count
                            // (don't perform that check when count == 0 => will cause a division by zero exception)
                            if (count != 0) {
                                if (size > std::numeric_limits<uint64_t>::max() / count) {
                                    throw Error(kerInvalidMalloc);             // we got number bigger than 2^64
                                }
                            }
                                                             // more than we can handle

                            if (size * count > std::numeric_limits<uint64_t>::max() - pad)
                                throw Error(kerInvalidMalloc);             // again more than 2^64

                            const uint64_t allocate = size*count + pad;
                            if ( allocate > io.size() ) {
                                throw Error(kerInvalidMalloc);
                            }

                            DataBuf buf(static_cast<long>(allocate));

                            const uint64_t offset = header_.format() == Header::StandardTiff?
                                    byteSwap4(data, 0, doSwap_):
                                    byteSwap8(data, 0, doSwap_);

                            // big data? Use 'data' as pointer to real data
                            const bool usePointer = (size_t) count*size > (size_t) dataSize_;

                            if ( usePointer )                          // read into buffer
                            {
                                size_t   restore = io.tell();          // save
                                io.seek(offset, BasicIo::beg);         // position
                                io.read(buf.pData_, (long) count * size);     // read
                                io.seek(restore, BasicIo::beg);        // restore
                            }
                            else  // use 'data' as data :)
                                std::memcpy(buf.pData_, data.pData_, (size_t) count * size);     // copy data

                            if ( bPrint )
                            {
                                const uint64_t entrySize = header_.format() == Header::StandardTiff? 12: 20;
                                const uint64_t address = dir_offset + 2 + i * entrySize;
                                const std::string offsetString = usePointer?
                                    Internal::stringFormat("%10u", offset):
                                    "";

                                out << Internal::indent(depth)
                                    << Internal::stringFormat("%8u | %#06x %-25s |%10s |%9u |%10s | ",
                                        address, tag, tagName(tag).c_str(), typeName(type), count, offsetString.c_str());

                                if ( isShortType(type) )
                                {
                                    for ( size_t k = 0 ; k < kount ; k++ )
                                    {
                                        out << sp << byteSwap2(buf, k*size, doSwap_);
                                        sp = " ";
                                    }
                                }
                                else if ( isLongType(type) )
                                {
                                    for ( size_t k = 0 ; k < kount ; k++ )
                                    {
                                        out << sp << byteSwap4(buf, k*size, doSwap_);
                                        sp = " ";
                                    }
                                }
                                else if ( isLongLongType(type) )
                                {
                                    for ( size_t k = 0 ; k < kount ; k++ )
                                    {
                                        out << sp << byteSwap8(buf, k*size, doSwap_);
                                        sp = " ";
                                    }
                                }
                                else if ( isRationalType(type) )
                                {
                                    for ( size_t k = 0 ; k < kount ; k++ )
                                    {
                                        uint32_t a = byteSwap4(buf, k*size+0, doSwap_);
                                        uint32_t b = byteSwap4(buf, k*size+4, doSwap_);
                                        out << sp << a << "/" << b;
                                        sp = " ";
                                    }
                                }
                                else if ( isStringType(type) )
                                    out << sp << Internal::binaryToString(buf, (size_t) kount);

                                sp = kount == count ? "" : " ...";
                                out << sp << std::endl;

                                if ( option == kpsRecursive &&
                                        (tag == 0x8769 /* ExifTag */ || tag == 0x014a/*SubIFDs*/ || type == tiffIfd || type == tiffIfd8) )
                                {
                                    for ( size_t k = 0 ; k < count ; k++ )
                                    {
                                        const size_t restore = io.tell();
                                        const uint64_t ifdOffset = type == tiffIfd8?
                                            byteSwap8(buf, k*size, doSwap_):
                                            byteSwap4(buf, k*size, doSwap_);

                                        printIFD(out, option, ifdOffset, depth);
                                        io.seek(restore, BasicIo::beg);
                                    }
                                }
                                else if ( option == kpsRecursive && tag == 0x83bb /* IPTCNAA */ )
                                {
                                    const size_t restore = io.tell();      // save
                                    io.seek(offset, BasicIo::beg);         // position
                                    byte* bytes=new byte[(size_t)count] ;  // allocate memory
                                    io.read(bytes,(long)count)        ;    // read
                                    io.seek(restore, BasicIo::beg);        // restore
                                    IptcData::printStructure(out,bytes,(size_t)count,depth);
                                    delete[] bytes;                        // free
                                }
                                else if ( option == kpsRecursive && tag == 0x927c /* MakerNote */ && count > 10)
                                {
                                    size_t   restore = io.tell();  // save

                                    long jump= 10           ;
                                    byte     bytes[20]          ;
                                    const char* chars = (const char*) &bytes[0] ;
                                    io.seek(dir_offset, BasicIo::beg);  // position
                                    io.read(bytes,jump    )     ;  // read
                                    bytes[jump]=0               ;
                                    if ( ::strcmp("Nikon",chars) == 0 )
                                    {
                                      // tag is an embedded tiff
                                      std::vector<byte> nikon_bytes(count - jump);

                                      io.read(&nikon_bytes.at(0), (long)nikon_bytes.size());
                                      MemIo memIo(&nikon_bytes.at(0), (long)count - jump); // create a file
                                      std::cerr << "Nikon makernote" << std::endl;
                                      // printTiffStructure(memIo,out,option,depth);
                                      // TODO: fix it
                                    }
                                    else
                                    {
                                        // tag is an IFD
                                        io.seek(0, BasicIo::beg);  // position
                                        std::cerr << "makernote" << std::endl;
                                        printIFD(out,option,offset,depth);
                                    }

                                    io.seek(restore,BasicIo::beg); // restore
                                }
                            }
                        }

                        const uint64_t nextDirOffset = readData(dataSize_);

                        dir_offset = tooBig ? 0 : nextDirOffset;
                        out.flush();
                    } while (dir_offset != 0);

                    if ( bPrint )
                        out << Internal::indent(depth) << "END " << io.path() << std::endl;

                    depth--;
                }

                uint64_t readData(int size) const
                {
                    const DataBuf data = Image::io().read(size);
                    enforce(data.size_ != 0, kerCorruptedMetadata);

                    uint64_t result = 0;

                    if (data.size_ == 1)
                        {}
                    else if (data.size_ == 2)
                        result = byteSwap2(data, 0, doSwap_);
                    else if (data.size_ == 4)
                        result = byteSwap4(data, 0, doSwap_);
                    else if (data.size_ == 8)
                        result = byteSwap8(data, 0, doSwap_);
                    else
                        throw Exiv2::Error(kerCorruptedMetadata);

                    return result;
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
