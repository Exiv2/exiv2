#include <gtest/gtest.h>

#include <exiv2/datasets.hpp>
#include <exiv2/error.hpp>

using namespace Exiv2;

TEST(IptcKey, creationWithNonValidStringFormatThrows)
{
    try {
      IptcKey key("Yeah");
      FAIL();
    }  catch (const Exiv2::Error& e) {
      ASSERT_EQ(kerInvalidKey, e.code());
      ASSERT_STREQ("Invalid key 'Yeah'", e.what());
    }
}

TEST(IptcKey, creationWithNonValidRecordNameThrows)
{
    try {
      IptcKey key("Iptc.WrongRecordName.ModelVersion");
      FAIL();
    }  catch (const Exiv2::Error& e) {
      ASSERT_EQ(kerInvalidRecord, e.code());
      ASSERT_STREQ("Invalid record name 'WrongRecordName'", e.what());
    }
}

TEST(IptcKey, creationWithNonValidDatasetNameThrows)
{
    try {
      IptcKey key("Iptc.Envelope.WrongDataset");
      FAIL();
    }  catch (const Exiv2::Error& e) {
      ASSERT_EQ(kerInvalidDataset, e.code());
      ASSERT_STREQ("Invalid dataset name 'WrongDataset'", e.what());
    }
}

TEST(IptcKey, creationWithValidStringDoesNotThrow)
{
    ASSERT_NO_THROW(IptcKey ("Iptc.Envelope.ModelVersion"));
}
