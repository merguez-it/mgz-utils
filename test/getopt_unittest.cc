#include <limits.h>
#include <stdio.h>
#include <time.h>
#include "gtest/gtest.h"
#include "config-test.h"
#include "util/getopt.h"


#define ARGC (sizeof(argv)/sizeof(const char*))

TEST(Getopt, TestOne) {
  const char* argv[] = { "DUMMY_EXE_NAME", "-a", "--string", "arg", "-i", "123", "--float", "2.3", "hello", "world" };
  mgz::util::getopt options(ARGC, argv);

  EXPECT_TRUE(2 == options.argv().size());

  EXPECT_TRUE(options.exist('a'));
  EXPECT_STREQ("arg", options.option<std::string>('s', "string", "---").c_str());
  EXPECT_EQ(123, options.option<int>('i', "integer", 0));
  EXPECT_EQ((float)2.3, options.option<float>('f', "float", 1.2));
}

TEST(Getopt, TestTwo) {
  const char* argv[] = { "DUMMY_EXE_NAME", "-a", "--", "hello", "world" };
  mgz::util::getopt options(ARGC, argv);

  EXPECT_TRUE(2 == options.argv().size());
  EXPECT_TRUE(options.exist('a'));
}

TEST(Getopt, TestThree) {
  const char* argv[] = { "DUMMY_EXE_NAME", "-a", "--", "--hello", "--", "-world" };
  mgz::util::getopt options(ARGC, argv);

  EXPECT_TRUE(1 == options.argv().size());
  EXPECT_TRUE(options.exist('a'));
  EXPECT_TRUE(options.exist("hello"));
}

class Date {
  public:
    Date(int y, int m, int d) : year(y) , month(m) , day(d) {};
    static Date now() {
      time_t rawtime;
      struct tm *timeinfo;
      time(&rawtime);
      timeinfo = localtime(&rawtime);
      return Date(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
    }

  public:
    int year;
    int month;
    int day;
};

namespace mgz {
  namespace util {
    template<> inline Date from_string(std::string & data) {
      tm timeinfo;
      sscanf(data.c_str(),"%4d-%2d-%2d",&timeinfo.tm_year,&timeinfo.tm_mon,&timeinfo.tm_mday);
      return Date(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday);
    }
  }
}

TEST(Getopt, CustomConverter) {
  const char* argv[] = { "DUMMY_EXE_NAME", "--date", "2012-09-08" };
  mgz::util::getopt options(ARGC, argv);

  Date d = options.option<Date>('D', "date", Date::now());
  EXPECT_TRUE(2012 == d.year);
  EXPECT_TRUE(9 == d.month);
  EXPECT_TRUE(8 == d.day);
}

TEST(Getopt, EmptyValue) {
  const char* argv[] = { "DUMMY_EXE_NAME", "--empty" };
  mgz::util::getopt options(ARGC, argv);
  EXPECT_STREQ("default", options.option<std::string>('e', "empty", "default").c_str());
  EXPECT_TRUE(0 == options.argv().size());
  EXPECT_TRUE(options.argv().empty());
}

TEST(Getopt, SpaceInArgs) {
  const char* argv[] = { "DUMMY_EXE_NAME", "arg with space", "-o", "option with space", "another arg with space" };
  mgz::util::getopt options(ARGC, argv);
  EXPECT_EQ(2U, options.argv().size());
  std::vector<std::string> a = options.argv();
  EXPECT_EQ("arg with space", a[0]);
  EXPECT_EQ("another arg with space", a[1]);
  EXPECT_TRUE(options.exist('o'));
  EXPECT_EQ("option with space", options.option<std::string>('o', "option", ""));
}
