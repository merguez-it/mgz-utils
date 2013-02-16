#include "compress/compressor/zlib.h"
#include "compress/z.h"

namespace mgz {
  namespace compress {
    void zlib::compress() {
      FDF(ZLIB, deflate, file_, archive_)
    }
    
    void zlib::decompress() {
      FDF(ZLIB, inflate, archive_, file_)
    }
  }
}