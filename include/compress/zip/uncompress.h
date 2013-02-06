#ifndef __ZIP_UNCOMPRESS_H
#define __ZIP_UNCOMPRESS_H

#include "zip/zip.h"
#include "util/file.h"
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

class uncompress {
  public:
    uncompress(Glow::Util::file & archive);
    ~uncompress();

    void inflate();
    void inflate(Glow::Util::file & to);

    void inflate_file_at_index(int i);
    void inflate_file_at_index(int i, Glow::Util::file & to);

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
    int cdh_offset_;
    Glow::Util::file archive_;
};

#endif // __ZIP_UNCOMPRESS_H
