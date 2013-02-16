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

    // Public part ----------------------------------------------------------------

    Z::Z(CompressionType type) : type_(type), deflate_init_done_(false), inflate_init_done_(false) { }
    
    unsigned int Z::get_crc32() {
      return crc32_;
    }
    unsigned int Z::get_compressed_size() {
      return compress_size_;
    }
    unsigned int Z::get_uncompressed_size() {
      return uncompress_size_;
    }

    // compress -------------------------------------------------------------------

    int Z::deflate_init(int /* FIXME : unused */ level) {
      int rcod;

      checksum_ = 0;
      crc32_ = 0;
      nin_ = 0;
      nout_ = 0;
      compress_size_ = 0;
      uncompress_size_ = 0;
      header_size_ = 0;
      footer_size_ = 0;
      extra_size_ = 0;
      if(GZIP == type_ || PKZIP == type_) {
        crc32init();
      }

      stream.next_in = (unsigned char*)malloc(BUFFER_SIZE);
      memset(stream.next_in, 0, BUFFER_SIZE);
      stream.next_out = (unsigned char*)malloc(BUFFER_SIZE);
      memset(stream.next_out, 0, BUFFER_SIZE);
      stream.avail_in = 0;
      stream.avail_out = BUFFER_SIZE;
      stream.err = 0;
      stream.state = 0;

      switch(type_) {
        case GZIP:
          rcod = deflate_gzip_header(stream.next_out, stream.avail_out);
          break;
        case ZLIB:
          rcod = deflate_zlib_header(stream.next_out, stream.avail_out);
          break;
        case PKZIP:
          rcod = deflate_pkzip_header(stream.next_out, stream.avail_out);
          break;
        default:
          rcod = dummyheader(stream.next_out, stream.avail_out);
      }
      if (rcod == FLATE_ERR) {
        free(stream.next_in);
        free(stream.next_out);
        stream.err = strdup("header error.");
        last_flat_rcod_ = FLATE_ERR;
      } else {
        header_size_ = stream.avail_out = rcod;
        last_flat_rcod_ = FLATE_OUT;
      }

      return last_flat_rcod_;
    }

    int Z::deflate() {
      switch(last_flat_rcod_) {
        case FLATE_IN:
          nin_ += stream.avail_in;
          switch(type_) {
            case GZIP:
              crc32_ = checksum_ = crc32(stream.next_in, stream.avail_in, checksum_);
              break;
            case ZLIB:
              checksum_ = adler32(stream.next_in, stream.avail_in, checksum_);
              crc32_ = crc32(stream.next_in, stream.avail_in, crc32_);
              break;
            case PKZIP:
              crc32_ = checksum_ = crc32(stream.next_in, stream.avail_in, checksum_);
              break;
            default:
              checksum_ = dummysum(stream.next_in, stream.avail_in, checksum_);
              crc32_ = crc32(stream.next_in, stream.avail_in, crc32_);
          }
          break;
        case FLATE_OUT:
          nout_ += stream.avail_out;
          stream.avail_out = BUFFER_SIZE;
          break;
        default:
          break;
      }
      last_flat_rcod_ = ::mgz_deflate(&stream);
      return last_flat_rcod_;
    }

    int Z::deflate_end() {
      int rcod;

      if(NULL == stream.next_in || NULL == stream.next_out) {
        stream.err = strdup("footer error.");
        last_flat_rcod_ = FLATE_ERR;
      } else {
        switch(type_) {
          case GZIP:
            rcod = deflate_gzip_footer(stream.next_out, stream.avail_out, checksum_, nin_, nout_ - header_size_);
            break;
          case ZLIB:
            rcod = deflate_zlib_footer(stream.next_out, stream.avail_out, checksum_, nin_, nout_ - header_size_);
            break;
          case PKZIP:
            rcod = deflate_pkzip_footer(stream.next_out, stream.avail_out, checksum_, nin_, nout_ - header_size_);
            break;
          default:
            rcod = dummyfooter(stream.next_out, stream.avail_out, checksum_, nin_, nout_ - header_size_);
        }
        if (rcod == FLATE_ERR) {
          free(stream.next_in);
          free(stream.next_out);
          stream.err = strdup("footer error.");
          last_flat_rcod_ = FLATE_ERR;
        } else {
          stream.avail_out = rcod;
          footer_size_ = rcod;
          nout_ += rcod;
          last_flat_rcod_ = FLATE_END;
        }
      }

      compress_size_ = nout_;
      uncompress_size_ = nin_;

      return last_flat_rcod_;
    }

    void Z::deflate(const std::vector<unsigned char> & in, std::vector<unsigned char> & out) {
      int rcod;
      const unsigned char *data = reinterpret_cast<const unsigned char*>(&in[0]);
      int data_size = in.size();

      if(!deflate_init_done_) {
        rcod = deflate_init(9);
        deflate_init_done_ = true;
      } else {
        if(data_size == 0) {
          while(last_flat_rcod_ != FLATE_OK) {
            switch(last_flat_rcod_) {
              case FLATE_IN:
                stream.avail_in = 0;
                break;
              case FLATE_OUT:
                out.insert(out.end(), stream.next_out, stream.next_out+stream.avail_out);
                break;
              case FLATE_ERR:
              case FLATE_END:
                throw 2; // FIXME
                break;
              case FLATE_OK:
              default:
                break;
            }
            deflate();
          }

          rcod = deflate_end();
          if(FLATE_END == rcod) {
            out.insert(out.end(), stream.next_out, stream.next_out+stream.avail_out);
            deflate_init_done_ = false;
          } else {
            throw 1; // FIXME
          }

          return;
        } else {
          rcod = last_flat_rcod_;
        }
      }

      for(;;rcod = deflate()) {
        switch(rcod) {
          case FLATE_OUT:
            out.insert(out.end(), stream.next_out, stream.next_out+stream.avail_out);
            break;
          case FLATE_IN:
            if(data_size <= 0) {
              stream.avail_in = 0;
              return;
            }
            if(data_size < BUFFER_SIZE) {
              memcpy(stream.next_in, data, data_size);
              stream.avail_in = data_size;
              data_size = 0;
            } else {
              memcpy(stream.next_in, data, BUFFER_SIZE);
              stream.avail_in = BUFFER_SIZE;
              data_size -= BUFFER_SIZE;
              data += BUFFER_SIZE;
            }
            break;
          case FLATE_OK:
          case FLATE_END:
            return;
            break;
          case FLATE_ERR:
            throw 2; // FIXME
        }
      }
    }

    void Z::deflate(FILE *in, FILE *out) {
      unsigned char* buffer = (unsigned char*)malloc(BUFFER_SIZE);

      std::vector<unsigned char> vec_in;
      std::vector<unsigned char> vec_out;

      bool cont = true;
      while(cont) {
        if(int in_size = fread(buffer, 1, BUFFER_SIZE, in)) {
          vec_in = std::vector<unsigned char>(buffer, buffer+in_size);
        } else {
          cont = false;
        }
        deflate(vec_in, vec_out);
        if(vec_out.size() > 0) {
          if(vec_out.size() != fwrite(&vec_out.front(), 1, vec_out.size(), out)) {
            throw 1; // FIXME
          }
        }
        vec_out.clear();
        vec_in.clear();
      }

      free(buffer);
    }
    
    void Z::deflate(std::fstream & in, std::fstream & out) {
      unsigned char* buffer = (unsigned char*)malloc(BUFFER_SIZE);

      std::vector<unsigned char> vec_in;
      std::vector<unsigned char> vec_out;

      bool cont = true;
      while(cont) {
        in.read((char*)buffer, BUFFER_SIZE);
        if(int in_size = in.gcount()) {
          vec_in = std::vector<unsigned char>(buffer, buffer+in_size);
        } else {
          cont = false;
        }
        deflate(vec_in, vec_out);
        if(vec_out.size() > 0) {
          out.write((const char*)&vec_out.front(), vec_out.size());
        }
        vec_out.clear();
        vec_in.clear();
      }

      free(buffer);
    }

    // uncompress -------------------------------------------------------------

    int Z::inflate_init() {
      checksum_ = 0;
      crc32_ = 0;
      nin_ = 0;
      nout_ = 0;
      compress_size_ = 0;
      uncompress_size_ = 0;
      header_size_ = 0;
      footer_size_ = 0;
      extra_size_ = 0;
      if(GZIP == type_ || PKZIP == type_) {
        crc32init();
      }

      stream.next_in = stream.begin = (unsigned char*)malloc(BUFFER_SIZE);
      memset(stream.next_in, 0, BUFFER_SIZE);
      stream.next_out = (unsigned char*)malloc(BUFFER_SIZE);
      memset(stream.next_out, 0, BUFFER_SIZE);
      stream.avail_in = 0;
      stream.avail_out = BUFFER_SIZE;
      stream.err = 0;
      stream.state = 0;
      stream.inflate_header_read = 0;

      last_flat_rcod_ = FLATE_IN;
      return last_flat_rcod_;
    }

    int Z::inflate() {
      int k;
      switch(last_flat_rcod_) {
        case FLATE_IN:
          nin_ += stream.avail_in;

          if(0 == stream.inflate_header_read) {
            switch(type_) {
              case GZIP:
                k = inflate_gzip_header(stream.next_in, stream.avail_in);
                break;
              case ZLIB:
                k = inflate_zlib_header(stream.next_in, stream.avail_in);
                break;
              case PKZIP:
                k = inflate_pkzip_header(stream.next_in, stream.avail_in);
                break;
              default:
                k = dummyheader(stream.next_in, stream.avail_in);
            }

            if (k == FLATE_ERR) {
              stream.err = strdup("header error.");
              last_flat_rcod_ = FLATE_ERR;
              return last_flat_rcod_;
            } else {
              header_size_ = k;
              stream.avail_in -= k;
              stream.next_in += k;
              stream.inflate_header_read = 1;
            }
          }
          break;
        case FLATE_OUT:
          switch(type_) {
            case GZIP:
              crc32_ = checksum_ = crc32(stream.next_out, stream.avail_out, checksum_);
              break;
            case ZLIB:
              checksum_ = adler32(stream.next_out, stream.avail_out, checksum_);
              crc32_ = crc32(stream.next_out, stream.avail_out, crc32_);
              break;
            case PKZIP:
              crc32_ = checksum_ = crc32(stream.next_out, stream.avail_out, checksum_);
              break;
            default:
              checksum_ = dummysum(stream.next_out, stream.avail_out, checksum_);
              crc32_ = crc32(stream.next_out, stream.avail_out, crc32_);
          }
          nout_ += stream.avail_out;
          stream.avail_out = BUFFER_SIZE;
          break;
        default:
          break;
      }

      last_flat_rcod_ = ::mgz_inflate(&stream);

      switch(last_flat_rcod_) {
        case FLATE_IN:
          stream.next_in = stream.begin;
          break;
        case FLATE_OK:
            memmove(stream.begin, stream.next_in, stream.avail_in);
            break;
        default:
          break;
      }

      return last_flat_rcod_;
    }

    int Z::inflate_end() {
      int k;
      switch(type_) {
        case GZIP:
          k = inflate_gzip_footer(stream.begin, stream.avail_in, checksum_, nout_, nin_ - stream.avail_in - header_size_);
          break;
        case ZLIB:
          k = inflate_zlib_footer(stream.begin, stream.avail_in, checksum_, nout_, nin_ - stream.avail_in - header_size_);
          break;
        case PKZIP:
          k = inflate_pkzip_footer(stream.begin, stream.avail_in, checksum_, nout_, nin_ - stream.avail_in - header_size_);
          break;
        default:
          k = dummyfooter(stream.begin, stream.avail_in, checksum_, nout_, nin_ - stream.avail_in - header_size_);
      }
      if (k == FLATE_ERR) {
        stream.err = strdup("footer error.");
        last_flat_rcod_ = FLATE_ERR;
      } else {
        footer_size_ = k;
        extra_size_ = stream.avail_in - k;
        last_flat_rcod_ = FLATE_END;
      }

      compress_size_ = nin_;
      uncompress_size_ = nout_;

      return last_flat_rcod_;
    }

    void Z::inflate(const std::vector<unsigned char> & in, std::vector<unsigned char> & out) {
      int rcod;
      const unsigned char *data = reinterpret_cast<const unsigned char*>(&in[0]);
      int data_size = in.size();

      if(!inflate_init_done_) {
        rcod = inflate_init();
        if(FLATE_IN != rcod) {
          std::cout << "EXPECT FLATE_IN GOT " << rcod << std::endl;
          throw 1; // FIXME
        }
        inflate_init_done_ = true;
      } else {
        if(data_size == 0) {
          while(last_flat_rcod_ != FLATE_OK) {
            switch(last_flat_rcod_) {
              case FLATE_OUT:
                out.insert(out.end(), stream.next_out, stream.next_out+stream.avail_out);
                break;
              case FLATE_IN:
              case FLATE_ERR:
                std::cout << "UNWANTED FLATE_IN/FLATE_ERR " << last_flat_rcod_ << std::endl;
                throw 3; // FIXME
            }
            inflate();
          }

          if(FLATE_END != inflate_end()) {
            std::cout << "EXPECT FLATE_END" << std::endl;
            throw 2; // FIXME
          }
        } else {
          rcod = last_flat_rcod_;
        }
      }

      for(;;rcod = inflate()) {
        switch(last_flat_rcod_) {
          case FLATE_IN:
            if(data_size <= 0) {
              stream.avail_in = 0;
              return;
            }
            if(data_size < BUFFER_SIZE) {
              memcpy(stream.next_in, data, data_size);
              stream.avail_in = data_size;
              data_size = 0;
            } else {
              memcpy(stream.next_in, data, BUFFER_SIZE);
              stream.avail_in = BUFFER_SIZE;
              data_size -= BUFFER_SIZE;
              data += BUFFER_SIZE;
            }
            break;
          case FLATE_OUT:
            out.insert(out.end(), stream.next_out, stream.next_out+stream.avail_out);
            break;
          case FLATE_END:
          case FLATE_OK:
            return;
            break;
          case FLATE_ERR:
            std::cout << "INFLATE ERROR" << std::endl;
            throw 2; // FIXME
        }
      }

      return;
    }

    void Z::inflate(FILE *in, FILE *out) {
      unsigned char* buffer = (unsigned char*)malloc(BUFFER_SIZE);

      std::vector<unsigned char> vec_in;
      std::vector<unsigned char> vec_out;

      bool cont = true;
      while(cont) {
        if(int in_size = fread(buffer, 1, BUFFER_SIZE, in)) {
          vec_in = std::vector<unsigned char>(buffer, buffer+in_size);
        } else {
          cont = false;
        }
        inflate(vec_in, vec_out);
        if(vec_out.size() > 0) {
          if(vec_out.size() != fwrite(&vec_out.front(), 1, vec_out.size(), out)) {
            std::cout << "INFLATE FILE I/O ERROR" << std::endl;
            throw 1; // FIXME
          }
        }
        vec_out.clear();
        vec_in.clear();
      }

      free(buffer);
    }

    void Z::inflate(std::fstream & in, std::fstream & out) {
      unsigned char* buffer = (unsigned char*)malloc(BUFFER_SIZE);

      std::vector<unsigned char> vec_in;
      std::vector<unsigned char> vec_out;

      bool cont = true;
      while(cont) {
        in.read((char*)buffer, BUFFER_SIZE);
        if(int in_size = in.gcount()) {
          vec_in = std::vector<unsigned char>(buffer, buffer+in_size);
        } else {
          cont = false;
        }
        inflate(vec_in, vec_out);
        if(vec_out.size() > 0) {
          out.write((const char*)&vec_out.front(), vec_out.size());
        }
        vec_out.clear();
        vec_in.clear();
      }

      free(buffer);
    }
  }
}
