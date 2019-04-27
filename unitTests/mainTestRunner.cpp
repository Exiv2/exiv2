#include <gtest/gtest.h>

#include <exiv2/properties.hpp>

#include <iostream>

class Environment : public ::testing::Environment {
public:
  void SetUp() override {}

  void TearDown() override { Exiv2::XmpProperties::unregisterNs(); }
};

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::testing::AddGlobalTestEnvironment(new Environment);

  int ret = RUN_ALL_TESTS();

  std::cout << "Tests finished with return value: " << ret << std::endl;

  return ret;
}
