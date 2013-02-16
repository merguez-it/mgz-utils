#include <sstream>
#include <iostream>
#include <vector>
#include <iomanip>

namespace mgz {
  namespace debug {
    class hex_dumper {
      public:
        hex_dumper(int line_size = 16, std::string separator = " ") : separator_(separator), line_size_(line_size) {}

        template <class T> std::string dump(std::vector<T> & data) {
          std::stringstream ss;
          std::string sep("");
          typename std::vector<T>::iterator it;
          int i = 0;
          for(it = data.begin(); it != data.end(); it++) {
            ss << sep << std::hex << std::uppercase << std::setfill( '0' ) << std::setw(2) << (int)(*it);
            if(++i < line_size_) {
              sep = separator_;
            } else {
              sep = std::string("\n");
              i = 0;
            }    
          }
          return ss.str();
        }

      private:
        std::string separator_;
        int line_size_;
    };

    template <class T> std::string hexdump(std::vector<T> & data) {
      hex_dumper hd(16, " ");
      return hd.dump<T>(data);
    }
  }
}

//int main() {
//  hex_dumper hd(8, ":");
//  std::string hw("Hello World");
//  std::vector<unsigned char> v(hw.begin(), hw.end());
//  std::cout << hd.dump(v) << std::endl;
//
//  return 0;
//}
