#include "FileStreamerTest.hpp"
#include "SortTest.hpp"
#include <gtest/gtest.h>

// execute all tests
int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
