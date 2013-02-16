#ifndef __MGZ_COMPRESS_COMPRESSOR_GZIP_H
#define __MGZ_COMPRESS_COMPRESSOR_GZIP_H

#include "compress/compressor.h"

namespace mgz {
  namespace compress {

    class gzip : public compressor {
      void compress();
      void decompress();
    };
  }
}

#endif // __MGZ_COMPRESS_COMPRESSOR_GZIP_H