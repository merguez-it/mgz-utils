#include "io/stream.h"

std::istream& mgz::io::get_line(std::istream& is, std::string& t) {
  std::string myline;
  if(std::getline(is, myline)) {
    if(myline.size() && myline[myline.size()-1] == '\r') {
      t = myline.substr(0, myline.size() - 1);
    } else {
      t = myline;
    }
  }
  return is;
}
