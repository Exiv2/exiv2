
#include "bigtiffimage.hpp"



namespace Exiv2
{

Image::AutoPtr newBigTiffInstance(BasicIo::AutoPtr, bool)
{
    return Image::AutoPtr();
}

bool isBigTiffType(BasicIo &, bool)
{
    return false;
}

}
