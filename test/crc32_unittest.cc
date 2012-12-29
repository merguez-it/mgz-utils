#include "security/crc32.h"
#include "gtest/gtest.h"

TEST(Hash, TestCrc32_buffer) {
  crc32_t c = mgz::security::crc32("Hello World!");
  EXPECT_EQ((crc32_t)472456355, c);
}

