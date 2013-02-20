#ifndef __MGZ_NET_URI_H
#define __MGZ_NET_URI_H

/*!
 * \file net/uri.h
 * \brief URI manipulations
 * \author Grégoire Lejeune
 */
#include <map>
#include <string>
#include "mgz/export.h"
// scheme://[username[:password]@][host[:port]/path[/path]*[?key=value[&key=value[&...]]]

/*! \namespace mgz
 * 
 * \brief Root namespace for all mgz specifics functions and classes
 */
namespace mgz {
  /*! \namespace mgz::net
   *
   * \brief Namespace for mgz network tools
   */ 
  namespace net {
    /*!
     * \class mgz::net::uri
     * \brief Class that parse std::string to URI
     */
    class MGZ_API uri {
      public:
        /*!
         * \brief Create an empty uri object
         */
        uri();
        /*!
         * \brief Create an uri object with the given string
         * \param str : A std::string
         */
        uri(const std::string & str);
        /*!
         * \brief Create a new uri object with the given string, and parse the string
         * \param str : A std::string 
         * \return A new uri object
         */
        static uri parse(const std::string & str);
        /*!
         * \brief Escapes the string, replacing all unsafe characters with codes.
         * \param str : The string to encode
         * \return The encoded string
         */
        static std::string encode(const std::string & str);
        /*!
         * \brief Unescape the string.
         * \param str : The encoded string
         * \return The unescaped string
         */
        static std::string decode(const std::string & str);

        /*!
         * \brief Parse the uri string
         * \return The current uri object
         */
        uri parse();
        /*!
         * \brief Escape the uri string, replacing all unsafe characters with codes.
         * \return The encoded string
         */
        std::string encode() const;
        /*!
         * \brief Unescape the uri string
         * \return The unescaped string
         */
        std::string decode() const;
        /*!
         * \brief Return the uri scheme
         * \return The uri sheme as string
         */
        const std::string scheme() const;
        /*!
         * \brief Return the uri host
         * \return The uri host as string
         */
        const std::string host() const;
        /*!
         * \brief Return the uri port
         * \return The uri port as string
         */
        const int port() const;
        /*!
         * \brief Return the uri path
         * \return The uri path as string
         */
        const std::string path() const;
        /*!
         * \brief Return the uri query
         * \return A map of the uri query pairs
         */
        std::map<std::string, std::string> query() const;
        /*!
         * \brief Return the uri query string
         * \return The uri query as string
         */
        const std::string query_string() const;
        /*!
         * \brief Return the uri username
         * \return The uri username as string
         */
        const std::string username() const;
        /*!
         * \brief Return the uri password
         * \return The uri password as string
         */
        const std::string password() const;

      private:
        std::string uri_;
        std::string scheme_;
        std::string host_;
        int port_;
        std::string path_;
        std::string query_string_;
        std::map<std::string, std::string> query_;
        std::string username_;
        std::string password_;
    };
  }
}

#endif // __MGZ_NET_URI_H

