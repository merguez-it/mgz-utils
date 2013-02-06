#include <string.h>
#include "security/sha2.h"
#include "util/string.h"

#define MIN(x,y) ((x)<(y))?(x):(y)
#define CH(x,y,z) ((z) ^ ((x) & ((y) ^ (z))))
#define SH(x,n) ((x) >> (n))
#define MAJ(x,y,z) ((((x) | (y)) & (z)) | ((x) & (y)))
#define ROT64(x,n) (((x) >> ((n) & 63)) | ((x) << (64 - ((n) & 63))))
#define SIGMA0_64(x) (ROT64(x, 28) ^ ROT64(x, 34) ^ ROT64(x, 39))
#define SIGMA1_64(x) (ROT64(x, 14) ^ ROT64(x, 18) ^ ROT64(x, 41))
#define GAMMA0_64(x) (ROT64(x, 1) ^ ROT64(x, 8) ^ SH(x, 7))
#define GAMMA1_64(x) (ROT64(x, 19) ^ ROT64(x, 61) ^ SH(x, 6))
#define RND64(a,b,c,d,e,f,g,h,i) \
{ \
  t0 = h + SIGMA1_64(e) + CH(e, f, g) + K64[i] + W[i]; \
  t1 = SIGMA0_64(a) + MAJ(a, b, c); \
  d += t0; \
  h  = t0 + t1; \
};
#define ROT32(x,n) (((x) >> ((n) & 31)) | ((x) << (32 - ((n) & 31))))
#define SIGMA0_32(x) (ROT32(x, 2) ^ ROT32(x, 13) ^ ROT32(x, 22))
#define SIGMA1_32(x) (ROT32(x, 6) ^ ROT32(x, 11) ^ ROT32(x, 25))
#define GAMMA0_32(x) (ROT32(x, 7) ^ ROT32(x, 18) ^ SH(x, 3))
#define GAMMA1_32(x) (ROT32(x, 17) ^ ROT32(x, 19) ^ SH(x, 10))
#define RND32(a,b,c,d,e,f,g,h,i) \
{ \
  t0 = h + SIGMA1_32(e) + CH(e, f, g) + K32[i] + W[i]; \
  t1 = SIGMA0_32(a) + MAJ(a, b, c); \
  d += t0; \
  h  = t0 + t1; \
};

static const unsigned int K32[64] = {
  0x428a2f98UL, 0x71374491UL, 0xb5c0fbcfUL, 0xe9b5dba5UL, 0x3956c25bUL,
  0x59f111f1UL, 0x923f82a4UL, 0xab1c5ed5UL, 0xd807aa98UL, 0x12835b01UL,
  0x243185beUL, 0x550c7dc3UL, 0x72be5d74UL, 0x80deb1feUL, 0x9bdc06a7UL,
  0xc19bf174UL, 0xe49b69c1UL, 0xefbe4786UL, 0x0fc19dc6UL, 0x240ca1ccUL,
  0x2de92c6fUL, 0x4a7484aaUL, 0x5cb0a9dcUL, 0x76f988daUL, 0x983e5152UL,
  0xa831c66dUL, 0xb00327c8UL, 0xbf597fc7UL, 0xc6e00bf3UL, 0xd5a79147UL,
  0x06ca6351UL, 0x14292967UL, 0x27b70a85UL, 0x2e1b2138UL, 0x4d2c6dfcUL,
  0x53380d13UL, 0x650a7354UL, 0x766a0abbUL, 0x81c2c92eUL, 0x92722c85UL,
  0xa2bfe8a1UL, 0xa81a664bUL, 0xc24b8b70UL, 0xc76c51a3UL, 0xd192e819UL,
  0xd6990624UL, 0xf40e3585UL, 0x106aa070UL, 0x19a4c116UL, 0x1e376c08UL,
  0x2748774cUL, 0x34b0bcb5UL, 0x391c0cb3UL, 0x4ed8aa4aUL, 0x5b9cca4fUL,
  0x682e6ff3UL, 0x748f82eeUL, 0x78a5636fUL, 0x84c87814UL, 0x8cc70208UL,
  0x90befffaUL, 0xa4506cebUL, 0xbef9a3f7UL, 0xc67178f2UL
};

