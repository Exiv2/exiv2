// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <cstdint>
#include <exiv2/exiv2.hpp>
#include "slice.hpp"
#include "types.hpp"

using namespace Exiv2;

template <typename T>
class slice;

/*!
 * Fixture for slice testing. Has one public vector of ints with size vec_size
 * that is filled with the numbers from 0 to vec_size - 1.
 *
 * The vector vec_ is used to construct slices either from a std::vector, or
 * from raw C-arrays. Which type is used, is set by the template parameter
 * T. Thus we guarantee, that the interface is completely independent of the
 * underlying datatype.
 *
 * @tparam T  Type that is used to construct a slice for testing.
 */
template <typename T>
class slice : public ::testing::Test {
 public:
  static const size_t vec_size = 10;

  Slice<T> getTestSlice(size_t begin = 1, size_t end = vec_size - 1) {
    return {getTestData(), begin, end};
  }

  std::vector<int> vec_{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

 private:
  auto getTestData() {
    if constexpr (std::is_same_v<T, std::vector<int>>) {
      return std::ref(vec_);
    } else if constexpr (std::is_same_v<T, const std::vector<int>>) {
      return std::cref(vec_);
    } else if constexpr (std::is_same_v<T, int*>) {
      return vec_.data();
    } else if constexpr (std::is_same_v<T, const int*>) {
      return static_cast<const int*>(vec_.data());
    }
  }
};

/*!
 * Fixture to run test for mutable slices.
 *
 * It adds nothing new, it is just a separate class, so that we can run
 * different tests on it.
 */
template <typename T>
class mutableSlice : public slice<T> {};

TYPED_TEST_SUITE_P(slice);
TYPED_TEST_SUITE_P(mutableSlice);

TYPED_TEST_P(slice, atAccess) {
  // typedef Slice<TypeParam> slice_t;
  // const size_t begin = 1;
  // const size_t end = this->vec_.size() - 1;
  auto sl = this->getTestSlice();

  ASSERT_EQ(this->vec_.size() - 2, sl.size());

  for (unsigned int i = 0; i < sl.size(); ++i) {
    ASSERT_EQ(this->vec_.at(i + 1), sl.at(i));
  }
}

// TODO C++11: test range based for loop
TYPED_TEST_P(slice, iteratorAccess) {
  auto sl = this->getTestSlice();

  auto vec_it = this->vec_.begin() + 1;
  for (auto it = sl.cbegin(); it < sl.cend(); ++it, ++vec_it) {
    ASSERT_EQ(*it, *vec_it);
  }

  ASSERT_THROW(sl.at(sl.size()), std::out_of_range);
}

TYPED_TEST_P(slice, constructionFailsFromInvalidRange) {
  // start > end
  ASSERT_THROW(this->getTestSlice(2, 1), std::out_of_range);
}

TYPED_TEST_P(slice, constructionFailsWithZeroLength) {
  ASSERT_THROW(this->getTestSlice(1, 1), std::out_of_range);
}

/*!
 * Test the construction of subSlices and their behavior.
 */
TYPED_TEST_P(slice, subSliceSuccessfulConstruction) {
  // 0 1 2 3 4 5 6 7 8 9
  //       |     |       center_vals
  //         | |         middle
  auto center_vals = this->getTestSlice(3, 7);
  ASSERT_EQ(center_vals.size(), static_cast<size_t>(4));
  ASSERT_NO_THROW(static_cast<void>(center_vals.subSlice(1, 3)));

  ASSERT_NO_THROW(static_cast<void>(center_vals.subSlice(1, center_vals.size())));
}

TYPED_TEST_P(slice, subSliceFunctions) {
  auto middle = this->getTestSlice(3, 7).subSlice(1, 3);

  ASSERT_EQ(middle.size(), static_cast<size_t>(2));
  ASSERT_EQ(middle.at(1), static_cast<typename Slice<TypeParam>::value_type>(5));
}

TYPED_TEST_P(slice, subSliceFailedConstruction) {
  // 0 1 2 3 4 5 6 7 8 9
  //         | |         middle
  auto middle = this->getTestSlice(4, 6);

  ASSERT_THROW(static_cast<void>(middle.subSlice(1, 5)), std::out_of_range);
  ASSERT_THROW(static_cast<void>(middle.subSlice(2, 1)), std::out_of_range);
  ASSERT_THROW(static_cast<void>(middle.subSlice(2, 2)), std::out_of_range);
}

/*! try to cause integer overflows in a sub-optimal implementation */
TYPED_TEST_P(slice, subSliceConstructionOverflowResistance) {
  auto center_vals = this->getTestSlice(3, 7);

  ASSERT_THROW(static_cast<void>(center_vals.subSlice(std::numeric_limits<size_t>::max() - 2, 3)), std::out_of_range);
  ASSERT_THROW(static_cast<void>(center_vals.subSlice(2, std::numeric_limits<size_t>::max() - 1)), std::out_of_range);
}

/*!
 * This function's purpose is only to check whether we can pass all slices by
 * constant reference.
 */
template <typename T>
void checkConstSliceValueAt(const Slice<T>& sl, typename Slice<T>::value_type value, size_t index) {
  ASSERT_EQ(sl.at(index), value);
}

/*!
 * Check that the contents of the slice are ascending via an iterator based for
 * loop.
 */
template <typename T>
void checkConstSliceIterator(const Slice<T>& sl, typename Slice<T>::value_type first_value) {
  for (auto it = sl.cbegin(); it < sl.cend(); ++it) {
    ASSERT_EQ(*it, first_value++);
  }
}

template <typename T>
void checkSubSlice(const Slice<T>& sl) {
  ASSERT_EQ(sl.at(1), sl.subSlice(1, sl.size()).at(0));
}

/*!
 * Test that all slices can be also passed as const references and still work
 */
TYPED_TEST_P(slice, constMethodsPreserveConst) {
  // 0 1 2 3 4 5 6 7 8 9
  //       |     |       center_vals
  auto center_vals = this->getTestSlice(3, 7);

  // check at() const works
  checkConstSliceValueAt(center_vals, 4, 1);

  checkConstSliceIterator(center_vals, 3);

  checkSubSlice(center_vals);
}

/*!
 * Test the non-const iterators
 */
TYPED_TEST_P(mutableSlice, iterators) {
  auto sl = this->getTestSlice();

  ASSERT_EQ(*sl.begin(), static_cast<typename decltype(sl)::value_type>(1));
  ASSERT_EQ(*sl.end(), static_cast<typename decltype(sl)::value_type>(this->vec_size - 1));

  for (auto it = sl.begin(); it < sl.end(); ++it) {
    *it *= 2;
  }

  ASSERT_EQ(this->vec_.at(0), 0);
  for (size_t j = 1; j < this->vec_size - 1; ++j) {
    ASSERT_EQ(this->vec_.at(j), static_cast<typename decltype(sl)::value_type>(2 * j));
    ASSERT_EQ(this->vec_.at(j), sl.at(j - 1));
  }
  ASSERT_EQ(this->vec_.at(this->vec_size - 1), static_cast<typename decltype(sl)::value_type>(this->vec_size - 1));
}

/*!
 * Test the non-const version of at()
 */
TYPED_TEST_P(mutableSlice, at) {
  auto sl = this->getTestSlice(2, 4);

  sl.at(0) = 6;
  sl.at(1) = 12;

  ASSERT_EQ(this->vec_.at(2), 6);
  ASSERT_EQ(this->vec_.at(3), 12);
  for (size_t j = 0; j < this->vec_size - 1; ++j) {
    if (j == 2 || j == 3) {
      continue;
    }
    ASSERT_EQ(this->vec_.at(j), static_cast<typename decltype(sl)::value_type>(j));
  }
}

TEST(pointerSlice, failedConstructionFromNullpointer) {
  ASSERT_THROW(Slice<long*>(nullptr, 1, 2), std::invalid_argument);
}

/*!
 * Test the construction of an invalid slices from a container (so that a proper
 * range check can be conducted)
 */
TEST(containerSlice, failedConstructionFromContainer) {
  std::vector<int> tmp(10);
  // slice end too large
  ASSERT_THROW(Slice<std::vector<int>>(tmp, 1, tmp.size() + 1), std::out_of_range);
}

/*!
 * Test all functions from the makeSlice* family.
 */
TEST(containerSlice, makeSlice) {
  std::string str = "this is a sentence";

  auto is = makeSlice(str, 5, 7);
  ASSERT_TRUE(std::equal(is.begin(), is.end(), "is"));

  auto sl_this = makeSliceUntil(str, 4);
  ASSERT_TRUE(std::equal(sl_this.begin(), sl_this.end(), "this"));

  auto sl_sentence = makeSliceFrom(str, 10);
  ASSERT_TRUE(std::equal(sl_sentence.begin(), sl_sentence.end(), "sentence"));

  auto sl_full = makeSlice(str);
  ASSERT_TRUE(std::equal(sl_full.begin(), sl_full.end(), str.c_str()));
}

struct stringSlice : public ::testing::Test {
  std::string sentence;

