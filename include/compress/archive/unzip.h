#ifndef __MGZ_COMPRESS_ARCHIVE_UNZIP_H
#define __MGZ_COMPRESS_ARCHIVE_UNZIP_H

#include "compress/archive/lib_zip.h"
#include "io/file.h"
#include "util/exception.h"

#include <ios>
#include <fstream>
#include <vector>

class MalformatedEndOfCentralDirectoryHeader {};
class MalformatedCentralDirectoryHeader {};

class MalformatedLocalFileHeader {};

class UncompressError {};
class UnsupportedCompressionMethod {};

struct entry {
  unsigned int crc32;
  unsigned int compressed_size;
  unsigned int uncompressed_size;
  unsigned short time;
  unsigned short date;
  std::string file_name;
  std::string file_comment;
  unsigned short compression_method;
  unsigned int file_offset;
};

namespace mgz {
  namespace compress {
    namespace archive {
      class unzip {
        public:
          unzip(mgz::io::file & archive);
          ~unzip();

          void inflate();
          void inflate(mgz::io::file & to);

          void inflate_file_at_index(int i);
          void inflate_file_at_index(int i, mgz::io::file & to);

          int number_of_entries();
          entry file_stat_at_index(int i);

        private:
          void read_eocdh();
          void read_cdh();
          local_file_header read_lfh_at_index(int i);

        private:
          std::vector<central_directory_header> cdh_;
          end_of_central_directory_header eocdh_;
          std::fstream is_;
          int archive_size_;
          mgz::io::file archive_;
      };
    }
  }
}

#endif // __MGZ_COMPRESS_ARCHIVE_UNZIP_H
