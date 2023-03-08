// SPDX-License-Identifier: GPL-2.0-or-later

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <exiv2/datasets.hpp>
#include <exiv2/error.hpp>

#include <array>
#include <sstream>

using namespace Exiv2;
using ::testing::StartsWith;

TEST(IptcDataSets, dataSetNameReturnsValidNamesWhenRequestingNumbersAvailableInEnvelopeRecord) {
  ASSERT_EQ("ModelVersion", IptcDataSets::dataSetName(IptcDataSets::ModelVersion, IptcDataSets::envelope));
  ASSERT_EQ("Destination", IptcDataSets::dataSetName(IptcDataSets::Destination, IptcDataSets::envelope));
  ASSERT_EQ("FileFormat", IptcDataSets::dataSetName(IptcDataSets::FileFormat, IptcDataSets::envelope));
  ASSERT_EQ("FileVersion", IptcDataSets::dataSetName(IptcDataSets::FileVersion, IptcDataSets::envelope));
  ASSERT_EQ("ServiceId", IptcDataSets::dataSetName(IptcDataSets::ServiceId, IptcDataSets::envelope));
  ASSERT_EQ("EnvelopeNumber", IptcDataSets::dataSetName(IptcDataSets::EnvelopeNumber, IptcDataSets::envelope));
}

TEST(IptcDataSets, dataSetNameReturnsValidNamesWhenRequestingNumbersAvailableInApplicationRecord) {
  ASSERT_EQ("ObjectType", IptcDataSets::dataSetName(IptcDataSets::ObjectType, IptcDataSets::application2));
  ASSERT_EQ("ObjectAttribute", IptcDataSets::dataSetName(IptcDataSets::ObjectAttribute, IptcDataSets::application2));
}

TEST(IptcDataSets, dataSetNameReturnsWrongNamesWhenRequestingNumbersNotAvailableInEnvelopeRecord) {
  ASSERT_EQ("0x0003", IptcDataSets::dataSetName(IptcDataSets::ObjectType, IptcDataSets::envelope));
  ASSERT_EQ("0x0004", IptcDataSets::dataSetName(IptcDataSets::ObjectAttribute, IptcDataSets::envelope));
}

TEST(IptcDataSets, dataSetTitleReturnsValidTitleWhenRequestingNumbersAvailableInRecord) {
  ASSERT_STREQ("Model Version", IptcDataSets::dataSetTitle(IptcDataSets::ModelVersion, IptcDataSets::envelope));
  ASSERT_STREQ("Destination", IptcDataSets::dataSetTitle(IptcDataSets::Destination, IptcDataSets::envelope));
  ASSERT_STREQ("File Format", IptcDataSets::dataSetTitle(IptcDataSets::FileFormat, IptcDataSets::envelope));

  ASSERT_STREQ("Object Type", IptcDataSets::dataSetTitle(IptcDataSets::ObjectType, IptcDataSets::application2));
  ASSERT_STREQ("Object Attribute",
               IptcDataSets::dataSetTitle(IptcDataSets::ObjectAttribute, IptcDataSets::application2));
}

TEST(IptcDataSets, dataSetTitleReturnsUnknownStringWhenRequestingNumbersNotAvailableInEnvelopeRecord) {
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

TEST(IptcDataSets, dataSetTitleReturnsUnknownStringWhenRequestingNumbersNotAvailableInApplicationRecord) {
  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetTitle(1, IptcDataSets::envelope));
  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetTitle(2, IptcDataSets::envelope));
}

// ----------------------

