#include <safe_op.hpp>

#include <gtest/gtest.h>

namespace si = Safe::Internal;

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
template <typename T, typename = void>
struct AdditionTestValues;

/*!
 * Overload for unsigned types.
 */
template <typename T>
struct AdditionTestValues<T, typename si::enable_if<!si::is_signed<T>::VALUE>::type>
{
    static const size_t case_count = 5;
    static const T summand[case_count];
    static const bool overflow[case_count][case_count];
};

template <typename T>
const T AdditionTestValues<T, typename si::enable_if<!si::is_signed<T>::VALUE>::type>::summand[] = {
    0, 1, 2, static_cast<T>(std::numeric_limits<T>::max() - 1), std::numeric_limits<T>::max()};

template <typename T>
const bool
    AdditionTestValues<T, typename si::enable_if<!si::is_signed<T>::VALUE>::type>::overflow[case_count][case_count] = {
        // 0
        {false, false, false, false, false},
        // 1
        {false, false, false, false, true},
        // 2
        {false, false, false, true, true},
        // max - 1
        {false, false, true, true, true},
        // max
        {false, true, true, true, true}};

/*!
 * Overload for signed integers
 */
template <typename T>
struct AdditionTestValues<T, typename si::enable_if<si::is_signed<T>::VALUE>::type>
{
    static const size_t case_count = 8;
    static const T summand[case_count];
    static const bool overflow[case_count][case_count];
};

template <typename T>
const T AdditionTestValues<T, typename si::enable_if<si::is_signed<T>::VALUE>::type>::summand[] = {
    std::numeric_limits<T>::min(),
    static_cast<T>(std::numeric_limits<T>::min() + 1),
    -1,
    0,
    1,
    2,
    static_cast<T>(std::numeric_limits<T>::max() - 1),
    std::numeric_limits<T>::max()};

