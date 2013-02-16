#include "compress/compressor.h"

namespace mgz {
  namespace compress {
    compressor::compressor(std::fstream & file, std::fstream & archive, int level) : file_(file), archive_(archive), level_(level),compressed_size_(0), uncompressed_size_(0) {
      if (level < COMPRESSION_LEVEL_0 || level > COMPRESSION_LEVEL_9) {
        THROW(mgz::compress::UnsupportedCompressionLevelException,"Compression level %u not supported by compressor class",level);
      }
    };

    unsigned long compressor::get_crc32() {
      return crc_.crc;
    }
    unsigned int compressor::get_compressed_size() {
      return compressed_size_;
    }
    unsigned int compressor::get_uncompressed_size() {
      return uncompressed_size_;
    }
  }
}