TEST(IptcDataSets, dataSetDescriptionReturnsValidDescriptionWhenRequestingNumbersAvailableInRecord) {
  ASSERT_THAT(IptcDataSets::dataSetDesc(IptcDataSets::ModelVersion, IptcDataSets::envelope),
              StartsWith("A binary number identifying the version of the Information Interchange Model"));
  ASSERT_THAT(IptcDataSets::dataSetDesc(IptcDataSets::FileFormat, IptcDataSets::envelope),
              StartsWith("A binary number representing the file format. The file format must be registered with"));

  ASSERT_THAT(IptcDataSets::dataSetDesc(IptcDataSets::RecordVersion, IptcDataSets::application2),
              StartsWith("A binary number identifying the version of the Information Interchange Model"));
  ASSERT_THAT(IptcDataSets::dataSetDesc(IptcDataSets::ObjectType, IptcDataSets::application2),
              StartsWith("The Object Type is used to distinguish between different types of objects within the IIM"));
}

TEST(IptcDataSets, dataSetDescriptionReturnsUnknownStringWhenRequestingNumbersNotAvailableInRecord) {
  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetDesc(1, IptcDataSets::envelope));
  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetDesc(2, IptcDataSets::envelope));

  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetDesc(1, IptcDataSets::application2));
  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetDesc(2, IptcDataSets::application2));
}

// ----------------------

TEST(IptcDataSets, dataSetPsNameReturnsValidPsNameWhenRequestingNumbersAvailableInRecord) {
  ASSERT_STREQ("", IptcDataSets::dataSetPsName(IptcDataSets::FileFormat, IptcDataSets::envelope));

  ASSERT_STREQ("Document Title", IptcDataSets::dataSetPsName(IptcDataSets::ObjectName, IptcDataSets::application2));
  ASSERT_STREQ("Urgency", IptcDataSets::dataSetPsName(IptcDataSets::Urgency, IptcDataSets::application2));
}

TEST(IptcDataSets, dataSetPsNameReturnsUnknownStringWhenRequestingNumbersNotAvailableInRecord) {
  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetPsName(1, IptcDataSets::envelope));
  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetPsName(2, IptcDataSets::envelope));

  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetPsName(1, IptcDataSets::application2));
  ASSERT_STREQ("Unknown dataset", IptcDataSets::dataSetPsName(2, IptcDataSets::application2));
}

// ----------------------

TEST(IptcDataSets, dataSetRepeatableReturnsExpectedValueNameWhenRequestingNumbersAvailableInRecord) {
  ASSERT_TRUE(IptcDataSets::dataSetRepeatable(IptcDataSets::Destination, IptcDataSets::envelope));
  ASSERT_FALSE(IptcDataSets::dataSetRepeatable(IptcDataSets::FileFormat, IptcDataSets::envelope));

  ASSERT_FALSE(IptcDataSets::dataSetRepeatable(IptcDataSets::ObjectType, IptcDataSets::application2));
  ASSERT_TRUE(IptcDataSets::dataSetRepeatable(IptcDataSets::ObjectAttribute, IptcDataSets::application2));
}

/// \todo check if we want to return true in this case or throw an exception ...
TEST(IptcDataSets, dataSetRepeatableReturnsTrueWhenRequestingNumbersNotAvailableInRecord) {
  ASSERT_TRUE(IptcDataSets::dataSetRepeatable(1, IptcDataSets::envelope));
  ASSERT_TRUE(IptcDataSets::dataSetRepeatable(2, IptcDataSets::envelope));

  ASSERT_TRUE(IptcDataSets::dataSetRepeatable(1, IptcDataSets::application2));
  ASSERT_TRUE(IptcDataSets::dataSetRepeatable(2, IptcDataSets::application2));
}

// ----------------------

TEST(IptcDataSets, dataSetReturnsExpectedValueWhenRequestingValidDatasetName) {
  ASSERT_EQ(IptcDataSets::ModelVersion, IptcDataSets::dataSet("ModelVersion", IptcDataSets::envelope));
  ASSERT_EQ(IptcDataSets::FileFormat, IptcDataSets::dataSet("FileFormat", IptcDataSets::envelope));

  ASSERT_EQ(IptcDataSets::RecordVersion, IptcDataSets::dataSet("RecordVersion", IptcDataSets::application2));
  ASSERT_EQ(IptcDataSets::FixtureId, IptcDataSets::dataSet("FixtureId", IptcDataSets::application2));
}