static const unsigned long long K64[80] = {
  0x428a2f98d728ae22ULL, 0x7137449123ef65cdULL, 0xb5c0fbcfec4d3b2fULL, 
  0xe9b5dba58189dbbcULL, 0x3956c25bf348b538ULL, 0x59f111f1b605d019ULL, 
  0x923f82a4af194f9bULL, 0xab1c5ed5da6d8118ULL, 0xd807aa98a3030242ULL, 
  0x12835b0145706fbeULL, 0x243185be4ee4b28cULL, 0x550c7dc3d5ffb4e2ULL, 
  0x72be5d74f27b896fULL, 0x80deb1fe3b1696b1ULL, 0x9bdc06a725c71235ULL,
  0xc19bf174cf692694ULL, 0xe49b69c19ef14ad2ULL, 0xefbe4786384f25e3ULL, 
  0x0fc19dc68b8cd5b5ULL, 0x240ca1cc77ac9c65ULL, 0x2de92c6f592b0275ULL, 
  0x4a7484aa6ea6e483ULL, 0x5cb0a9dcbd41fbd4ULL, 0x76f988da831153b5ULL, 
  0x983e5152ee66dfabULL, 0xa831c66d2db43210ULL, 0xb00327c898fb213fULL, 
  0xbf597fc7beef0ee4ULL, 0xc6e00bf33da88fc2ULL, 0xd5a79147930aa725ULL,
  0x06ca6351e003826fULL, 0x142929670a0e6e70ULL, 0x27b70a8546d22ffcULL, 
  0x2e1b21385c26c926ULL, 0x4d2c6dfc5ac42aedULL, 0x53380d139d95b3dfULL, 
  0x650a73548baf63deULL, 0x766a0abb3c77b2a8ULL, 0x81c2c92e47edaee6ULL, 
  0x92722c851482353bULL, 0xa2bfe8a14cf10364ULL, 0xa81a664bbc423001ULL, 
  0xc24b8b70d0f89791ULL, 0xc76c51a30654be30ULL, 0xd192e819d6ef5218ULL,
  0xd69906245565a910ULL, 0xf40e35855771202aULL, 0x106aa07032bbd1b8ULL, 
  0x19a4c116b8d2d0c8ULL, 0x1e376c085141ab53ULL, 0x2748774cdf8eeb99ULL, 
  0x34b0bcb5e19b48a8ULL, 0x391c0cb3c5c95a63ULL, 0x4ed8aa4ae3418acbULL, 
  0x5b9cca4f7763e373ULL, 0x682e6ff3d6b2b8a3ULL, 0x748f82ee5defb2fcULL, 
  0x78a5636f43172f60ULL, 0x84c87814a1f0ab72ULL, 0x8cc702081a6439ecULL,
  0x90befffa23631e28ULL, 0xa4506cebde82bde9ULL, 0xbef9a3f7b2c67915ULL, 
  0xc67178f2e372532bULL, 0xca273eceea26619cULL, 0xd186b8c721c0c207ULL, 
  0xeada7dd6cde0eb1eULL, 0xf57d4f7fee6ed178ULL, 0x06f067aa72176fbaULL, 
  0x0a637dc5a2c898a6ULL, 0x113f9804bef90daeULL, 0x1b710b35131c471bULL, 
  0x28db77f523047d84ULL, 0x32caab7b40c72493ULL, 0x3c9ebe0a15c9bebcULL, 
  0x431d67c49c100d4cULL, 0x4cc5d4becb3e42b6ULL, 0x597f299cfc657e2aULL, 
  0x5fcb6fab3ad6faecULL, 0x6c44198c4a475817ULL
};

namespace mgz {
  namespace security {

    // -- sha2common ----------------------------------------------------------

    void sha2common::store32(unsigned int x, unsigned char* y) {
      for(int i = 0; i != 4; ++i) {
        y[i] = (x >> ((3-i) * 8)) & 255;
      }
    }
    int sha2common::load32(const unsigned char* y) {
      return ((unsigned int)(y[0]) << 24) | ((unsigned int)(y[1]) << 16) | 
        ((unsigned int)(y[2]) << 8) | ((unsigned int)(y[3]) << 0);
    }
    void sha2common::store64(unsigned long long x, unsigned char* y) {
      for(int i = 0; i != 8; ++i) {
        y[i] = (x >> ((7-i) * 8)) & 255;
      }
    }
    unsigned long long sha2common::load64(const unsigned char* y) {
      unsigned long long res = 0;
      for(int i = 0; i != 8; ++i) {
        res |= (unsigned long long)(y[i]) << ((7-i) * 8);
      }
      return res;
    }
    std::string sha2common::hexdigest(const unsigned char *data, size_t size) {
      std::string result("");
      for(int i = 0; i < size; i++) {
        result.append(mgz::util::format("%02x", data[i]));
      }
      return result;
    }

