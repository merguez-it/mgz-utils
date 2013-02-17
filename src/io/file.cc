#include "config.h"
#ifdef __WIN32__
#include <time.h>
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif // HAVE_WINDOWS_H
#endif //__WIN32__

#include "regex/re.h"
#include "io/file.h"
#include "util/string.h"
#include "util/exception.h"

#include <sys/param.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>


namespace mgz {
  namespace io {
    file::file(std::string path) : status_set_(false) {
      filepath_ = tilde_to_home(mgz::util::replace_all(path, ALTERNATE_FILE_SEPARATOR_CHAR, FILE_SEPARATOR_CHAR));
    }
    file::file() : status_set_(false) {
      filepath_ = "";
    }

    bool file::operator==(file f) {
      return get_path().compare(f.get_path()) == 0;
    }

    void file::initialize_status() {
      if(exist() && false == status_set_) {
        int stat_result;
        int lstat_result;
#ifdef __WIN32__
        // On F***ing Win$, stat does not support FILE_SEPARATOR at end of path for a directory
        std::string path_ = filepath_;
        if (path_.find_last_of(FILE_SEPARATOR) == (path_.size() - 1)) {
          path_ = path_.substr(0, path_.size() - 1);
          //TODO: Attention, pansement ci-dessous - Investiguer pourquoi on reçoit parfois \ dans filepath_ sur PC.
          if (path_.empty()) {
            return;
          }
        }
        stat_result = _stat(path_.c_str(), &status_);
        lstat_result = _stat(path_.c_str(), &lstatus_);
#else
        stat_result = stat(filepath_.c_str(), &status_);
        lstat_result = lstat(filepath_.c_str(), &lstatus_);
#endif
        if (0 != stat_result || 0 != lstat_result) {
          THROW(CantReadFileStatusException,"Cannot read the attributes of the file %s (err = %d)",filepath_.c_str(),stat_result);
        }

        status_set_ = true;
      }
    }

    mode_t file::get_mode() {
      initialize_status();
      return status_.st_mode;
    }

    mode_t file::get_lmode() {
      initialize_status();
      return lstatus_.st_mode;
    }

    bool file::is_type(mode_t m) {
      if (exist()) {
        return (((get_mode()) & S_IFMT) == m);
      } else {
        return false;
      }
    }

    bool file::is_ltype(mode_t m) {
      if (exist()) {
        return (((get_lmode()) & S_IFMT) == m);
      } else {
        return false;
      }
    }

    long file::size() {
      struct stat status;
      if (exist() && !is_directory()) {
        if(0 != stat(filepath_.c_str(), &status)) {
          return -1;
        }

        return (long)status.st_size;
      } else {
        return 0L;
      }
    }

    bool file::is_file() {
      return is_type(S_IFREG);
    }

    bool file::is_directory() {
      return is_type(S_IFDIR);
    }

    bool file::is_symlink() {
#ifdef __WIN32__
      return false;
#else
      return is_ltype(S_IFLNK) || is_broken_symlink();
#endif
    }

    bool file::is_under_symlink() {
#ifdef __WIN32__
      return false;
#else
      return get_absolute_path() != get_normalize_path();
#endif
    }

    bool file::is_broken_symlink() {
#ifdef __WIN32__
      return false;
#else
      char buf[PATH_MAX] = {0};
      long size = readlink(get_path().c_str(), buf, PATH_MAX-1);
      if(size > 0) {
        return !file(buf).exist();
      }

      return false;
#endif
    }

    bool file::is_defined() {
      return !filepath_.empty();
    }

    bool file::has_executable_flag() {
#ifdef __WIN32__
      return false;
#else
      if (is_file() && exist()) {
        mode_t m = get_mode();
        return (0 == ::access(get_path().c_str(), X_OK)) && ((m & 0777) != 0777);
      }
      return false;
#endif
    }

    file file::join(file parent, file child) {
      std::string f(parent.get_path());
      if(f.find_last_of(FILE_SEPARATOR) != (f.size() - 1)) {
        f.append(FILE_SEPARATOR);
      }
      f.append(child.get_path());
      return file(f);
    }