template <typename T>
const bool
    AdditionTestValues<T, typename si::enable_if<si::is_signed<T>::VALUE>::type>::overflow[case_count][case_count] = {
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

/*!
 * Test the addition of all combinations of AdditionTestValues<T>::summand[i],
 * AdditionTestValues<T>::summand[j] using fallback_add_overflow::add and
 * builtin_add_overflow::add. The return value is checked against
 * AdditionTestValues<T>::overflow[i][j]. If no overflow occurs, then the result
 * is checked too.
 */
template <typename T>
void test_add()
{
    typedef AdditionTestValues<T> TestValues;

#define TEST_ADD(func)                                                                                        \
    for (size_t i = 0; i < TestValues::case_count; ++i) {                                                     \
        for (size_t j = 0; j < TestValues::case_count; ++j) {                                                 \
            T res = 0;                                                                                        \
            ASSERT_EQ(func(TestValues::summand[i], TestValues::summand[j], res), TestValues::overflow[i][j]); \
            if (!TestValues::overflow[i][j]) {                                                                \
                ASSERT_EQ(res, TestValues::summand[i] + TestValues::summand[j]);                              \
            }                                                                                                 \
        }                                                                                                     \
    }

    TEST_ADD(si::fallback_add_overflow)
    TEST_ADD(si::builtin_add_overflow)

#undef TEST_ADD
}

/*!
 * Test the addition of all combinations of AdditionTestValues<T>::summand[i],
 * AdditionTestValues<T>::summand[j] using Safe::add.
 *
 * It is checked whether an exception is thrown when
 * AdditionTestValues<T>::overflow[i][j] is true, otherwise the result is
 * checked.
 */
template <typename T>
void test_safe_add()
{
    typedef AdditionTestValues<T> TestValues;

    for (size_t i = 0; i < TestValues::case_count; ++i) {
        for (size_t j = 0; j < TestValues::case_count; ++j) {
            if (TestValues::overflow[i][j]) {
                ASSERT_THROW(Safe::add(TestValues::summand[i], TestValues::summand[j]), std::overflow_error);
            } else {
                ASSERT_EQ(Safe::add(TestValues::summand[i], TestValues::summand[j]),
                          TestValues::summand[i] + TestValues::summand[j]);
            }
        }
    }
}

TEST(lowLevelAddOverflow, checkUnsignedOverflow)
{
    test_add<unsigned char>();
    test_add<unsigned short>();
    test_add<unsigned int>();
    test_add<unsigned long>();
    test_add<unsigned long long>();
}

TEST(lowLevelAddOverflow, checkSignedOverflow)
{
    test_add<char>();
    test_add<short>();
    test_add<int>();
    test_add<long>();
    test_add<long long>();
}

TEST(safeAdd, checkUnsignedOverflow)
{
    test_safe_add<unsigned char>();
    test_safe_add<unsigned short>();
    test_safe_add<unsigned int>();
    test_safe_add<unsigned long>();
    test_safe_add<unsigned long long>();
}

TEST(safeAdd, checkSignedOverflow)
{
    test_safe_add<char>();
    test_safe_add<short>();
    test_safe_add<int>();
    test_safe_add<long>();
    test_safe_add<long long>();
}

TEST(safeAbs, checkValues)
{
    static const int values[] = {-1, 1, std::numeric_limits<int>::max(), std::numeric_limits<int>::min() + 1};
    for (size_t i = 0; i < sizeof(values) / sizeof(*values); ++i) {
        ASSERT_EQ(Safe::abs(values[i]), abs(values[i]));
    }
    ASSERT_EQ(Safe::abs(std::numeric_limits<int>::min()), std::numeric_limits<int>::max());
}

//
// sanity checks of is_safely_convertible
//
static_assert(si::is_safely_convertible<uint8_t, uint16_t>::value, "uint8_t must be always convertible to uint16_t");
static_assert(!si::is_safely_convertible<uint16_t, uint8_t>::value, "uint16_t must not always convertible to uint8_t");

static_assert(si::is_safely_convertible<uint8_t, int16_t>::value, "uint8_t must be always convertible to int16_t");
static_assert(!si::is_safely_convertible<int16_t, uint8_t>::value, "int16_t must not always be convertible to uint8_t");

//
// sanity checks for have_same_signedness
//
static_assert(si::have_same_signedness<uint16_t, uint8_t>::value, "uint8_t must have the same signedness as uint16_t");
static_assert(!si::have_same_signedness<int16_t, uint8_t>::value,
              "uint8_t must have a different signedness as int16_t");

//
// sanity checks for Safe::cast<>
//
static_assert(std::is_same<decltype(Safe::cast<int>(static_cast<short>(8))), int>::value,
              "Return value of Safe::cast<int>(short) must be int");
static_assert(std::is_same<decltype(Safe::cast<int>(8ull)), int>::value,
              "Return value of Safe::cast<int>(unsigned long long) must be int");

TEST(SafeCast, TriviallyConvertible)
{
    ASSERT_EQ(Safe::cast<int>(static_cast<short>(5)), 5);
}

//
// Test Safe::cast to a signed integer
//
template <typename T>
struct SafeCastToInt16 : public ::testing::Test
{
};

using BiggerRangeThanInt16 = ::testing::Types<uint16_t, int32_t, uint32_t, int64_t, uint64_t>;

TYPED_TEST_CASE(SafeCastToInt16, BiggerRangeThanInt16);

TYPED_TEST(SafeCastToInt16, ThrowsForTooLargeValue)
{
    ASSERT_THROW(Safe::cast<int16_t>(static_cast<TypeParam>(std::numeric_limits<int16_t>::max()) + 1),
                 std::overflow_error);
}

TYPED_TEST(SafeCastToInt16, ThrowsForTooSmallValue)
{
    if (std::is_signed<TypeParam>::value) {
        ASSERT_THROW(Safe::cast<int16_t>(static_cast<TypeParam>(std::numeric_limits<int16_t>::min()) - 1),
                     std::overflow_error);
    }
}

TYPED_TEST(SafeCastToInt16, DoesNotThrowForRepresentableValue)
{
    constexpr TypeParam test_value = std::numeric_limits<int16_t>::max() - 1;
    ASSERT_EQ(Safe::cast<int16_t>(test_value), test_value);
}

//
// Test Safe::cast to an unsigned integer
//
template <typename T>
struct SafeCastToUInt32 : public ::testing::Test
{
};

using BiggerRangeThanUInt32 = ::testing::Types<int64_t, uint64_t>;

TYPED_TEST_CASE(SafeCastToUInt32, BiggerRangeThanUInt32);

TYPED_TEST(SafeCastToUInt32, ThrowsForTooLargeValue)
{
    ASSERT_THROW(Safe::cast<uint32_t>(static_cast<TypeParam>(std::numeric_limits<uint32_t>::max()) + 1),
                 std::overflow_error);
}

TYPED_TEST(SafeCastToUInt32, DoesNotThrowForRepresentableValue)
{
    constexpr TypeParam test_value = std::numeric_limits<uint32_t>::max() - 1;
    ASSERT_EQ(Safe::cast<uint32_t>(test_value), test_value);
}

TYPED_TEST(SafeCastToUInt32, ThrowsForTooSmallValue)
{
    if (std::is_signed<TypeParam>::value) {
        ASSERT_THROW(Safe::cast<uint32_t>(static_cast<TypeParam>(-1)), std::overflow_error);
    }
}
