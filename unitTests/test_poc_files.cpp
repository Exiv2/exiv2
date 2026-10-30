// SPDX-License-Identifier: GPL-2.0-or-later

// Malformed files from bug reports and fuzzing must be rejected with a
// specific error, or handled, instead of crashing. These used to be Python
// system tests that ran exiv2 on the file and compared its output.

#include <exiv2/error.hpp>
#include <exiv2/image.hpp>
#include <exiv2/preview.hpp>

#include <ostream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

using namespace Exiv2;

namespace {
//! What the exiv2 command did with the file, as library calls
enum class Action {
  read,            //!< exiv2 file: readMetadata()
  printAll,        //!< exiv2 -pa: print every Exif, IPTC and XMP value
  previews,        //!< exiv2 -pp: list the preview images
  printBasic,      //!< exiv2 -pS: printStructure(kpsBasic)
  printRecursive,  //!< exiv2 -pR: printStructure(kpsRecursive)
  printXmp,        //!< exiv2 -pX: printStructure(kpsXMP)
  printIcc,        //!< exiv2 -pC: printStructure(kpsIccProfile)
};

struct Outcome {
  enum Kind { none, any, code } kind;
  ErrorCode error;
};
//! The file is handled without an error
constexpr Outcome noError{Outcome::none, ErrorCode::kerSuccess};
//! Some Exiv2::Error is thrown
constexpr Outcome anyError{Outcome::any, ErrorCode::kerSuccess};
//! This Exiv2::Error is thrown
constexpr Outcome throws(ErrorCode error) {
  return {Outcome::code, error};
}
//! Some files fail differently depending on the size of pointers
constexpr bool is64bit = sizeof(void*) == 8;

struct PocFile {
  const char* name;  //!< test name, the issue or CVE
  const char* file;  //!< file in test/data
  Action action;
  Outcome outcome;
};

// Lets gtest print the parameter in failure messages
std::ostream& operator<<(std::ostream& os, const PocFile& poc) {
  return os << poc.file;
}

void run(Action action, const std::string& path) {
  auto image = ImageFactory::open(path);
  std::ostringstream os;
  switch (action) {
    case Action::printBasic:
      return image->printStructure(os, kpsBasic);
    case Action::printRecursive:
      return image->printStructure(os, kpsRecursive);
    case Action::printXmp:
      return image->printStructure(os, kpsXMP);
    case Action::printIcc:
      return image->printStructure(os, kpsIccProfile);
    default:
      break;
  }
  image->readMetadata();
  if (action == Action::previews) {
    PreviewManager previews(*image);
    for (const auto& p : previews.getPreviewProperties())
      os << p.mimeType_ << p.width_ << p.height_ << p.size_;
  } else if (action == Action::printAll) {
    const auto& exifData = image->exifData();
    for (const auto& md : exifData)
      os << md.print(&exifData);
    for (const auto& md : image->iptcData())
      os << md.print();
    for (const auto& md : image->xmpData())
      os << md.print();
  }
}

const PocFile pocFiles[] = {
    {"CVE_2017_1000126", "cve_2017_1000126_stack-oob-read.webp", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"CVE_2017_14860", "003-heap-buffer-over", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"CVE_2017_17669", "issue_187", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"CVE_2017_17722", "2018-01-09-exiv2-crash-001.tiff", Action::read,
     throws(ErrorCode::kerFileContainsUnknownImageType)},
    {"CVE_2018_4868", "exiv2-memorymmap-error", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_170", "issue_170_poc", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_253", "3-stringformat-outofbound-read", Action::read, throws(ErrorCode::kerNotAJpeg)},
    {"issue_426", "h02.psd", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_460", "issue_460", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_839", "issue_839_poc.rw2", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_841", "issue_841_poc.crw", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_843", "issue_843_poc.crw", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_1011", "Jp2Image_readMetadata_loop.poc", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_1097", "issue_1097_poc.crw", Action::read, throws(ErrorCode::kerOffsetOutOfRange)},
    {"issue_1812", "issue_1812_poc.jp2", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_3519", "issue_3519_poc.xmp", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_ghsa_hxph_pv7w_8649", "issue_ghsa_hxph_pv7w_8649_poc.crw", Action::read,
     throws(ErrorCode::kerCorruptedMetadata)},
    {"redmine_issue_841", "exiv2-bug841.png", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"CVE_2017_1000127", "heap-oob-write.tiff", Action::read, noError},
    {"CVE_2017_11591", "POC8", Action::read, noError},
    {"CVE_2017_11683", "POC", Action::read, noError},
    {"CVE_2017_14859", "005-invalid-mem", Action::read,
     throws(is64bit ? ErrorCode::kerFailedToReadImageData : ErrorCode::kerCorruptedMetadata)},
    {"CVE_2017_14864", "02-Invalid-mem-def", Action::read,
     throws(is64bit ? ErrorCode::kerFailedToReadImageData : ErrorCode::kerCorruptedMetadata)},
    {"CVE_2017_14865", "004-heap-buffer-over", Action::read, noError},
    {"CVE_2017_14866", "006-heap-buffer-over", Action::read, noError},
    {"CVE_2017_17724_1", "2018-01-09-exiv2-crash-002.tiff", Action::printRecursive, anyError},
    {"CVE_2017_17724_2", "2018-01-09-exiv2-crash-002.tiff", Action::printBasic, anyError},
    {"CVE_2017_17724_3", "2018-01-09-exiv2-crash-002.tiff", Action::read, noError},
    {"CVE_2017_17725", "poc_2017-12-12_issue188", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"CVE_2017_9953", "POC1", Action::read, noError},
    {"CVE_2018_11531", "pocIssue283.jpg", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"CVE_2018_2018_14046", "issue_378_1-poc-heapoverflow", Action::previews, throws(ErrorCode::kerCorruptedMetadata)},
    {"CVE_2018_9145", "4-DataBuf-abort-1", Action::printRecursive, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_1019", "POC-file_issue_1019", Action::printAll, throws(ErrorCode::kerOffsetOutOfRange)},
    {"issue_1522_1", "poc_1522.jp2", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_1522_2", "poc_1522.jp2", Action::printBasic, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_159_1", "printStructure", Action::read, noError},
    {"issue_159_2", "printStructure2", Action::printBasic, anyError},
    {"issue_1817", "issue_1817_poc.png", Action::printBasic, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_1833", "issue_1833_poc.jpg", Action::printBasic, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_211_1", "2018-01-09-exiv2-crash-003.tiff", Action::printRecursive, anyError},
    {"issue_211_2", "2018-01-09-exiv2-crash-003.tiff", Action::printBasic, anyError},
    {"issue_211_3", "2018-01-09-exiv2-crash-003.tiff", Action::read, noError},
    {"issue_216", "exiv2_0-26_exiv2_uncontrolled-recursion_printIFDStructure.tif", Action::printRecursive, anyError},
    {"issue_2268", "issue_2268_poc.jp2", Action::printBasic, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_2427", "issue_2427_poc.jpg", Action::read, anyError},
    {"issue_246", "1-string-format.jpg", Action::printBasic, throws(ErrorCode::kerNoImageInInputData)},
    {"issue_262", "7-printIFD-divbyzero-1", Action::printXmp, throws(ErrorCode::kerFileContainsUnknownImageType)},
    {"issue_263_1", "9-printStructure-outbound-read-1", Action::printRecursive, anyError},
    {"issue_263_2", "10-printStructure-outbound-read-2", Action::printRecursive, anyError},
    {"issue_3511", "issue_3511_poc.eps", Action::previews, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_3513", "issue_3513_poc.psd", Action::previews, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_400_1", "issue_400_poc1", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_400_2", "issue_400_poc2", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_428_1", "issue_428_poc1.png", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_428_2", "issue_428_poc3.png", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_428_3", "issue_428_poc4.png", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_428_4", "issue_428_poc5.png", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_428_5", "issue_428_poc8.png", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_428_6", "issue_428_poc7.png", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_428_7", "issue_428_poc2.png", Action::read, throws(ErrorCode::kerInputDataReadFailed)},
    {"issue_428_8", "issue_428_poc6.png", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_457", "issue_457_poc", Action::read, anyError},
    {"issue_511", "pocIssue511", Action::printRecursive, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_547", "issue_547.poc", Action::printRecursive, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_561", "pocIssue561", Action::read, noError},
    {"issue_742", "issue_742_poc", Action::printXmp, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_789", "issue_789_poc1.png", Action::read, anyError},
    {"issue_790", "issue_790_poc2.png", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_791", "issue_791_poc1.webp", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_828", "issue_828_poc.png", Action::printRecursive, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_845", "issue_845_poc.png", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_847", "issue_847_poc.pgf", Action::read, throws(ErrorCode::kerInputDataReadFailed)},
    {"issue_855", "issue_855_poc.psd", Action::printAll, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_857_1", "issue_857_poc.raf", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_857_2", "issue_857_coverage.raf", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_869", "issue_869_poc.png", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_943_1", "issue_943_poc1.mrm", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_943_2", "issue_943_poc2.mrm", Action::read, throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_960", "issue_960.poc.webp", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_g44w_q3vm_gwjq", "issue_ghsa_g44w_q3vm_gwjq_poc.jpg", Action::printRecursive,
     throws(ErrorCode::kerInvalidTypeValue)},
    {"issue_ghsa_583f_w9pm_99r2", "issue_ghsa_583f_w9pm_99r2_poc.jp2", Action::printIcc,
     throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_ghsa_h9x9_4f77_336w", "issue_ghsa_h9x9_4f77_336w_poc.exv", Action::printAll, noError},
    {"issue_ghsa_hrw9_ggg3_3r4r", "issue_ghsa_hrw9_ggg3_3r4r_poc.jpg", Action::read,
     throws(ErrorCode::kerFailedToReadImageData)},
    {"issue_ghsa_m479_7frc_gqqg", "issue_ghsa_m479_7frc_gqqg_poc.crw", Action::printIcc,
     throws(ErrorCode::kerCorruptedMetadata)},
    {"redmine_issue_1247", "exiv2-bug1247.jpg", Action::printAll, throws(ErrorCode::kerFailedToReadImageData)},
    {"redmine_issue_1305", "IMGP0006-min.jpg", Action::read, noError},
    {"redmine_issue_495", "exiv2-bug495.jpg", Action::printAll, noError},
#ifdef EXV_ENABLE_BMFF
    {"issue_1570", "issue_1570_poc.bmff", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_1793", "issue_1793_poc.heic", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
#endif
#ifdef EXV_ENABLE_VIDEO
    {"issue_2340", "issue_2340_poc.mp4", Action::printAll, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_2345", "issue_2345_poc.mp4", Action::printAll, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_2376", "issue_2376_poc.mp4", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_2383", "issue_2383_poc.mp4", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_2423", "issue_2423_poc.mp4", Action::read, throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_ghsa_crmj_qh74_2r36", "issue_ghsa_crmj_qh74_2r36_poc.mov", Action::read,
     throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_ghsa_fgw8_p7pr_37cp", "issue_ghsa_fgw8_p7pr_37cp_poc.mov", Action::read,
     throws(ErrorCode::kerCorruptedMetadata)},
    {"issue_ghsa_g9xm_7538_mq8w", "issue_ghsa_g9xm_7538_mq8w_poc.mov", Action::read, anyError},
#endif
};

class MalformedFile : public ::testing::TestWithParam<PocFile> {};
}  // namespace

TEST_P(MalformedFile, isRejectedOrHandled) {
  const auto& poc = GetParam();
  const std::string path = std::string(TESTDATA_PATH "/") + poc.file;
  switch (poc.outcome.kind) {
    case Outcome::none:
      EXPECT_NO_THROW(run(poc.action, path));
      break;
    case Outcome::any:
      EXPECT_THROW(run(poc.action, path), Error);
      break;
    case Outcome::code:
      try {
        run(poc.action, path);
        FAIL() << "no exception thrown";
      } catch (const Error& e) {
        EXPECT_EQ(e.code(), poc.outcome.error) << e.what();
      }
      break;
  }
}

INSTANTIATE_TEST_SUITE_P(PoC, MalformedFile, ::testing::ValuesIn(pocFiles),
                         [](const auto& info) { return std::string(info.param.name); });