    file file::join(file child) {
      std::string f2(get_path());
      if (child.filepath_.empty()) {
        return file(f2);
      }
      if(f2.find_last_of(FILE_SEPARATOR) != (f2.size() - 1)) {
        f2.append(FILE_SEPARATOR);
      }
      f2.append(child.get_path());
      return file(f2);
    }

    file file::join(const char *child) {
      return join(std::string(child));
    }

    std::string file::get_path() {
      if(is_directory() && filepath_.find_last_of(FILE_SEPARATOR) != (filepath_.size() - 1)) {
        return filepath_ + FILE_SEPARATOR;
      }
      return filepath_;
    }

    std::string file::get_unix_path() {
#ifdef __WIN32__
      std::string fp(filepath_);
      mgz::regex::RE disk(CAPTURED_ROOT_PATH_REGEX);
      if(disk.find(fp)) {
        fp = disk.replace(0, FILE_SEPARATOR);
      }

      return mgz::util::replace_delim(fp, FILE_SEPARATOR_CHAR, UNIX_FILE_SEPARATOR_C);
#else
      return get_path();
#endif
    }

    std::string file::get_name() {
      size_t found = filepath_.find_last_of(FILE_SEPARATOR);
      if(found != std::string::npos) {
        if((filepath_.size() - 1) == found) { // Ignore final FILE_SEPARATOR (folder mark), to get folder name
          found = filepath_.find_last_of(FILE_SEPARATOR,found-1);
        }
        if (found != std::string::npos) {
          return filepath_.substr(found+1);
        }
      }
      return filepath_;
    }

    std::string file::tilde_to_home(std::string path) {
      char *home;
#ifdef __WIN32__
      home = getenv("USERPROFILE");
      if(NULL == home) {
        home = strcat(getenv("HOMEDRIVE"), getenv("HOMEPATH"));
      }
#else
      home = getenv("HOME");
#endif
      if(!path.empty() && path.at(0) == '~' && home) {
        return std::string(home).append(path.substr(1));
      } else {
        return std::string(path);
      }
    }

    std::string file::get_absolute_path() {
      if(filepath_.empty()) {
        return "";
      } else {
        if(exist()) {
          char buf[PATH_MAX];
          char *ptr;
          ptr = realpath((const char*)filepath_.c_str(), buf);
          return ptr;
        } else {
          std::string path = filepath_;
          if(false == mgz::regex::RE(ROOT_PATH_REGEX).find(filepath_)) {
            std::vector<std::string> tmp;
            tmp.push_back(current_directory_path());
            tmp.push_back(path);
            path = mgz::util::join(tmp, FILE_SEPARATOR_CHAR);
          }

          std::vector<std::string> path_cut = mgz::util::split(path, FILE_SEPARATOR_CHAR);
          std::vector<std::string> path_final;
          for ( std::vector<std::string>::iterator it = path_cut.begin() ; it < path_cut.end(); ++it ) {
            if(0 == (*it).compare(std::string(".."))) {
              if(!path_final.empty()) {
                path_final.pop_back();
              }
            } else if(!(*it).empty() && 0 != (*it).compare(std::string("."))){
              path_final.push_back(*it);
            }
          }
#ifdef __WIN32__
          return mgz::util::join(path_final, FILE_SEPARATOR_CHAR);
#else
          std::string complete(UNIX_FILE_SEPARATOR);
          return complete.append(mgz::util::join(path_final, FILE_SEPARATOR_CHAR));
#endif
        }
      }
    }

    file file::get_absolute_file() {
      std::string path = get_absolute_path();
      if(path.empty()) {
        return file();
      } else {
        return file(path);
      }
    }

