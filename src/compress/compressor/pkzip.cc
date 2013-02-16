#include "compress/compressor/pkzip.h"
#include "compress/z.h"

namespace mgz {
  namespace compress {
    void pkzip::compress() {
      FDF(PKZIP, deflate, file_, archive_)
    }
    
    void pkzip::decompress() {
      FDF(PKZIP, inflate, archive_, file_)
    }
  }
}