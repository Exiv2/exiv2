#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <exiv2/datasets.hpp>
#include <exiv2/error.hpp>

#include <array>

using namespace Exiv2;
using ::testing::StartsWith;

TEST(IptcDataSets, dataSetName_returnsValidNamesWhenRequestingNumbersAvailableInEnvelopeRecord)
{
    ASSERT_EQ("ModelVersion", IptcDataSets::dataSetName(IptcDataSets::ModelVersion, IptcDataSets::envelope));
    ASSERT_EQ("Destination", IptcDataSets::dataSetName(IptcDataSets::Destination, IptcDataSets::envelope));
    ASSERT_EQ("FileFormat", IptcDataSets::dataSetName(IptcDataSets::FileFormat, IptcDataSets::envelope));
    ASSERT_EQ("FileVersion", IptcDataSets::dataSetName(IptcDataSets::FileVersion, IptcDataSets::envelope));
    ASSERT_EQ("ServiceId", IptcDataSets::dataSetName(IptcDataSets::ServiceId, IptcDataSets::envelope));
    ASSERT_EQ("EnvelopeNumber", IptcDataSets::dataSetName(IptcDataSets::EnvelopeNumber, IptcDataSets::envelope));
}

TEST(IptcDataSets, dataSetName_returnsValidNamesWhenRequestingNumbersAvailableInApplicationRecord)
{
    ASSERT_EQ("ObjectType", IptcDataSets::dataSetName(IptcDataSets::ObjectType, IptcDataSets::application2));
    ASSERT_EQ("ObjectAttribute", IptcDataSets::dataSetName(IptcDataSets::ObjectAttribute, IptcDataSets::application2));
}

TEST(IptcDataSets, dataSetName_returnsWrongNamesWhenRequestingNumbersNotAvailableInEnvelopeRecord)
{
    ASSERT_EQ("0x0003", IptcDataSets::dataSetName(IptcDataSets::ObjectType, IptcDataSets::envelope));
    ASSERT_EQ("0x0004", IptcDataSets::dataSetName(IptcDataSets::ObjectAttribute, IptcDataSets::envelope));
}

TEST(IptcDataSets, dataSetTitle_returnsValidTitleWhenRequestingNumbersAvailableInRecord)
{
    ASSERT_STREQ("Model Version", IptcDataSets::dataSetTitle(IptcDataSets::ModelVersion, IptcDataSets::envelope));
    ASSERT_STREQ("Destination", IptcDataSets::dataSetTitle(IptcDataSets::Destination, IptcDataSets::envelope));
    ASSERT_STREQ("File Format", IptcDataSets::dataSetTitle(IptcDataSets::FileFormat, IptcDataSets::envelope));

    ASSERT_STREQ("Object Type", IptcDataSets::dataSetTitle(IptcDataSets::ObjectType, IptcDataSets::application2));
    ASSERT_STREQ("Object Attribute",
                 IptcDataSets::dataSetTitle(IptcDataSets::ObjectAttribute, IptcDataSets::application2));
}

TEST(IptcDataSets, dataSetTitle_returnsUnknownStringWhenRequestingNumbersNotAvailableInEnvelopeRecord)
{
    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetTitle(IptcDataSets::ObjectType, IptcDataSets::envelope));
    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetTitle(IptcDataSets::ObjectAttribute, IptcDataSets::envelope));
}

// Unfortunately, some constants such as ModelVersion, Destination or FileFormat has the same values as other constants
// available for other records (RecordVersion, ObjectName & SuppCategory respectively)

// TEST(IptcDataSets, dataSetTitle_returnsUnknownStringWhenRequestingNumbersNotAvailableInApplicationRecord)
//{
//  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetTitle(IptcDataSets::ModelVersion, IptcDataSets::application2));
//  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetTitle(IptcDataSets::Destination, IptcDataSets::application2));
//  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetTitle(IptcDataSets::FileFormat, IptcDataSets::application2));
//}

TEST(IptcDataSets, dataSetTitle_returnsUnknownStringWhenRequestingNumbersNotAvailableInApplicationRecord)
{
    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetTitle(1, IptcDataSets::envelope));
    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetTitle(2, IptcDataSets::envelope));
}

// ----------------------

TEST(IptcDataSets, dataSetDescription_returnsValidDescriptionWhenRequestingNumbersAvailableInRecord)
{
    ASSERT_THAT(IptcDataSets::dataSetDesc(IptcDataSets::ModelVersion, IptcDataSets::envelope),
                StartsWith("A binary number identifying the version of the Information Interchange Model"));
    ASSERT_THAT(IptcDataSets::dataSetDesc(IptcDataSets::FileFormat, IptcDataSets::envelope),
                StartsWith("A binary number representing the file format. The file format must be registered with"));

    ASSERT_THAT(IptcDataSets::dataSetDesc(IptcDataSets::RecordVersion, IptcDataSets::application2),
                StartsWith("A binary number identifying the version of the Information Interchange Model"));
    ASSERT_THAT(IptcDataSets::dataSetDesc(IptcDataSets::ObjectType, IptcDataSets::application2),
                StartsWith("The Object Type is used to distinguish between different types of objects within the IIM"));
}

