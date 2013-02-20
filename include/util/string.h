#ifndef __MGZ_UTIL_STRING_H
#define __MGZ_UTIL_STRING_H

#include "mgz/export.h"
#include <string>
#include <vector>
#include <utility>
#include <istream>

namespace mgz {
  namespace util {
    MGZ_API std::string __cdecl trim(const std::string &str);
    MGZ_API std::string __cdecl squeeze(const std::string & str);
    MGZ_API std::vector<std::string>& __cdecl split(const std::string &s, char delim, std::vector<std::string> &elems);
    MGZ_API std::vector<std::string> __cdecl split(const std::string &s, char delim);
    MGZ_API std::string __cdecl join(const std::vector<std::string> &vector, char delim);
    MGZ_API std::string __cdecl replace_delim(const std::string &s, char i, char t);
    MGZ_API std::string __cdecl to_upper(const std::string &s);
    MGZ_API std::string __cdecl to_lower(const std::string &s);
    MGZ_API std::string __cdecl classify(const std::string &s);
    MGZ_API std::string __cdecl format(const std::string & format, ...);
    MGZ_API std::string __cdecl random_string(const int len, const std::string &prefix = "");

    MGZ_API void __cdecl replace_all(std::string& str, const std::string& from, const std::string& to);
    MGZ_API std::string __cdecl replace_all(const std::string &s, char i, char t);
    MGZ_API std::string __cdecl replace_all(const std::string & str, const std::string& from, const std::string& to);

    MGZ_API int __cdecl asInt(const std::string &intString);
    MGZ_API int __cdecl levenshtein_distance(const std::string &source, const std::string &target);

    enum soundex_lang {
      FR,
      EN
    };
    MGZ_API std::string __cdecl soundex(enum soundex_lang lang, const std::string & input, const int resultLength);

    class MGZ_API line {
      std::string data;
      public:
      friend std::istream &operator>>(std::istream &is, line &l) {
        std::getline(is, l.data);
        return is;
      }

      operator std::string() const { return data; }
    };

    MGZ_API std::string __cdecl pluralize(const std::string &singular);
    MGZ_API std::vector<std::string> __cdecl explode(const std::string &s, const std::string &e);

    enum cut_keep {
      KEEP_LEFT,
      KEEP_RIGHT,
      KEEP_BOOTH,
      KEEP_NONE
    };
    MGZ_API std::pair<std::string, std::string> __cdecl cut(const std::string &data, const std::string & sep, cut_keep keep = KEEP_NONE);

    MGZ_API std::string __cdecl red(const std::string & str);
    MGZ_API std::string __cdecl green(const std::string & str);
    MGZ_API std::string __cdecl yellow(const std::string & str);
    MGZ_API std::string __cdecl blue(const std::string & str);
    MGZ_API std::string __cdecl magenta(const std::string & str);
    MGZ_API std::string __cdecl cyan(const std::string & str);
  }
}
#endif // __MGZ_UTIL_STRING_H