    std::string file::get_normalize_path() {
      file completed_file(filepath_);

      if(false == mgz::regex::RE(ROOT_PATH_REGEX).find(filepath_)) {
        char pwd[PATH_MAX];
        if (getcwd(pwd, sizeof(pwd)) == NULL) {
          throw 1; // todo
        }
        file root(pwd);
        completed_file = root.join(filepath_);
      }

      std::string completed_path = completed_file.get_path();

      std::vector<std::string> completed_path_cut = mgz::util::split(completed_path, FILE_SEPARATOR_CHAR);
      std::vector<std::string> normalized_path_cut;
      std::vector<std::string>::iterator it;
      for(it = completed_path_cut.begin(); it != completed_path_cut.end(); it++) {
        if(*it == ".." && !normalized_path_cut.empty()) {
          normalized_path_cut.pop_back();
        } else if(!(*it).empty() && (*it) != ".") {
          normalized_path_cut.push_back(*it);
        }
      }

#ifdef __WIN32__
      return mgz::util::join(normalized_path_cut, FILE_SEPARATOR_CHAR);
#else
      std::string complete(UNIX_FILE_SEPARATOR);
      return complete.append(mgz::util::join(normalized_path_cut, FILE_SEPARATOR_CHAR));
#endif
    }

    file file::get_normalize_file() {
      std::string path = get_normalize_path();
      if(path.empty()) {
        return file();
      } else {
        return file(path);
      }
    }

    std::string file::get_parent_path() {
      size_t found = filepath_.find_last_of(FILE_SEPARATOR);
      if(found != std::string::npos) {
        if((filepath_.size() - 1) == found) { // Ignore final FILE_SEPARATOR (folder mark), to get the real parent
          found = filepath_.find_last_of(FILE_SEPARATOR,found-1);
        }
        if(found != std::string::npos) {
          return filepath_.substr(0, found);
        }
      }
      return "";
    }

    file file::get_parent_file() {
      std::string parent = get_parent_path();
      if(parent.empty()) {
        return file();
      } else {
        return file(parent);
      }
    }

    std::string file::get_extension() {
      return get_name().substr(get_name().find_last_of(".") + 1);
    }

    std::string file::get_name_without_extension() {
      size_t pos=get_name().find_last_of(".");
      if (std::string::npos==pos) {
        return get_name();
      } else {
        return get_name().substr(0,get_name().size()-get_extension().size()-1);
      }
    }

    file file::relative_file_from(const std::string & root) {
      return relative_file_from(file(root));
    }

    file file::relative_file_from(file root) {
      return file(relative_path_from(root));
    }

    std::string file::relative_path_from(const std::string & root) {
      return relative_path_from(file(root));
    }

    std::string file::relative_path_from(file root) {
      std::string path = get_absolute_path();
      std::string root_path(root.get_absolute_path());

      if(root_path[root_path.size()-1] != FILE_SEPARATOR_CHAR) {
        root_path = root_path.append(FILE_SEPARATOR);
      }

      size_t pos = path.find(root_path);
      if(pos == std::string::npos) {
        std::vector<std::string> path_cut = mgz::util::split(path, FILE_SEPARATOR_CHAR);
        std::vector<std::string> root_path_cut = mgz::util::split(root_path, FILE_SEPARATOR_CHAR);

        int i = 0;
        int j;
        while(i < path_cut.size() && i < root_path_cut.size()) {
          if(path_cut[i] != root_path_cut[i]) {
            break;
          }
          i++;
        }

        file result;
        for(j = i; j < root_path_cut.size(); j++) {
          result = result.join("..");
        }
        for(j = i; j < path_cut.size(); j++) {
          result = result.join(path_cut[j]);
        }

        return result.get_path();
      } else {
        return path.substr(pos+root_path.size());
      }
    }

    bool file::exist() {
      bool result = !filepath_.empty();
      if(result) {
#ifdef HAVE_GETFILEATTRIBUTES
        result = result && (::GetFileAttributes(filepath_.c_str()) != 0xFFFFFFFF);
#else
        result = result && (-1 != access(filepath_.c_str(), 0));
#endif
      }
      return result;
    }

