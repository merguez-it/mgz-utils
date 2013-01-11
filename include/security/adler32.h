#ifndef __MGZ_SECURITY_ADLER32_H
#define __MGZ_SECURITY_ADLER32_H

#include <string>
#include <iostream>
#include <stdint.h>
#include <cstdio>
#include <vector>

typedef unsigned long adler32_t;

namespace mgz {
  namespace security {
    class adler32sum {
      public:
        adler32sum();

        void update(const unsigned char *buf, size_t length);
        void update(const char *buf, size_t length);
        void update(const std::string & buffer);
        void update(const std::vector<unsigned char> & buffer);
        void update(const std::vector<char> & buffer);
        void update(const std::vector<unsigned char> & buffer, size_t length);
        void update(const std::vector<char> & buffer, size_t length);

        adler32sum& finalize();

        std::string hexdigest() const;
        friend std::ostream& operator<<(std::ostream&, adler32sum adler32);

      public:
        adler32_t adler;
    };

    adler32_t adler32(const std::string & buffer);
  }
}

#endif // __MGZ_SECURITY_ADLER32_H

