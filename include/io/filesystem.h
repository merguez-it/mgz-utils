#ifndef __MGZ_IO_FILESYSTEM_INCLUDE
#define __MGZ_IO__FILESYSTEM_INCLUDE
/*!
 * \file io/filesystem.h
 * \brief Filesystem manipulation
 * \author Grégoire Lejeune & Mathias Franck
 * \version 0.1
 * \date february 2013
 */
#include <iostream>
#include <string>
#include <vector>

#include "io/file.h"
#include "util/units.h"

/*! \namespace mgz
 * 
 * Root namespace for all Merguez-it functions and classes
 */
namespace mgz {
   /*! \namespace mgz::io
    *
    * Namespace for mgz tools file io 
    */ 
  namespace io {
    enum fs_diff_status {
      LEFT_ONLY,
      RIGHT_ONLY,
      BOTH,
      FORCE_REPLACE,
      NONE
    };
    struct fs_diff {
      file left;
      file left_root;

      file right;
      file right_root;

      enum fs_diff_status status;
    };

    /*!
     * \class fsfilter
     * \brief A fsfilter class is used to filter results of mgz::io::fs::get_diff
     *
     * To filter mgz::io::fs::get_diff results, create a subclass of mgz::io::fsfilter and implement
     * the method 
     * \code
     * bool filter(fs_diff d);
     * \endcode
     */
    class fsfilter {
      public:
      virtual bool filter(/*const*/ mgz::io::fs_diff& d) {
          return true;
        }
    };

    class fs {
      public:
        fs(std::string);
        fs(file & f);
        fs();

        /*!
         * \brief Return the free space for this abstract filesystem
         * \param os_size_unit : The unit for the result
         * \return The free space size or 0 on error
         */
        unsigned long free_space(enum os_size_unit unit = MEGA_BYTES);

        /*!
         * \brief Return the total space for this abstract filesystem
         * \param os_size_unit : The unit for the result
         * \return The total space size or 0 on error
         */
        unsigned long total_space(enum os_size_unit unit = MEGA_BYTES);

        /*!
         * \brief Return the used space for this abstract filesystem
         * \param os_size_unit : The unit for the result
         * \return The used space size or 0 on error
         */
        unsigned long used_space(enum os_size_unit unit = MEGA_BYTES);

        /*!
         * \brief Return a vector of differences between this abstract filesystem and a given one
         * \param fs : The abstract filesystem to compare
         * \param filter : A mgz::io::fsfilter object
         * \return A vector of differences
         */
        std::vector<fs_diff> get_diff(fs dest, fsfilter *filter = NULL);

        /*!
         * \brief Return a vector of differences between this abstract filesystem and a given one
         * \param dest : The abstract filesystem to compare
         * \param keep_version : If false, the two file systems are compared, without taking care of version in the file names
         * \param filter : A mgz::io::fsfilter object
         * \return A vector of differences
         */
        std::vector<fs_diff> get_diff(fs dest, bool keep_version, fsfilter *filter = NULL);

        /*!
         * \brief Return a vector of files from this abstract file system
         * \param recursive : Search files recursivly
         * \return A vector of files
         * 
         * use EACH_FILES or EACH_FILES_R
         */
        std::vector<file> all_files(bool recursive); 

        /*!
         * \brief Return a vector of files from this abstract file system
         * \param recursive : Search files recursivly
         * \param relative : Keep path relative in the output vector
         * \return A vector of files
         * 
         * use EACH_RFILES or EACH_RFILES_R
         */
        std::vector<file> all_files(bool recursive, bool relative);

        std::vector<file> content();

        /*!
         * \brief Remove the content of the current abstract filesystem
         * \return True on success, false otherwise
         */
        bool remove_content();

        /*!
         * \brief Copy the content of the current abstract filesystem to destination
         * \return True on success, false otherwise
         */
        bool copy_content(const fs & destination);

        /*!
         * \brief the root path string of this abstract filesystem
         */
        std::string root_path;

        /*!
         * \brief Returns true if this abstract filesystem maps an existing directory
         * \return A boolean
         */
        bool exist();
      
      /*!
       * \brief Return a vector of files from this abstract file system,
       *        filtered according to the given filter.
       * \param filter : The fsfilter containeing filtering rules.
       * \param keep_ignored : if set, the function only returns files that DO NOT pass the filter.
       *                       This parameter is set to "false" as a default.
       * \return A vector of files
       * 
       */
      std::vector<file> all_files_filtered(/*const*/ fsfilter &filter, bool keep_ignored=false);

      private:
        static bool search_predicate_with_version(file, file);
        static bool search_predicate_without_version(file, file);

        void set_root_path(std::string path); 

        int getdir_(std::string root, bool recursive, bool relative, bool keep_dir);
        int getdir_(bool recursive, bool relative, bool keep_dir);
        std::vector<file> files_;
    };
  }
}

#define MGZ_FS_MERGE(a,b)  a##b
#define MGZ_FS_VECTOR(a) MGZ_FS_MERGE(__mgz_util_fs__vector, a)
#define MGZ_FS_UNIQUE_VECTOR MGZ_FS_VECTOR(__LINE__)

#define FS_EACH_FILE(F, P, REC, REL) \
  std::vector<mgz::io::file> MGZ_FS_UNIQUE_VECTOR = mgz::io::fs(P).all_files(REC, REL); for(std::vector<mgz::io::file>::iterator F = MGZ_FS_UNIQUE_VECTOR.begin(); F < MGZ_FS_UNIQUE_VECTOR.end(); F++)

#define EACH_FILES(F, P) FS_EACH_FILE(F, P, false, false)
#define EACH_FILES_R(F, P) FS_EACH_FILE(F, P, true, false)
#define EACH_RFILES(F, P) FS_EACH_FILE(F, P, false, true)
#define EACH_RFILES_R(F, P) FS_EACH_FILE(F, P, true, true)

#endif // __MGZ_IO_FILESYSTEM_INCLUDE

