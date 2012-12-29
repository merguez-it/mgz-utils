#ifndef __MGZ_SECURITY_MD5_H
#define __MGZ_SECURITY_MD5_H

#include <string>
#include <iostream>
#include <stdint.h>
#include <cstdio>
#include <vector>

namespace mgz {
  namespace security {
    class md5sum {
      public:
        md5sum();

        void update(const unsigned char *buf, size_t length);
        void update(const char *buf, size_t length);
        void update(const std::string & buffer);
        void update(const std::vector<unsigned char> & buffer);
        void update(const std::vector<char> & buffer);
        void update(const std::vector<unsigned char> & buffer, size_t length);
        void update(const std::vector<char> & buffer, size_t length);

        md5sum& finalize();

        std::string hexdigest() const;
        friend std::ostream& operator<<(std::ostream&, md5sum md5);

      private:
        void init();
        typedef unsigned char uint1;
        typedef unsigned int uint4;
        enum {blocksize = 64};

        void transform(const uint1 block[blocksize]);
        static void decode(uint4 output[], const uint1 input[], size_t len);
        static void encode(uint1 output[], const uint4 input[], size_t len);

        bool finalized;
        uint1 buffer[blocksize];
        uint4 count[2];
        uint4 state[4];
        uint1 digest[16];

        static inline uint4 F(uint4 x, uint4 y, uint4 z);
        static inline uint4 G(uint4 x, uint4 y, uint4 z);
        static inline uint4 H(uint4 x, uint4 y, uint4 z);
        static inline uint4 I(uint4 x, uint4 y, uint4 z);
        static inline uint4 rotate_left(uint4 x, int n);
        static inline void FF(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
        static inline void GG(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
        static inline void HH(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
        static inline void II(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
    };

    std::string md5(const std::string str);
  }
}

#endif // __MGZ_SECURITY_MD5_H

