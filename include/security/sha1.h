#ifndef __MGZ_SECURITY_SHA1_H
#define __MGZ_SECURITY_SHA1_H

#include <string>
#include <iostream>
#include <stdint.h>
#include <cstdio>
#include <vector>

#define SHA_BLOCKSIZE  64

namespace mgz {
  namespace security {
    class sha1sum {
      public:
        sha1sum();

        void update(const unsigned char *buf, size_t length);
        void update(const char *buf, size_t length);
        void update(const std::string & buffer);
        void update(const std::vector<unsigned char> & buffer);
        void update(const std::vector<char> & buffer);
        void update(const std::vector<unsigned char> & buffer, size_t length);
        void update(const std::vector<char> & buffer, size_t length);

        sha1sum& finalize();

        std::string hexdigest() const;
        friend std::ostream& operator<<(std::ostream&, sha1sum sh1);

      private:
        void init();
        void process_message();

      private:
        unsigned int digest[5];
        unsigned int length_low;
        unsigned int length_hight;
        unsigned char data[SHA_BLOCKSIZE];
        int local;
    };

    std::string sha1(const std::string & data);
  }
}

#endif // __MGZ_SECURITY_SHA1_H

