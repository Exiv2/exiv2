#include <exiv2/error.hpp>

#include <gtest/gtest.h>

using namespace Exiv2;

TEST(error, parameterInsertion)
{
    const Error err_with_3_params(ErrorCode::kerGeneralError, "foo", "bar", "baz");
    ASSERT_STREQ(err_with_3_params.what(), "Error 1: arg2=bar, arg3=baz, arg1=foo.");

    const Error err_with_no_params(ErrorCode::kerSuccess);
    ASSERT_STREQ(err_with_no_params.what(), "Success");
}

TEST(error, tooManyParameters)
{
    const Error err_with_no_params(ErrorCode::kerSuccess, "a param", "another param");
    ASSERT_STREQ(err_with_no_params.what(), "Success");
}
