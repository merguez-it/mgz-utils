#ifndef __MGZ_UTIL_GETOPT_INCLUDE
#define __MGZ_UTIL_GETOPT_INCLUDE

#include <vector>
#include <iostream>
#include <map>
#include <string>
#include "util/types.h"

namespace mgz {
  namespace util {
    enum arg_type {
      LONG_OPTION,
      SHORT_OPTION,
      DATA
    };

    class getopt {
      public:
        getopt(int argc, const char *argv[]);

        std::vector<std::string> argv();

        template <class T> T option(char shortop, std::string longop, T defaultvalue) {
          if(exist(shortop) && !shortopts_[shortop].empty()) {
            return from_string<T>(shortopts_[shortop]);
          } else if(exist(longop) && !longopts_[longop].empty()) {
            return from_string<T>(longopts_[longop]);
          }
          return defaultvalue;
        };
        bool exist(char shortop);
        bool exist(std::string longop);
        std::string application();

      private:
        void save_data(enum arg_type previous, std::string previous_data);
        void save_data(enum arg_type previous, std::string previous_data, std::string current_data);
        void parse(int argc, const char *argv[]);

      private:
        std::map<std::string, std::string> longopts_;
        std::map<char, std::string> shortopts_;
        std::vector<std::string> argv_;
        std::string exe_;
    };
  }
}

#endif // __MGZ_UTIL_GETOPT_INCLUDE

