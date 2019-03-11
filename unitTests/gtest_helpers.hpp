#ifndef EXIV2_UNIT_TESTS_GTEST_HELPERS_HPP
#define EXIV2_UNIT_TESTS_GTEST_HELPERS_HPP

#include "gtestwrapper.h"

#include "exiv2/error.hpp"

/// GTest helper that checks whether an expr; throws an Exiv2::Error with the given error_code
#define ASSERT_THROW_ERROR_CODE(expr, error_code)                                    \
    do {                                                                             \
        try {                                                                        \
            expr;                                                                    \
            FAIL() << #expr " did not throw an exception";                           \
        } catch (const Error& err) {                                                 \
            ASSERT_EQ(err.code(), error_code);                                       \
        } catch (...) {                                                              \
            FAIL() << "Expected Exiv2::Error but got a different exception instead"; \
        }                                                                            \
    } while (0)

#endif  // EXIV2_UNIT_TESTS_GTEST_HELPERS_HPP
