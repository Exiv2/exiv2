// SPDX-License-Identifier: GPL-2.0-or-later
#include <gtest/gtest.h>
#include <exiv2/exiv2.hpp>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>

#ifdef EXV_ENABLE_FILESYSTEM
namespace {
using namespace Exiv2;
namespace fs = std::filesystem;

// Each test owns its directory; parallel test processes never share fixtures.
class XmpFileWrite : public testing::Test {
 protected:
  fs::path directory;
  fs::path target;
  void SetUp() override {
    std::random_device random;
    do {
      directory = fs::temp_directory_path() / ("exiv2-xmp-test-" + std::to_string(random()));
    } while (!fs::create_directory(directory));
    target = directory / "sidecar.xmp";
  }
  void TearDown() override {
    std::error_code ignored;
    fs::remove_all(directory, ignored);
  }
  static std::string contents(const fs::path& path) {
    std::ifstream stream(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
  }
  void seed() {
    std::ofstream(target, std::ios::binary) << "original-canary";
  }
  void write(Image& image) {
    image.writeMetadata();
    const auto path = target.u8string();
    FileIo destination(std::string(path.begin(), path.end()));
    destination.transfer(image.io());
  }
  static void raw(Image& image, const std::string& value) {
    image.xmpPacket() =
        "<?xml version=\"1.0\"?><x:xmpmeta xmlns:x=\"adobe:ns:meta/\">"
        "<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">"
        "<rdf:Description rdf:about=\"\" xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\" xmp:Label=\"" +
        value + "\"/></rdf:RDF></x:xmpmeta>";
    image.writeXmpFromPacket(true);
  }
};

TEST_F(XmpFileWrite, PreservesImmediateFactoryContract) {
  ImageFactory::create(ImageType::xmp, target.string());
  EXPECT_EQ(54U, fs::file_size(target));
  seed();
  ImageFactory::create(ImageType::xmp, target.string());
  EXPECT_EQ(54U, fs::file_size(target));
}

// The existing in-memory factory needs no destination and may be abandoned.
TEST_F(XmpFileWrite, StagesWithoutChangingDestination) {
  for (const bool existing : {false, true}) {
    if (existing)
      seed();
    {
      auto image = ImageFactory::create(ImageType::xmp);
      EXPECT_TRUE(image->good());
      EXPECT_NO_THROW(image->readMetadata());
      raw(*image, "staged");
      EXPECT_NO_THROW(image->writeMetadata());
    }
    EXPECT_EQ(existing, fs::exists(target));
    if (existing) {
      EXPECT_EQ("original-canary", contents(target));
    }
  }
}

// Empty and raw writes also work without the toolkit, including repeated saves.
TEST_F(XmpFileWrite, TransfersRawPacketsAndEmptySidecars) {
  auto image = ImageFactory::create(ImageType::xmp);
  write(*image);
  EXPECT_EQ(54U, fs::file_size(target));
  for (const auto* label : {"first", "second"}) {
    raw(*image, label);
    const auto expected = image->xmpPacket();
    write(*image);
    EXPECT_EQ(expected, contents(target));
    EXPECT_TRUE(image->good());
  }
}

TEST_F(XmpFileWrite, WrapsRawPacketOnlyOnce) {
  auto image = ImageFactory::create(ImageType::xmp);
  raw(*image, "wrapped");
  image->xmpPacket().erase(0, std::string("<?xml version=\"1.0\"?>").size());
  write(*image);
  const auto expected = contents(target);
  EXPECT_TRUE(expected.starts_with("<?xpacket"));
  EXPECT_TRUE(expected.ends_with("<?xpacket end=\"w\"?>"));
  write(*image);
  EXPECT_EQ(expected, contents(target));
}

TEST_F(XmpFileWrite, TransfersToUnicodePath) {
  target = directory / fs::path(std::u8string(u8"Žluť-©.xmp"));
  auto image = ImageFactory::create(ImageType::xmp);
  raw(*image, "unicode");
  write(*image);
  EXPECT_EQ(image->xmpPacket(), contents(target));
}

#ifdef EXV_HAVE_XMP_TOOLKIT
// Values from #9551: malformed UserComment, Model and Latin-1 Copyright.
TEST_F(XmpFileWrite, RejectsInvalidExifBeforeTransferAndAllowsRetry) {
  for (const auto* key : {"Exif.Photo.UserComment", "Exif.Image.Model", "Exif.Image.Copyright"}) {
    for (const bool existing : {false, true}) {
      SCOPED_TRACE(testing::Message() << key << ", existing=" << existing);
      fs::remove(target);
      if (existing)
        seed();
      auto image = ImageFactory::create(ImageType::xmp);
      ExifData exif;
      if (std::string(key) == "Exif.Photo.UserComment") {
        const byte data[]{10, 0, 0, 0, 74, 75, 74, 75, 39, 3, 171};
        CommentValue value;
        value.read(data, sizeof(data), littleEndian);
        exif.add(ExifKey(key), &value);
      } else {
        exif[key] = std::string(key) == "Exif.Image.Model" ? "\xe5\x80\xf0" : "\xa9 Example";
      }
      image->setExifData(exif);
      image->xmpData()["Xmp.xmp.Label"] = "keep";
      EXPECT_THROW(write(*image), Error);
      EXPECT_EQ(existing, fs::exists(target));
      if (existing) {
        EXPECT_EQ("original-canary", contents(target));
      }

      image->clearExifData();
      image->clearXmpData();
      image->xmpData()["Xmp.xmp.Label"] = "repaired";
      ASSERT_NO_THROW(write(*image));
      auto saved = ImageFactory::open(target.string());
      saved->readMetadata();
      EXPECT_EQ("repaired", saved->xmpData()["Xmp.xmp.Label"].toString());
    }
  }
}

TEST_F(XmpFileWrite, AcceptsUtf8Copyright) {
  for (const bool existing : {false, true}) {
    if (existing)
      seed();
    auto image = ImageFactory::create(ImageType::xmp);
    ExifData exif;
    exif["Exif.Image.Copyright"] = "\xc2\xa9 Example";
    image->setExifData(exif);
    ASSERT_NO_THROW(write(*image));
    auto saved = ImageFactory::open(target.string());
    saved->readMetadata();
    EXPECT_EQ("lang=\"x-default\" \xc2\xa9 Example", saved->xmpData()["Xmp.dc.rights"].toString());
  }
}

// An old packet cannot hide encoding failure; repaired edits remain writable.
TEST_F(XmpFileWrite, PreservesOpenedSidecarOnEncodingFailure) {
  auto created = ImageFactory::create(ImageType::xmp);
  raw(*created, "original");
  write(*created);
  const auto before = contents(target);
  auto image = ImageFactory::open(target.string());
  image->readMetadata();
  image->xmpData()["Xmp.xmpMM.DerivedFrom/stRef:documentID"] = "child";
  image->xmpData()["Xmp.xmpMM.DerivedFrom"] = "conflict";
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_EQ(before, contents(target));
  EXPECT_EQ(before, static_cast<const Image&>(*image).xmpPacket());
  image->clearXmpData();
  image->xmpData()["Xmp.xmp.Label"] = "repaired";
  ASSERT_NO_THROW(image->writeMetadata());
  image->readMetadata();
  EXPECT_EQ("repaired", image->xmpData()["Xmp.xmp.Label"].toString());
}
#endif

// FileIo subclasses retain their transfer override and propagate its errors.
TEST_F(XmpFileWrite, RetainsCustomTransfer) {
  struct CustomIo : FileIo {
    using FileIo::FileIo;
    bool transferred = false;
    void transfer(BasicIo&) override {
      transferred = true;
      throw Error(ErrorCode::kerImageWriteFailed);
    }
  };
  ImageFactory::create(ImageType::xmp, target.string());
  auto io = std::make_unique<CustomIo>(target.string());
  auto* custom = io.get();
  auto image = ImageFactory::open(std::move(io));
  ASSERT_NE(nullptr, image);
  const auto before = contents(target);
  raw(*image, "custom");
  EXPECT_THROW(image->writeMetadata(), Error);
  EXPECT_TRUE(custom->transferred);
  EXPECT_EQ(before, contents(target));
}

// Destination-open errors are reported by transfer, after successful encoding.
TEST_F(XmpFileWrite, ReportsMissingParentAndKeepsDirectoryDestination) {
  target = directory / "missing" / "sidecar.xmp";
  auto image = ImageFactory::create(ImageType::xmp);
  raw(*image, "valid");
  EXPECT_THROW(write(*image), Error);
  EXPECT_FALSE(fs::exists(target.parent_path()));
  fs::create_directory(target.parent_path());
  ASSERT_NO_THROW(write(*image));
  EXPECT_EQ(image->xmpPacket(), contents(target));
  target = directory / "directory.xmp";
  fs::create_directory(target);
  EXPECT_THROW(write(*image), Error);
  EXPECT_TRUE(fs::is_directory(target));
}
}  // namespace
#endif
