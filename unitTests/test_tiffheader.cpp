#include <tiffimage_int.hpp>

#include "gtestwrapper.h"
#include <sstream>

using namespace Exiv2;

static const byte tiffLittleEndian[] = {0x49, 0x49, 0x2a, 0x00, 0x10, 0x00, 0x00, 0x00};

class ATiffHeader: public ::testing::Test
{
public:
    Internal::TiffHeader header;
};

TEST_F(ATiffHeader, hasExpectedValuesAfterCreation)
{
    ASSERT_EQ(8u, header.size());
    ASSERT_EQ(42, header.tag());
    ASSERT_EQ(8u, header.offset());
    ASSERT_EQ(littleEndian, header.byteOrder());
}
