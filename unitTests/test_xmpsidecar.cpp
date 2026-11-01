// SPDX-License-Identifier: GPL-2.0-or-later

#include <exiv2/exiv2.hpp>
#include "config.h"

#include <gtest/gtest.h>

#include <string>

namespace {
using namespace Exiv2;

// Snapshot owned I/O bytes independently of the current stream position.
std::string bytes(Image& image) {
  auto& io = image.io();
  io.open();
  io.seek(0, BasicIo::beg);
  const auto data = io.read(io.size());
  return {reinterpret_cast<const char*>(data.c_data()), data.size()};
}

// Construct an invalid caller-supplied subtree unrelated to DeviceSettings.
#ifdef EXV_HAVE_XMP_TOOLKIT
void makeUnencodable(Image& image) {
  XmpData data;
  data["Xmp.xmpMM.DerivedFrom/stRef:documentID"] = "child-canary";
  data["Xmp.xmpMM.DerivedFrom"] = "scalar-conflict";
  image.setXmpData(data);
}
#endif
}  // namespace

// A failed encode on a new sidecar must report failure before writing more bytes.
TEST(XmpSidecarWrite, ReportsEncodingFailureForNewTarget) {
#ifdef EXV_HAVE_XMP_TOOLKIT
  auto image = ImageFactory::create(ImageType::xmp, std::make_unique<MemIo>());
  const auto before = bytes(*image);
  makeUnencodable(*image);
  try {
    image->writeMetadata();
    FAIL() << "Expected encoding failure";
  } catch (const Error& error) {
    EXPECT_EQ(ErrorCode::kerImageWriteFailed, error.code());
  }
  EXPECT_EQ(before, bytes(*image));
#else
  GTEST_SKIP() << "XMP encoding requires the toolkit";
#endif
}

// An old packet must not mask failure; repairing the buffered XMP permits retry.
TEST(XmpSidecarWrite, PreservesExistingBytesAndAllowsRetry) {
#ifdef EXV_HAVE_XMP_TOOLKIT
  auto image = ImageFactory::create(ImageType::xmp, std::make_unique<MemIo>());
  image->xmpData()["Xmp.xmp.Label"] = "original-canary";
  image->writeMetadata();
  image->readMetadata();
  const auto before = bytes(*image);
  makeUnencodable(*image);
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(before, bytes(*image));

  image->clearXmpData();
  image->xmpData()["Xmp.xmp.Label"] = "repaired-canary";
  ASSERT_NO_THROW(image->writeMetadata());
  image->readMetadata();
  EXPECT_EQ("repaired-canary", image->xmpData()["Xmp.xmp.Label"].toString());
#else
  GTEST_SKIP() << "XMP encoding requires the toolkit";
#endif
}

// Raw-packet output bypasses structured encoding, including without the toolkit.
TEST(XmpSidecarWrite, PreservesRawPacketWriting) {
  const std::string packet =
      "<?xml version=\"1.0\"?><x:xmpmeta xmlns:x=\"adobe:ns:meta/\">"
      "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">"
      "<rdf:Description rdf:about=\"\"/></rdf:RDF></x:xmpmeta>";
  auto image = ImageFactory::create(ImageType::xmp, std::make_unique<MemIo>());
  image->xmpPacket() = packet;
  image->writeXmpFromPacket(true);
  image->writeMetadata();
  EXPECT_EQ(packet, bytes(*image));
}