TEST(IptcDataSets, dataSetDescription_returnsUnknownStringWhenRequestingNumbersNotAvailableInRecord)
{
    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetDesc(1, IptcDataSets::envelope));
    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetDesc(2, IptcDataSets::envelope));

    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetDesc(1, IptcDataSets::application2));
    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetDesc(2, IptcDataSets::application2));
}

// ----------------------

TEST(IptcDataSets, dataSetPsName_returnsValidPsNameWhenRequestingNumbersAvailableInRecord)
{
    ASSERT_STREQ("", IptcDataSets::dataSetPsName(IptcDataSets::FileFormat, IptcDataSets::envelope));

    ASSERT_STREQ("Document Title", IptcDataSets::dataSetPsName(IptcDataSets::ObjectName, IptcDataSets::application2));
    ASSERT_STREQ("Urgency", IptcDataSets::dataSetPsName(IptcDataSets::Urgency, IptcDataSets::application2));
}

TEST(IptcDataSets, dataSetPsName_returnsUnknownStringWhenRequestingNumbersNotAvailableInRecord)
{
    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetPsName(1, IptcDataSets::envelope));
    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetPsName(2, IptcDataSets::envelope));

    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetPsName(1, IptcDataSets::application2));
    ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetPsName(2, IptcDataSets::application2));
}

// ----------------------

TEST(IptcDataSets, dataSetRepeatable_returnsExpectedValueNameWhenRequestingNumbersAvailableInRecord)
{
    ASSERT_TRUE(IptcDataSets::dataSetRepeatable(IptcDataSets::Destination, IptcDataSets::envelope));
    ASSERT_FALSE(IptcDataSets::dataSetRepeatable(IptcDataSets::FileFormat, IptcDataSets::envelope));

    ASSERT_FALSE(IptcDataSets::dataSetRepeatable(IptcDataSets::ObjectType, IptcDataSets::application2));
    ASSERT_TRUE(IptcDataSets::dataSetRepeatable(IptcDataSets::ObjectAttribute, IptcDataSets::application2));
}

/// \todo check if we want to return true in this case or throw an exception ...
TEST(IptcDataSets, dataSetRepeatable_returnsTrueWhenRequestingNumbersNotAvailableInRecord)
{
    ASSERT_TRUE(IptcDataSets::dataSetRepeatable(1, IptcDataSets::envelope));
    ASSERT_TRUE(IptcDataSets::dataSetRepeatable(2, IptcDataSets::envelope));

    ASSERT_TRUE(IptcDataSets::dataSetRepeatable(1, IptcDataSets::application2));
    ASSERT_TRUE(IptcDataSets::dataSetRepeatable(2, IptcDataSets::application2));
}

// ----------------------

TEST(IptcDataSets, dataSet_returnsExpectedValueWhenRequestingValidDatasetName)
{
    ASSERT_EQ(IptcDataSets::ModelVersion, IptcDataSets::dataSet("ModelVersion", IptcDataSets::envelope));
    ASSERT_EQ(IptcDataSets::FileFormat, IptcDataSets::dataSet("FileFormat", IptcDataSets::envelope));

    ASSERT_EQ(IptcDataSets::RecordVersion, IptcDataSets::dataSet("RecordVersion", IptcDataSets::application2));
    ASSERT_EQ(IptcDataSets::FixtureId, IptcDataSets::dataSet("FixtureId", IptcDataSets::application2));
}

TEST(IptcDataSets, dataSet_throwWithNonExistingDatasetName)
{
    try {
        IptcDataSets::dataSet("NonExistingName", IptcDataSets::envelope);
        FAIL();
    } catch (const Exiv2::Error& e) {
        ASSERT_EQ(kerInvalidDataset, e.code());
        ASSERT_STREQ("Invalid dataset name `NonExistingName'", e.what());
    }
}

/// \todo Weird error reporting here. It should indicate that the record specified does not exist
TEST(IptcDataSets, dataSet_throwWithNonExistingRecordId)
{
    try {
        IptcDataSets::dataSet("ModelVersion", 5);
        FAIL();
    } catch (const Exiv2::Error& e) {
        ASSERT_EQ(kerInvalidDataset, e.code());
        ASSERT_STREQ("Invalid dataset name `ModelVersion'", e.what());
    }
}
