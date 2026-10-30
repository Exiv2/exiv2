// SPDX-License-Identifier: GPL-2.0-or-later

// Malformed files from bug reports and fuzzing must be rejected with a
// specific error instead of crashing. These used to be Python system tests
// that ran `exiv2 <file>` and compared the error message on stderr.

#include <exiv2/error.hpp>
#include <exiv2/image.hpp>

#include <ostream>
#include <string>

#include <gtest/gtest.h>

using namespace Exiv2;

namespace {
struct PocFile {
  const char* name;  //!< test name, the issue or CVE
  const char* file;  //!< file in test/data
  ErrorCode error;   //!< error that reading the file must throw
};

// Lets gtest print the parameter in failure messages
std::ostream& operator<<(std::ostream& os, const PocFile& poc) {
  return os << poc.file;
}

class MalformedFile : public ::testing::TestWithParam<PocFile> {};
}  // namespace

TEST_P(MalformedFile, readMetadataThrowsExpectedError) {
  const auto& poc = GetParam();
  try {
    auto image = ImageFactory::open(std::string(TESTDATA_PATH "/") + poc.file);
    image->readMetadata();
    FAIL() << "no exception thrown";
  } catch (const Error& e) {
    EXPECT_EQ(e.code(), poc.error) << e.what();
  }
}

INSTANTIATE_TEST_SUITE_P(
    PoC, MalformedFile,
    ::testing::Values(
        PocFile{"CVE_2017_1000126", "cve_2017_1000126_stack-oob-read.webp", ErrorCode::kerCorruptedMetadata},
        PocFile{"CVE_2017_14860", "003-heap-buffer-over", ErrorCode::kerCorruptedMetadata},
        PocFile{"CVE_2017_17669", "issue_187", ErrorCode::kerFailedToReadImageData},
        PocFile{"CVE_2017_17722", "2018-01-09-exiv2-crash-001.tiff", ErrorCode::kerFileContainsUnknownImageType},
        PocFile{"CVE_2018_4868", "exiv2-memorymmap-error", ErrorCode::kerCorruptedMetadata},
        PocFile{"issue_170", "issue_170_poc", ErrorCode::kerFailedToReadImageData},
        PocFile{"issue_253", "3-stringformat-outofbound-read", ErrorCode::kerNotAJpeg},
        PocFile{"issue_426", "h02.psd", ErrorCode::kerCorruptedMetadata},
        PocFile{"issue_460", "issue_460", ErrorCode::kerCorruptedMetadata},
        PocFile{"issue_839", "issue_839_poc.rw2", ErrorCode::kerCorruptedMetadata},
        PocFile{"issue_841", "issue_841_poc.crw", ErrorCode::kerCorruptedMetadata},
        PocFile{"issue_843", "issue_843_poc.crw", ErrorCode::kerCorruptedMetadata},
        PocFile{"issue_1011", "Jp2Image_readMetadata_loop.poc", ErrorCode::kerCorruptedMetadata},
        PocFile{"issue_1097", "issue_1097_poc.crw", ErrorCode::kerOffsetOutOfRange},
        PocFile{"issue_1812", "issue_1812_poc.jp2", ErrorCode::kerCorruptedMetadata},
        PocFile{"issue_3519", "issue_3519_poc.xmp", ErrorCode::kerCorruptedMetadata},
        PocFile{"issue_ghsa_hxph_pv7w_8649", "issue_ghsa_hxph_pv7w_8649_poc.crw", ErrorCode::kerCorruptedMetadata},
        PocFile{"redmine_issue_841", "exiv2-bug841.png", ErrorCode::kerFailedToReadImageData}),
    [](const auto& info) { return std::string(info.param.name); });
