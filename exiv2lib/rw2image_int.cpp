#include "rw2image_int.hpp"

namespace Exiv2 {
    namespace Internal {

    Rw2Header::Rw2Header()
        : TiffHeaderBase(0x0055, 24, littleEndian, 0x00000018)
    {
    }

    Rw2Header::~Rw2Header()
    {
    }

    DataBuf Rw2Header::write() const
    {
        // Todo: Implement me!
        return DataBuf();
    }

}}                                      // namespace Internal, Exiv2
