#ifndef __MGZ_COMPRESS_COMPRESSOR_RAW_H
#define __MGZ_COMPRESS_COMPRESSOR_RAW_H

#include "mgz/export.h"
#include "compress/compressor.h"

namespace mgz {
  namespace compress {
    
    class MGZ_API raw : public compressor {
    public :
      raw(std::fstream & file, std::fstream & archive, int level = COMPRESSION_LEVEL_4) : compressor(file,archive,level) {};
      void compress();
      void decompress();
    };
  }
}

#endif // __MGZ_COMPRESS_COMPRESSOR_RAW_H
