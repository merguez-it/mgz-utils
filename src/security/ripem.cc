// http://homes.esat.kuleuven.be/~bosselae/ripemd160.html

#include "security/ripem.h"
#include "util/string.h"

#define ROL(x, n) (((x) << (n)) | ((x) >> (32-(n))))
#define R_B1(x, y, z) ((x) ^ (y) ^ (z)) 
#define R_B2(x, y, z) (((x) & (y)) | (~(x) & (z))) 
#define R_B3(x, y, z) (((x) | ~(y)) ^ (z))
#define R_B4(x, y, z) (((x) & (z)) | ((y) & ~(z))) 
#define R_B5(x, y, z) ((x) ^ ((y) | ~(z)))
#define BYTES_TO_DWORD(p) \
  (((unsigned int) *((p)+3) << 24) | \
   ((unsigned int) *((p)+2) << 16) | \
   ((unsigned int) *((p)+1) <<  8) | \
   ((unsigned int) *(p)))

namespace mgz {
  namespace security {
    // -- ripem ---------------------------------------------------------------
    ripem::ripem(int size) : ripem_size_(size), current_data_size_(0), data_size_(0) {
      memset(data_, 0, 1024);
    }

    void ripem::perform_update() {
      int i, j;
      unsigned int X[16];
      for(i = 0; i < (current_data_size_>>6); i++) {
         for(j = 0; j < 16; j++) {
            X[j] = BYTES_TO_DWORD(data_+64*i+4*j);
         }
         compress(X);
      }
    }

    void ripem::update(const unsigned char *buf, size_t length) {
      if(current_data_size_ + length >= 1024) {
        int add_to_data_ = 1024 - current_data_size_;
        memcpy(data_+current_data_size_, buf, add_to_data_);
        current_data_size_ += add_to_data_;
        perform_update();

        int buf_offset_ = add_to_data_;
        int keep_size_ = length - add_to_data_;
        while(keep_size_ >= 1024) {
          memcpy(data_, buf+buf_offset_, 1024);
          current_data_size_ = 1024;
          perform_update();

          keep_size_ -= 1024;
          buf_offset_ += 1024;
        }

        if(keep_size_ > 0) {
          memcpy(data_, buf+buf_offset_, keep_size_);
          current_data_size_ = keep_size_;
        } else {
          current_data_size_ = 0;
        }
      } else {
        memcpy(data_+current_data_size_, buf, length);
        current_data_size_ += length;
      }
      data_size_ += length;
    }
    void ripem::update(const char *buf, size_t length) {
      update((const unsigned char*)buf, length);
    }
    void ripem::update(const std::string & buffer) {
      update(buffer.c_str(), buffer.size());
    }
    void ripem::update(const std::vector<unsigned char> & buffer) {
      update(&buffer[0], buffer.size());
    }
    void ripem::update(const std::vector<char> & buffer) {
      update(&buffer[0], buffer.size());
    }
    void ripem::update(const std::vector<unsigned char> & buffer, size_t length) {
      update(&buffer[0], length);
    }
    void ripem::update(const std::vector<char> & buffer, size_t length) {
      update(&buffer[0], length);
    }

    void ripem::finalize() {
      // int mswlen = 0;
      unsigned int i;
      unsigned int X[16];
      unsigned int offset = data_size_ & 0x3C0;
      unsigned char *strptr = data_+offset;

      if(current_data_size_ > 0) {
        perform_update();
      }

      memset(X, 0, 16*sizeof(unsigned int));

      for (i=0; i<(data_size_&63); i++) {
        X[i>>2] ^= (unsigned int) *strptr++ << (8 * (i&3));
      }

      X[(data_size_>>2)&15] ^= (unsigned int)1 << (8*(data_size_&3) + 7);

      if ((data_size_ & 63) > 55) {
        compress(X);
        memset(X, 0, 16*sizeof(unsigned int));
      }

      X[14] = data_size_ << 3;
      X[15] = (data_size_ >> 29); // | (mswlen << 3);
      compress(X);
    }

    std::string ripem::hexdigest() const {
      std::string result;
      for(int i = 0; i < (ripem_size_/32); i++) {
        result = result + mgz::util::format("%02x%02x%02x%02x",
            (unsigned char)ripem_[i],
            (unsigned char)(ripem_[i] >>  8),
            (unsigned char)(ripem_[i] >> 16),
            (unsigned char)(ripem_[i] >> 24));
      }
      return result;
    }

