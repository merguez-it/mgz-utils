#ifndef __MGZ_UTIL_VECTOR_H
#define __MGZ_UTIL_VECTOR_H

#include <vector>

namespace mgz {
  namespace util {
    template <typename T> class create_vector {
      private:
        std::vector<T> m_vec;
      public:
        create_vector(const T& val) {
          m_vec.push_back(val);
        }
        create_vector<T>& operator()(const T& val) {
          m_vec.push_back(val);
          return *this;
        }
        operator std::vector<T>() {
          return m_vec;
        }
    };
  }
}

#endif // __MGZ_UTIL_VECTOR_H

