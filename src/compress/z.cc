#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compress/z.h"
#include "compress/internal/flate.h"
#ifdef __WIN32__
#define strdup _strdup
#endif

namespace mgz {
  namespace compress {
    // Adler32 part ---------------------------------------------------------------

    enum {
      AdlerBase = 65521, /* largest 16bit prime */
      AdlerN    = 5552   /* max iters before 32bit overflow */
    };

    unsigned int Z::adler32(unsigned char *p, int n, unsigned int adler) {
      unsigned int s1 = adler & 0xffff;
      unsigned int s2 = (adler >> 16) & 0xffff;
      unsigned char *ep;
      int k;

      for (; n >= 16; n -= k) {
        k = n < AdlerN ? n : AdlerN;
        k &= ~0xf;
        for (ep = p + k; p < ep; p += 16) {
          s1 += p[0];
          s2 += s1;
          s1 += p[1];
          s2 += s1;
          s1 += p[2];
          s2 += s1;
          s1 += p[3];
          s2 += s1;
          s1 += p[4];
          s2 += s1;
          s1 += p[5];
          s2 += s1;
          s1 += p[6];
          s2 += s1;
          s1 += p[7];
          s2 += s1;
          s1 += p[8];
          s2 += s1;
          s1 += p[9];
          s2 += s1;
          s1 += p[10];
          s2 += s1;
          s1 += p[11];
          s2 += s1;
          s1 += p[12];
          s2 += s1;
          s1 += p[13];
          s2 += s1;
          s1 += p[14];
          s2 += s1;
          s1 += p[15];
          s2 += s1;
        }
        s1 %= AdlerBase;
        s2 %= AdlerBase;
      }
      if (n) {
        for (ep = p + n; p < ep; p++) {
          s1 += p[0];
          s2 += s1;
        }
        s1 %= AdlerBase;
        s2 %= AdlerBase;
      }
      return (s2 << 16) + s1;
    }

    // CRC part -------------------------------------------------------------------

    void Z::crc32init(void) {
      static const unsigned int poly = 0xedb88320;
      int i,j;

      for (i = 0; i < 256; ++i) {
        unsigned int crc = i;

        for (j = 0; j < 8; j++) {
          if (crc & 1)
            crc = (crc >> 1) ^ poly;
          else
            crc >>= 1;
        }
        crc_table_[i] = crc;
      }
    }

    unsigned int Z::crc32(unsigned char *p, int n, unsigned int crc) {
      unsigned char *ep = p + n;

      crc ^= 0xffffffff;
      while (p < ep)
        crc = crc_table_[(crc & 0xff) ^ *p++] ^ (crc >> 8);
      return crc ^ 0xffffffff;
    }

    // Util part ------------------------------------------------------------------

    void Z::set32(unsigned char *p, unsigned int n) {
      p[0] = n >> 24;
      p[1] = n >> 16;
      p[2] = n >> 8;
      p[3] = n;
    }

    void Z::set32le(unsigned char *p, unsigned int n) {
      p[0] = n;
      p[1] = n >> 8;
      p[2] = n >> 16;
      p[3] = n >> 24;
    }

    int Z::check32(unsigned char *p, unsigned int n) {
      return n == ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
    }

    int Z::check32le(unsigned char *p, unsigned int n) {
      return n == (p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
    }

    // Zlib part ------------------------------------------------------------------

    enum {
      ZLIB_CM    = 7 << 4,
      ZLIB_CINFO = 8,
      ZLIB_FLEV  = 3 << 6,
      ZLIB_FDICT = 1 << 5,
      ZLIB_FCHK  = 31 - (((ZLIB_CM | ZLIB_CINFO) << 8) | ZLIB_FLEV) % 31
    };

    int Z::deflate_zlib_header(unsigned char *p, int n) {
      if (n < 2)
        return FLATE_ERR;
      p[0] = ZLIB_CM | ZLIB_CINFO;  /* deflate method, 32K window size */
      p[1] = ZLIB_FLEV | ZLIB_FCHK; /* highest compression */
      return 2;
    }

    int Z::deflate_zlib_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen) {
      if (n < 4)
        return FLATE_ERR;
      set32(p, sum);
      return 4;
    }

