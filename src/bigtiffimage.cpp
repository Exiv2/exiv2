
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

    bool isBigTiffType(BasicIo &, bool)
    {
        return false;
    }

}
