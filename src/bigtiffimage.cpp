
#include "bigtiffimage.hpp"


namespace Exiv2
{

    namespace
    {
        class BigTiffImage: public Image
        {
            public:
                BigTiffImage(BasicIo::AutoPtr io): Image(ImageType::bigtiff, mdExif, io) {}

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
        };
    }


    Image::AutoPtr newBigTiffInstance(BasicIo::AutoPtr io, bool)
    {
        return Image::AutoPtr(new BigTiffImage(io));
    }

    bool isBigTiffType(BasicIo& io, bool)
    {
        byte header[2];
        io.read(header, 2);

        ByteOrder byteOrder = invalidByteOrder;
        if (header[0] == 'I' && header[1] == 'I')
            byteOrder = littleEndian;
        else if (header[0] == 'M' && header[1] == 'M')
            byteOrder = bigEndian;

        bool isBigTiff = false;
        if (byteOrder != invalidByteOrder)
        {
            byte version[2];
            io.read(version, 2);

            const uint16_t magic = getUShort(version, byteOrder);

            isBigTiff = magic == 0x2A || magic == 0x2B;
        }

        return isBigTiff;
    }

}
