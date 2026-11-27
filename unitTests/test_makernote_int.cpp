// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include <exiv2/config.h>
#include <makernote_int.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

using namespace Exiv2::Internal;

#if defined(EXV_ENABLE_INIH) && defined(EXV_ENABLE_FILESYSTEM)
namespace {
#ifdef _WIN32
constexpr auto configName = "exiv2.ini";
#else
constexpr auto configName = ".exiv2";
#endif

// The config file is looked up in the current directory first
class Exiv2Config : public ::testing::Test {
 protected:
  void SetUp() override {
    oldCwd_ = fs::current_path();
    dir_ = fs::path(::testing::TempDir()) / "exiv2-config-test";
    fs::create_directories(dir_);
    fs::current_path(dir_);
  }

  void TearDown() override {
    fs::current_path(oldCwd_);
    fs::remove_all(dir_);
  }

  static void writeConfig(const char* content) {
    std::ofstream(configName) << content;
  }

  fs::path oldCwd_;
  fs::path dir_;
};

constexpr auto config = R"(; comment
[user]
name = Bob Smith       ; spaces around '=' and comments are stripped

[canon]
169=Sigma 35mm F1.4 DG HSM ART
)";
}  // namespace

TEST_F(Exiv2Config, isFoundInTheCurrentDirectory) {
  writeConfig(config);
  EXPECT_TRUE(fs::equivalent(getExiv2ConfigPath(), dir_ / configName));
}

TEST_F(Exiv2Config, readsValuesFromSections) {
  writeConfig(config);
  EXPECT_EQ(readExiv2Config("canon", "169", "UNDEFINED"), "Sigma 35mm F1.4 DG HSM ART");
  EXPECT_EQ(readExiv2Config("user", "name", "UNDEFINED"), "Bob Smith");
}

TEST_F(Exiv2Config, returnsTheDefaultForMissingValues) {
  writeConfig(config);
  EXPECT_EQ(readExiv2Config("canon", "170", "UNDEFINED"), "UNDEFINED");
  EXPECT_EQ(readExiv2Config("nikon", "169", "UNDEFINED"), "UNDEFINED");
}

TEST_F(Exiv2Config, returnsTheDefaultForAnInvalidFile) {
  writeConfig("[canon\n169=Sigma 35mm F1.4 DG HSM ART\n");
  EXPECT_EQ(readExiv2Config("canon", "169", "UNDEFINED"), "UNDEFINED");
}
#endif
