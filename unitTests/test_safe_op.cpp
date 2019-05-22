#include <safe_op.hpp>

#include <gtest/gtest.h>

namespace si = Safe::Internal;

enum class Operation
{
    add,
    subtract,
    multiply,
    divide
};

// see:
// https://stackoverflow.com/questions/52256953/c-multiple-parameters-with-gtest-typed-test

/*!
 * struct that contains test numbers for Safe::add. Since different test values
 * are required for signed and unsigned types, this struct is specialized via
 * SFINAE.
 *
 * Each overload has two arrays:
 * T summand[] - the test numbers
 * bool overflow[][] - overflow[i][j] indicates whether summand[i] + summand[j]
 *                     is expected to overflow
 *
 * The numbers in summand are chosen accordingly to test all "interesting" cases
 * and cause some overflows. The actual test should simply check all possible
 * sums of summand against the value in overflow[][]
 */

template <typename T>
struct SignedOverflowFixture
{
    static_assert(std::is_signed<T>::value, "Type T must be signed");

    static constexpr size_t case_count = 8;

    static constexpr T test_values[case_count] = {std::numeric_limits<T>::min(),
                                                  static_cast<T>(std::numeric_limits<T>::min() + 1),
                                                  -1,
                                                  0,
                                                  1,
                                                  2,
                                                  static_cast<T>(std::numeric_limits<T>::max() - 1),
                                                  std::numeric_limits<T>::max()};

    static constexpr bool is_addition_overflow[case_count][case_count] = {
        // min
        {true, true, true, false, false, false, false, false},
        // min + 1
        {true, true, false, false, false, false, false, false},
        // -1
        {true, false, false, false, false, false, false, false},
        // 0
        {false, false, false, false, false, false, false, false},
        // 1
        {false, false, false, false, false, false, false, true},
        // 2
        {false, false, false, false, false, false, true, true},
        // max - 1
        {false, false, false, false, false, true, true, true},
        // max
        {false, false, false, false, true, true, true, true}};
};

template <typename T>
struct UnsignedOverflowFixture
{
    static_assert(!std::is_signed<T>::value, "Type T must be unsigned");

    static constexpr size_t case_count = 8;

    static constexpr T test_values[case_count] = {0,
                                                  1,
                                                  2,
                                                  static_cast<T>((std::numeric_limits<T>::max() / 2) - 1),
                                                  static_cast<T>(std::numeric_limits<T>::max() / 2),
                                                  static_cast<T>((std::numeric_limits<T>::max() / 2) + 1),
                                                  static_cast<T>(std::numeric_limits<T>::max() - 1),
                                                  std::numeric_limits<T>::max()};

    static constexpr bool is_addition_overflow[case_count][case_count] = {
        // 0
        {false, false, false, false, false, false, false, false},
        // 1
        {false, false, false, false, false, false, false, true},
        // 2
        {false, false, false, false, false, false, true, true},
        // max/2 - 1
        {false, false, false, false, false, false, true, true},
        // max/2
        {false, false, false, false, false, false, true, true},
        // max/2 + 1
        {false, false, false, false, false, true, true, true},
        // max - 1
        {false, false, true, true, true, true, true, true},
        // max
        {false, true, true, true, true, true, true, true}};
};

template <typename T>
constexpr T UnsignedOverflowFixture<T>::test_values[UnsignedOverflowFixture<T>::case_count];

template <typename T>
constexpr bool UnsignedOverflowFixture<T>::is_addition_overflow[UnsignedOverflowFixture<T>::case_count]
                                                               [UnsignedOverflowFixture<T>::case_count];

template <typename T>
constexpr T SignedOverflowFixture<T>::test_values[SignedOverflowFixture<T>::case_count];

template <typename T>
constexpr bool SignedOverflowFixture<T>::is_addition_overflow[SignedOverflowFixture<T>::case_count]
                                                             [SignedOverflowFixture<T>::case_count];

template <typename T>
struct AdditionOverflow
    : public ::testing::Test,
      std::conditional<std::is_signed<T>::value, SignedOverflowFixture<T>, UnsignedOverflowFixture<T> >::type
{
};

using IntegerTypes = ::testing::Types<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t>;

TYPED_TEST_CASE(AdditionOverflow, IntegerTypes);

TYPED_TEST(AdditionOverflow, Add)
{
    for (size_t i = 0; i < this->case_count; ++i) {
        for (size_t j = 0; j < this->case_count; ++j) {
            if (this->is_addition_overflow[i][j]) {
                ASSERT_THROW(Safe::add(this->test_values[i], this->test_values[j]), std::overflow_error);
            } else {
                ASSERT_EQ(Safe::add(this->test_values[i], this->test_values[j]),
                          this->test_values[i] + this->test_values[j]);
            }
        }
    }
}

template <typename T>
struct LowLevelAdditionOverflow : public AdditionOverflow<T>
{
};

TYPED_TEST_CASE(LowLevelAdditionOverflow, IntegerTypes);

TYPED_TEST(LowLevelAdditionOverflow, SanityCheck)
{
    for (size_t i = 0; i < this->case_count; ++i) {
        for (size_t j = 0; j < this->case_count; ++j) {
            TypeParam res_builtin = 0;
            TypeParam res_fallback = 0;
            ASSERT_EQ(si::builtin_add_overflow(this->test_values[i], this->test_values[j], res_builtin),
                      this->is_addition_overflow[i][j]);
            ASSERT_EQ(si::fallback_add_overflow(this->test_values[i], this->test_values[j], res_fallback),
                      this->is_addition_overflow[i][j]);

            if (!this->is_addition_overflow[i][j]) {
                ASSERT_EQ(res_builtin, this->test_values[i] + this->test_values[j]);
                ASSERT_EQ(res_fallback, this->test_values[i] + this->test_values[j]);
            }
        }
    }
}

TEST(safeAbs, checkValues)
{
    static const int values[] = {-1, 1, std::numeric_limits<int>::max(), std::numeric_limits<int>::min() + 1};
    for (size_t i = 0; i < sizeof(values) / sizeof(*values); ++i) {
        ASSERT_EQ(Safe::abs(values[i]), abs(values[i]));
    }
    ASSERT_EQ(Safe::abs(std::numeric_limits<int>::min()), std::numeric_limits<int>::max());
}
