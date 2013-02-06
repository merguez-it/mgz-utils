#ifndef __MGZ_IO_FILE_H
#define __MGZ_IO_FILE_H

/*!
 * \file io/file.h
 * \brief An abstract representation of file and directory pathnames.
 * \author Grégoire Lejeune
 * \version 0.1
 * \date mars 2012
 */
#include <iostream>
#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __WIN32__
#include <direct.h>

#define make_directory(X) ::mkdir(X)
#define realpath( p, r ) _fullpath( (r), (p), _MAX_PATH )
#define ALTERNATE_FILE_SEPARATOR "/"
#define FILE_SEPARATOR "\\"
#define FILE_SEPARATOR_REGEX "\\\\"
#define UNIX_FILE_SEPARATOR "/"
#define UNIX_FILE_SEPARATOR_C '/'
#define ROOT_PATH_REGEX "^[a-zA-Z]:\\\\"
#else // __WIN32__
#include <unistd.h>

#define make_directory(X) ::mkdir(X, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)
#define ALTERNATE_FILE_SEPARATOR "\\"
#define FILE_SEPARATOR "/"
#define FILE_SEPARATOR_REGEX "/"
#define UNIX_FILE_SEPARATOR "/"
#define UNIX_FILE_SEPARATOR_C '/'
#define ROOT_PATH_REGEX "^\\/"
#endif // __WIN32__

#define FILE_SEPARATOR_CHAR ((std::string(FILE_SEPARATOR).c_str())[0])
#define ALTERNATE_FILE_SEPARATOR_CHAR ((std::string(ALTERNATE_FILE_SEPARATOR).c_str())[0])

