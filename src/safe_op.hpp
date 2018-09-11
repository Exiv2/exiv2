// ********************************************************* -*- C++ -*-
/*
 * Copyright (C) 2004-2018 Exiv2 authors
 * This program is part of the Exiv2 distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, 5th Floor, Boston, MA 02110-1301 USA.
 */
/*!
  @file    safe_op.hpp
  @brief   Overflow checks for integers
  @author  Dan Čermák (D4N)
           <a href="mailto:dan.cermak@cgc-instruments.com">dan.cermak@cgc-instruments.com</a>
  @date    14-Dec-17, D4N: created
 */

#ifndef SAFE_OP_HPP_
#define SAFE_OP_HPP_

#include <limits>
#include <stdexcept>

#ifdef _MSC_VER
#include <Intsafe.h>
#endif

/*!
 * @brief Arithmetic operations with overflow checks
 */
namespace Safe
{
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
    namespace Internal
    {
        /*!
         * @brief Helper struct to determine whether a type is signed or unsigned
         *
         * This struct is a backport of std::is_signed from C++11. It has a public
         * enum with the property VALUE which is true when the type is signed or
         * false if it is unsigned.
         */
        template <typename T>
        struct is_signed
        {
            enum
            {
                VALUE = T(-1) < T(0)
            };
        };

        /*!
         * @brief Helper struct for SFINAE, from C++11

         * This struct has a public typedef called type typedef'd to T if B is
         * true. Otherwise there is no typedef.
         */
        template <bool B, class T = void>
        struct enable_if
        {
        };

        /*!
         * @brief Specialization of enable_if for the case B == true
         */
        template <class T>
        struct enable_if<true, T>
        {
            typedef T type;
        };

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
         * @param[in] summand_1, summand_2 The summands with are added
         * @param[out] result Result of the addition, only populated when no
         * overflow occurs.
         *
         * Further information:
         * https://wiki.sei.cmu.edu/confluence/display/c/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow
         */
        template <typename T>
        typename enable_if<is_signed<T>::VALUE && sizeof(T) >= sizeof(int), bool>::type fallback_add_overflow(
            T summand_1, T summand_2, T& result)
        {
            if (((summand_2 >= 0) && (summand_1 > std::numeric_limits<T>::max() - summand_2)) ||
                ((summand_2 < 0) && (summand_1 < std::numeric_limits<T>::min() - summand_2))) {
                return true;
            } else {
                result = summand_1 + summand_2;
                return false;
            }
        }

        /*!
         * @brief Check the addition of two numbers for overflows for signed
         * integer types smaller than int.
         *
         * This function adds summand_1 and summand_2 exploiting integer
         * promotion rules, thereby not causing undefined behavior. The
         * result is checked against the limits of T and true is returned if
         * they are exceeded. Otherwise the sum is saved in result and false
         * is returned.
         *
         * @return true on overflow, false on no overflow
         *
         * @param[in] summand_1, summand_2 The summands with are added
         * @param[out] result Result of the addition, only populated when no
         * overflow occurs.
         *
         * Further information:
         * https://wiki.sei.cmu.edu/confluence/display/c/INT02-C.+Understand+integer+conversion+rules
         */
        template <typename T>
        typename enable_if<is_signed<T>::VALUE && sizeof(T) < sizeof(int), bool>::type fallback_add_overflow(
            T summand_1, T summand_2, T& result)
        {
            const int res = summand_1 + summand_2;
            if ((res > std::numeric_limits<T>::max()) || (res < std::numeric_limits<T>::min())) {
                return true;
            } else {
                result = static_cast<T>(res);
                return false;
            }
        }