    int Z::inflate_zlib_header(unsigned char *p, int n) {
      if (n < 2)
        return FLATE_ERR;
      if (((p[0] << 8) | p[1]) % 31)
        return FLATE_ERR;
      if ((p[0] & 0xf0) != ZLIB_CM || (p[0] & 0x0f) > ZLIB_CINFO)
        return FLATE_ERR;
      if (p[1] & ZLIB_FDICT)
        return FLATE_ERR;
      return 2;
    }

    int Z::inflate_zlib_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen) {
      if (n < 4 || !check32(p, sum))
        return FLATE_ERR;
      return 4;
    }

    // Gzip part ------------------------------------------------------------------

    enum {
      GZIP_ID1    = 0x1f,
      GZIP_ID2    = 0x8b,
      GZIP_CM     = 8,
      GZIP_FHCRC  = 1 << 1,
      GZIP_FEXTRA = 1 << 2,
      GZIP_FNAME  = 1 << 3,
      GZIP_FCOMM  = 1 << 4,
      GZIP_XFL    = 2,
      GZIP_OS     = 255
    };

    int Z::deflate_gzip_header(unsigned char *p, int n) {
      if (n < 10)
        return FLATE_ERR;
      memset(p, 0, 10);
      p[0] = GZIP_ID1;
      p[1] = GZIP_ID2;
      p[2] = GZIP_CM;
      p[8] = GZIP_XFL;
      p[9] = GZIP_OS;
      return 10;
    }

    int Z::deflate_gzip_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen) {
      if (n < 8)
        return FLATE_ERR;
      set32le(p, sum);
      set32le(p+4, len);
      return 8;
    }

    int Z::inflate_gzip_header(unsigned char *p, int n) {
      int k = 10;

      if (k > n)
        return FLATE_ERR;
      if (p[0] != GZIP_ID1 || p[1] != GZIP_ID2 || p[2] != GZIP_CM)
        return FLATE_ERR;
      if (p[3] & GZIP_FEXTRA) {
        k += 2 + ((p[k] << 8) | p[k+1]);
        if (k > n)
          return FLATE_ERR;
      }
      if (p[3] & GZIP_FNAME) {
        for (; k < n; k++)
          if (p[k] == 0)
            break;
        k++;
        if (k > n)
          return FLATE_ERR;
      }
      if (p[3] & GZIP_FCOMM) {
        for (; k < n; k++)
          if (p[k] == 0)
            break;
        k++;
        if (k > n)
          return FLATE_ERR;
      }
      if (p[3] & GZIP_FHCRC) {
        k += 2;
        if (k > n)
          return FLATE_ERR;
      }
      return k;
    }

    int Z::inflate_gzip_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen) {
      if (n < 8 || !check32le(p, sum) || !check32le(p+4, len))
        return FLATE_ERR;
      return 8;
    }

    // PKZip part -----------------------------------------------------------------

    char pkname[] = "sflate_stream";

    enum {
      PKHeadID   = 0x04034b50,
      PKDataID   = 0x08074b50,
      PKDirID    = 0x02014b50,
      PKFootID   = 0x06054b50,
      PKVersion  = 20,
      PKFlag     = 1 << 3,
      PKMethod   = 8,
      PKDate     = ((2009 - 1980) << 25) | (1 << 21) | (1 << 16),
      PKHeadSize = 30,
      PKDirSize  = 46,
      PKNameLen  = sizeof(pkname) - 1
    };

    int Z::deflate_pkzip_header(unsigned char *p, int n) {
      if (n < PKHeadSize + PKNameLen)
        return FLATE_ERR;
      memset(p, 0, PKHeadSize);
      set32le(p, PKHeadID);
      set32le(p+4, PKVersion);
      set32le(p+6, PKFlag);
      set32le(p+8, PKMethod);
      set32le(p+10, PKDate);
      set32le(p+26, PKNameLen);
      memcpy(p + PKHeadSize, pkname, PKNameLen);
      return PKHeadSize + PKNameLen;
    }

    int Z::deflate_pkzip_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen) {
      if (n < PKDirSize + PKNameLen + 22)
        return FLATE_ERR;
      /* unzip bug */
      /*
         if (n < 16 + PKDirSize + PKNameLen + 22)
         return FLATE_ERR;
         set32le(p, PKDataID);
         set32le(p+4, sum);
         set32le(p+8, zlen);
         set32le(p+12, len);
         p += 16;
         */
      memset(p, 0, PKDirSize);
      set32le(p, PKDirID);
      set32le(p+4, PKVersion | (PKVersion << 16));
      set32le(p+8, PKFlag);
      set32le(p+10, PKMethod);
      set32le(p+12, PKDate);
      set32le(p+16, sum);
      set32le(p+20, zlen);
      set32le(p+24, len);
      set32le(p+28, PKNameLen);
      memcpy(p + PKDirSize, pkname, PKNameLen);
      p += PKDirSize + PKNameLen;
      memset(p, 0, 22);
      set32le(p, PKFootID);
      p[8] = p[10] = 1;
      set32le(p+12, PKDirSize + PKNameLen);
      set32le(p+16, zlen + PKHeadSize + PKNameLen);
      return PKDirSize + PKNameLen + 22;
      /*
         set32le(p+12, 16 + PKDirSize + PKNameLen);
         set32le(p+16, zlen + PKHeadSize + PKNameLen);
         return 16 + PKDirSize + PKNameLen + 22;
         */
    }

    int Z::inflate_pkzip_header(unsigned char *p, int n) {
      int k = 30;

      if (k > n)
        return FLATE_ERR;
      if (!check32le(p, PKHeadID))
        return FLATE_ERR;
      if ((p[4] | (p[5] << 8)) > PKVersion)
        return FLATE_ERR;
      if ((p[8] | (p[9] << 8)) != PKMethod)
        return FLATE_ERR;
      k += p[26] | (p[27] << 8);
      k += p[28] | (p[29] << 8);
      if (k > n)
        return FLATE_ERR;
      return k;
    }

    int Z::inflate_pkzip_footer(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen) {
      int k = PKDirSize + 22;

      if (k > n)
        return FLATE_ERR;
      if (check32le(p, PKDataID)) {
        p += 16;
        k += 16;
        if (k > n)
          return FLATE_ERR;
      }
      if (!check32le(p, PKDirID))
        return FLATE_ERR;
      if (!check32le(p+16, sum))
        return FLATE_ERR;
      if (!check32le(p+20, zlen))
        return FLATE_ERR;
      if (!check32le(p+24, len))
        return FLATE_ERR;
      return k;
    }

    // Dummy part -----------------------------------------------------------------

    int Z::dummyheader(unsigned char *p, int n) {
      return 0;
    }
    int Z::dummyfooter(unsigned char *p, int n, unsigned int sum, unsigned int len, unsigned int zlen) {
      return 0;
    }
    unsigned int Z::dummysum(unsigned char *p, int n, unsigned int sum) {
      return 0;
    }

    // Common part ----------------------------------------------------------------

