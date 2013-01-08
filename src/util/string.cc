#include "util/string.h"
#include "util/internal/varg.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <ctime>

namespace mgz {
  namespace util {
    std::string trim(const std::string &str) {
      std::string str_ = str;
      std::string::size_type pos = str_.find_last_not_of(' ');
      if(pos != std::string::npos) {
        str_.erase(pos + 1);
        pos = str_.find_first_not_of(' ');
        if(pos != std::string::npos) str_.erase(0, pos);
      } else {
        str_.erase(str_.begin(), str_.end());
      }
      return str_;
    }

    std::string squeeze(const std::string & str) {
      std::string result;
      bool keep = true;
      for(size_t i = 0; i < str.size(); i++) {
        char c = str[i];
        if(c == ' ' || c == '\t') {
          if(keep) {
            keep = false;
            result.append(1, ' ');
          }   
        } else {
          keep = true;
          result.append(1, c); 
        }   
      }   

      return result;
    }

    std::vector<std::string>& split(const std::string &s, char delim, std::vector<std::string> &elems) {
      std::stringstream ss(s);
      std::string item;
      while(std::getline(ss, item, delim)) {
        elems.push_back(trim(item));
      }
      return elems;
    }

    std::vector<std::string> split(const std::string &s, char delim) {
      std::vector<std::string> elems;
      return split(s, delim, elems);
    }

    std::string join(const std::vector<std::string> &vector, char delim) {
      std::string result;
      std::vector<std::string>::const_iterator it;
      for (it = vector.begin() ; it < vector.end(); it++) {
        if(!result.empty()) {
          result.append(1, delim);
        }
        result.append(*it);
      }
      return result;
    }

    std::string to_upper(const std::string &s) {
      std::string r = s;
      std::transform(r.begin(), r.end(), r.begin(), ::toupper);
      return r;
    }

    std::string to_lower(const std::string &s) {
      std::string r = s;
      std::transform(r.begin(), r.end(), r.begin(), ::tolower);
      return r;
    }

    std::string classify(const std::string &s) {
      std::string r("");
      std::vector<std::string> l = split(s, '_');
      std::vector<std::string>::iterator it; 
      for(it = l.begin(); it < l.end(); it++) {
        std::string p(to_lower(*it));
        p[0] = toupper(p[0]);
        r += p;
      }
      return r;
    }

    std::string format(const std::string &format, ...) {
      VARARGS_TO_STRING(format, str);
      return str;
    }

    std::string random_string(const int len, const std::string &prefix) {
      static bool init = false;
      if(!init) {
        srand(time(NULL));
        init = true;
      }
      std::string s(prefix);

      static const char alphanum[] = "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
      for (int i = 0; i < len; ++i) {
        s.append(1, alphanum[rand() % (sizeof(alphanum) - 1)]);
      }

      return s;
    }

