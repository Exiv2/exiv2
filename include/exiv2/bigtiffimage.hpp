
#include "basicio.hpp"
#include "image.hpp"

namespace Exiv2
{

namespace ImageType
{
    const int bigtiff = 25;
}

Image::UniquePtr newBigTiffInstance(BasicIo::UniquePtr, bool);
bool isBigTiffType(BasicIo &, bool);

}