    bool file::mkdir() {
      if(!exist()) {
        return make_directory(filepath_.c_str()) == 0;
      }
      return true;
    }
    bool file::mkdirs() {
      if(!exist() && !filepath_.empty()) {
        size_t found = filepath_.find_first_of(FILE_SEPARATOR);
        std::string current = filepath_.substr(0, found);
        std::string keep = filepath_.substr(found+1);
        if(found == std::string::npos) {
          current = filepath_;
          keep = "";
        }

        while(current.compare(filepath_) != 0) {
          if(!current.empty()) {
            if(!file(current).mkdir()) {
              return false;
            }
          }

          found = keep.find_first_of(FILE_SEPARATOR);
          if(found == std::string::npos) {
            current = filepath_;
            keep = "";
          } else {
            std::string next = keep.substr(0, found);
            keep = keep.substr(found+1);
            current = current.append(FILE_SEPARATOR).append(next);
          }
        }
        if(!current.empty()) {
          if(!file(current).mkdir()) {
            return false;
          }
        }

        return true;
      } else {
        return true;
      }
    }

    /*
     * Remove the file or directory denoted by this abstract pathname.
     */
    bool file::remove() {
      if(exist() || is_broken_symlink()) {
#ifdef HAVE_REMOVEDIRECTORY
        if(is_directory()) {
          return ::RemoveDirectory(filepath_.c_str());
        }
#endif
        return ::remove(filepath_.c_str()) == 0;
      }
      return true;
    }

    /*
     * Remove the file or directory (recursivly) denoted by this abstract pathname.
     */
    bool file::force_remove() {
      if(is_directory() && !is_symlink()) {
        DIR *dp;
        struct dirent *dirp;
        if((dp = opendir(filepath_.c_str())) == NULL) {
          return false;
        }

        while ((dirp = readdir(dp)) != NULL) {
          std::string path = std::string(dirp->d_name);

          if(0 != path.compare(".") && 0 != path.compare("..")) {
            std::string current = filepath_;
            current.append(FILE_SEPARATOR).append(path);
            if(!file(current).force_remove()) {
              closedir(dp);
              return false;
            }
          }
        }
        closedir(dp);

        return remove();
      } else {
        return remove();
      }
    }

    bool file::copy(file destination) {
      if(is_directory()) {
        if(exist() && destination.mkdirs()) {
          DIR *dp;
          struct dirent *dirp;
          if((dp  = opendir(filepath_.c_str())) == NULL) {
            return false;
          }

          while ((dirp = readdir(dp)) != NULL) {
            std::string path = std::string(dirp->d_name);

            if(0 != path.compare(".") && 0 != path.compare("..")) {
              std::string current = filepath_;
              current.append(FILE_SEPARATOR).append(path);

              std::string current_copy = destination.get_path();
              if(current_copy.find_last_of(FILE_SEPARATOR) != (current_copy.size() - 1)) {
                current_copy.append(FILE_SEPARATOR);
              }
              current_copy.append(path);
              if(!file(current).copy(current_copy)) {
                return false;
              }
            }
          }

          closedir(dp);
          return true;
        } else {
          return false;
        }
      } else {
        if(destination.is_directory()) {
          mgz::io::file dest_file = destination.join(get_name());
          return copy(dest_file);
        } else {
          if(exist() && destination.get_parent_file().mkdirs()) {
#ifdef __WIN32__
            return (0 != ::CopyFile(filepath_.c_str(), destination.get_path().c_str(), FALSE));
#else
            mode_t origin_mode=get_mode();
            std::ifstream src(filepath_.c_str(), std::ios::in | std::ios::binary);
            std::ofstream dst(destination.get_path().c_str(), std::ios::out | std::ios::binary);
            if(src.is_open() && dst.is_open()) {
              dst << src.rdbuf();
            } else {
              return false;
            }
            src.close();
            dst.close();
            destination.set_permissions(origin_mode);
            return true;
#endif
          } else {
            return false;
          }
        }
      }
    }

    bool file::relative_link(file source) {
#ifdef __WIN32__
      return false;
#else
      if(exist()) {
        return false;
      }

      file n_path = get_normalize_path();
      if(!n_path.get_parent_file().exist()) {
        n_path.get_parent_file().mkdirs();
      }

      file dest = source.get_normalize_file().relative_file_from(n_path.get_parent_file());

      return 0 == ::symlink(dest.get_path().c_str(), get_path().c_str());
#endif
    }

