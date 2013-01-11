#include "security/adler32.h"

#define BASE 65521

namespace mgz {
  namespace security {
    adler32sum::adler32sum() : adler(1L) {}

    void adler32sum::update(const unsigned char *buf, size_t length) {
      unsigned long s1 = adler & 0xffff;
      unsigned long s2 = (adler >> 16) & 0xffff;
      int n;
      for (n = 0; n < length; n++) {
        s1 = (s1 + buf[n]) % BASE;
        s2 = (s2 + s1) % BASE;
      }
      adler = (s2 << 16) + s1;
    }
    void adler32sum::update(const char *buf, size_t length) {
      update((const unsigned char*)buf, length);
    }
    void adler32sum::update(const std::string & buffer) {
      update(buffer.c_str(), buffer.size());
    }
    void adler32sum::update(const std::vector<unsigned char> & buffer) {
      update(&buffer[0], buffer.size());
    }
    void adler32sum::update(const std::vector<char> & buffer) {
      update(&buffer[0], buffer.size());
    }
    void adler32sum::update(const std::vector<unsigned char> & buffer, size_t length) {
      update(&buffer[0], length);
    }
    void adler32sum::update(const std::vector<char> & buffer, size_t length) {
      update(&buffer[0], length);
    }

    adler32sum& adler32sum::finalize() {
      /* just for compatibility */
      return *this;
    }

    std::string adler32sum::hexdigest() const {
      char buf[sizeof(adler32_t)+3] = { 0 };
      sprintf(buf, "0x%lx", adler);
      return std::string(buf);
    }

    std::ostream& operator<<(std::ostream& out, adler32sum adler32) {
      return out << adler32.adler;
    }

    adler32_t adler32(const std::string & buffer) {
      adler32sum a;
      a.update(buffer);
      a.finalize();

      return a.adler;
    }
  }
}
