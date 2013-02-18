#include "util/getopt.h"
#include <sstream>
#include <iostream>
#include <string.h>

namespace mgz {
  namespace util {
    getopt::getopt(int argc, const char *argv[]) : exe_(argv[0]) {
      parse(argc, argv);
    }

    std::vector<std::string> getopt::argv() {
      return argv_;
    }

    bool getopt::exist(char shortop) {
      return (shortopts_.find(shortop) != shortopts_.end());
    }

    bool getopt::exist(std::string longop) {
      return (longopts_.find(longop) != longopts_.end());
    }

    std::string getopt::application() {
      return exe_;
    }

    void getopt::save_data(enum arg_type previous, std::string previous_data) {
      switch(previous) {
        case LONG_OPTION:
          longopts_[previous_data.substr(2)] = "";
          break;

        case SHORT_OPTION:
          shortopts_[previous_data[1]] = "";
          break;

        case DATA:
        default:
          break;
      }
    }

    void getopt::save_data(enum arg_type previous, std::string previous_data, std::string current_data) {
      switch(previous) {
        case LONG_OPTION:
          longopts_[previous_data.substr(2)] = current_data;
          break;

        case SHORT_OPTION:
          shortopts_[previous_data[1]] = current_data;
          break;

        case DATA:
        default:
          argv_.push_back(current_data);
          break;
      }
    }

    void getopt::parse(int argc, const char *argv[]) {
      enum arg_type current = DATA;
      enum arg_type previous = DATA;
      std::string current_data;
      std::string previous_data;

      for(int i = 1; i < argc; i++) {
        current_data = argv[i];

        char first = argv[i][0];
        char second = argv[i][1];

        if(first == '-') {
          if(second == 0) { // "-"
            current = DATA;
          } else {
            if(second == '-') {
              if(argv[i][2] == 0) { // "--"
                save_data(previous, previous_data);
                previous = DATA;
                continue;
              } else { // "--abc"
                current = LONG_OPTION;
              }
            } else {
              if(2 == strlen(argv[i])) { // "-a"
                current = SHORT_OPTION;
              } else { // "-abc"
                current = DATA;
              }
            }
          }
        } else {
          current = DATA;
        }

        switch(current) {
          case LONG_OPTION:
            save_data(previous, previous_data);
            break;

          case SHORT_OPTION:
            save_data(previous, previous_data);
            break;

          case DATA:
          default:
            save_data(previous, previous_data, current_data);
            break;
        }

        previous = current;
        previous_data = argv[i];
      }

      save_data(previous, previous_data);
    }
  }
}
