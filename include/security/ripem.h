#ifndef __MGZ_SECURITY_RIPEM_H
#define __MGZ_SECURITY_RIPEM_H

#include <string>
#include <vector>

namespace mgz {
  namespace security {
    class ripem {
      public:
        ripem(int size);

        void update(const unsigned char *buf, size_t length);
        void update(const char *buf, size_t length);
        void update(const std::string & buffer);
        void update(const std::vector<unsigned char> & buffer);
        void update(const std::vector<char> & buffer);
        void update(const std::vector<unsigned char> & buffer, size_t length);
        void update(const std::vector<char> & buffer, size_t length);

        void finalize();

        std::string hexdigest() const;

      private:
        void perform_update();
        virtual void compress(unsigned int *X) = 0;

      protected:
        int ripem_size_;
        unsigned char data_[1024];
        unsigned int current_data_size_;
        unsigned int data_size_;
        unsigned int ripem_[5];
    };

    class ripem128sum : public ripem {
      public:
        ripem128sum();

      private:
        void compress(unsigned int *X);
    };

    std::string ripem128(const std::string & buffer);

    class ripem160sum : public ripem {
      public:
        ripem160sum();

      private:
        void compress(unsigned int *X);
    };

    std::string ripem160(const std::string & buffer);
  }
}

#endif // __MGZ_SECURITY_RIPEM_H

