#ifndef __MGZ_COMPRESS_Z_H
#define __MGZ_COMPRESS_Z_H

#include <string>
#include <vector>

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

        std::vector<unsigned char> compress(std::vector<unsigned char> data);
        std::vector<unsigned char> uncompress(std::vector<unsigned char> data);

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

        int compress_stream(FILE *in, FILE *out);
        std::vector<unsigned char> compress_vector(std::vector<unsigned char> data);

        int decompress_stream(FILE *in, FILE *out);
        std::vector<unsigned char> decompress_vector(std::vector<unsigned char> data);

      private:
        CompressionType type_;
        unsigned int crc_table_[256];

        char *last_error_;
        unsigned int checksum_;
        unsigned int nin_;
        unsigned int nout_;
        unsigned int header_size_;
        unsigned int footer_size_;
        unsigned int extra_size_;
    };
  }
}

#endif // __MGZ_COMPRESS_Z_H
