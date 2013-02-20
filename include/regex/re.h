#ifndef __MGZ_REGEX_RE_H
#define __MGZ_REGEX_RE_H

/*!
 * \file regex/re.h
 * \brief Simple regex class
 * \author Grégoire Lejeune
 */
#include "mgz/export.h"
#include "regex/trex.h"
#include <string>
#include <vector>

/*! \namespace mgz
 * 
 * \brief Root namespace for all mgz specifics functions and classes
 */
namespace mgz {
  /*! \namespace mgz::regex
   *
   * \brief Namespace for mgz regex tools
   */ 
  namespace regex {
    class MGZ_API RegexException {};
    /*!
     * \class mgz::regex::RE
     * \brief Simple regex class
     */
    class MGZ_API RE {
      struct capture_t {
        size_t pos;
        size_t len;
        std::string capture;
        bool replaced;
      };

      public:
        /*!
         * \brief Create a RE object with the given pattern
         * \param pattern : The String pattern
         */
        RE(std::string pattern);
        ~RE();
        /*!
         * \brief Return true if the given string match with the pattern
         * \return True if the pattern match, false otherwise
         */
        bool find(std::string data);
        /*!
         * \brief Return the number of captures in the last matching (see mgz::return::RE::find())
         * \return The number of captures
         */
        size_t captures();
        /*!
         * \brief Return the capture at position i
         * \param i : The capture number.
         * \return The string of the corresponding capture
         */
        std::string match(size_t i);
        /*!
         * \brief Return the capture at position i
         * \param i : The capture number.
         * \return The string of the corresponding capture
         */
        std::string operator[](size_t i);
        std::string replace(size_t i, std::string repl);
        std::string replace(size_t i, size_t j);
        std::string replace_all(std::string repl);
        std::string replace_all(size_t j);
        std::string undo(size_t i);
        std::string undo_all();

        static std::string quote(std::string pattern);

      private:
        bool find_matchings(const TRexChar * text_begin, const TRexChar * text_end); 
        void repos(size_t pos, int corr);

      private:
        std::string pattern_;
        char * data_c_;
        std::string data_;
        TRex * preg_;
        std::vector<capture_t> captures_;
    };
  }
}

#endif // __MGZ_REGEX_RE_H
