// SPDX-License-Identifier: GPL-2.0-or-later

#ifndef SAFE_OP_HPP_
#define SAFE_OP_HPP_

#include <limits>
#include <stdexcept>
#include <type_traits>

#ifdef _MSC_VER
#include <Intsafe.h>
#endif

/*!
 * @brief Arithmetic operations with overflow checks
 */
namespace Safe {
/*!
 * @brief Helper functions for providing integer overflow checks.
 *
 * This namespace contains internal helper functions fallback_$op_overflow
 * and builtin_$op_overflow (where $op is an arithmetic operation like add,
 * subtract, etc.). Both provide the following interface:
 *
 * bool fallback/builtin_$op_overflow(T first, T second, T& result);
 *
 * where T is an integer type.
 *
 * Each function performs checks whether first $op second can be safely
 * performed without overflows. If yes, the result is saved in result and
 * false is returned. Otherwise true is returned and the contents of result
 * are unspecified.
 *
 * fallback_$op_overflow implements a portable but slower overflow check.
 * builtin_$op_overflow uses compiler builtins (when available) and should
 * be faster. As builtins are not available for all types,
 * builtin_$op_overflow falls back to fallback_$op_overflow when no builtin
 * is available.
 */
namespace Internal {
/*!
 * @brief Check the addition of two numbers for overflows for signed
 * integer types larger than int or with the same size as int.
 *
 * This function performs a check if summand_1 + summand_2 would
 * overflow and returns true in that case. If no overflow occurs,
 * the sum is saved in result and false is returned.
 *
 * @return true on overflow, false on no overflow
 *
 * @param[in] summand_1 The summand with is added
 * @param[in] summand_2 The summand with is added
 * @param[out] result Result of the addition, only populated when no
 * overflow occurs.
 *
 * Further information:
 * https://wiki.sei.cmu.edu/confluence/display/c/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow
 * https://wiki.sei.cmu.edu/confluence/display/c/INT02-C.+Understand+integer+conversion+rules
 * https://wiki.sei.cmu.edu/confluence/display/c/INT30-C.+Ensure+that+unsigned+integer+operations+do+not+wrap
 */
template <typename T>
bool fallback_add_overflow(T summand_1, T summand_2, T& result) {
  if constexpr (std::is_signed_v<T> && sizeof(T) >= sizeof(int)) {
    if (((summand_2 >= 0) && (summand_1 > std::numeric_limits<T>::max() - summand_2)) ||
        ((summand_2 < 0) && (summand_1 < std::numeric_limits<T>::min() - summand_2)))
      return true;
    result = summand_1 + summand_2;
    return false;
  } else if constexpr (std::is_signed_v<T> && sizeof(T) < sizeof(int)) {
    const int res = summand_1 + summand_2;
    if ((res > std::numeric_limits<T>::max()) || (res < std::numeric_limits<T>::min())) {
      return true;
    }
    result = static_cast<T>(res);
    return false;
  } else {
    result = summand_1 + summand_2;
    return result < summand_1;
  }
}

/*!
 * @brief Overflow addition check using compiler intrinsics.
 *
 * This function behaves exactly like fallback_add_overflow() but it
 * relies on compiler intrinsics instead. This version should be faster
 * than the fallback version as it can fully utilize available CPU
 * instructions & the compiler's diagnostic.
 *
 * However, as some compilers don't provide intrinsics for certain
 * types, the default implementation is the version from fallback.
 *
 * This function is fully specialized for each compiler.
 */
template <typename T>
bool builtin_add_overflow(T summand_1, T summand_2, T& result) {
  return fallback_add_overflow(summand_1, summand_2, result);
}

#if defined(__GNUC__) || defined(__clang__)
#if __GNUC__ >= 5 || __clang_major__ >= 3

/*!
 * This macro pastes a specialization of builtin_add_overflow using gcc's &
 * clang's __builtin_(s/u)add(l)(l)_overlow()
 *
 * The add function is implemented by forwarding the parameters to the intrinsic
 * and returning its value.
 *
 * The intrinsics are documented here:
 * https://gcc.gnu.org/onlinedocs/gcc/Integer-Overflow-Builtins.html#Integer-Overflow-Builtins
 */
#define SPECIALIZE_builtin_add_overflow(type, builtin_name)                                  \
  /* Full specialization of builtin_add_overflow for type using the */                       \
  /* builtin_name intrinsic */                                                               \
  template <>                                                                                \
  constexpr bool builtin_add_overflow<type>(type summand_1, type summand_2, type & result) { \
    return builtin_name(summand_1, summand_2, &result);                                      \
  }

SPECIALIZE_builtin_add_overflow(int, __builtin_sadd_overflow);
SPECIALIZE_builtin_add_overflow(long, __builtin_saddl_overflow);
SPECIALIZE_builtin_add_overflow(long long, __builtin_saddll_overflow);

SPECIALIZE_builtin_add_overflow(unsigned int, __builtin_uadd_overflow);
SPECIALIZE_builtin_add_overflow(unsigned long, __builtin_uaddl_overflow);
SPECIALIZE_builtin_add_overflow(unsigned long long, __builtin_uaddll_overflow);

#undef SPECIALIZE_builtin_add_overflow
#endif  // __GNUC__ >= 5 || __clang_major >= 3

#endif

}  // namespace Internal

/*!
 * @brief Safe addition, throws an exception on overflow.
 *
 * This function returns the result of summand_1 and summand_2 only when the
 * operation would not overflow, otherwise an exception of type
 * std::overflow_error is thrown.
 *
 * @param[in] summand_1 summand to be summed up
 * @param[in] summand_2 summand to be summed up
 * @return  the sum of summand_1 and summand_2
 * @throws  std::overflow_error if the addition would overflow
 *
 * This function utilizes compiler builtins when available and should have a
 * very small performance hit then. When builtins are unavailable, a more
 * extensive check is required.
 *
 * Builtins are available for the following configurations:
 * - GCC/Clang for signed and unsigned int, long and long long (not char & short)
 * - MSVC for unsigned int, long and long long
 */
template <typename T>
T add(T summand_1, T summand_2) {
  T res = 0;
  if (Internal::builtin_add_overflow(summand_1, summand_2, res)) {
    throw std::overflow_error("Overflow in addition");
  }
  return res;
}

/*!
 * @brief Calculates the absolute value of a number without producing
 * negative values.
 *
 * The "standard" implementation of `abs(num)` (`num < 0 ? -num : num`)
 * produces negative values when `num` is the smallest negative number. This
 * is caused by `-1 * INTMAX = INTMIN + 1`, i.e. the real result of
 * `abs(INTMIN)` overflows the integer type and results in `INTMIN` again
 * (this is not guaranteed as it invokes undefined behavior).
 *
 * This function does not exhibit this behavior, it returns
 * `std::numeric_limits<T>::max()` when the input is
 * `std::numeric_limits<T>::min()`. The downside of this is that two
 * negative values produce the same absolute value:
 * `std::numeric_limits<T>::min()` and `std::numeric_limits<T>::min() + 1`.
 *
 * @tparam T  a signed integer type
 * @param[in] num  The number which absolute value should be computed.
 * @throws  Never throws an exception.
 * @return  The absolute value of `num` or `std::numeric_limits<T>::max()`
 *          when `num == std::numeric_limits<T>::min()`.
 */
template <typename T>
T abs(T num) noexcept {
  if constexpr (std::is_signed_v<T>) {
    if (num == std::numeric_limits<T>::min())
      return std::numeric_limits<T>::max();
    return num < 0 ? -num : num;
  }
  return num;
}

}  // namespace Safe

#endif  // SAFE_OP_HPP_
