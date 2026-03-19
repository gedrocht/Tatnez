#include <gtest/gtest.h>

auto main(int argumentCount, char** argumentValues) -> int {
  ::testing::InitGoogleTest(&argumentCount, argumentValues);
  return RUN_ALL_TESTS();
}
