#include <limits.h>
#include "util/string.h"
#include "gtest/gtest.h"

TEST(StringUtil, Trim) {
  std::string data_trimed ("trimed");
  std::string data_not_trimed ("    trimed   ");

  EXPECT_EQ(data_trimed, mgz::util::trim(data_not_trimed));
  EXPECT_EQ(data_trimed, mgz::util::trim(data_trimed));

  EXPECT_TRUE(mgz::util::trim("     ").empty());
  EXPECT_TRUE(mgz::util::trim("").empty());
}

TEST(StringUtil, ToUpper) {
  EXPECT_STREQ(std::string("HELLO").c_str(), mgz::util::to_upper("hello").c_str());
  EXPECT_STREQ(std::string("HELLO").c_str(), mgz::util::to_upper("HeLLo").c_str());
  EXPECT_STREQ(std::string("HELLO").c_str(), mgz::util::to_upper("HELLO").c_str());
}

TEST(StringUtil, ToLower) {
  EXPECT_STREQ(std::string("hello").c_str(), mgz::util::to_lower("hello").c_str());
  EXPECT_STREQ(std::string("hello").c_str(), mgz::util::to_lower("HeLLo").c_str());
  EXPECT_STREQ(std::string("hello").c_str(), mgz::util::to_lower("HELLO").c_str());
}

TEST(StringUtil, Classify) {
  EXPECT_STREQ(std::string("Hello").c_str(), mgz::util::classify("hello").c_str());
  EXPECT_STREQ(std::string("HelloWorld").c_str(), mgz::util::classify("hello_world").c_str());
  EXPECT_STREQ(std::string("HelloWorld").c_str(), mgz::util::classify("HeLLo_WoRLd").c_str());
  EXPECT_STREQ(std::string("HelloWorld").c_str(), mgz::util::classify("hEllO_wOrld").c_str());
}

TEST(StringUtil, Format) {
  EXPECT_STREQ(std::string("hello 123 12.3").c_str(), mgz::util::format("%s %d %*.*f", "hello", 123, 2, 1, 12.3).c_str());
}

TEST(StringUtil, asIntWithIntStrings) {
  EXPECT_EQ(123,mgz::util::asInt(std::string("123")));
  EXPECT_EQ(-123,mgz::util::asInt(std::string("-123")));
}

TEST(StringUtil, replaceAll) {
  std::string initial("Hello World");
  std::string final("Hexo World");
  std::string from("ll");
  std::string to("x");
  mgz::util::replace_all(initial, from, to);
  EXPECT_STREQ(initial.c_str(), final.c_str());
}

TEST(StringUtil, replaceAllForPath) {
  std::string initial("C:\\toto\\titi\\tata");
  std::string final("C:\\\\toto\\\\titi\\\\tata");
  std::string from("\\");
  std::string to("\\\\");
  mgz::util::replace_all(initial, from, to);
  EXPECT_STREQ(initial.c_str(), final.c_str());
}
//TEST(StringUtil, asIntWithNotIntStrings) { //TODO: protection contre entrées invalides
//  EXPECT_ANY_THROW(mgz::util::asInt(std::string("Hello, World")));
//}


TEST(StringUtil, testRandom) {
  std::string s0 = mgz::util::random_string(10);
  std::string s1 = mgz::util::random_string(10);
  std::string s2 = mgz::util::random_string(10);
  EXPECT_EQ(10U, s0.size());
  EXPECT_EQ(10U, s1.size());
  EXPECT_EQ(10U, s2.size());

  EXPECT_FALSE(s0 == s1);
  EXPECT_FALSE(s1 == s2); 
}

