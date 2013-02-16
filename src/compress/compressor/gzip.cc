#include "compress/compressor/gzip.h"
#include "compress/z.h"

namespace mgz {
  namespace compress {
    void gzip::compress() {
      FDF(GZIP, deflate, file_, archive_)
    }
    
    void gzip::decompress() {
      FDF(GZIP, inflate, archive_, file_)
    }
  }
}