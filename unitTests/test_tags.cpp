// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>

#include <exiv2/tags.hpp>

#include <set>

using namespace Exiv2;

namespace {
// Run f(group, tagList) for every group which has a tag list
template <typename F>
void forEachTagList(F f) {
  for (auto group = ExifTags::groupList(); group->tagList_; ++group) {
    f(*group, group->tagList_());
  }
}
}  // namespace

TEST(ExifTags, tagNumbersAreUniqueInEachTagList) {
  forEachTagList([](const GroupInfo& group, const TagInfo* tagList) {
    std::set<uint16_t> seen;
    for (auto ti = tagList; ti->tag_ != 0xffff; ++ti) {
      EXPECT_TRUE(seen.insert(ti->tag_).second)
          << group.groupName_ << ": tag 0x" << std::hex << ti->tag_ << " (" << ti->name_ << ") is listed twice";
    }
  });
}

TEST(ExifTags, allEntriesOfATagListHaveTheSameIfdId) {
  forEachTagList([](const GroupInfo& group, const TagInfo* tagList) {
    const IfdId ifdId = tagList->ifdId_;
    // Include the end of list marker, it is returned for unknown tags
    for (auto ti = tagList;; ++ti) {
      EXPECT_EQ(ti->ifdId_, ifdId) << group.groupName_ << ": tag 0x" << std::hex << ti->tag_ << " (" << ti->name_
                                   << ") has a different IFD id than the rest of the list";
      if (ti->tag_ == 0xffff)
        break;
    }
  });
}
