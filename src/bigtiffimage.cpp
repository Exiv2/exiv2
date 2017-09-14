
#include "bigtiffimage.hpp"

#include <cassert>

namespace Exiv2
{

    namespace
    {
        struct Header
        {
            ByteOrder byteOrder;
            int version;         // 42 or 43     - regular tiff or big tiff
            int data_size;       // 4 or 8
            uint64_t dir_offset;

            bool isValid() const { return version != -1; }

            Header(): byteOrder(invalidByteOrder), version(-1), data_size(0), dir_offset(0) {}
            Header(const ByteOrder& order, int v, int size, uint64_t offset):
                byteOrder(order),
                version(v),
                data_size(size),
                dir_offset(offset)
            {

            }
        };

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

        class BigTiffImage: public Image
        {
            public:
                BigTiffImage(BasicIo::AutoPtr io): Image(ImageType::bigtiff, mdExif, io)
                {

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

                }
        };
    }


    Image::AutoPtr newBigTiffInstance(BasicIo::AutoPtr io, bool)
    {
        return Image::AutoPtr(new BigTiffImage(io));
    }

    bool isBigTiffType(BasicIo& io, bool)
    {
        const Header header = readHeader(io);

        return header.isValid();
    }

}
