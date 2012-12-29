#ifndef __MGZ_UTIL_STRING_H
#define __MGZ_UTIL_STRING_H

#include <string>
#include <vector>
#include <utility>

namespace mgz {
  namespace util {
    std::string trim(const std::string &str);
    std::string squeeze(const std::string & str);
    std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems);
    std::vector<std::string> split(const std::string &s, char delim);
    std::string join(const std::vector<std::string> &vector, char delim);
    std::string to_upper(const std::string &s); 
    std::string to_lower(const std::string &s); 
    std::string classify(const std::string &s); 
    std::string format(const std::string & format, ...);
    std::string random_string(const int len, const std::string &prefix = ""); 

    void replace_all(std::string& str, const std::string& from, const std::string& to);
    std::string replace_all(const std::string &s, char i, char t);
    std::string replace_all(const std::string & str, const std::string& from, const std::string& to);

    int asInt(const std::string &intString);
    int levenshtein_distance(const std::string &source, const std::string &target);

    enum soundex_lang { 
      FR,
      EN
    };
    std::string soundex(enum soundex_lang lang, const std::string & input, const int resultLength);

    class line {
      std::string data;
      public:
      friend std::istream &operator>>(std::istream &is, line &l) {
        std::getline(is, l.data);
        return is;
      }

      operator std::string() const { return data; }    
    };

    std::string pluralize(const std::string &singular);
    std::vector<std::string> explode(const std::string &s, const std::string &e);

    enum cut_keep {
      KEEP_LEFT,
      KEEP_RIGHT,
      KEEP_BOOTH,
      KEEP_NONE
    };
    std::pair<std::string, std::string> cut(const std::string &data, const std::string & sep, cut_keep keep = KEEP_NONE);
  }
}
#endif // __MGZ_UTIL_STRING_H

