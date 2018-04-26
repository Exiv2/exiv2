#include <XMP_Const.h>
#include "gtestwrapper.h"


TEST(XmpTypes, bitsize)
{
    ASSERT_EQ(1, sizeof(XMP_Int8));
    ASSERT_EQ(2, sizeof(XMP_Int16));
    ASSERT_EQ(4, sizeof(XMP_Int32));
    ASSERT_EQ(8, sizeof(XMP_Int64));

    ASSERT_EQ(1, sizeof(XMP_Uns8));
    ASSERT_EQ(2, sizeof(XMP_Uns16));
    ASSERT_EQ(4, sizeof(XMP_Uns32));
    ASSERT_EQ(8, sizeof(XMP_Uns64));
}

