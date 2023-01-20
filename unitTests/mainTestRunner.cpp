// SPDX-License-Identifier: GPL-2.0-or-later

#include <gtest/gtest.h>
#include <iostream>
using namespace std;
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  int ret = RUN_ALL_TESTS();

  cout << "Tests finished with return value: " << ret << endl;

  return ret;
}
