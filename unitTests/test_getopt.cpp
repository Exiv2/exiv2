// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include "getopt.hpp"  // from app/, not part of libexiv2

#include <string>
#include <utility>
#include <vector>

namespace {
// The exiv2 command's option string
constexpr auto optstring = ":hVvqfbuktTFa:Y:O:D:r:p:P:d:e:i:c:m:M:l:S:g:K:n:Q:";

// argv as getopt() expects it, the strings must outlive the parsing
class Args {
 public:
  Args(std::initializer_list<const char*> args) : strings_(args.begin(), args.end()) {
    for (auto& s : strings_)
      argv_.push_back(s.data());
    argv_.push_back(nullptr);
  }
  [[nodiscard]] int argc() const {
    return static_cast<int>(strings_.size());
  }
  char** argv() {
    return argv_.data();
  }

 private:
  std::vector<std::string> strings_;
  std::vector<char*> argv_;
};

class RecordingGetopt : public Util::Getopt {
 public:
  int option(int opt, const std::string& optarg, int optopt) override {
    options.emplace_back(opt, optarg);
    lastOptopt = optopt;
    // Like the exiv2 command, count errors. Getopt::getopt() would loop
    // forever on a missing option argument (':') otherwise.
    return opt == '?' || opt == ':';
  }
  int nonoption(const std::string& argv) override {
    nonoptions.push_back(argv);
    return 0;
  }

  std::vector<std::pair<int, std::string>> options;
  std::vector<std::string> nonoptions;
  int lastOptopt = 0;
};
}  // namespace

TEST(Getopt, stopsAtTheFirstNonOption) {
  Args args{"exiv2", "-v", "pr", "-P", "EIXxgklnycsvth", "file1", "file2"};
  Util::optind = 0;
  EXPECT_EQ('v', Util::getopt(args.argc(), args.argv(), optstring));
  EXPECT_EQ(2, Util::optind);
  EXPECT_EQ(-1, Util::getopt(args.argc(), args.argv(), optstring));
  EXPECT_EQ(2, Util::optind);
}

TEST(Getopt, passesOptionsAndNonOptionsToTheCallbacks) {
  Args args{"exiv2", "-v", "pr", "-P", "EIXxgklnycsvth", "file1", "file2"};
  RecordingGetopt getopt;
  EXPECT_EQ(0, getopt.getopt(args.argc(), args.argv(), optstring));
  ASSERT_EQ(1u, getopt.options.size());
  EXPECT_EQ('v', getopt.options[0].first);
  EXPECT_EQ((std::vector<std::string>{"pr", "-P", "EIXxgklnycsvth", "file1", "file2"}), getopt.nonoptions);
}

TEST(Getopt, optionArgumentsAttachedOrSeparate) {
  Args args{"exiv2", "-Pkv", "-g", "Model", "-qb", "print", "file"};
  RecordingGetopt getopt;
  EXPECT_EQ(0, getopt.getopt(args.argc(), args.argv(), optstring));
  // optarg is only meaningful for options that take an argument, it is not
  // reset for the others
  ASSERT_EQ(4u, getopt.options.size());
  EXPECT_EQ(std::make_pair(int('P'), std::string("kv")), getopt.options[0]);
  EXPECT_EQ(std::make_pair(int('g'), std::string("Model")), getopt.options[1]);
  EXPECT_EQ('q', getopt.options[2].first);
  EXPECT_EQ('b', getopt.options[3].first);
  EXPECT_EQ((std::vector<std::string>{"print", "file"}), getopt.nonoptions);
}

TEST(Getopt, doubleDashEndsTheOptions) {
  Args args{"exiv2", "-v", "--", "-q", "file"};
  RecordingGetopt getopt;
  EXPECT_EQ(0, getopt.getopt(args.argc(), args.argv(), optstring));
  ASSERT_EQ(1u, getopt.options.size());
  EXPECT_EQ((std::vector<std::string>{"-q", "file"}), getopt.nonoptions);
}

TEST(Getopt, unknownOptionIsReportedAsQuestionMark) {
  Args args{"exiv2", "-Z", "file"};
  RecordingGetopt getopt;
  EXPECT_EQ(1, getopt.getopt(args.argc(), args.argv(), optstring));
  ASSERT_EQ(1u, getopt.options.size());
  EXPECT_EQ('?', getopt.options[0].first);
  EXPECT_EQ('Z', getopt.lastOptopt);
}

TEST(Getopt, missingOptionArgumentIsReportedAsColon) {
  Args args{"exiv2", "-g"};
  RecordingGetopt getopt;
  EXPECT_EQ(1, getopt.getopt(args.argc(), args.argv(), optstring));
  ASSERT_EQ(1u, getopt.options.size());
  EXPECT_EQ(':', getopt.options[0].first);
  EXPECT_EQ('g', getopt.lastOptopt);
}
