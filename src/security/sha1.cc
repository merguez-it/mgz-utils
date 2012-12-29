#include "security/sha1.h"
#include "util/string.h"
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>

#define HASH_BUFFER_SIZE (2048 * 1024)

namespace mgz {
  namespace security {
#define FUNC1(x,y,z) ((x & y) | (~x & z))
#define FUNC2(x,y,z) (x ^ y ^ z)
#define FUNC3(x,y,z) ((x & y) | (x & z) | (y & z))
#define FUNC4(x,y,z) (x ^ y ^ z)

#define CONST1 0x5a827999L
#define CONST2 0x6ed9eba1L
#define CONST3 0x8f1bbcdcL
#define CONST4 0xca62c1d6L

#define TRUNCATE32(x) ((x) & 0xffffffffL)

#define ROL32(x,n) TRUNCATE32(((x << n) | (x >> (32 - n))))

#define FG(n) \
  T = TRUNCATE32(ROL32(A,5) + FUNC##n(B,C,D) + E + *WP++ + CONST##n); \
  E = D; D = C; C = ROL32(B,30); B = A; A = T

#define FA(n) \
  T = TRUNCATE32(ROL32(A,5) + FUNC##n(B,C,D) + E + *WP++ + CONST##n); B = ROL32(B,30)

#define FB(n) \
  E = TRUNCATE32(ROL32(T,5) + FUNC##n(A,B,C) + D + *WP++ + CONST##n); A = ROL32(A,30)

#define FC(n) \
  D = TRUNCATE32(ROL32(E,5) + FUNC##n(T,A,B) + C + *WP++ + CONST##n); T = ROL32(T,30)

#define FD(n) \
  C = TRUNCATE32(ROL32(D,5) + FUNC##n(E,T,A) + B + *WP++ + CONST##n); E = ROL32(E,30)

#define FE(n) \
  B = TRUNCATE32(ROL32(C,5) + FUNC##n(D,E,T) + A + *WP++ + CONST##n); D = ROL32(D,30)

#define FT(n) \
  A = TRUNCATE32(ROL32(B,5) + FUNC##n(C,D,E) + T + *WP++ + CONST##n); C = ROL32(C,30)

    sha1sum::sha1sum() {
      init();
    }

    void sha1sum::update(const unsigned char *buf, size_t length) {
      size_t i;
      unsigned long clo;

      if(!length) {       
        return;
      }       

      clo = TRUNCATE32(length_low + ((unsigned long) length << 3));
      if(clo < length_low) {
        ++length_hight;
      }
      length_low = clo;
      length_hight += (unsigned long) length >> 29;
      if(local) {
        i = SHA_BLOCKSIZE - local;
        if(i > length) {
          i = length;
        }
        memcpy(((unsigned char *) data) + local, buf, i);
        length -= i;
        buf += i;
        local += i;
        if(local == SHA_BLOCKSIZE) {
          process_message();
        } else {
          return;
        }
      }
      while(length >= SHA_BLOCKSIZE) {
        memcpy(data, buf, SHA_BLOCKSIZE);
        buf += SHA_BLOCKSIZE;
        length -= SHA_BLOCKSIZE;
        process_message();
      }
      memcpy(data, buf, length);
      local = length;
    }
    void sha1sum::update(const char *buf, size_t length) {
      update((const unsigned char*)buf, length);
    }
    void sha1sum::update(const std::string & buffer) {
      update(buffer.c_str(), buffer.size());
    }
    void sha1sum::update(const std::vector<unsigned char> & buffer) {
      update(&buffer[0], buffer.size());
    }
    void sha1sum::update(const std::vector<char> & buffer) {
      update(&buffer[0], buffer.size());
    }
    void sha1sum::update(const std::vector<unsigned char> & buffer, size_t length) {
      update(&buffer[0], length);
    }
    void sha1sum::update(const std::vector<char> & buffer, size_t length) {
      update(&buffer[0], length);
    }

    std::string sha1sum::hexdigest() const {
      char buf[41] = { 0 };
      sprintf(buf, "%08x%08x%08x%08x%08x", 
          digest[0],
          digest[1],
          digest[2],
          digest[3],
          digest[4]
          );
      return std::string(buf);
    }

    void sha1sum::init() {
      digest[0] = 0x67452301L;
      digest[1] = 0xefcdab89L;
      digest[2] = 0x98badcfeL;
      digest[3] = 0x10325476L;
      digest[4] = 0xc3d2e1f0L;
      length_low = 0L;
      length_hight = 0L;
      local = 0;
    }

    sha1sum & sha1sum::finalize() {
      int count;
      unsigned long lo_bit_count, hi_bit_count;

      lo_bit_count = length_low;
      hi_bit_count = length_hight;
      count = (int) ((lo_bit_count >> 3) & 0x3f);
      ((unsigned char *) data)[count++] = 0x80;
      if (count > SHA_BLOCKSIZE - 8) {
        memset(((unsigned char *) data) + count, 0, SHA_BLOCKSIZE - count);
        process_message();
        memset((unsigned char *) data, 0, SHA_BLOCKSIZE - 8);
      } else {
        memset(((unsigned char *) data) + count, 0, SHA_BLOCKSIZE - 8 - count);
      }
      data[56] = (unsigned char)((hi_bit_count >> 24) & 0xff);
      data[57] = (unsigned char)((hi_bit_count >> 16) & 0xff);
      data[58] = (unsigned char)((hi_bit_count >>  8) & 0xff);
      data[59] = (unsigned char)((hi_bit_count >>  0) & 0xff);
      data[60] = (unsigned char)((lo_bit_count >> 24) & 0xff);
      data[61] = (unsigned char)((lo_bit_count >> 16) & 0xff);
      data[62] = (unsigned char)((lo_bit_count >>  8) & 0xff);
      data[63] = (unsigned char)((lo_bit_count >>  0) & 0xff);
      process_message();

      return *this;
    }

    void sha1sum::process_message() {
      int i;
      unsigned char *dp;
      unsigned long T, A, B, C, D, E, W[80], *WP;

      dp = data;

      /* assert(sizeof(unsigned long) == 4); */
      for (i = 0; i < 16; ++i) {
        T = *((unsigned long *) dp);
        dp += 4;
        W[i] =  ((T << 24) & 0xff000000) | ((T <<  8) & 0x00ff0000) |
          ((T >>  8) & 0x0000ff00) | ((T >> 24) & 0x000000ff);
      }

      for (i = 16; i < 80; ++i) {
        W[i] = W[i-3] ^ W[i-8] ^ W[i-14] ^ W[i-16];
        W[i] = ROL32(W[i], 1);
      }
      A = digest[0];
      B = digest[1];
      C = digest[2];
      D = digest[3];
      E = digest[4];
      WP = W;

      FA(1); FB(1); FC(1); FD(1); FE(1); FT(1); FA(1); FB(1); FC(1); FD(1);
      FE(1); FT(1); FA(1); FB(1); FC(1); FD(1); FE(1); FT(1); FA(1); FB(1);
      FC(2); FD(2); FE(2); FT(2); FA(2); FB(2); FC(2); FD(2); FE(2); FT(2);
      FA(2); FB(2); FC(2); FD(2); FE(2); FT(2); FA(2); FB(2); FC(2); FD(2);
      FE(3); FT(3); FA(3); FB(3); FC(3); FD(3); FE(3); FT(3); FA(3); FB(3);
      FC(3); FD(3); FE(3); FT(3); FA(3); FB(3); FC(3); FD(3); FE(3); FT(3);
      FA(4); FB(4); FC(4); FD(4); FE(4); FT(4); FA(4); FB(4); FC(4); FD(4);
      FE(4); FT(4); FA(4); FB(4); FC(4); FD(4); FE(4); FT(4); FA(4); FB(4);
      digest[0] = TRUNCATE32(digest[0] + E);
      digest[1] = TRUNCATE32(digest[1] + T);
      digest[2] = TRUNCATE32(digest[2] + A);
      digest[3] = TRUNCATE32(digest[3] + B);
      digest[4] = TRUNCATE32(digest[4] + C);
    }

    std::ostream& operator<<(std::ostream& out, sha1sum sha1) {
      return out << sha1.hexdigest();
    }

    std::string sha1(const std::string & str) {
      sha1sum sha1;
      sha1.update(str.c_str(), str.length());
      sha1.finalize();

      return sha1.hexdigest();
    }
  }
}
