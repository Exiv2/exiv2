#include "gtestwrapper.h"

#include <iostream>

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    int ret = RUN_ALL_TESTS();

    std::cout << "Tests finished with return value: " << ret << std::endl;

    return ret;
}