    void replace_all(std::string& str, const std::string& from, const std::string& to) {
      size_t start_pos = 0;
      while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); 
      }
    }

    std::string replace_all(const std::string &s, char i, char t) {
      return replace_all(s,std::string(1,i),std::string(1,t));
    }

    std::string replace_all(const std::string & str, const std::string& from, const std::string& to) {
      std::string out(str);
      replace_all(out, from, to);
      return out;
    }

    int asInt(const std::string &intString) {
      std::stringstream s;
      int result;
      s << intString;
      s >> result;
      return result;
    }

    int levenshtein_distance(const std::string &source, const std::string &target) {
      const int n = source.length();
      const int m = target.length();
      if (n == 0) {
        return m;
      }
      if (m == 0) {
        return n;
      }

      typedef std::vector< std::vector<int> > Tmatrix; 
      Tmatrix matrix(n+1);

      for (int i = 0; i <= n; i++) {
        matrix[i].resize(m+1);
      }

      for (int i = 0; i <= n; i++) {
        matrix[i][0]=i;
      }

      for (int j = 0; j <= m; j++) {
        matrix[0][j]=j;
      }

      for (int i = 1; i <= n; i++) {
        const char s_i = source[i-1];

        for (int j = 1; j <= m; j++) {
          const char t_j = target[j-1];

          int cost;
          if (s_i == t_j) {
            cost = 0;
          } else {
            cost = 1;
          }

          const int above = matrix[i-1][j];
          const int left = matrix[i][j-1];
          const int diag = matrix[i-1][j-1];
          int cell = std::min( above + 1, std::min(left + 1, diag + cost));

          if (i>2 && j>2) {
            int trans = matrix[i-2][j-2]+1;
            if (source[i-2] != t_j) trans++;
            if (s_i != target[j-2]) trans++;
            if (cell > trans) cell = trans;
          }

          matrix[i][j] = cell;
        }
      }

      return matrix[n][m];
    }

    static char lookup_en[] = {
      '0',    /* A */
      '1',    /* B */
      '2',    /* C */
      '3',    /* D */
      '0',    /* E */
      '1',    /* F */
      '2',    /* G */
      '0',    /* H */
      '0',    /* I */
      '2',    /* J */
      '2',    /* K */
      '4',    /* L */
      '5',    /* M */
      '5',    /* N */
      '0',    /* O */
      '1',    /* P */
      '0',    /* Q */
      '6',    /* R */
      '2',    /* S */
      '3',    /* T */
      '0',    /* U */
      '1',    /* V */
      '0',    /* W */
      '2',    /* X */
      '0',    /* Y */
      '2',    /* Z */
    };

    static char lookup_fr[] = {
      '0',    /* A */
      '1',    /* B */
      '2',    /* C */
      '3',    /* D */
      '0',    /* E */
      '9',    /* F */
      '7',    /* G */
      '0',    /* H */
      '0',    /* I */
      '7',    /* J */
      '2',    /* K */
      '4',    /* L */
      '5',    /* M */
      '5',    /* N */
      '0',    /* O */
      '1',    /* P */
      '2',    /* Q */
      '6',    /* R */
      '8',    /* S */
      '3',    /* T */
      '0',    /* U */
      '9',    /* V */
      '0',    /* W */
      '8',    /* X */
      '0',    /* Y */
      '8',    /* Z */
    };

    std::string soundex(enum soundex_lang lang, const std::string & input, const size_t resultLength) {
      char *lookup;

      switch(lang) {
        case FR:
          lookup = lookup_fr;
          break;
        default:
          lookup = lookup_en;
      }

      std::string result = input.substr(0,1);

      for(size_t i=1; i < input.length(); i++){
        if(!isalpha(input[i])){
          continue;
        }

        const char lookupInput = islower(input[i]) ? toupper(input[i]) : input[i];
        const char *lookupVal = &lookup[lookupInput-'A'];

        if(result.find(lookupVal, 0) != 0 ){
          result.append(lookupVal);
        }
      }

      if(result.length() >= resultLength){
        return result.substr(0,resultLength-1);
      }

      return "Z000";
    }