#define BUFFER_SIZE 1<<15

    std::vector<unsigned char> Z::compress_vector(std::vector<unsigned char> data) {
      std::vector<unsigned char> out = std::vector<unsigned char>();
      // TODO
      return out;
    }

    /* compress, using flat_stream interface */
    int Z::compress_stream(FILE *in, FILE *out) {
      flat_stream s;
      int k, n;

      s.next_in = (unsigned char*)malloc(BUFFER_SIZE);
      s.next_out = (unsigned char*)malloc(BUFFER_SIZE);
      s.avail_in = 0;
      s.avail_out = BUFFER_SIZE;
      s.err = 0;
      s.state = 0;

      switch(type_) {
        case GZIP:
          k = deflate_gzip_header(s.next_out, s.avail_out);
          break;
        case ZLIB:
          k = deflate_zlib_header(s.next_out, s.avail_out);
          break;
        case PKZIP:
          k = deflate_pkzip_header(s.next_out, s.avail_out);
          break;
        default:
          k = dummyheader(s.next_out, s.avail_out);
      }
      if (k == FLATE_ERR) {
        s.err = strdup("header error.");
        n = FLATE_ERR;
      } else {
        header_size_ = s.avail_out = k;
        n = FLATE_OUT;
      }
      for (;; n = deflate(&s)) {
        switch (n) {
          case FLATE_OK:
            switch(type_) {
              case GZIP:
                k = deflate_gzip_footer(s.next_out, s.avail_out, checksum_, nin_, nout_ - header_size_);
                break;
              case ZLIB:
                k = deflate_zlib_footer(s.next_out, s.avail_out, checksum_, nin_, nout_ - header_size_);
                break;
              case PKZIP:
                k = deflate_pkzip_footer(s.next_out, s.avail_out, checksum_, nin_, nout_ - header_size_);
                break;
              default:
                k = dummyfooter(s.next_out, s.avail_out, checksum_, nin_, nout_ - header_size_);
            }
            if (k == FLATE_ERR) {
              s.err = strdup("footer error.");
              n = FLATE_ERR;
            } else if (k != fwrite(s.next_out, 1, k, out)) {
              s.err = strdup("write error.");
              n = FLATE_ERR;
            } else {
              footer_size_ = k;
              nout_ += k;
            }
          case FLATE_ERR:
            free(s.next_in);
            free(s.next_out);
            last_error_ = s.err;
            return n;
          case FLATE_IN:
            s.avail_in = fread(s.next_in, 1, BUFFER_SIZE, in);
            nin_ += s.avail_in;
            switch(type_) {
              case GZIP:
                checksum_ = crc32(s.next_in, s.avail_in, checksum_);
                break;
              case ZLIB:
                checksum_ = adler32(s.next_in, s.avail_in, checksum_);
                break;
              case PKZIP:
                checksum_ = crc32(s.next_in, s.avail_in, checksum_);
                break;
              default:
                checksum_ = dummysum(s.next_in, s.avail_in, checksum_);
            }
            break;
          case FLATE_OUT:
            k = fwrite(s.next_out, 1, s.avail_out, out);
            if (k != s.avail_out)
              s.err = strdup("write error.");
            nout_ += k;
            s.avail_out = BUFFER_SIZE;
            break;
        }
      }
    }

    std::vector<unsigned char> Z::decompress_vector(std::vector<unsigned char> data) {
      std::vector<unsigned char> out = std::vector<unsigned char>();
      flat_stream s;
      unsigned char *begin;
      int k, n;

      s.next_in = begin = reinterpret_cast<unsigned char*>(&data[0]);
      s.avail_in = data.size();
      s.next_out = (unsigned char*)malloc(BUFFER_SIZE);
      s.avail_out = BUFFER_SIZE;
      s.err = 0;
      s.state = 0;
      nin_ += s.avail_in;

      switch(type_) {
        case GZIP:
          k = inflate_gzip_header(s.next_in, s.avail_in);
          break;
        case ZLIB:
          k = inflate_zlib_header(s.next_in, s.avail_in);
          break;
        case PKZIP:
          k = inflate_pkzip_header(s.next_in, s.avail_in);
          break;
        default:
          k = dummyheader(s.next_in, s.avail_in);
      }
      if (k == FLATE_ERR) {
        s.err = strdup("header error.");
        n = FLATE_ERR;
      } else {
        header_size_ = k;
        s.avail_in -= k;
        s.next_in += k;
        n = inflate(&s);
      }
      for (;; n = inflate(&s)) {
        switch (n) {
          case FLATE_OK:
            memmove(begin, s.next_in, s.avail_in);
            k = 0; 
            nin_ += k;
            s.avail_in += k;
            switch(type_) {
              case GZIP:
                k = inflate_gzip_footer(begin, s.avail_in, checksum_, nout_, nin_ - s.avail_in - header_size_);
                break;
              case ZLIB:
                k = inflate_zlib_footer(begin, s.avail_in, checksum_, nout_, nin_ - s.avail_in - header_size_);
                break;
              case PKZIP:
                k = inflate_pkzip_footer(begin, s.avail_in, checksum_, nout_, nin_ - s.avail_in - header_size_);
                break;
              default:
                k = dummyfooter(begin, s.avail_in, checksum_, nout_, nin_ - s.avail_in - header_size_);
            }
            if (k == FLATE_ERR) {
              s.err = strdup("footer error.");
              n = FLATE_ERR;
              throw 2;
            } else {
              footer_size_ = k;
              extra_size_ = s.avail_in - k;
            }
            free(s.next_out);
            return out;
          case FLATE_ERR:
            throw 3;
          case FLATE_IN:
            throw 1;
          case FLATE_OUT:
            out.insert(out.end(), s.next_out, s.next_out+s.avail_out);
            k = s.avail_out;
            switch(type_) {
              case GZIP:
                checksum_ = crc32(s.next_out, k, checksum_);
                break;
              case ZLIB:
                checksum_ = adler32(s.next_out, k, checksum_);
                break;
              case PKZIP:
                checksum_ = crc32(s.next_out, k, checksum_);
                break;
              default:
                checksum_ = dummysum(s.next_out, k, checksum_);
            }
            nout_ += k;
            s.avail_out = BUFFER_SIZE;
            break;
        }
      }

    }

    /* decompress, using flat_stream interface */
    int Z::decompress_stream(FILE *in, FILE *out) {
      flat_stream s;
      unsigned char *begin;
      int k, n;

      s.next_in = begin = (unsigned char*)malloc(BUFFER_SIZE);
      s.next_out = (unsigned char*)malloc(BUFFER_SIZE);
      s.avail_out = BUFFER_SIZE;
      s.err = 0;
      s.state = 0;

      s.avail_in = fread(s.next_in, 1, BUFFER_SIZE, in);
      nin_ += s.avail_in;
      switch(type_) {
        case GZIP:
          k = inflate_gzip_header(s.next_in, s.avail_in);
          break;
        case ZLIB:
          k = inflate_zlib_header(s.next_in, s.avail_in);
          break;
        case PKZIP:
          k = inflate_pkzip_header(s.next_in, s.avail_in);
          break;
        default:
          k = dummyheader(s.next_in, s.avail_in);
      }
      if (k == FLATE_ERR) {
        s.err = strdup("header error.");
        n = FLATE_ERR;
      } else {
        header_size_ = k;
        s.avail_in -= k;
        s.next_in += k;
        n = inflate(&s);
      }
      for (;; n = inflate(&s)) {
        switch (n) {
          case FLATE_OK:
            memmove(begin, s.next_in, s.avail_in);
            k = fread(begin, 1, BUFFER_SIZE-s.avail_in, in);
            nin_ += k;
            s.avail_in += k;
            switch(type_) {
              case GZIP:
                k = inflate_gzip_footer(begin, s.avail_in, checksum_, nout_, nin_ - s.avail_in - header_size_);
                break;
              case ZLIB:
                k = inflate_zlib_footer(begin, s.avail_in, checksum_, nout_, nin_ - s.avail_in - header_size_);
                break;
              case PKZIP:
                k = inflate_pkzip_footer(begin, s.avail_in, checksum_, nout_, nin_ - s.avail_in - header_size_);
                break;
              default:
                k = dummyfooter(begin, s.avail_in, checksum_, nout_, nin_ - s.avail_in - header_size_);
            }
            if (k == FLATE_ERR) {
              s.err = strdup("footer error.");
              n = FLATE_ERR;
            } else {
              footer_size_ = k;
              extra_size_ = s.avail_in - k;
            }
          case FLATE_ERR:
            free(begin);
            free(s.next_out);
            last_error_ = s.err;
            return n;
          case FLATE_IN:
            s.next_in = begin;
            s.avail_in = fread(s.next_in, 1, BUFFER_SIZE, in);
            nin_ += s.avail_in;
            break;
          case FLATE_OUT:
            k = fwrite(s.next_out, 1, s.avail_out, out);
            if (k != s.avail_out)
              s.err = strdup("write error.");
            switch(type_) {
              case GZIP:
                checksum_ = crc32(s.next_out, k, checksum_);
                break;
              case ZLIB:
                checksum_ = adler32(s.next_out, k, checksum_);
                break;
              case PKZIP:
                checksum_ = crc32(s.next_out, k, checksum_);
                break;
              default:
                checksum_ = dummysum(s.next_out, k, checksum_);
            }
            nout_ += k;
            s.avail_out = BUFFER_SIZE;
            break;
        }
      }
    }

    // Public part ----------------------------------------------------------------

    Z::Z(CompressionType type) : type_(type) { }

    std::vector<unsigned char> Z::compress(std::vector<unsigned char> data) {
      checksum_ = 0;
      nin_ = 0;
      nout_ = 0;
      header_size_ = 0;
      footer_size_ = 0;
      extra_size_ = 0;
      if(GZIP == type_ || PKZIP == type_) {
        crc32init();
      }
      if(FLATE_OK != compress_stream(stdin, stdout)) {
        std::cout << last_error_ << std::endl;
      }

      std::vector<unsigned char> out;
      return out;
    }

    std::vector<unsigned char> Z::uncompress(std::vector<unsigned char> data) {
      checksum_ = 0;
      nin_ = 0;
      nout_ = 0;
      header_size_ = 0;
      footer_size_ = 0;
      extra_size_ = 0;
      if(GZIP == type_ || PKZIP == type_) {
        crc32init();
      }

      return decompress_vector(data);
    }

  }
}
