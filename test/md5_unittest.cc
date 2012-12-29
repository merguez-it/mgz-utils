#include "security/md5.h"
#include "gtest/gtest.h"

TEST(Hash, TestMd5_buffer) {
  std::string m = mgz::security::md5("Hello World!");
  EXPECT_STREQ("ed076287532e86365e841e92bfc50d8c", m.c_str());
}

