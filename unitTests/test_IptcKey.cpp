// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include <exiv2/datasets.hpp>
#include <exiv2/error.hpp>

using namespace Exiv2;

TEST(IptcKey, creationWithNonValidStringFormatThrows) {
  try {
    IptcKey key("Yeah");
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerInvalidKey, e.code());
    ASSERT_STREQ("Invalid key 'Yeah'", e.what());
  }
}

TEST(IptcKey, creationWithNonValidRecordNameThrows) {
  try {
    IptcKey key("Iptc.WrongRecordName.ModelVersion");
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerInvalidRecord, e.code());
    ASSERT_STREQ("Invalid record name 'WrongRecordName'", e.what());
  }
}

TEST(IptcKey, creationWithNonValidDatasetNameThrows) {
  try {
    IptcKey key("Iptc.Envelope.WrongDataset");
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerInvalidDataset, e.code());
    ASSERT_STREQ("Invalid dataset name 'WrongDataset'", e.what());
  }
}

TEST(IptcKey, creationWithNonValidFamiltyNameThrows) {
  try {
    IptcKey key("JOJO.Envelope.WrongDataset");
    FAIL();
  } catch (const Exiv2::Error& e) {
    ASSERT_EQ(ErrorCode::kerInvalidKey, e.code());
    ASSERT_STREQ("Invalid key 'JOJO.Envelope.WrongDataset'", e.what());
  }
}

TEST(IptcKey, creationWithValidStringDoesNotThrow) {
  ASSERT_NO_THROW(IptcKey("Iptc.Envelope.ModelVersion"));
}

TEST(IptcKey, copyConstructor) {
  IptcKey key("Iptc.Envelope.ModelVersion");
  IptcKey keyCopy(key);
}

TEST(IptcKey, clone) {
  IptcKey key("Iptc.Envelope.ModelVersion");
  auto keyClone = key.clone();
  ASSERT_EQ("Iptc.Envelope.ModelVersion", keyClone->key());
}

TEST(IptcKey, keyReturnsTheFullString) {
  IptcKey key("Iptc.Envelope.ModelVersion");
  ASSERT_EQ("Iptc.Envelope.ModelVersion", key.key());
}

TEST(IptcKey, familyNameReturnsTheFullString) {
  IptcKey key("Iptc.Envelope.ModelVersion");
  ASSERT_STREQ("Iptc", key.familyName());
}

TEST(IptcKey, groupNameReturnsTheRecordName) {
  IptcKey key("Iptc.Envelope.ModelVersion");
  ASSERT_EQ("Envelope", key.groupName());
}

TEST(IptcKey, recordNameReturnsTheRecordName) {
  IptcKey key("Iptc.Envelope.ModelVersion");
  ASSERT_EQ("Envelope", key.recordName());
}

TEST(IptcKey, tagNameReturnsTheDatasetName) {
  IptcKey key("Iptc.Envelope.ModelVersion");
  ASSERT_EQ("ModelVersion", key.tagName());
}

TEST(IptcKey, tagLabelReturnsTheDatasetTitle) {
  IptcKey key("Iptc.Envelope.ModelVersion");
  ASSERT_EQ("Model Version", key.tagLabel());
}

TEST(IptcKey, tag) {
  IptcKey key("Iptc.Envelope.ModelVersion");
  ASSERT_EQ(IptcDataSets::ModelVersion, key.tag());
}

TEST(IptcKey, record) {
  IptcKey key("Iptc.Envelope.ModelVersion");
  ASSERT_EQ(IptcDataSets::envelope, key.record());
}