    // -- sha224256 -----------------------------------------------------------

    void sha224256::update(const unsigned char *src, size_t length) {
      const unsigned int block_size = sizeof(buf);

      while(length > 0) {
        if(curlen == 0 && length >= block_size) {
          sha_compress(src);
          length += block_size * 8;
          src += block_size;
          length -= block_size;
        } else {
          unsigned int n = MIN(length, (block_size - curlen));
          memcpy(buf + curlen, src, n);
          curlen += n;
          src += n;
          length -= n;

          if(curlen == block_size) {
            sha_compress(buf);
            length += 8*block_size;
            curlen = 0;
          }
        }
      }
    }

    void sha224256::finalize(void *out) {
      length += curlen * 8ULL;
      buf[curlen++] = static_cast<unsigned char>(0x80);
      if(curlen > 56) {
        while(curlen < 64) {
          buf[curlen++] = 0;
        }
        sha_compress(buf);
        curlen = 0;
      } 

      while(curlen < 56) {
        buf[curlen++] = 0;
      }

      store64(length, buf+56);
      sha_compress(buf);

      for(int i = 0; i < 8; i++) {
        store32(state[i], static_cast<unsigned char*>(out)+(4*i));
      }
    }

    void sha224256::sha_compress(const unsigned char* buf) {
      unsigned int S[8], W[64], t0, t1, t;

      for(int i = 0; i < 8; i++) {
        S[i] = state[i];
      }

      for(int i = 0; i < 16; i++) {
        W[i] = load32(buf + (4*i));
      }

      for(int i = 16; i < 64; i++) {
        W[i] = GAMMA1_32(W[i - 2]) + W[i - 7] + GAMMA0_32(W[i - 15]) + W[i - 16];
      }

      for(int i = 0; i < 64; ++i) {
        RND32(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],i);
        t = S[7]; S[7] = S[6]; S[6] = S[5]; S[5] = S[4];
        S[4] = S[3]; S[3] = S[2]; S[2] = S[1]; S[1] = S[0]; S[0] = t;
      }

      for(int i = 0; i < 8; i++) {
        state[i] = state[i] + S[i];
      }
    }

    // -- sha224interface -----------------------------------------------------

    void sha224interface::init() {
      hexsize = SHA224_SIZE; 
      curlen = 0;
      length = 0;
      state[0] = 0xc1059ed8UL;
      state[1] = 0x367cd507UL;
      state[2] = 0x3070dd17UL;
      state[3] = 0xf70e5939UL;
      state[4] = 0xffc00b31UL;
      state[5] = 0x68581511UL;
      state[6] = 0x64f98fa7UL;
      state[7] = 0xbefa4fa4UL;
    }

    void sha224interface::finalize() {
      sha224256::finalize(data_);
    }

    std::string sha224interface::hexdigest() {
      return sha2common::hexdigest(data_, hexsize);
    }

    // -- sha256interface -----------------------------------------------------

    void sha256interface::init() {
      hexsize = SHA256_SIZE;
      curlen = 0;
      length = 0;
      state[0] = 0x6A09E667UL;
      state[1] = 0xBB67AE85UL;
      state[2] = 0x3C6EF372UL;
      state[3] = 0xA54FF53AUL;
      state[4] = 0x510E527FUL;
      state[5] = 0x9B05688CUL;
      state[6] = 0x1F83D9ABUL;
      state[7] = 0x5BE0CD19UL;
    }

    void sha256interface::finalize() {
      sha224256::finalize(data_);
    }

    std::string sha256interface::hexdigest() {
      return sha2common::hexdigest(data_, hexsize);
    }

    // -- tools ---------------------------------------------------------------

    std::string sha224(const std::string & data) {
      sha224sum s224;
      s224.update(data);
      s224.finalize();
      return s224.hexdigest();
    }

    std::string sha256(const std::string & data) {
      sha256sum s256;
      s256.update(data);
      s256.finalize();
      return s256.hexdigest();
    }

    // -- sha384512 -----------------------------------------------------------

    void sha384512::update(const unsigned char *src, size_t length) {
      const unsigned int block_size = sizeof(buf);

      while(length > 0) {
        if(curlen == 0 && length >= block_size) {
          sha_compress(src);
          length += block_size * 8;
          src += block_size;
          length -= block_size;
        } else {
          unsigned int n = MIN(length, (block_size - curlen));
          memcpy(buf + curlen, src, n);
          curlen += n;
          src += n;
          length -= n;

          if(curlen == block_size) {
            sha_compress(buf);
            length += 8*block_size;
            curlen = 0;
          }
        }
      }
    }

