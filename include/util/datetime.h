#ifndef __MGZ_UTIL_DATETIME_H
#define __MGZ_UTIL_DATETIME_H

#include "mgz/export.h"
#include <ctime>
#include <string>
#include <iostream>

namespace mgz {
  namespace util {
    class MGZ_API datetime {
      public:
        datetime(const datetime & dt);
        datetime(const std::string & f, const std::string & s);
        datetime(const std::string & s);
        datetime(int year = -1, int month = -1, int day = -1, int hour = -1, int min = -1, int sec = -1);
        static datetime now();
        static datetime from_sql(const std::string & s);

        int year() const;
        int mon() const;
        int day() const;
        int hour() const;
        int min() const;
        int sec() const;

        bool leapYear() const;
        int daysInMonth() const;

        double interval(datetime dt);
        time_t to_time();
        std::string to_sql() const;
        std::string to_http() const;

        datetime & next_sec(int i = 1);
        datetime & next_min(int i = 1);
        datetime & next_hour(int i = 1);
        datetime & next_day(int i = 1);
        datetime & next_month(int i = 1);
        datetime & next_year(int i = 1);

        datetime & prev_sec(int i = 1);
        datetime & prev_min(int i = 1);
        datetime & prev_hour(int i = 1);
        datetime & prev_day(int i = 1);
        datetime & prev_month(int i = 1);
        datetime & prev_year(int i = 1);

        std::string strftime(std::string format) const;

        bool operator==(datetime dt);
        bool operator<(datetime dt);
        bool operator>(datetime dt);
        bool operator<=(datetime dt);
        bool operator>=(datetime dt);
        operator std::string() const;

        datetime & operator=(const std::string & dt);

        friend std::ostream& operator<<(std::ostream& out, datetime dt);

        std::string format;
      private:
        struct tm timeinfo_;
    };
  }
}

#endif // __MGZ_UTIL_DATETIME_H
