#ifndef __MGZ_SECURITY_CRC32_H
#define __MGZ_SECURITY_CRC32_H

#include <string>
#include <iostream>
#include <stdint.h>
#include <cstdio>
#include <vector>
#include "mgz/export.h"

typedef unsigned long crc32_t;

namespace mgz {
  namespace security {
    class MGZ_API crc32sum {
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

    MGZ_API std::string __cdecl crc32hex(const std::string & buffer);
    MGZ_API crc32_t __cdecl crc32(const std::string & buffer);
  }
}

#endif // __MGZ_SECURITY_CRC32_H