    void sha384512::finalize(void *out) {
      length += curlen * 8ULL;
      buf[curlen++] = static_cast<unsigned char>(0x80);

      if(curlen > 112) {
        while(curlen < 128) {
          buf[curlen++] = 0;
        }
        sha_compress(buf);
        curlen = 0;
      } 

      while(curlen < 120) {
        buf[curlen++] = 0;
      }

      store64(length, buf+120);
      sha_compress(buf);

      for(int i = 0; i < 8; i++) {
        store64(state[i], static_cast<unsigned char*>(out)+(8*i));
      }
    }

    void sha384512::sha_compress(const unsigned char* buf) {
      unsigned long long S[8], W[80], t0, t1;

      for(int i = 0; i < 8; i++) {
        S[i] = state[i]; 
      }
      for(int i = 0; i < 16; i++) {
        W[i] = load64(buf + (8*i));
      }
      for(int i = 16; i < 80; i++) {
        W[i] = GAMMA1_64(W[i - 2]) + W[i - 7] + GAMMA0_64(W[i - 15]) + W[i - 16];
      }
      for(int i = 0; i < 80; i += 8) {
        RND64(S[0],S[1],S[2],S[3],S[4],S[5],S[6],S[7],i+0);
        RND64(S[7],S[0],S[1],S[2],S[3],S[4],S[5],S[6],i+1);
        RND64(S[6],S[7],S[0],S[1],S[2],S[3],S[4],S[5],i+2);
        RND64(S[5],S[6],S[7],S[0],S[1],S[2],S[3],S[4],i+3);
        RND64(S[4],S[5],S[6],S[7],S[0],S[1],S[2],S[3],i+4);
        RND64(S[3],S[4],S[5],S[6],S[7],S[0],S[1],S[2],i+5);
        RND64(S[2],S[3],S[4],S[5],S[6],S[7],S[0],S[1],i+6);
        RND64(S[1],S[2],S[3],S[4],S[5],S[6],S[7],S[0],i+7);
      } 
      for(int i = 0; i < 8; i++) {
        state[i] = state[i] + S[i];
      }
    }

    // -- sha384interface -----------------------------------------------------

    void sha384interface::init() {
      hexsize = SHA384_SIZE; 
      curlen = 0;
      length = 0;
      state[0] = 0xcbbb9d5dc1059ed8ULL;
      state[1] = 0x629a292a367cd507ULL;
      state[2] = 0x9159015a3070dd17ULL;
      state[3] = 0x152fecd8f70e5939ULL;
      state[4] = 0x67332667ffc00b31ULL;
      state[5] = 0x8eb44a8768581511ULL;
      state[6] = 0xdb0c2e0d64f98fa7ULL;
      state[7] = 0x47b5481dbefa4fa4ULL;
    }

    void sha384interface::finalize() {
      sha384512::finalize(data_);
    }

    std::string sha384interface::hexdigest() {
      return sha2common::hexdigest(data_, hexsize);
    }

    // -- sha512interface -----------------------------------------------------

    void sha512interface::init() {
      hexsize = SHA512_SIZE;
      curlen = 0;
      length = 0;
      state[0] = 0x6a09e667f3bcc908ULL;
      state[1] = 0xbb67ae8584caa73bULL;
      state[2] = 0x3c6ef372fe94f82bULL;
      state[3] = 0xa54ff53a5f1d36f1ULL;
      state[4] = 0x510e527fade682d1ULL;
      state[5] = 0x9b05688c2b3e6c1fULL;
      state[6] = 0x1f83d9abfb41bd6bULL;
      state[7] = 0x5be0cd19137e2179ULL;
    }

    void sha512interface::finalize() {
      sha384512::finalize(data_);
    }

    std::string sha512interface::hexdigest() {
      return sha2common::hexdigest(data_, hexsize);
    }

    // -- tools ---------------------------------------------------------------

    std::string sha384(const std::string & data) {
      std::string __r;
      sha384sum s384;
      s384.update(data);
      s384.finalize();
      __r = s384.hexdigest();
      return __r;
    }

    std::string sha512(const std::string & data) {
      sha512sum s512;
      s512.update(data);
      s512.finalize();
      return s512.hexdigest();
    }
  }
}