#define MAP(suffix, plural) \
    if (singular.length() >= std::string(suffix).length()) { \
      if (singular.substr(singular.length() - std::string(suffix).length(), std::string(suffix).length()) == std::string(suffix)) { \
        return singular.substr(0, singular.length() - std::string(suffix).length()) + plural; \
      } \
    }

    /* NOTE: We assume all lower case! */
    std::string pluralize(const std::string &singular) {
      /* Irregular */
      MAP("person", "people");
      MAP("man", "men");
      MAP("child", "children");
      MAP("sex", "sexes");
      MAP("move", "moves");
      MAP("cow", "kine");
      MAP("zombie", "zombies");

      /* Mappings */
      MAP("quiz", "quizzes");
      if (singular == "oxen") {
        return "oxen";
      }
      if (singular == "ox") {
        return "oxen";
      }
      MAP("mice", "mice");
      MAP("lice", "lice");
      MAP("mouse", "mice");
      MAP("louse", "lice");
      MAP("matrix", "matrices");
      MAP("vertix", "vertices");
      MAP("indix", "indices");
      MAP("matrex", "matrices");
      MAP("vertex", "vertices");
      MAP("index", "indices");
      MAP("x", "xes");
      MAP("ch", "ches");
      MAP("ss", "sses");
      MAP("sh", "shes");
      if (singular.length() >= 2) {
        if (singular.substr(singular.length() - 1, 1) == "y") {
          if (singular.substr(singular.length() - 2, 1) != "a" &&
              singular.substr(singular.length() - 2, 1) != "e" &&
              singular.substr(singular.length() - 2, 1) != "i" &&
              singular.substr(singular.length() - 2, 1) != "o" &&
              singular.substr(singular.length() - 2, 1) != "u" &&
              singular.substr(singular.length() - 2, 1) != "y") {
            return singular.substr(0, singular.length() - 1) + "ies";
          }
        }
      }
      MAP("quy", "quies");
      MAP("hive", "hives");
      if (singular.length() >= 3) {
        if (singular.substr(singular.length() - 2, 2) == "fe") {
          if (singular.substr(singular.length() - 2, 1) != "f") {
            return singular.substr(0, singular.length() - 2) + "ves";
          }
        }
      }
      MAP("lf", "lves");
      MAP("rf", "rves");
      MAP("sis", "ses");
      MAP("ta", "ta");
      MAP("ia", "ia");
      MAP("tum", "ta");
      MAP("ium", "ia");
      MAP("buffalo", "buffaloes");
      MAP("tomato", "tomatoes");
      MAP("bus", "buses");
      MAP("alias", "aliases");
      MAP("status", "statuses");
      MAP("octopi", "octopii");
      MAP("viri", "virii");
      MAP("octopus", "octopuses");
      MAP("virus", "viruses");
      MAP("axis", "axes");
      MAP("testis", "testes");
      MAP("s", "s");
      return singular + "s";
    }

    std::vector<std::string> explode(const std::string &s, const std::string &e) {
      std::vector<std::string> ret;
      std::string s_ = s;
      int iPos = s_.find(e, 0);
      int iPit = e.length();
      while(iPos>-1) {
        if(iPos!=0)
          ret.push_back(s.substr(0,iPos));
        s_.erase(0,iPos+iPit);
        iPos = s_.find(e, 0);
      }
      if(s_!="")
        ret.push_back(s_);
      return ret;
    };

    std::pair<std::string, std::string> cut(const std::string &data, const std::string & sep, cut_keep keep) {
      std::string first_;
      std::string second_;
      int keep_left_ = 0;
      int keep_right_ = sep.size();

      switch(keep) {
        case KEEP_LEFT:
          keep_left_ = sep.size();
          keep_right_ = sep.size();
          break;
        case KEEP_RIGHT:
          keep_left_ = 0;
          keep_right_ = 0;
          break;
        case KEEP_BOOTH:
          keep_left_ = sep.size();
          keep_right_ = 0;
          break;
        case KEEP_NONE:
        default:
          keep_left_ = 0;
          keep_right_ = sep.size();
          break;
      }

      std::string::const_iterator pos_ = std::search(data.begin(), data.end(), sep.begin(), sep.end());
      unsigned int dis_ = std::distance(data.begin(), pos_);
      if(dis_ >= data.size()) {
        first_ = data;
        second_ = "";
      } else {
        first_ = std::string(data.begin(), data.begin()+dis_+keep_left_);
        second_ = std::string(data.begin()+dis_+keep_right_, data.end());
      }

      return std::make_pair(first_, second_);
    }

#define ANSI_COLOR_RED     std::string("\x1b[31m")
#define ANSI_COLOR_GREEN   std::string("\x1b[32m")
#define ANSI_COLOR_YELLOW  std::string("\x1b[33m")
#define ANSI_COLOR_BLUE    std::string("\x1b[34m")
#define ANSI_COLOR_MAGENTA std::string("\x1b[35m")
#define ANSI_COLOR_CYAN    std::string("\x1b[36m")
#define ANSI_COLOR_RESET   std::string("\x1b[0m")
    std::string red(const std::string & str) {
      return ANSI_COLOR_RED + str + ANSI_COLOR_RESET;
    }
    std::string green(const std::string & str) {
      return ANSI_COLOR_GREEN + str + ANSI_COLOR_RESET;
    }
    std::string yellow(const std::string & str) {
      return ANSI_COLOR_YELLOW + str + ANSI_COLOR_RESET;
    }
    std::string blue(const std::string & str) {
      return ANSI_COLOR_BLUE + str + ANSI_COLOR_RESET;
    }
    std::string magenta(const std::string & str) {
      return ANSI_COLOR_MAGENTA + str + ANSI_COLOR_RESET;
    }
    std::string cyan(const std::string & str) {
      return ANSI_COLOR_CYAN + str + ANSI_COLOR_RESET;
    }
  }
}