    // -- ripem128 ------------------------------------------------------------

#define R128_R1(a, b, c, d, x, s) {\
  (a) += R_B1((b), (c), (d)) + (x);\
  (a) = ROL((a), (s));\
}
#define R128_R2(a, b, c, d, x, s) {\
  (a) += R_B2((b), (c), (d)) + (x) + 0x5a827999;\
  (a) = ROL((a), (s));\
}
#define R128_R3(a, b, c, d, x, s) {\
  (a) += R_B3((b), (c), (d)) + (x) + 0x6ed9eba1;\
  (a) = ROL((a), (s));\
}
#define R128_R4(a, b, c, d, x, s) {\
  (a) += R_B4((b), (c), (d)) + (x) + 0x8f1bbcdc;\
  (a) = ROL((a), (s));\
}

#define R128_PR4(a, b, c, d, x, s) {\
  (a) += R_B1((b), (c), (d)) + (x);\
  (a) = ROL((a), (s));\
}
#define R128_PR3(a, b, c, d, x, s) {\
  (a) += R_B2((b), (c), (d)) + (x) + 0x6d703ef3;\
  (a) = ROL((a), (s));\
}
#define R128_PR2(a, b, c, d, x, s) {\
  (a) += R_B3((b), (c), (d)) + (x) + 0x5c4dd124;\
  (a) = ROL((a), (s));\
}
#define R128_PR1(a, b, c, d, x, s) {\
  (a) += R_B4((b), (c), (d)) + (x) + 0x50a28be6;\
  (a) = ROL((a), (s));\
}

    ripem128sum::ripem128sum() : ripem(128) {
      ripem_[0] = 0x67452301;
      ripem_[1] = 0xefcdab89;
      ripem_[2] = 0x98badcfe;
      ripem_[3] = 0x10325476;
    }

    void ripem128sum::compress(unsigned int *X) {
      unsigned int aa = ripem_[0],  bb = ripem_[1],  cc = ripem_[2],  dd = ripem_[3];
      unsigned int aaa = ripem_[0], bbb = ripem_[1], ccc = ripem_[2], ddd = ripem_[3];

      /* round 1 */
      R128_R1(aa, bb, cc, dd, X[ 0], 11);
      R128_R1(dd, aa, bb, cc, X[ 1], 14);
      R128_R1(cc, dd, aa, bb, X[ 2], 15);
      R128_R1(bb, cc, dd, aa, X[ 3], 12);
      R128_R1(aa, bb, cc, dd, X[ 4],  5);
      R128_R1(dd, aa, bb, cc, X[ 5],  8);
      R128_R1(cc, dd, aa, bb, X[ 6],  7);
      R128_R1(bb, cc, dd, aa, X[ 7],  9);
      R128_R1(aa, bb, cc, dd, X[ 8], 11);
      R128_R1(dd, aa, bb, cc, X[ 9], 13);
      R128_R1(cc, dd, aa, bb, X[10], 14);
      R128_R1(bb, cc, dd, aa, X[11], 15);
      R128_R1(aa, bb, cc, dd, X[12],  6);
      R128_R1(dd, aa, bb, cc, X[13],  7);
      R128_R1(cc, dd, aa, bb, X[14],  9);
      R128_R1(bb, cc, dd, aa, X[15],  8);

      /* round 2 */
      R128_R2(aa, bb, cc, dd, X[ 7],  7);
      R128_R2(dd, aa, bb, cc, X[ 4],  6);
      R128_R2(cc, dd, aa, bb, X[13],  8);
      R128_R2(bb, cc, dd, aa, X[ 1], 13);
      R128_R2(aa, bb, cc, dd, X[10], 11);
      R128_R2(dd, aa, bb, cc, X[ 6],  9);
      R128_R2(cc, dd, aa, bb, X[15],  7);
      R128_R2(bb, cc, dd, aa, X[ 3], 15);
      R128_R2(aa, bb, cc, dd, X[12],  7);
      R128_R2(dd, aa, bb, cc, X[ 0], 12);
      R128_R2(cc, dd, aa, bb, X[ 9], 15);
      R128_R2(bb, cc, dd, aa, X[ 5],  9);
      R128_R2(aa, bb, cc, dd, X[ 2], 11);
      R128_R2(dd, aa, bb, cc, X[14],  7);
      R128_R2(cc, dd, aa, bb, X[11], 13);
      R128_R2(bb, cc, dd, aa, X[ 8], 12);

      /* round 3 */
      R128_R3(aa, bb, cc, dd, X[ 3], 11);
      R128_R3(dd, aa, bb, cc, X[10], 13);
      R128_R3(cc, dd, aa, bb, X[14],  6);
      R128_R3(bb, cc, dd, aa, X[ 4],  7);
      R128_R3(aa, bb, cc, dd, X[ 9], 14);
      R128_R3(dd, aa, bb, cc, X[15],  9);
      R128_R3(cc, dd, aa, bb, X[ 8], 13);
      R128_R3(bb, cc, dd, aa, X[ 1], 15);
      R128_R3(aa, bb, cc, dd, X[ 2], 14);
      R128_R3(dd, aa, bb, cc, X[ 7],  8);
      R128_R3(cc, dd, aa, bb, X[ 0], 13);
      R128_R3(bb, cc, dd, aa, X[ 6],  6);
      R128_R3(aa, bb, cc, dd, X[13],  5);
      R128_R3(dd, aa, bb, cc, X[11], 12);
      R128_R3(cc, dd, aa, bb, X[ 5],  7);
      R128_R3(bb, cc, dd, aa, X[12],  5);

      /* round 4 */
      R128_R4(aa, bb, cc, dd, X[ 1], 11);
      R128_R4(dd, aa, bb, cc, X[ 9], 12);
      R128_R4(cc, dd, aa, bb, X[11], 14);
      R128_R4(bb, cc, dd, aa, X[10], 15);
      R128_R4(aa, bb, cc, dd, X[ 0], 14);
      R128_R4(dd, aa, bb, cc, X[ 8], 15);
      R128_R4(cc, dd, aa, bb, X[12],  9);
      R128_R4(bb, cc, dd, aa, X[ 4],  8);
      R128_R4(aa, bb, cc, dd, X[13],  9);
      R128_R4(dd, aa, bb, cc, X[ 3], 14);
      R128_R4(cc, dd, aa, bb, X[ 7],  5);
      R128_R4(bb, cc, dd, aa, X[15],  6);
      R128_R4(aa, bb, cc, dd, X[14],  8);
      R128_R4(dd, aa, bb, cc, X[ 5],  6);
      R128_R4(cc, dd, aa, bb, X[ 6],  5);
      R128_R4(bb, cc, dd, aa, X[ 2], 12);

      /* parallel round 1 */
      R128_PR1(aaa, bbb, ccc, ddd, X[ 5],  8); 
      R128_PR1(ddd, aaa, bbb, ccc, X[14],  9);
      R128_PR1(ccc, ddd, aaa, bbb, X[ 7],  9);
      R128_PR1(bbb, ccc, ddd, aaa, X[ 0], 11);
      R128_PR1(aaa, bbb, ccc, ddd, X[ 9], 13);
      R128_PR1(ddd, aaa, bbb, ccc, X[ 2], 15);
      R128_PR1(ccc, ddd, aaa, bbb, X[11], 15);
      R128_PR1(bbb, ccc, ddd, aaa, X[ 4],  5);
      R128_PR1(aaa, bbb, ccc, ddd, X[13],  7);
      R128_PR1(ddd, aaa, bbb, ccc, X[ 6],  7);
      R128_PR1(ccc, ddd, aaa, bbb, X[15],  8);
      R128_PR1(bbb, ccc, ddd, aaa, X[ 8], 11);
      R128_PR1(aaa, bbb, ccc, ddd, X[ 1], 14);
      R128_PR1(ddd, aaa, bbb, ccc, X[10], 14);
      R128_PR1(ccc, ddd, aaa, bbb, X[ 3], 12);
      R128_PR1(bbb, ccc, ddd, aaa, X[12],  6);

      /* parallel round 2 */
      R128_PR2(aaa, bbb, ccc, ddd, X[ 6],  9);
      R128_PR2(ddd, aaa, bbb, ccc, X[11], 13);
      R128_PR2(ccc, ddd, aaa, bbb, X[ 3], 15);
      R128_PR2(bbb, ccc, ddd, aaa, X[ 7],  7);
      R128_PR2(aaa, bbb, ccc, ddd, X[ 0], 12);
      R128_PR2(ddd, aaa, bbb, ccc, X[13],  8);
      R128_PR2(ccc, ddd, aaa, bbb, X[ 5],  9);
      R128_PR2(bbb, ccc, ddd, aaa, X[10], 11);
      R128_PR2(aaa, bbb, ccc, ddd, X[14],  7);
      R128_PR2(ddd, aaa, bbb, ccc, X[15],  7);
      R128_PR2(ccc, ddd, aaa, bbb, X[ 8], 12);
      R128_PR2(bbb, ccc, ddd, aaa, X[12],  7);
      R128_PR2(aaa, bbb, ccc, ddd, X[ 4],  6);
      R128_PR2(ddd, aaa, bbb, ccc, X[ 9], 15);
      R128_PR2(ccc, ddd, aaa, bbb, X[ 1], 13);
      R128_PR2(bbb, ccc, ddd, aaa, X[ 2], 11);

      /* parallel round 3 */   
      R128_PR3(aaa, bbb, ccc, ddd, X[15],  9);
      R128_PR3(ddd, aaa, bbb, ccc, X[ 5],  7);
      R128_PR3(ccc, ddd, aaa, bbb, X[ 1], 15);
      R128_PR3(bbb, ccc, ddd, aaa, X[ 3], 11);
      R128_PR3(aaa, bbb, ccc, ddd, X[ 7],  8);
      R128_PR3(ddd, aaa, bbb, ccc, X[14],  6);
      R128_PR3(ccc, ddd, aaa, bbb, X[ 6],  6);
      R128_PR3(bbb, ccc, ddd, aaa, X[ 9], 14);
      R128_PR3(aaa, bbb, ccc, ddd, X[11], 12);
      R128_PR3(ddd, aaa, bbb, ccc, X[ 8], 13);
      R128_PR3(ccc, ddd, aaa, bbb, X[12],  5);
      R128_PR3(bbb, ccc, ddd, aaa, X[ 2], 14);
      R128_PR3(aaa, bbb, ccc, ddd, X[10], 13);
      R128_PR3(ddd, aaa, bbb, ccc, X[ 0], 13);
      R128_PR3(ccc, ddd, aaa, bbb, X[ 4],  7);
      R128_PR3(bbb, ccc, ddd, aaa, X[13],  5);

      /* parallel round 4 */
      R128_PR4(aaa, bbb, ccc, ddd, X[ 8], 15);
      R128_PR4(ddd, aaa, bbb, ccc, X[ 6],  5);
      R128_PR4(ccc, ddd, aaa, bbb, X[ 4],  8);
      R128_PR4(bbb, ccc, ddd, aaa, X[ 1], 11);
      R128_PR4(aaa, bbb, ccc, ddd, X[ 3], 14);
      R128_PR4(ddd, aaa, bbb, ccc, X[11], 14);
      R128_PR4(ccc, ddd, aaa, bbb, X[15],  6);
      R128_PR4(bbb, ccc, ddd, aaa, X[ 0], 14);
      R128_PR4(aaa, bbb, ccc, ddd, X[ 5],  6);
      R128_PR4(ddd, aaa, bbb, ccc, X[12],  9);
      R128_PR4(ccc, ddd, aaa, bbb, X[ 2], 12);
      R128_PR4(bbb, ccc, ddd, aaa, X[13],  9);
      R128_PR4(aaa, bbb, ccc, ddd, X[ 9], 12);
      R128_PR4(ddd, aaa, bbb, ccc, X[ 7],  5);
      R128_PR4(ccc, ddd, aaa, bbb, X[10], 15);
      R128_PR4(bbb, ccc, ddd, aaa, X[14],  8);

      /* combine results */
      ddd += cc + ripem_[1];
      ripem_[1] = ripem_[2] + dd + aaa;
      ripem_[2] = ripem_[3] + aa + bbb;
      ripem_[3] = ripem_[0] + bb + ccc;
      ripem_[0] = ddd;
    }

    std::string ripem128(const std::string & buffer) {
      ripem128sum r128;
      r128.update(buffer);
      r128.finalize();

      return r128.hexdigest();
    }

    // -- ripem160 ------------------------------------------------------------