class CantReadFileStatusException{};
class CantSetPermissionsException{};

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
    /*! \class mgz::io::file
     *
     * An abstract representation of file and directory pathnames.
     */
    class file {
      public:
        /*!
         * \brief Create a file with the given path
         * \param path : Path of the file
         */
        file(std::string path);

        /*!
         * \brief Create a file with an empty path
         */
        file();

        /*!
         * \brief This operator (<<), applied to an output stream, perform an output ot the file path
         */
        friend std::ostream & operator<<(std::ostream & os, file & f) {
          return os << f.get_path();
        }

        /*!
         * \brief Compare two file
         * \param file : The file object to compare to
         */
        bool operator==(file);

        /*!
         * \brief Tests whether the file denoted by this abstract pathname is a regular file
         * 
         * \return True if the file is a regular file
         */
        bool is_file();

        /*!
         * \brief Tests whether the file denoted by this abstract pathname is a directory
         * 
         * \return True if the file is a directory
         */
        bool is_directory();

        /*!
         * \brief Tests whether the file denoted by this abstract pathname is a symbolic link
         * 
         * \return True if the file is a symbolic link
         */
        bool is_symlink();

        /*!
         * \brief Test whether the path of the file is part of a symlink
         * \return True if the path is part of a symlink
         */
        bool is_under_symlink();

        /*!
         * \brief Test whether the path of the file is a broken symlink
         * \return True is the path is a broken symlink
         */
        bool is_broken_symlink();

        /*!
         * \brief Tests whether the file denoted by this abstract pathname is defined
         * 
         * \return True if the file is defined
         */
        bool is_defined();
      
        /*!
         * \brief Tests whether the file denoted by this abstract pathname has
         *        at least one permission for execution (and is not a directory).
         *        Always returns false for Windows platforms.
         * \return True if the file is a unix-like executable.
         */
        bool has_executable_flag();

        /*!
         * \brief Give the file size in bytes. If the file exist and is not a directory
         * \return The file size in bytes or 0
         */
        long size();

        /*!
         * \brief Creates a new File instance from a parent file and a child file.
         * 
         * \param parent : The parent file
         * \param child : The parent file
         * \return A new file instance
         */
        file join(file parent, file child);

        /*!
         * \brief Creates a new File instance from this abstract file and a child file.
         * 
         * \param child : The child file
         * \return A new file instance
         */
        file join(file child);

        /*!
         * \brief Creates a new File instance from this abstract file and a child file.
         * 
         * \param child : The child file
         * \return A new file instance
         */
        file join(const char *child);

        /*!
         * \brief Converts this abstract pathname into a pathname string.
         * 
         * \return The pathname string
         */
        std::string get_path();

        /*!
         * \brief Returns the name of the file or directory denoted by this abstract pathname.
         * 
         * \return The name string
         */
        std::string get_name();

        /*!
         * \brief Returns the absolute pathname string of this abstract pathname.
         * 
         * \return The absolute pathname string
         */
        std::string get_absolute_path();

        /*!
         * \brief Returns a copy the absolute pathname file of this abstract pathname.
         * 
         * \return The absolute pathname file
         */
        file get_absolute_file();

        /*!
         * \brief Return the normalized pathname string of this abstract pathname
         * \return The normalized path
         */
        std::string get_normalize_path();

        /*!
         * \brief Create a new normalized pathname with the normalized path of the current abstract pathname, as path
         * \return The abstract pathname of this abstract pathname with it normalized path
         */
        file get_normalize_file();

        /*!
         * \brief Returns the pathname string of this abstract pathname's parent, or an empty string if this pathname does not name a parent directory.
         * 
         * \return The parent pathname string or en empty string
         */
        std::string get_parent_path();
        /*!
         * \brief Returns the abstract pathname of this abstract pathname's parent, or an empty abstract pathname if this pathname does not name a parent directory.
         * 
         * \return The abstract pathname of this abstract pathname's parent, or an empty abstract pathname
         */
        file get_parent_file();

        /*!
         * \brief Returns the extension string of this abstract pathname.
         * 
         * \return The extension string
         */
        std::string get_extension();
      
        /*!
         * \brief Returns the name of the file,without its last extension.
         * 
         * \return The name without extension
         */
        std::string get_name_without_extension();
      
        /*!
         * \brief Returns the relative abstract pathname of this relative pathname from the given relative pathname.
         * 
         * \param root : An abstract pathname
         * \return The relative pathname
         */
        file relative_file_from(file root);

        /*!
         * \brief Returns the relative abstract pathname of this relative pathname from the relative path
         * \param root : A string path
         * \return The relative pathname
         */
        file relative_file_from(const std::string & root);

        /*!
         * \brief Returns the relative pathname string of this relative pathname from the given relative pathname.
         * 
         * \param root : An abstract pathname
         * \return The relative pathname string
         */
        std::string relative_path_from(file root);

        /*!
         * \brief Returns the relative pathname string of this relative pathname from the given relative pathname.
         * 
         * \param root : A string path
         * \return The relative pathname string
         */
        std::string relative_path_from(const std::string & root);

        /*!
         * \brief Tests whether the file or directory denoted by this abstract pathname exists.
         * \return true if the file or directory exist, false otherwise
         */
        bool exist();

        /*!
         * \brief Create the directory named by this abstract pathname
         * \return true on success, false otherwise
         */
        bool mkdir();

        /*!
         * \brief Creates the directory named by this abstract pathname, including any necessary but nonexistent parent directories.
         * \return true on success, false otherwise
         */
        bool mkdirs();

        /*!
         * \brief Remove the file or directory denoted by this abstract pathname
         * \return true on success, false otherwise
         */
        bool remove();

        /*!
         * \brief Remove the file or directory (including any child directories) denoted by this abstract pathname.
         * \return true on success, false otherwise
         */
        bool force_remove();

        /*!
         * \brief Copy the file or directory denoted by this abstract pathname.
         * \param file : destination the abstract pathname 
         * \return true on success, false otherwise
         */
        bool copy(file);

        /*!
         * \brief Create a relative link from the abstract pathname to the given abstract pathname
         * \pathname source : The abstract pathname of the original copy
         * \return True on success, false otherwise
         */
        bool relative_link(file source);

        /*!
         * \brief Create an absolute link from the abstract pathname to the given abstract pathname
         * \pathname source : The abstract pathname of the original copy
         * \return True on success, false otherwise
         */
        bool absolute_link(file source);

        /*!
         * \brief Move the file or directory denoted by this abstract pathname.
         * \param file : destination the abstract pathname 
         * \return true on success, false otherwise
         */
        bool move(file);

        /*!
         * \brief Return the pathname string of the current directory.
         * \return the pathname string
         */
        std::string current_directory_path();
        
        /*!
         * \brief Return the abstract pathname file of the current directory.
         * \return The abstract pathname
         */
        file current_directory_file();

        /*!
         * \brief Return the name of this abstract pathname, without the version number, if any
         * \return The name without version
         */
        std::string get_name_without_version();

        /*!
         * \brief Return the pathname string of this abstract pathname, without the version number, if any
         * \return The name without version
         */
        std::string get_path_without_version();
      
        /*!
         * \brief Tells if the abstract pathname represent a directory, whatever it exist or not on disk.
         * \return True if pathname is teminated by a FILE_SEARATOR
         */
        bool represents_directory();

        /*!
         * \brief Make a file executable on OS that require such flag (i.e: Mac, Linux, U*X...)
         */
        void set_executable_flag();
      
        /*!
         * \brief Set (Unix) permissions to the file (no effect on directories).
         * \param perms : Unix permissions, like "0755" 
         */
         void set_permissions(mode_t perms);
      
         mode_t get_mode();
         mode_t get_lmode();

      private:
        std::string tilde_to_home(std::string path);
        void initialize_status();
        bool is_type(mode_t);
        bool is_ltype(mode_t);

      private:
        std::string filepath_;

        bool status_set_;
#ifdef __WIN32__
        struct _stat status_;
        struct _stat lstatus_;
#else
        struct stat status_;
        struct stat lstatus_;
#endif
    };
  }
}

#endif
