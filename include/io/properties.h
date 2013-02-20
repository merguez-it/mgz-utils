#ifndef __MGZ_IO_PROPERTIES_H
#define __MGZ_IO_PROPERTIES_H
/*!
 * \file io/properties.h
 * \brief The Properties class represents a persistent set of properties.
 * \author Grégoire Lejeune
 * \version 0.1
 * \date mars 2012
 */

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <utility>
#include <algorithm>

#include "mgz/export.h"
#include "io/file.h"

#define PROPERTIES_SEPARATOR ','

class MGZ_API CantStorePropertiesException{};

/*! \namespace mgz
 * 
 * Root namespace for all mgz specifics functions ans classes
 */
namespace mgz {
  /*! \namespace mgz::io
   *
   * Namespace for mgz util tools
   */ 
  namespace io {
    
    class MGZ_API PropertyFileNotFoundException {};
    class MGZ_API PropertyFileOpenException {};
    
    class MGZ_API properties {
      public:
        /*!
         * \brief Create an empty property list
         */
        properties();
        /*!
         * \brief Create a property list by reading it from the input abstract pathname
         * \param file : The abstract pathname file
         */
        properties(mgz::io::file file);

        /*!
         * \brief Searches for the property with the specified key in this property list.
         * \param key : The property name
         * \return The property value, or an empty string
         */
        std::string get_property(std::string key);

        /*!
         * \brief Searches for the property with the specified key in this property list.
         * \param key : The property name
         * \param defaultValue : The default value of this property
         * \return The property value, or the default value
         */
        std::string get_property(std::string key, std::string defaultValue);

        /*!
         * \brief Searches for the properties with the specified key in this property list.
         * \param key : The property name
         * \return The property value list, or an empty list
         */
        std::vector<std::string> get_properties(std::string key);

        /*!
         * \brief Set the value for the given property
         * \param key : The property name
         * \param value : The property value
         * \return The old property value, or an empty string
         */
        std::string set_property(std::string key, std::string value);

        /*!
         * \brief Set a list of value for the given property
         * \param key : The property name
         * \param value : The property value list
         * \return The old property value list, or an empty list
         */
        std::vector<std::string> set_properties(std::string key, std::vector<std::string> values);

        /*!
         * \brief Append a value to a given property
         * \param key : The property name
         * \param value : The property value
         */
        void add_property(std::string key, std::string value);

        /*!
         * \brief Append a list of value to a given property
         * \param key : The property name
         * \param value : The property value list
         */
        void add_properties(std::string key, std::vector<std::string> values);

        /*!
         * \brief Create a property list by reading it from the input abstract pathname
         * \param file : The abstract pathname file
         */
        void load(mgz::io::file file);

        /*!
         * \brief Writes this property list (key and element pairs) in this Properties table to the file denoted by the given abstract pathname
         * \param file : The abstract pathname file
         */
        void store(mgz::io::file file);

        /*!
         * \brief Writes this property list (key and element pairs) in this Properties table to the file used to load it.
         */
        void store();

        /*!
         * \brief Create a property list by reading it from the XML input abstract pathname
         * \param file : The abstract pathname file
         */
        void load_xml(mgz::io::file file);

        /*!
         * \brief Emits an XML document representing all of the properties contained in this table.
         * \param file : The abstract pathname file
         */
        void store_xml(mgz::io::file file);

        /*!
         * \brief Emits an XML document representing all of the properties contained in this table.
         * \param file : The abstract pathname file
         */
        void store_xml();
		
        /*!
         * \brief Remove a property with the given key.
         * \param key : The key of the property to remove
		 * \return true if something has been removed
         */
		bool remove(std::string key);
		
		/*!
         * \brief Returns the total number of entries (single or multi-valued).
         */
		int count_all_props() {return datas.size();}
		
      private:
        std::vector<std::string> split(const std::string &s, char delim);
        std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
        std::string join(std::vector<std::string> vector, char delim);

      private:
        std::map<std::string, std::vector<std::string> > datas;
        mgz::io::file propertiesFile;
    };
  }
}

#endif
