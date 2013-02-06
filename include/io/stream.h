#ifndef __MGZ_IO_STREAM_H
#define __MGZ_IO_STREAM_H

/*!
 * \file io/stream.h
 * \brief Extensions for the standard Input/Output library
 * \author Grégoire Lejeune
 */
#include <string>
#include <fstream>

/*! \namespace mgz
 *
 * \brief Root namespace for all mgz specifics functions and classes
 */
namespace mgz {
  /*! \namespace mgz::io
   *
   * \brief Namespace for mgz io tools
   */
  namespace io {
    /*!
     * \brief Extracts characters from is and stores them into str until 
     *        the newline characters are found (or the end of file)
     * \param is : istream object from which characters are extracted.
     * \param t : string object where the extracted line is stored.
     * \return The same as is
     */
    std::istream & get_line(std::istream & is, std::string & t);
  }
}
#endif // __MGZ_IO_STREAM_H