    bool file::absolute_link(file source) {
#ifdef __WIN32__
      return false;
#else
      if(exist()) {
        return false;
      }

      file n_path = get_normalize_path();
      if(!n_path.get_parent_file().exist()) {
        n_path.get_parent_file().mkdirs();
      }

      file dest = source.get_normalize_file();

      return 0 == ::symlink(dest.get_path().c_str(), get_path().c_str());
#endif
    }

    bool file::represents_directory() {
      return filepath_.find_last_of(FILE_SEPARATOR)==filepath_.size()-1;
    }

    bool file::move(file destination) {
      if(exist() && destination.get_parent_file().mkdirs()) {
        if (is_directory()) {
          bool move_into=destination.represents_directory() || destination.is_directory();
          if (move_into) {
            destination.mkdirs();
            destination = destination.join(this->get_name());
          } //else, it is a "rename" of the dir (+ optionnally a "move")
        }
        int result = rename( filepath_.c_str() , destination.filepath_.c_str());
        if (0 ==result) {
          return true;
        }
        return copy(destination) && force_remove();
      } else {
        return false;
      }
    }

    std::string file::current_directory_path() {
      char buffer[PATH_MAX];
      getcwd(buffer, PATH_MAX);
      return std::string(buffer);
    }
    file file::current_directory_file() {
      return file(current_directory_path());
    }

    std::string file::get_name_without_version() {
      return file(get_name()).get_path_without_version();
    }

    std::string file::get_path_without_version() {
      std::string path(get_path());

      if(mgz::regex::RE(".*[\\d]+\\.[\\d]+.*").find(get_path())) {
        mgz::regex::RE re("(-\\d[\\d\\w\\.]*)(.*)");
        while(re.find(path)) {
          re.replace(0, "");
          path = re.replace(1, "");
        }
        path = path + ".jar";
      }

      return path;
    }

    void file::set_executable_flag() {
#ifndef __WIN32__
      mode_t save_mode=get_mode();
      int chmod_result=chmod( filepath_.c_str(), save_mode | S_IEXEC | S_IXGRP | S_IXOTH  );
      if (0 != chmod_result) {
        THROW(CantSetPermissionsException,"Cannot set the executable attributes of the file %s (err = %d)",filepath_.c_str(),chmod_result);
      }
#endif
    }

    void file::set_permissions(mode_t perms) {
#ifndef __WIN32__
      int chmod_result=chmod( filepath_.c_str(), perms  );
      if (0 != chmod_result) {
        THROW(CantSetPermissionsException,"Cannot set the permissions for element %s (err = %d)",filepath_.c_str(),chmod_result);
      }
#endif
    }

    /*!
     * \brief Reads the modification date+time of this file into a timespec structure.
     */
    struct tm file::get_modification_datetime() {
      time_t time_sec;
      initialize_status();
#ifdef __WIN32__
      time_sec=status_.st_mtime;
#else
      time_sec=status_.st_mtimespec.tv_sec;
#endif
      return (*localtime(&time_sec));
    }

    /*!
     * \brief Reads the creation date+time of this file into a timespec structure..
     */
    struct tm file::get_creation_datetime() {
      time_t time_sec;
      initialize_status();
#ifdef __WIN32__
      time_sec=status_.st_ctime;
#else
      time_sec=status_.st_birthtimespec.tv_sec;
#endif
      return (*localtime(&time_sec));
    }

#define HASH_BUFFER_SIZE ( 1024 * 2048 ) //2MB buffer

    uint32_t file::crc32() {
      if (!exist()) {
        THROW(FileShouldExistException,"Cannot open the file %s as it does not exist",filepath_.c_str())
      }
      if (is_directory()) {
        THROW(FileShouldNotBeFolderException, "Cannot compute the crc32 of %s as it is a folder",filepath_.c_str() );
      }
      char * buffer = new char[HASH_BUFFER_SIZE];
      security::crc32sum crc32;

      std::ifstream file(get_path().c_str(), std::ios::in | std::ios::binary);
      while(file.good()) {
        memset(buffer, 0, HASH_BUFFER_SIZE);
        file.read(buffer, HASH_BUFFER_SIZE);
        int read_size = file.gcount();
        crc32.update(buffer, read_size);
      }
      crc32.finalize();

      delete buffer;

      return crc32.crc;
    }
  }
}