#define R160_R1(a, b, c, d, e, x, s) {\
  (a) += R_B1((b), (c), (d)) + (x);\
  (a) = ROL((a), (s)) + (e);\
  (c) = ROL((c), 10);\
}
#define R160_R2(a, b, c, d, e, x, s) {\
  (a) += R_B2((b), (c), (d)) + (x) + 0x5a827999;\
  (a) = ROL((a), (s)) + (e);\
  (c) = ROL((c), 10);\
}
#define R160_R3(a, b, c, d, e, x, s) {\
  (a) += R_B3((b), (c), (d)) + (x) + 0x6ed9eba1;\
  (a) = ROL((a), (s)) + (e);\
  (c) = ROL((c), 10);\
}
#define R160_R4(a, b, c, d, e, x, s) {\
  (a) += R_B4((b), (c), (d)) + (x) + 0x8f1bbcdc;\
  (a) = ROL((a), (s)) + (e);\
  (c) = ROL((c), 10);\
}
#define R160_R5(a, b, c, d, e, x, s) {\
  (a) += R_B5((b), (c), (d)) + (x) + 0xa953fd4e;\
  (a) = ROL((a), (s)) + (e);\
  (c) = ROL((c), 10);\
}
#define R160_PR5(a, b, c, d, e, x, s) {\
  (a) += R_B1((b), (c), (d)) + (x);\
  (a) = ROL((a), (s)) + (e);\
  (c) = ROL((c), 10);\
}
#define R160_PR4(a, b, c, d, e, x, s) {\
  (a) += R_B2((b), (c), (d)) + (x) + 0x7a6d76e9;\
  (a) = ROL((a), (s)) + (e);\
  (c) = ROL((c), 10);\
}
#define R160_PR3(a, b, c, d, e, x, s) {\
  (a) += R_B3((b), (c), (d)) + (x) + 0x6d703ef3;\
  (a) = ROL((a), (s)) + (e);\
  (c) = ROL((c), 10);\
}
#define R160_PR2(a, b, c, d, e, x, s) {\
  (a) += R_B4((b), (c), (d)) + (x) + 0x5c4dd124;\
  (a) = ROL((a), (s)) + (e);\
  (c) = ROL((c), 10);\
}
#define R160_PR1(a, b, c, d, e, x, s) {\
  (a) += R_B5((b), (c), (d)) + (x) + 0x50a28be6;\
  (a) = ROL((a), (s)) + (e);\
  (c) = ROL((c), 10);\
}

    ripem160sum::ripem160sum() : ripem(160) {
      ripem_[0] = 0x67452301;
      ripem_[1] = 0xefcdab89;
      ripem_[2] = 0x98badcfe;
      ripem_[3] = 0x10325476;
      ripem_[4] = 0xc3d2e1f0;
    }

    void ripem160sum::compress(unsigned int *X) {
      unsigned int aa = ripem_[0],  bb = ripem_[1],  cc = ripem_[2], dd = ripem_[3],  ee = ripem_[4];
      unsigned int aaa = ripem_[0], bbb = ripem_[1], ccc = ripem_[2], ddd = ripem_[3], eee = ripem_[4];

      /* round 1 */
      R160_R1(aa, bb, cc, dd, ee, X[ 0], 11);
      R160_R1(ee, aa, bb, cc, dd, X[ 1], 14);
      R160_R1(dd, ee, aa, bb, cc, X[ 2], 15);
      R160_R1(cc, dd, ee, aa, bb, X[ 3], 12);
      R160_R1(bb, cc, dd, ee, aa, X[ 4],  5);
      R160_R1(aa, bb, cc, dd, ee, X[ 5],  8);
      R160_R1(ee, aa, bb, cc, dd, X[ 6],  7);
      R160_R1(dd, ee, aa, bb, cc, X[ 7],  9);
      R160_R1(cc, dd, ee, aa, bb, X[ 8], 11);
      R160_R1(bb, cc, dd, ee, aa, X[ 9], 13);
      R160_R1(aa, bb, cc, dd, ee, X[10], 14);
      R160_R1(ee, aa, bb, cc, dd, X[11], 15);
      R160_R1(dd, ee, aa, bb, cc, X[12],  6);
      R160_R1(cc, dd, ee, aa, bb, X[13],  7);
      R160_R1(bb, cc, dd, ee, aa, X[14],  9);
      R160_R1(aa, bb, cc, dd, ee, X[15],  8);

      /* round 2 */
      R160_R2(ee, aa, bb, cc, dd, X[ 7],  7);
      R160_R2(dd, ee, aa, bb, cc, X[ 4],  6);
      R160_R2(cc, dd, ee, aa, bb, X[13],  8);
      R160_R2(bb, cc, dd, ee, aa, X[ 1], 13);
      R160_R2(aa, bb, cc, dd, ee, X[10], 11);
      R160_R2(ee, aa, bb, cc, dd, X[ 6],  9);
      R160_R2(dd, ee, aa, bb, cc, X[15],  7);
      R160_R2(cc, dd, ee, aa, bb, X[ 3], 15);
      R160_R2(bb, cc, dd, ee, aa, X[12],  7);
      R160_R2(aa, bb, cc, dd, ee, X[ 0], 12);
      R160_R2(ee, aa, bb, cc, dd, X[ 9], 15);
      R160_R2(dd, ee, aa, bb, cc, X[ 5],  9);
      R160_R2(cc, dd, ee, aa, bb, X[ 2], 11);
      R160_R2(bb, cc, dd, ee, aa, X[14],  7);
      R160_R2(aa, bb, cc, dd, ee, X[11], 13);
      R160_R2(ee, aa, bb, cc, dd, X[ 8], 12);

      /* round 3 */
      R160_R3(dd, ee, aa, bb, cc, X[ 3], 11);
      R160_R3(cc, dd, ee, aa, bb, X[10], 13);
      R160_R3(bb, cc, dd, ee, aa, X[14],  6);
      R160_R3(aa, bb, cc, dd, ee, X[ 4],  7);
      R160_R3(ee, aa, bb, cc, dd, X[ 9], 14);
      R160_R3(dd, ee, aa, bb, cc, X[15],  9);
      R160_R3(cc, dd, ee, aa, bb, X[ 8], 13);
      R160_R3(bb, cc, dd, ee, aa, X[ 1], 15);
      R160_R3(aa, bb, cc, dd, ee, X[ 2], 14);
      R160_R3(ee, aa, bb, cc, dd, X[ 7],  8);
      R160_R3(dd, ee, aa, bb, cc, X[ 0], 13);
      R160_R3(cc, dd, ee, aa, bb, X[ 6],  6);
      R160_R3(bb, cc, dd, ee, aa, X[13],  5);
      R160_R3(aa, bb, cc, dd, ee, X[11], 12);
      R160_R3(ee, aa, bb, cc, dd, X[ 5],  7);
      R160_R3(dd, ee, aa, bb, cc, X[12],  5);

      /* round 4 */
      R160_R4(cc, dd, ee, aa, bb, X[ 1], 11);
      R160_R4(bb, cc, dd, ee, aa, X[ 9], 12);
      R160_R4(aa, bb, cc, dd, ee, X[11], 14);
      R160_R4(ee, aa, bb, cc, dd, X[10], 15);
      R160_R4(dd, ee, aa, bb, cc, X[ 0], 14);
      R160_R4(cc, dd, ee, aa, bb, X[ 8], 15);
      R160_R4(bb, cc, dd, ee, aa, X[12],  9);
      R160_R4(aa, bb, cc, dd, ee, X[ 4],  8);
      R160_R4(ee, aa, bb, cc, dd, X[13],  9);
      R160_R4(dd, ee, aa, bb, cc, X[ 3], 14);
      R160_R4(cc, dd, ee, aa, bb, X[ 7],  5);
      R160_R4(bb, cc, dd, ee, aa, X[15],  6);
      R160_R4(aa, bb, cc, dd, ee, X[14],  8);
      R160_R4(ee, aa, bb, cc, dd, X[ 5],  6);
      R160_R4(dd, ee, aa, bb, cc, X[ 6],  5);
      R160_R4(cc, dd, ee, aa, bb, X[ 2], 12);

      /* round 5 */
      R160_R5(bb, cc, dd, ee, aa, X[ 4],  9);
      R160_R5(aa, bb, cc, dd, ee, X[ 0], 15);
      R160_R5(ee, aa, bb, cc, dd, X[ 5],  5);
      R160_R5(dd, ee, aa, bb, cc, X[ 9], 11);
      R160_R5(cc, dd, ee, aa, bb, X[ 7],  6);
      R160_R5(bb, cc, dd, ee, aa, X[12],  8);
      R160_R5(aa, bb, cc, dd, ee, X[ 2], 13);
      R160_R5(ee, aa, bb, cc, dd, X[10], 12);
      R160_R5(dd, ee, aa, bb, cc, X[14],  5);
      R160_R5(cc, dd, ee, aa, bb, X[ 1], 12);
      R160_R5(bb, cc, dd, ee, aa, X[ 3], 13);
      R160_R5(aa, bb, cc, dd, ee, X[ 8], 14);
      R160_R5(ee, aa, bb, cc, dd, X[11], 11);
      R160_R5(dd, ee, aa, bb, cc, X[ 6],  8);
      R160_R5(cc, dd, ee, aa, bb, X[15],  5);
      R160_R5(bb, cc, dd, ee, aa, X[13],  6);

      /* parallel round 1 */
      R160_PR1(aaa, bbb, ccc, ddd, eee, X[ 5],  8);
      R160_PR1(eee, aaa, bbb, ccc, ddd, X[14],  9);
      R160_PR1(ddd, eee, aaa, bbb, ccc, X[ 7],  9);
      R160_PR1(ccc, ddd, eee, aaa, bbb, X[ 0], 11);
      R160_PR1(bbb, ccc, ddd, eee, aaa, X[ 9], 13);
      R160_PR1(aaa, bbb, ccc, ddd, eee, X[ 2], 15);
      R160_PR1(eee, aaa, bbb, ccc, ddd, X[11], 15);
      R160_PR1(ddd, eee, aaa, bbb, ccc, X[ 4],  5);
      R160_PR1(ccc, ddd, eee, aaa, bbb, X[13],  7);
      R160_PR1(bbb, ccc, ddd, eee, aaa, X[ 6],  7);
      R160_PR1(aaa, bbb, ccc, ddd, eee, X[15],  8);
      R160_PR1(eee, aaa, bbb, ccc, ddd, X[ 8], 11);
      R160_PR1(ddd, eee, aaa, bbb, ccc, X[ 1], 14);
      R160_PR1(ccc, ddd, eee, aaa, bbb, X[10], 14);
      R160_PR1(bbb, ccc, ddd, eee, aaa, X[ 3], 12);
      R160_PR1(aaa, bbb, ccc, ddd, eee, X[12],  6);

      /* parallel round 2 */
      R160_PR2(eee, aaa, bbb, ccc, ddd, X[ 6],  9); 
      R160_PR2(ddd, eee, aaa, bbb, ccc, X[11], 13);
      R160_PR2(ccc, ddd, eee, aaa, bbb, X[ 3], 15);
      R160_PR2(bbb, ccc, ddd, eee, aaa, X[ 7],  7);
      R160_PR2(aaa, bbb, ccc, ddd, eee, X[ 0], 12);
      R160_PR2(eee, aaa, bbb, ccc, ddd, X[13],  8);
      R160_PR2(ddd, eee, aaa, bbb, ccc, X[ 5],  9);
      R160_PR2(ccc, ddd, eee, aaa, bbb, X[10], 11);
      R160_PR2(bbb, ccc, ddd, eee, aaa, X[14],  7);
      R160_PR2(aaa, bbb, ccc, ddd, eee, X[15],  7);
      R160_PR2(eee, aaa, bbb, ccc, ddd, X[ 8], 12);
      R160_PR2(ddd, eee, aaa, bbb, ccc, X[12],  7);
      R160_PR2(ccc, ddd, eee, aaa, bbb, X[ 4],  6);
      R160_PR2(bbb, ccc, ddd, eee, aaa, X[ 9], 15);
      R160_PR2(aaa, bbb, ccc, ddd, eee, X[ 1], 13);
      R160_PR2(eee, aaa, bbb, ccc, ddd, X[ 2], 11);

      /* parallel round 3 */
      R160_PR3(ddd, eee, aaa, bbb, ccc, X[15],  9);
      R160_PR3(ccc, ddd, eee, aaa, bbb, X[ 5],  7);
      R160_PR3(bbb, ccc, ddd, eee, aaa, X[ 1], 15);
      R160_PR3(aaa, bbb, ccc, ddd, eee, X[ 3], 11);
      R160_PR3(eee, aaa, bbb, ccc, ddd, X[ 7],  8);
      R160_PR3(ddd, eee, aaa, bbb, ccc, X[14],  6);
      R160_PR3(ccc, ddd, eee, aaa, bbb, X[ 6],  6);
      R160_PR3(bbb, ccc, ddd, eee, aaa, X[ 9], 14);
      R160_PR3(aaa, bbb, ccc, ddd, eee, X[11], 12);
      R160_PR3(eee, aaa, bbb, ccc, ddd, X[ 8], 13);
      R160_PR3(ddd, eee, aaa, bbb, ccc, X[12],  5);
      R160_PR3(ccc, ddd, eee, aaa, bbb, X[ 2], 14);
      R160_PR3(bbb, ccc, ddd, eee, aaa, X[10], 13);
      R160_PR3(aaa, bbb, ccc, ddd, eee, X[ 0], 13);
      R160_PR3(eee, aaa, bbb, ccc, ddd, X[ 4],  7);
      R160_PR3(ddd, eee, aaa, bbb, ccc, X[13],  5);

      /* parallel round 4 */   
      R160_PR4(ccc, ddd, eee, aaa, bbb, X[ 8], 15);
      R160_PR4(bbb, ccc, ddd, eee, aaa, X[ 6],  5);
      R160_PR4(aaa, bbb, ccc, ddd, eee, X[ 4],  8);
      R160_PR4(eee, aaa, bbb, ccc, ddd, X[ 1], 11);
      R160_PR4(ddd, eee, aaa, bbb, ccc, X[ 3], 14);
      R160_PR4(ccc, ddd, eee, aaa, bbb, X[11], 14);
      R160_PR4(bbb, ccc, ddd, eee, aaa, X[15],  6);
      R160_PR4(aaa, bbb, ccc, ddd, eee, X[ 0], 14);
      R160_PR4(eee, aaa, bbb, ccc, ddd, X[ 5],  6);
      R160_PR4(ddd, eee, aaa, bbb, ccc, X[12],  9);
      R160_PR4(ccc, ddd, eee, aaa, bbb, X[ 2], 12);
      R160_PR4(bbb, ccc, ddd, eee, aaa, X[13],  9);
      R160_PR4(aaa, bbb, ccc, ddd, eee, X[ 9], 12);
      R160_PR4(eee, aaa, bbb, ccc, ddd, X[ 7],  5);
      R160_PR4(ddd, eee, aaa, bbb, ccc, X[10], 15);
      R160_PR4(ccc, ddd, eee, aaa, bbb, X[14],  8);

      /* parallel round 5 */
      R160_PR5(bbb, ccc, ddd, eee, aaa, X[12] ,  8);
      R160_PR5(aaa, bbb, ccc, ddd, eee, X[15] ,  5);
      R160_PR5(eee, aaa, bbb, ccc, ddd, X[10] , 12);
      R160_PR5(ddd, eee, aaa, bbb, ccc, X[ 4] ,  9);
      R160_PR5(ccc, ddd, eee, aaa, bbb, X[ 1] , 12);
      R160_PR5(bbb, ccc, ddd, eee, aaa, X[ 5] ,  5);
      R160_PR5(aaa, bbb, ccc, ddd, eee, X[ 8] , 14);
      R160_PR5(eee, aaa, bbb, ccc, ddd, X[ 7] ,  6);
      R160_PR5(ddd, eee, aaa, bbb, ccc, X[ 6] ,  8);
      R160_PR5(ccc, ddd, eee, aaa, bbb, X[ 2] , 13);
      R160_PR5(bbb, ccc, ddd, eee, aaa, X[13] ,  6);
      R160_PR5(aaa, bbb, ccc, ddd, eee, X[14] ,  5);
      R160_PR5(eee, aaa, bbb, ccc, ddd, X[ 0] , 15);
      R160_PR5(ddd, eee, aaa, bbb, ccc, X[ 3] , 13);
      R160_PR5(ccc, ddd, eee, aaa, bbb, X[ 9] , 11);
      R160_PR5(bbb, ccc, ddd, eee, aaa, X[11] , 11);

      /* combine results */
      ddd += cc + ripem_[1];
      ripem_[1] = ripem_[2] + dd + eee;
      ripem_[2] = ripem_[3] + ee + aaa;
      ripem_[3] = ripem_[4] + aa + bbb;
      ripem_[4] = ripem_[0] + bb + ccc;
      ripem_[0] = ddd;
    }

    std::string ripem160(const std::string & buffer) {
      ripem160sum r160;
      r160.update(buffer);
      r160.finalize();

      return r160.hexdigest();
    }
  }
}
