#ifndef __MGZ_COMPRESS_Z_H
#define __MGZ_COMPRESS_Z_H

#include <string>
#include <vector>
#include <fstream>

#include "compress/mgz_stream.h"
#define BUFFER_SIZE 1<<15

namespace mgz {
  namespace compress {

    enum CompressionType {
      RAW,
      GZIP,
      ZLIB,
      PKZIP
    };

    class Z {
      public:
        Z(CompressionType type);

        // compress
        int deflate_init(int level = 9);
        int deflate();
        int deflate_end();

        void deflate(const std::vector<unsigned char> & in, std::vector<unsigned char> & out);
        void deflate(FILE *in, FILE *out);
        void deflate(std::fstream & in, std::fstream & out);

        // uncompress
        int inflate_init();
        int inflate();
        int inflate_end();

        void inflate(const std::vector<unsigned char> & in, std::vector<unsigned char> & out);
        void inflate(FILE *in, FILE *out);
        void inflate(std::fstream & in, std::fstream & out);

      public:
        mgz_stream stream;

      private:
        unsigned int adler32(unsigned char *p, int n, unsigned int adler);
        void crc32init(void);
        unsigned int crc32(unsigned char *p, int n, unsigned int crc);

        void set32(unsigned char *p, unsigned int n);
        void set32le(unsigned char *p, unsigned int n);
        int check32(unsigned char *p, unsigned int n);
        int check32le(unsigned char *p, unsigned int n);

        int deflate_zlib_header(unsigned char *p, int n);
        int deflate_zlib_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen);
        int inflate_zlib_header(unsigned char *p, int n);
        int inflate_zlib_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen);

        int deflate_gzip_header(unsigned char *p, int n);
        int deflate_gzip_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen);
        int inflate_gzip_header(unsigned char *p, int n);
        int inflate_gzip_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen);

        int deflate_pkzip_header(unsigned char *p, int n);
        int deflate_pkzip_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen);
        int inflate_pkzip_header(unsigned char *p, int n);
        int inflate_pkzip_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen);

        int dummyheader(unsigned char *p, int n);
        int dummyfooter(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen);
        unsigned int dummysum(unsigned char *p, int n, unsigned int sum);

      private:
        CompressionType type_;

        unsigned int crc_table_[256];
        int last_flat_rcod_;

        unsigned int checksum_;
        unsigned int nin_;
        unsigned int nout_;
        unsigned int header_size_;
        unsigned int footer_size_;
        unsigned int extra_size_;

        bool deflate_init_done_;
        bool inflate_init_done_;
    };
  }
}

#endif // __MGZ_COMPRESS_Z_H
