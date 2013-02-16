#ifndef __MGZ_COMPRESS_COMPRESSOR_H
#define __MGZ_COMPRESS_COMPRESSOR_H

#include <iostream>
#include <fstream>
#include "util/exception.h"
#include "security/crc32.h"

namespace mgz {
  namespace compress {
    class CompressorInitException {};
    class CompressorReadException {};
    class CompressorWriteException {};
    class CompressorDeflateError {};
    class CompressorInflateError {};
    class UnsupportedCompressionLevelException {};

    enum {
      COMPRESSION_LEVEL_0 = 0,
      COMPRESSION_LEVEL_1 = 1,
      COMPRESSION_LEVEL_2 = 2,
      COMPRESSION_LEVEL_3 = 3,
      COMPRESSION_LEVEL_4 = 4,
      COMPRESSION_LEVEL_5 = 5,
      COMPRESSION_LEVEL_6 = 6,
      COMPRESSION_LEVEL_7 = 7,
      COMPRESSION_LEVEL_8 = 8,
      COMPRESSION_LEVEL_9 = 9,
    };

    class compressor {
      public:
        compressor(std::fstream & file, std::fstream & archive,int level=COMPRESSION_LEVEL_4);
        virtual ~compressor() {};

        virtual void compress() = 0;
        virtual void decompress(int size = -1) = 0;

        unsigned long get_crc32();
        unsigned int get_compressed_size();
        unsigned int get_uncompressed_size();

      protected:
        std::fstream & file_;
        std::fstream & archive_;
        int level_;
        mgz::security::crc32sum crc_;
        unsigned int compressed_size_;
        unsigned int uncompressed_size_;
    };
  }
}

#endif // __MGZ_COMPRESS_COMPRESSOR_H
