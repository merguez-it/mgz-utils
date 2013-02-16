#include "util/string.h"
#include "util/exception.h"
#include "xml/xml.h"
#include "io/properties.h"

#define CR '\r'

namespace mgz {
   namespace io {
      properties::properties() {
      }

      properties::properties(mgz::io::file file) {
         load(file);
      }

     void properties::load(mgz::io::file file) {
       propertiesFile = file;
       std::string line;
       std::string key;
       std::vector<std::string> values;
       size_t found;
       
       std::ifstream propertyFile (file.get_path().c_str(), std::ios::in);
       if (!file.exist()) {
         THROW(PropertyFileNotFoundException,"Properties file %s does not exist", file.get_path().c_str());
       }
       if( propertyFile.is_open() ) {
         while( propertyFile.good() ) {
           getline( propertyFile, line );
           if(!line.empty() && *line.rbegin() == CR) {
             line.erase( line.length()-1, 1);
           }
           if(!mgz::util::trim(line).empty() && line.find_first_of("#")!=0) {
             found = line.find_first_of("=");
             key = line.substr(0, found);
             values = mgz::util::split(line.substr(found+1), PROPERTIES_SEPARATOR);
             add_properties(key, values);
           }
         }
         propertyFile.close();
       } else {
         THROW(PropertyFileOpenException,"Properties file %s cannot be opened", file.get_path().c_str());
       }
     }

      void properties::store(mgz::io::file file) {
         propertiesFile = file;
         store();
      }

      void properties::store() {
         std::ofstream propertyFile (propertiesFile.get_path().c_str(), std::ios::out);
         if (propertyFile.is_open()) {
            std::map<std::string, std::vector<std::string> >::const_iterator iter;
            for (iter = datas.begin(); iter != datas.end(); ++iter) {
              propertyFile << iter->first << "=" << mgz::util::join(iter->second, PROPERTIES_SEPARATOR) << std::endl;
              if (0 !=propertyFile.rdstate()) {
                THROW (CantStorePropertiesException,"Cannot write to property file %s", propertiesFile.get_path().c_str());
              }
            }
            propertyFile.close();
         } else {
           THROW(CantStorePropertiesException, "Cannot open property file %s for write-access", propertiesFile.get_path().c_str());
         }
      }

      std::string properties::get_property(std::string key, std::string defaultValue) {
         std::string value = get_property(key);
         if(value.empty()) {
            value = defaultValue;
         }
         return value;
      }

      std::string properties::get_property(std::string key) {
         std::vector<std::string> values = get_properties(key);
         if(1 != values.size()) {
            return "";
         }
         return values.at(0);
      }

      std::vector<std::string> properties::get_properties(std::string key) {
        std::vector<std::string> result;
         if(datas.count(key)!=0) // Avoids side-effect creation in the map.
         {
           result=datas[mgz::util::trim(key)];
         }
        return result;
      }

      std::string properties::set_property(std::string key, std::string value) {
         std::string oldValue = get_property(key, "");
         std::vector<std::string> values;
         values.push_back(mgz::util::trim(value));
         set_properties(key, values);
         return oldValue;
      }

      std::vector<std::string> properties::set_properties(std::string key, std::vector<std::string> values) {
         std::vector<std::string> oldValues = get_properties(key);
         datas[mgz::util::trim(key)] = values;
         return oldValues;
      }

      void properties::add_property(std::string key, std::string value) {
         std::vector<std::string> values;
         values.push_back(mgz::util::trim(value));
         add_properties(key, values);
      }

      void properties::add_properties(std::string key, std::vector<std::string> values) {
         std::vector<std::string> currentValues = get_properties(key);
         std::vector<std::string>::iterator it;
         for (it = values.begin() ; it < values.end(); it++) {
            currentValues.push_back(*it);
         }
         set_properties(key, currentValues);
      }
	   

	   bool properties::remove(std::string key) {
		   return datas.erase(mgz::util::trim(key))!=0;
	   }

      void properties::load_xml(mgz::io::file file) {
         mgz::xml::document doc(file.get_path());
         if(doc.LoadFile()) {
            mgz::xml::handle hDoc(&doc);
            mgz::xml::element *element;

            element = hDoc.FirstChild("properties").FirstChild("entry").ToElement();
            for(; element; element = element->NextSiblingElement()) {
               // const char * node = element->Value();
               const char * key = element->Attribute("key");
               const char * value = element->GetText();
               add_property(key, value);
            }
         }
      }

      void properties::store_xml(mgz::io::file file) {
         propertiesFile = file;
         store_xml();
      }

      void properties::store_xml() {
         std::string file_name = propertiesFile.get_path(); 
         mgz::xml::document doc;
         mgz::xml::declaration * decl = new mgz::xml::declaration("1.0", "UTF-8", "");
         mgz::xml::element * props = new mgz::xml::element("properties");


         std::map<std::string, std::vector<std::string> >::const_iterator iter;
         for (iter = datas.begin(); iter != datas.end(); ++iter) {
            std::vector<std::string>::iterator it;
            std::vector<std::string> values = iter->second;

            for(it = values.begin(); it < values.end(); it++) {
               //iter->first = *it
               mgz::xml::element * entry = new mgz::xml::element("entry");
               entry->SetAttribute("key", iter->first);
               mgz::xml::text * value = new mgz::xml::text(*it);
               entry->LinkEndChild(value);
               props->LinkEndChild(entry);
            }
         }

         doc.LinkEndChild(decl);
         doc.LinkEndChild(props);
         doc.SaveFile(file_name);
      }
   };
};
