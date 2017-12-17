// ********************************************************* -*- C++ -*-
/*
 * Copyright (C) 2004-2017 Exiv2 maintainers
 *
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
     * @brief Helper structs for providing integer overflow checks.
     *
     * This namespace contains the internal helper structs fallback_add_overflow
     * and builtin_add_overflow. Both have a public static member function add
     * with the following interface:
     *
     * bool add(T summand_1, T summand_2, T& result)
     *
     * where T is the type over which the struct is templated.
     *
     * The function performs a check whether the addition summand_1 + summand_2
     * can be performed without an overflow. If the operation would overflow,
     * true is returned and the addition is not performed if it would result in
     * undefined behavior. If no overflow occurs, the sum is saved in result and
     * false is returned.
     *
     * fallback_add_overflow implements a portable but slower overflow check.
     * builtin_add_overflow uses compiler builtins (when available) and should
     * be considerably faster. As builtins are not available for all types,
     * builtin_add_overflow falls back to fallback_add_overflow when no builtin
     * is available.
     */
    namespace Internal
    {
        /*!
         * @brief Helper struct to determine whether a type is signed or unsigned

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
         * @brief Fallback overflow checker, specialized via SFINAE
         *
         * This struct implements a 'fallback' addition with an overflow check,
         * i.e. it does not rely on compiler intrinsics.  It is specialized via
         * SFINAE for signed and unsigned integer types and provides a public
         * static member function add.
         */
        template <typename T, typename = void>
        struct fallback_add_overflow;

        /*!
         * @brief Overload of fallback_add_overflow for signed integers
         */
        template <typename T>
        struct fallback_add_overflow<T, typename enable_if<is_signed<T>::VALUE>::type>
        {
            /*!
             * @brief Adds the two summands only if no overflow occurs
             *
             * This function performs a check if summand_1 + summand_2 would
             * overflow and returns true in that case. If no overflow occurs,
             * the sum is saved in result and false is returned.
             *
             * @return true on overflow, false on no overflow
             *
             * The check for an overflow is performed before the addition to
             * ensure that no undefined behavior occurs. The value in result is
             * only valid when the function returns false.
             *
             * Further information:
             * https://wiki.sei.cmu.edu/confluence/display/c/INT32-C.+Ensure+that+operations+on+signed+integers+do+not+result+in+overflow
             */
            static bool add(T summand_1, T summand_2, T& result)
            {
                if (((summand_2 >= 0) && (summand_1 > std::numeric_limits<T>::max() - summand_2)) ||
                    ((summand_2 < 0) && (summand_1 < std::numeric_limits<T>::min() - summand_2))) {
                    return true;
                } else {
                    result = summand_1 + summand_2;
                    return false;
                }
            }
        };

        /*!
         * @brief Overload of fallback_add_overflow for unsigned integers
         */
        template <typename T>
        struct fallback_add_overflow<T, typename enable_if<!is_signed<T>::VALUE>::type>
        {
            /*!
             * @brief Adds the two summands only if no overflow occurs
             *
             * This function performs a check if summand_1 + summand_2 would
             * overflow and returns true in that case. If no overflow occurs,
             * the sum is saved in result and false is returned.
             *
             * @return true on overflow, false on no overflow
             *
             * Further information:
             * https://wiki.sei.cmu.edu/confluence/display/c/INT30-C.+Ensure+that+unsigned+integer+operations+do+not+wrap
             */
            static bool add(T summand_1, T summand_2, T& result)
            {
                if (summand_1 > std::numeric_limits<T>::max() - summand_2) {
                    return true;
                } else {
                    result = summand_1 + summand_2;
                    return false;
                }
            }
        };

        /*!
         * @brief Overflow checker using compiler intrinsics
         *
         * This struct provides an add function with the same interface &
         * behavior as fallback_add_overload::add but it relies on compiler
         * intrinsics instead. This version should be considerably faster than
         * the fallback version as it can fully utilize available CPU
         * instructions & the compiler's diagnostic.
         *
         * However, as some compilers don't provide intrinsics for certain
         * types, the default implementation of add is the version from falback.
         *
         * The struct is explicitly specialized for each type via #ifdefs for
         * each compiler.
         */
        template <typename T>
        struct builtin_add_overflow
        {
            /*!
             * @brief Add summand_1 and summand_2 and check for overflows.
             *
             * This is the default add() function that uses
             * fallback_add_overflow<T>::add(). All specializations must have
             * exactly the same interface and behave the same way.
             */
            static inline bool add(T summand_1, T summand_2, T& result)
            {
                return fallback_add_overflow<T>::add(summand_1, summand_2, result);
            }
        };

#if defined(__GNUC__) || defined(__clang__)
#if __GNUC__ >= 5

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
#define SPECIALIZE_builtin_add_overflow(type, builtin_name)                  \
    template <>                                                              \
    struct builtin_add_overflow<type>                                        \
    {                                                                        \
        static inline bool add(type summand_1, type summand_2, type& result) \
        {                                                                    \
            return builtin_name(summand_1, summand_2, &result);              \
        }                                                                    \
    }

        SPECIALIZE_builtin_add_overflow(int, __builtin_sadd_overflow);
        SPECIALIZE_builtin_add_overflow(long, __builtin_saddl_overflow);
        SPECIALIZE_builtin_add_overflow(long long, __builtin_saddll_overflow);

        SPECIALIZE_builtin_add_overflow(unsigned int, __builtin_uadd_overflow);
        SPECIALIZE_builtin_add_overflow(unsigned long, __builtin_uaddl_overflow);
        SPECIALIZE_builtin_add_overflow(unsigned long long, __builtin_uaddll_overflow);

#undef SPECIALIZE_builtin_add_overflow
#endif

#elif defined(_MSC_VER)

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
#define SPECIALIZE_builtin_add_overflow_WIN(type, builtin_name)              \
    template <>                                                              \
    struct builtin_add_overflow<type>                                        \
    {                                                                        \
        static inline bool add(type summand_1, type summand_2, type& result) \
        {                                                                    \
            return builtin_name(summand_1, summand_2, &result) != S_OK;      \
        }                                                                    \
    }

        SPECIALIZE_builtin_add_overflow_WIN(unsigned int, UIntAdd);
        SPECIALIZE_builtin_add_overflow_WIN(unsigned long, ULongAdd);
        SPECIALIZE_builtin_add_overflow_WIN(unsigned long long, ULongLongAdd);

#undef SPECIALIZE_builtin_add_overflow_WIN

#endif

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
        if (Internal::builtin_add_overflow<T>::add(summand_1, summand_2, res)) {
            throw std::overflow_error("Overflow in addition");
        }
        return res;
    }

}  // namespace Safe

#endif  // SAFE_OP_HPP_
