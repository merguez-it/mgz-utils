#ifndef __MGZ_COMPRESS_COMPRESSOR_ZLIB_H
#define __MGZ_COMPRESS_COMPRESSOR_ZLIB_H

#include "compress/compressor.h"

namespace mgz {
  namespace compress {
    
    class zlib : public compressor {
      void compress();
      void decompress();
    };
  }
}

#endif // __MGZ_COMPRESS_COMPRESSOR_ZLIB_H