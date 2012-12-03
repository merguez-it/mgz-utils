#include "util/exception.h"
#include "gtest/gtest.h"

class TestingException {};

TEST(ExceptionMacros, TestTryThrowCatch) {
  bool passedTheCatch=false;
  TRY {
    THROW(TestingException,"A %s cool exception was thrown with %d int value and %1.2f float value in message","super", 123,1.234f);
    // Don't add lines before this point, as we test Exception::getLine() below
  }
  CATCH(TestingException,e) {
    EXPECT_EQ(0U,  e.getMessage().find("A super cool exception was thrown with 123 int value and 1.23 float value in message"));
    EXPECT_EQ(9U, e.getLine());
    EXPECT_NE(std::string::npos,e.getFileName().find("exception_unittest.cc"));
    passedTheCatch=true;
  }
  EXPECT_TRUE(passedTheCatch);
}

