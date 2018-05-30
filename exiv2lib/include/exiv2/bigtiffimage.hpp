
#include "basicio.hpp"
#include "image.hpp"

namespace Exiv2
{

namespace ImageType
{
    const int bigtiff = 22;
}

Image::AutoPtr newBigTiffInstance(BasicIo::AutoPtr, bool);
bool isBigTiffType(BasicIo &, bool);

}
