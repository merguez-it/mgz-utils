#include "compress/compressor/raw.h"
#include "compress/z.h"

namespace mgz {
  namespace compress {
    void raw::compress() {
      FDF(RAW, deflate, file_, archive_)
    }
    
    void raw::decompress() {
      FDF(RAW, inflate, archive_, file_)
    }
  }
}