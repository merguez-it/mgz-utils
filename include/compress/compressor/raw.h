#ifndef __MGZ_COMPRESS_COMPRESSOR_RAW_H
#define __MGZ_COMPRESS_COMPRESSOR_RAW_H

#include "compress/compressor.h"

namespace mgz {
  namespace compress {
    
    class raw : public compressor {
      void compress();
      void decompress();
    };
  }
}

#endif // __MGZ_COMPRESS_COMPRESSOR_RAW_H