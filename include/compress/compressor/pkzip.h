#ifndef __MGZ_COMPRESS_COMPRESSOR_PKZIP_H
#define __MGZ_COMPRESS_COMPRESSOR_PKZIP_H

#include "compress/compressor.h"

namespace mgz {
  namespace compress {
    
    class pkzip : public compressor {
      void compress();
      void decompress();
    };
  }
}

#endif // __MGZ_COMPRESS_COMPRESSOR_PKZIP_H