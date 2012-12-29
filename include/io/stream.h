#ifndef __MGZ_IO_STREAM_H
#define __MGZ_IO_STREAM_H

#include <string>
#include <fstream>

namespace mgz {
  namespace io {
    std::istream & get_line(std::istream & is, std::string & t);
  }
}
#endif // __MGZ_IO_STREAM_H

