
#include "basicio.hpp"
#include "image.hpp"

namespace Exiv2
{

Image::UniquePtr newBigTiffInstance(BasicIo::UniquePtr, bool);
bool isBigTiffType(BasicIo &, bool);

}
