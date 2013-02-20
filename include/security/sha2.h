#ifndef __MGZ_SECURITY_SHA2_H
#define __MGZ_SECURITY_SHA2_H

#include <string>
#include <vector>
#include <stdint.h>
#include "mgz/export.h"

#define SHA224_SIZE 224/8
#define SHA256_SIZE 256/8
#define SHA384_SIZE 384/8
#define SHA512_SIZE 512/8

namespace mgz {
  namespace security {
    class sha2common {
      protected:
        void store32(unsigned int x, unsigned char* y);
        int load32(const unsigned char* y);
        void store64(unsigned long long x, unsigned char* y);
        unsigned long long load64(const unsigned char* y);
        std::string hexdigest(const unsigned char *data, size_t size);

      public:
        virtual std::string hexdigest() = 0;
    };

    template <typename T> class sha2sum {
      public:
        sha2sum() {
          sha_.init();
        }
        void update(const unsigned char *src, size_t length) {
          sha_.update(src, length);
        }
        void update(const char *buf, size_t length) {
          update((const unsigned char*)buf, length);
        }
        void update(const std::string & buffer) {
          update(buffer.c_str(), buffer.size());
        }
        void update(const std::vector<unsigned char> & buffer) {
          update(&buffer[0], buffer.size());
        }
        void update(const std::vector<char> & buffer) {
          update(&buffer[0], buffer.size());
        }
        void update(const std::vector<unsigned char> & buffer, size_t length) {
          update(&buffer[0], length);
        }
        void update(const std::vector<char> & buffer, size_t length) {
          update(&buffer[0], length);
        }
        void finalize() {
          sha_.finalize();
        }
        std::string hexdigest() {
          return sha_.hexdigest();
        }
      private:
        T sha_;
    };

    class sha224256 : public sha2common {
      public:
        virtual void init() = 0;
        void update(const unsigned char *src, size_t length);
        void finalize(void *out);
        virtual void finalize() = 0;
        std::string hexdigest() = 0;

      private:
        void sha_compress(const unsigned char* buf);

      protected:
        int hexsize;
        unsigned long long length;
        unsigned int state[8];
        unsigned int curlen;
        unsigned char buf[64];
    };

    class sha224interface : public sha224256 {
      public:
        void init();
        void finalize();
        std::string hexdigest();

      private:
        unsigned char data_[SHA224_SIZE];
    };

    class MGZ_API sha224sum : public sha2sum<sha224interface> {
      public: 
        sha224sum() : sha2sum<sha224interface>() {};
    };

    MGZ_API std::string __cdecl sha224(const std::string & data);

    class sha256interface : public sha224256 {
      public:
        void init();
        void finalize();
        std::string hexdigest();

      private:
        unsigned char data_[SHA256_SIZE];
    };

    class MGZ_API sha256sum : public sha2sum<sha256interface> {
      public:
        sha256sum() : sha2sum<sha256interface>() {};
    };

    MGZ_API std::string __cdecl sha256(const std::string & data);

    class sha384512 : public sha2common {
      public:
        virtual void init() = 0;
        void update(const unsigned char *src, size_t length);
        void finalize(void *out);
        virtual void finalize() = 0;
        std::string hexdigest() = 0;

      private:
        void sha_compress(const unsigned char* buf);

      protected:
        int hexsize;
        unsigned long long length;
        unsigned long long state[8];
        unsigned int curlen;
        unsigned char buf[128];
    };

    class sha384interface : public sha384512 {
      public:
        void init();
        void finalize();
        std::string hexdigest();

      private:
        unsigned char data_[SHA384_SIZE];
    };

    class MGZ_API sha384sum : public sha2sum<sha384interface> {
      public: 
        sha384sum() : sha2sum<sha384interface>() {};
    };

    MGZ_API std::string __cdecl sha384(const std::string & data);

    class sha512interface : public sha384512 {
      public:
        void init();
        void finalize();
        std::string hexdigest();

      private:
        unsigned char data_[SHA512_SIZE];
    };

    class MGZ_API sha512sum : public sha2sum<sha512interface> {
      public:
        sha512sum() : sha2sum<sha512interface>() {};
    };

    MGZ_API std::string __cdecl sha512(const std::string & data);
  }
}

#endif // __MGZ_SECURITY_SHA2_H

