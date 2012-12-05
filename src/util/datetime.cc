#include "util/datetime.h"
#include "util/vector.h"

#ifdef __WIN32__ 
#include "internal/windows/strptime.c"
#endif

#define STRFTIME_BUFFER_SIZE 1024

namespace mgz {
  namespace util {
    datetime::datetime(const datetime & dt) {
      time_t rawtime;
      time(&rawtime);
      localtime_r(&rawtime, &timeinfo_);

      timeinfo_.tm_year = dt.year() - 1900;
      timeinfo_.tm_mon = dt.mon() - 1;
      timeinfo_.tm_mday = dt.day();
      timeinfo_.tm_hour = dt.hour();
      timeinfo_.tm_min = dt.min();
      timeinfo_.tm_sec = dt.sec();
      timeinfo_.tm_isdst = -1;
      format = dt.format;
    }

    datetime::datetime(const std::string &f, const std::string &s) {
      memset(&timeinfo_, 0, sizeof(timeinfo_));
      if(NULL == ::strptime(s.c_str(), f.c_str(), &timeinfo_)) {
        throw 3; // TODO
      }
      format = f;
      timeinfo_.tm_isdst = -1;
    }

    datetime::datetime(const std::string & s) {
      std::vector<std::string> formats = mgz::util::create_vector<std::string>("%F %H:%M:%S")("%F")("%H:%M:%S");
      std::vector<std::string>::iterator it;
      for(it = formats.begin(); it != formats.end(); it++) {
        memset(&timeinfo_, 0, sizeof(timeinfo_));
        if(NULL != ::strptime(s.c_str(), (*it).c_str(), &timeinfo_)) {
          format = *it;
          break;
        }
      }
    }

    datetime::datetime(int year, int month, int day, int hour, int min, int sec) {
      time_t rawtime;
      time(&rawtime);
      localtime_r(&rawtime, &timeinfo_);
      timeinfo_.tm_isdst = -1;

      if(year > 1899) {
        timeinfo_.tm_year = year - 1900;
      }

      if(month > 0 && month < 13) {
        timeinfo_.tm_mon = month - 1;
      }

      if(day > 0 && day < 32) {
        timeinfo_.tm_mday = day;
      }

      if(hour > -1 && hour < 24) {
        timeinfo_.tm_hour = hour;
      }

      if(min > -1 && min < 60) {
        timeinfo_.tm_min = min;
      }

      if(sec > -1 && sec < 60) {
        timeinfo_.tm_sec = sec;
      }

      format = "%F %H:%M:%S";
    }

    datetime datetime::now() {
      return datetime();
    }

    datetime datetime::from_sql(const std::string & s) {
      return datetime("%F %H:%M:%S", s);
    }

    int datetime::year() const {
      return timeinfo_.tm_year + 1900;
    }
    int datetime::mon() const {
      return timeinfo_.tm_mon + 1;
    }
    int datetime::day() const {
      return timeinfo_.tm_mday;
    }
    int datetime::hour() const {
      return timeinfo_.tm_hour;
    }
    int datetime::min() const {
      return timeinfo_.tm_min;
    }
    int datetime::sec() const {
      return timeinfo_.tm_sec;
    }

    bool datetime::leapYear() const {
      if(year() % 4 != 0) return false;
      if(year() % 400 == 0) return true;
      if(year() % 100 == 0) return false;
      return true;
    }

    static int daysInMonths[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int datetime::daysInMonth() const {
      int days = daysInMonths[mon()-1];

      if(mon() == 1 && leapYear()) {
        days += 1;
      }

      return days;
    }

    double datetime::interval(datetime dt) {
      return difftime(dt.to_time(), to_time());
    }

    time_t datetime::to_time() {
      return mktime(&timeinfo_);
    }

    std::string datetime::to_sql() const {
      return strftime(format);
    }

    std::string datetime::to_http() const {
      return strftime("%a, %d %b %Y %T GMT");
    }

    datetime & datetime::next_sec(int i) {
      time_t rt = to_time() + i;
      timeinfo_ = *localtime(&rt);
      return *this;
    }
    datetime & datetime::next_min(int i) {
      return next_sec(60 * i);
    }
    datetime & datetime::next_hour(int i) {
      return next_min(60 * i);
    }
    datetime & datetime::next_day(int i) {
      return next_hour(24 * i);
    }
    datetime & datetime::next_month(int i) {
      bool isLastDayInMonth = day() == daysInMonth();

      int year = timeinfo_.tm_year + i / 12;
      int month = timeinfo_.tm_mon + i % 12;

      if(month > 11) {
        year += 1;
        month -= 12;
      }
      datetime dt(year+1900, month+1, 1);

      int day;
      if(isLastDayInMonth) {
        day = dt.daysInMonth();
      } else {
        day = std::min(timeinfo_.tm_mday, dt.daysInMonth());
      }

      timeinfo_.tm_year = year;
      timeinfo_.tm_mon = month;
      timeinfo_.tm_mday = day;

      return *this;
    }
    datetime & datetime::next_year(int i) {
      timeinfo_.tm_year += i;
      return *this;
    }

    datetime & datetime::prev_sec(int i) {
      time_t rt = to_time() - i;
      timeinfo_ = *localtime(&rt);
      return *this;
    }
    datetime & datetime::prev_min(int i) {
      return prev_sec(60 * i);
    }
    datetime & datetime::prev_hour(int i) {
      return prev_min(60 * i);
    }
    datetime & datetime::prev_day(int i) {
      return prev_hour(24 *i);
    }
    datetime & datetime::prev_month(int i) {
      next_month(-1*i);
      return *this;
    }
    datetime & datetime::prev_year(int i) {
      timeinfo_.tm_year -= i;
      return *this;
    }

    std::string datetime::strftime(std::string format) const {
      char buffer[STRFTIME_BUFFER_SIZE] = {0};
      if(0 == ::strftime(buffer, STRFTIME_BUFFER_SIZE, format.c_str(), &timeinfo_)) {
        throw 1; // TODO
      }

      return std::string(buffer);
    }

    bool datetime::operator==(datetime dt) {
      return interval(dt) == 0;
    }
    bool datetime::operator<(datetime dt) {
      return interval(dt) > 0; 
    }
    bool datetime::operator>(datetime dt) {
      return interval(dt) < 0; 
    }
    bool datetime::operator<=(datetime dt) {
      return interval(dt) >= 0; 
    }
    bool datetime::operator>=(datetime dt) {
      return interval(dt) <= 0; 
    }
    datetime::operator std::string() const {
      return to_sql();
    }

    datetime & datetime::operator=(const std::string & dt) {
      *this = datetime::datetime(dt);
      return *this;
    }

    std::ostream& operator<<(std::ostream& out, datetime dt) {
      return out << dt.strftime(dt.format);
    }
  }
}
