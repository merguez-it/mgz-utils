#include "security/sha1.h"
#include "gtest/gtest.h"

TEST(Hash, TestSha1_buffer) {
  std::string n = mgz::security::sha1("Hello World!");
  EXPECT_EQ("2ef7bde608ce5404e97d5f042f95f89f1c232871", n);
}