        /*!
         * @brief Check the addition of two numbers for overflows for unsigned
         * integer types.
         *
         * This function adds summand_1 and summand_2 and checks after that if
         * the operation overflowed. Since these are unsigned integers, no
         * undefined behavior is invoked.
         *
         * @return true on overflow, false on no overflow
         *
         * @param[in] summand_1, summand_2 The summands with are added
         * @param[out] result Result of the addition
         *
         * Further information:
         * https://wiki.sei.cmu.edu/confluence/display/c/INT30-C.+Ensure+that+unsigned+integer+operations+do+not+wrap
         */
        template <typename T>
        typename enable_if<!is_signed<T>::VALUE, bool>::type fallback_add_overflow(T summand_1, T summand_2, T& result)
        {
            result = summand_1 + summand_2;
            return result < summand_1;
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
        bool builtin_add_overflow(T summand_1, T summand_2, T& result)
        {
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
#define SPECIALIZE_builtin_add_overflow(type, builtin_name)                               \
    /* Full specialization of builtin_add_overflow for type using the */                  \
    /* builtin_name intrinsic */                                                          \
    template <>                                                                           \
    inline bool builtin_add_overflow<type>(type summand_1, type summand_2, type & result) \
    {                                                                                     \
        return builtin_name(summand_1, summand_2, &result);                               \
    }

        SPECIALIZE_builtin_add_overflow(int, __builtin_sadd_overflow);
        SPECIALIZE_builtin_add_overflow(long, __builtin_saddl_overflow);
        SPECIALIZE_builtin_add_overflow(long long, __builtin_saddll_overflow);

        SPECIALIZE_builtin_add_overflow(unsigned int, __builtin_uadd_overflow);
        SPECIALIZE_builtin_add_overflow(unsigned long, __builtin_uaddl_overflow);
        SPECIALIZE_builtin_add_overflow(unsigned long long, __builtin_uaddll_overflow);

#undef SPECIALIZE_builtin_add_overflow
#endif  // __GNUC__ >= 5 || __clang_major >= 3

#elif defined(_MSC_VER)
// intrinsics are not in available in MSVC 2005 and earlier
#if _MSC_VER >= 1400

/*!
 * This macro pastes a specialization of builtin_add_overflow using MSVC's
 * U(Int/Long/LongLong)Add.
 *
 * The add function is implemented by forwarding the parameters to the
 * intrinsic. As MSVC's intrinsics return S_OK on success, this specialization
 * returns whether the intrinsics return value does not equal S_OK. This ensures
 * a uniform interface of the add function (false is returned when no overflow
 * occurs, true on overflow).
 *
 * The intrinsics are documented here:
 * https://msdn.microsoft.com/en-us/library/windows/desktop/ff516460(v=vs.85).aspx
 */
#define SPECIALIZE_builtin_add_overflow_WIN(type, builtin_name)                    \
    template <>                                                                    \
    inline bool builtin_add_overflow(type summand_1, type summand_2, type& result) \
    {                                                                              \
        return builtin_name(summand_1, summand_2, &result) != S_OK;                \
    }

        SPECIALIZE_builtin_add_overflow_WIN(unsigned int, UIntAdd);
        SPECIALIZE_builtin_add_overflow_WIN(unsigned long, ULongAdd);
        SPECIALIZE_builtin_add_overflow_WIN(unsigned long long, ULongLongAdd);

#undef SPECIALIZE_builtin_add_overflow_WIN

#endif  // _MSC_VER >= 1400
#endif  // defined(_MSC_VER)

    }  // namespace Internal

    /*!
     * @brief Safe addition, throws an exception on overflow.
     *
     * This function returns the result of summand_1 and summand_2 only when the
     * operation would not overflow, otherwise an exception of type
     * std::overflow_error is thrown.
     *
     * @param[in] summand_1, summand_2  summands to be summed up
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
    T add(T summand_1, T summand_2)
    {
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
    typename Internal::enable_if<Internal::is_signed<T>::VALUE, T>::type abs(T num) throw()
    {
        if (num == std::numeric_limits<T>::min()) {
            return std::numeric_limits<T>::max();
        }
        return num < 0 ? -num : num;
    }

}  // namespace Safe

#endif  // SAFE_OP_HPP_
