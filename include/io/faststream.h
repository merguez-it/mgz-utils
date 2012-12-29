#ifndef __MGZ_IO_FASTSTREAM_H
#define __MGZ_IO_FASTSTREAM_H

#include <string>
#include <vector>
#include "io/file.h"

class CantGetPagesize{};
class InvalidPageSize{};
class CantReadFile{};
class CantWriteFile{};
class ErrorWhileReadingFile{};
class ErrorWhileWrittingingFile{};
class CantGetFileSize{};

namespace mgz {
  namespace io {
    enum mode {
      in,
      out
    };
    class faststream {
      public:
        faststream(const std::string & path, mode m);
        faststream(file f, mode m);
        ~faststream();
        bool open(long i = 1);
        bool close();
        std::vector<unsigned char> read();
        long write(const std::vector<unsigned char> & buffer);
        long gcount() const;

      private:
        void pages(long i);
        void get_file_size();
        void flush(bool force = false);

      private:
        std::string path_;
        mode mode_;
        bool open_;
        long pagesize_;
        long pages_;
        int file_descriptor_;
        long offset_;
        long file_size_;
        long read_size_;
        std::vector<unsigned char> buffer_;
    };
  }
}

#endif // __MGZ_IO_FASTSTREAM_H

