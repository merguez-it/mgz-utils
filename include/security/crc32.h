#ifndef __MGZ_SECURITY_CRC32_H
#define __MGZ_SECURITY_CRC32_H

#include <string>
#include <iostream>
#include <stdint.h>
#include <cstdio>
#include <vector>

typedef unsigned long crc32_t;

namespace mgz {
  namespace security {
    class crc32sum {
      public:
        crc32sum();

        void update(const unsigned char *buf, size_t length);
        void update(const char *buf, size_t length);
        void update(const std::string & buffer);
        void update(const std::vector<unsigned char> & buffer);
        void update(const std::vector<char> & buffer);
        void update(const std::vector<unsigned char> & buffer, size_t length);
        void update(const std::vector<char> & buffer, size_t length);

        crc32sum& finalize();

        std::string hexdigest() const;
        friend std::ostream& operator<<(std::ostream&, crc32sum crc32);

      private:
        void init();

      public:
        crc32_t crc;
    };

    std::string crc32hex(const std::string & buffer);
    crc32_t crc32(const std::string & buffer);
  }
}

#endif // __MGZ_SECURITY_CRC32_H