TEST(IptcDataSets, dataSetThrowWithNonExistingDatasetName) {
  try {
    IptcDataSets::dataSet("NonExistingName", IptcDataSets::envelope);
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerInvalidDataset, e.code());
    ASSERT_STREQ("Invalid dataset name 'NonExistingName'", e.what());
  }
}

/// \todo Weird error reporting here. It should indicate that the record specified does not exist
TEST(IptcDataSets, dataSetThrowWithNonExistingRecordId) {
  try {
    IptcDataSets::dataSet("ModelVersion", 5);
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerInvalidDataset, e.code());
    ASSERT_STREQ("Invalid dataset name 'ModelVersion'", e.what());
  }
}

// ----------------------

TEST(IptcDataSets, dataSetTypeReturnsExpectedTypeWhenRequestingValidDataset) {
  ASSERT_EQ(unsignedShort, IptcDataSets::dataSetType(IptcDataSets::ModelVersion, IptcDataSets::envelope));
  ASSERT_EQ(Exiv2::string, IptcDataSets::dataSetType(IptcDataSets::Destination, IptcDataSets::envelope));

  ASSERT_EQ(unsignedShort, IptcDataSets::dataSetType(IptcDataSets::RecordVersion, IptcDataSets::application2));
  ASSERT_EQ(Exiv2::string, IptcDataSets::dataSetType(IptcDataSets::ObjectType, IptcDataSets::application2));
}

/// \todo probably better to throw exception here?
TEST(IptcDataSets, dataSetTypeReturnsStringTypeWhenRecordIdDoesNotExist) {
  ASSERT_EQ(Exiv2::string, IptcDataSets::dataSetType(1, 5));
}

// ----------------------

TEST(IptcDataSets, recordNameReturnsExpectedNameWhenRequestingValidRecordId) {
  ASSERT_EQ("Envelope", IptcDataSets::recordName(IptcDataSets::envelope));
  ASSERT_EQ("Application2", IptcDataSets::recordName(IptcDataSets::application2));
}

TEST(IptcDataSets, recordNameReturnsHexStringWhenRecordIdDoesNotExist) {
  ASSERT_EQ("0x0000", IptcDataSets::recordName(0));
  ASSERT_EQ("0x0003", IptcDataSets::recordName(3));
}

// ----------------------

TEST(IptcDataSets, recordDescReturnsExpectedDescriptionWhenRequestingValidRecordId) {
  ASSERT_STREQ("IIM envelope record", IptcDataSets::recordDesc(IptcDataSets::envelope));
  ASSERT_STREQ("IIM application record 2", IptcDataSets::recordDesc(IptcDataSets::application2));
}

TEST(IptcDataSets, recordDesc) {
  ASSERT_STREQ("Unknown dataset", IptcDataSets::recordDesc(0));
  ASSERT_STREQ("Unknown dataset", IptcDataSets::recordDesc(3));
}

// ----------------------

TEST(IptcDataSets, recordIdReturnsExpectedIdWithValidRecordName) {
  ASSERT_EQ(IptcDataSets::envelope, IptcDataSets::recordId("Envelope"));
  ASSERT_EQ(IptcDataSets::application2, IptcDataSets::recordId("Application2"));
}

TEST(IptcDataSets, recordIdThrowsExceptionWithInvalidRecordName) {
  ASSERT_THROW(IptcDataSets::recordId("NonExistingName"), Exiv2::Error);
  ASSERT_THROW(IptcDataSets::recordId(""), Exiv2::Error);
}

// ----------------------

TEST(IptcDataSets, dataSetListsPrintDatasetsIntoOstream) {
  std::ostringstream stream;
  ASSERT_NO_THROW(IptcDataSets::dataSetList(stream));
  ASSERT_FALSE(stream.str().empty());
}
