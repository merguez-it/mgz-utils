#include <iostream>
#include "gtest/gtest.h"

#include "util/datetime.h"

TEST(DateTime, tests) {
  mgz::util::datetime dt = mgz::util::datetime::now();
  ASSERT_TRUE(dt.year() > 1900);

  mgz::util::datetime dtc(dt);
  ASSERT_TRUE(dtc.year() > 1900);

  ASSERT_TRUE(dt == dtc);
}

TEST(DateTime, testSql) {
  mgz::util::datetime dt = mgz::util::datetime::from_sql("1999-12-31 23:59:59");
  mgz::util::datetime dtc = mgz::util::datetime("%F %H:%M:%S", "1999-12-31 23:59:59");

  ASSERT_TRUE(dt == dtc);
  ASSERT_EQ(dt.year(), 1999);
  ASSERT_EQ(dt.mon(), 12);
  ASSERT_EQ(dt.day(), 31);
  ASSERT_EQ(dt.hour(), 23);
  ASSERT_EQ(dt.min(), 59);
  ASSERT_EQ(dt.sec(), 59);
}

TEST(DateTime, testEqual) {
  mgz::util::datetime dt;

  dt = "1999-12-31 23:59:59";
  ASSERT_EQ(dt.year(), 1999);
  ASSERT_EQ(dt.mon(), 12);
  ASSERT_EQ(dt.day(), 31);
  ASSERT_EQ(dt.hour(), 23);
  ASSERT_EQ(dt.min(), 59);
  ASSERT_EQ(dt.sec(), 59);

  dt = "2001-01-31";
  ASSERT_EQ(dt.year(), 2001);
  ASSERT_EQ(dt.mon(), 1);
  ASSERT_EQ(dt.day(), 31);
  ASSERT_EQ(dt.hour(), 0);
  ASSERT_EQ(dt.min(), 0);
  ASSERT_EQ(dt.sec(), 0);

  dt = "12:34:56";
  ASSERT_EQ(dt.year(), 1900);
  ASSERT_EQ(dt.mon(), 1);
  ASSERT_EQ(dt.day(), 0);
  ASSERT_EQ(dt.hour(), 12);
  ASSERT_EQ(dt.min(), 34);
  ASSERT_EQ(dt.sec(), 56);

  std::string toto = dt;
  ASSERT_EQ("12:34:56", toto);
}

TEST(DateTime, testNextSec) {
  mgz::util::datetime dt = mgz::util::datetime::from_sql("1999-12-31 23:59:59");
  dt.next_sec(2);

  ASSERT_EQ(dt.year(), 2000);
  ASSERT_EQ(dt.mon(), 1);
  ASSERT_EQ(dt.day(), 1);
  ASSERT_EQ(dt.hour(), 0);
  ASSERT_EQ(dt.min(), 0);
  ASSERT_EQ(dt.sec(), 1);
}

TEST(DateTime, testNextMin) {
  mgz::util::datetime dt = mgz::util::datetime::from_sql("1999-12-31 23:59:59");
  dt.next_min(2);

  ASSERT_EQ(dt.year(), 2000);
  ASSERT_EQ(dt.mon(), 1);
  ASSERT_EQ(dt.day(), 1);
  ASSERT_EQ(dt.hour(), 0);
  ASSERT_EQ(dt.min(), 1);
  ASSERT_EQ(dt.sec(), 59);
}

TEST(DateTime, testNextHour) {
  mgz::util::datetime dt = mgz::util::datetime::from_sql("1999-12-31 23:59:59");
  dt.next_hour(2);

  ASSERT_EQ(dt.year(), 2000);
  ASSERT_EQ(dt.mon(), 1);
  ASSERT_EQ(dt.day(), 1);
  ASSERT_EQ(dt.hour(), 1);
  ASSERT_EQ(dt.min(), 59);
  ASSERT_EQ(dt.sec(), 59);
}

TEST(DateTime, testNextDay) {
  mgz::util::datetime dt = mgz::util::datetime::from_sql("1999-12-31 23:59:59");
  dt.next_day(2);

  ASSERT_EQ(dt.year(), 2000);
  ASSERT_EQ(dt.mon(), 1);
  ASSERT_EQ(dt.day(), 2);
  ASSERT_EQ(dt.hour(), 23);
  ASSERT_EQ(dt.min(), 59);
  ASSERT_EQ(dt.sec(), 59);
}

TEST(DateTime, testNextMon) {
  mgz::util::datetime dt = mgz::util::datetime::from_sql("1999-12-31 23:59:59");
  dt.next_month(2);

  ASSERT_EQ(dt.year(), 2000);
  ASSERT_EQ(dt.mon(), 2);
  ASSERT_EQ(dt.day(), 28);
  ASSERT_EQ(dt.hour(), 23);
  ASSERT_EQ(dt.min(), 59);
  ASSERT_EQ(dt.sec(), 59);
}

TEST(DateTime, testNextYear) {
  mgz::util::datetime dt = mgz::util::datetime::from_sql("1999-12-31 23:59:59");
  dt.next_year(2);

  ASSERT_EQ(dt.year(), 2001);
  ASSERT_EQ(dt.mon(), 12);
  ASSERT_EQ(dt.day(), 31);
  ASSERT_EQ(dt.hour(), 23);
  ASSERT_EQ(dt.min(), 59);
  ASSERT_EQ(dt.sec(), 59);
}