  void SetUp() override {
    sentence = "this is a sentence";
  }
};

TEST_F(stringSlice, at) {
  auto is_a = makeSlice(static_cast<const std::string&>(this->sentence), 5, 10);

  ASSERT_EQ(is_a.at(0), 'i');
  ASSERT_EQ(is_a.at(4), ' ');
}

TEST_F(stringSlice, atFailure) {
  auto is_a = makeSlice(static_cast<const std::string&>(this->sentence), 5, 10);
  ASSERT_THROW(is_a.at(5), std::out_of_range);
}

TEST_F(stringSlice, size) {
  auto is_a = makeSlice(static_cast<const std::string&>(this->sentence), 5, 10);
  ASSERT_EQ(is_a.size(), static_cast<size_t>(5));
}

TEST_F(stringSlice, mutateString) {
  for (auto& m : makeSlice(this->sentence, 5, 10)) {
    m = ' ';
  }

  ASSERT_STREQ(this->sentence.c_str(), "this      sentence");
}

template <typename T>
struct dataBufSlice : public ::testing::Test {
  static constexpr byte data[4] = {0xde, 0xad, 0xbe, 0xef};
  DataBuf buf = DataBuf(data, sizeof(data));
};

TYPED_TEST_SUITE_P(dataBufSlice);

TYPED_TEST_P(dataBufSlice, successfulConstruction) {
  // just check that makeSlice appears to work
  ASSERT_EQ(makeSlice(static_cast<TypeParam>(this->buf), 1, 3).size(), static_cast<size_t>(2));
}

TYPED_TEST_P(dataBufSlice, failedConstruction) {
  // check that we get an exception when end is larger than LONG_MAX
  ASSERT_THROW(
      makeSlice(static_cast<TypeParam>(this->buf), 1, static_cast<size_t>(std::numeric_limits<long>::max()) + 1),
      std::invalid_argument);

  // check that we get an exception when end is larger than the DataBuf
  ASSERT_THROW(makeSlice(static_cast<TypeParam>(this->buf), 1, 5), std::out_of_range);
}

//
// GTest boilerplate to get the tests running for all the different types
//
REGISTER_TYPED_TEST_SUITE_P(slice, atAccess, iteratorAccess, constructionFailsFromInvalidRange,
                            constructionFailsWithZeroLength, subSliceSuccessfulConstruction, subSliceFunctions,
                            subSliceFailedConstruction, subSliceConstructionOverflowResistance,
                            constMethodsPreserveConst);

using test_types_t = ::testing::Types<const std::vector<int>, std::vector<int>, int*, const int*>;
INSTANTIATE_TYPED_TEST_SUITE_P(slice, slice, test_types_t);

REGISTER_TYPED_TEST_SUITE_P(mutableSlice, iterators, at);
using mut_test_types_t = ::testing::Types<std::vector<int>, int*>;
INSTANTIATE_TYPED_TEST_SUITE_P(slice, mutableSlice, mut_test_types_t);

REGISTER_TYPED_TEST_SUITE_P(dataBufSlice, successfulConstruction, failedConstruction);
using data_buf_types_t = ::testing::Types<DataBuf&, const DataBuf&>;
INSTANTIATE_TYPED_TEST_SUITE_P(slice, dataBufSlice, data_buf_types_t);
