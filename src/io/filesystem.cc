#include <sys/types.h>
#include <dirent.h>
#include <algorithm>

#include "config.h"
#ifdef __WIN32__
#include <windows.h>
#else
#include <sys/statvfs.h>
#endif // __WIN32__
#include "io/filesystem.h"
#include "regex/re.h"
#include "util/string.h"


//#include "util/log.h"

namespace mgz {
  namespace io {
    fs::fs() {
      root_path = ".";
    }

    fs::fs(file & f) {
      if(f.is_directory()) {
        set_root_path(f.get_path());
      } else {
        set_root_path(f.get_parent_path());
      }
    }

    fs::fs(std::string path) {
      set_root_path(path);
    }

    void fs::set_root_path(std::string path) {
      // Remove trailling FILE_SEPARATOR
      size_t found = path.find_last_of(FILE_SEPARATOR);
      if(found == path.size() - 1 && 1 != path.size()) {
        root_path = path.substr(0, path.size() - 1);
      } else {
        root_path = path;
      }
    }

    unsigned long fs::free_space(enum os_size_unit unit) {
#ifdef __WIN32__
#ifdef HAVE_GETDISKFREESPACEEX
      unsigned __int64 i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;
      if(!GetDiskFreeSpaceEx(root_path.c_str(), (PULARGE_INTEGER)&i64FreeBytesToCaller, (PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes)) {
        return 0L;
      } else {
        return i64FreeBytes / unit;
      }
#else
      DWORD lpSectorsPerCluster = 0;
      DWORD lpBytesPerSector = 0;
      DWORD lpNumberOfFreeClusters = 0;
      DWORD lpTotalNumberOfClusters = 0;
      if(!::GetDiskFreeSpace(root_path.c_str(), &lpSectorsPerCluster, &lpBytesPerSector, &lpNumberOfFreeClusters, &lpTotalNumberOfClusters)) {
        return 0L;
      } else {
        return (lpSectorsPerCluster * lpBytesPerSector * lpNumberOfFreeClusters) / unit;
      }
#endif
#else
      struct statvfs fs_data;

      if((statvfs(root_path.c_str(),&fs_data)) < 0) {
        return 0L;
      } else {
        return ((fs_data.f_bfree * fs_data.f_frsize * fs_data.f_bsize) / 1024 / 1024) / unit;
      }
#endif
    }
    unsigned long fs::total_space(enum os_size_unit unit) {
#ifdef __WIN32__
#ifdef HAVE_GETDISKFREESPACEEX
      unsigned __int64 i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;
      if(!GetDiskFreeSpaceEx(root_path.c_str(), (PULARGE_INTEGER)&i64FreeBytesToCaller, (PULARGE_INTEGER)&i64TotalBytes, (PULARGE_INTEGER)&i64FreeBytes)) {
        return 0L;
      } else {
        return i64TotalBytes / unit;
      }
#else
      DWORD lpSectorsPerCluster = 0;
      DWORD lpBytesPerSector = 0;
      DWORD lpNumberOfFreeClusters = 0;
      DWORD lpTotalNumberOfClusters = 0;
      if(!GetDiskFreeSpace(root_path.c_str(), &lpSectorsPerCluster, &lpBytesPerSector, &lpNumberOfFreeClusters, &lpTotalNumberOfClusters)) {
        return 0L;
      } else {
        return (lpSectorsPerCluster * lpBytesPerSector * lpTotalNumberOfClusters) / unit;
      }
#endif
#else
      struct statvfs fs_data;

      if((statvfs(root_path.c_str(),&fs_data)) < 0) {
        return 0L;
      } else {
        return (fs_data.f_blocks * fs_data.f_frsize) / unit;
      }
#endif
    }
    unsigned long fs::used_space(enum os_size_unit unit) {
      return (total_space(unit) - free_space(unit));
    }

    std::vector<file> fs::all_files(bool recursive) {
      return all_files(recursive, false);
    }

    std::vector<file> fs::all_files(bool recursive, bool relative) {
      files_.clear();
      if(0 != getdir_(recursive, relative, false)) {
        files_.clear();
      }
      return files_;
    }

    std::vector<file> fs::content() {
      files_.clear();
      if(0 != getdir_(false, false, true)) {
        files_.clear();
      }
      return files_;
    }

    bool fs::remove_content() {
      std::vector<file> content_list = content();
      std::vector<file>::iterator it;
      for(it = content_list.begin(); it < content_list.end(); it++) {
        if(!(*it).force_remove()) {
          return false;
        }
      }
      return true;
    }

    bool fs::copy_content(const fs & destination) {
      
      std::vector<file> content_list = all_files(true, true);
      std::vector<file>::iterator it;
      for(it = content_list.begin(); it < content_list.end(); it++) {
        mgz::io::file src = mgz::io::file(root_path).join(*it);
        mgz::io::file dst = mgz::io::file(destination.root_path).join(*it);
        if(!src.copy(dst)) { return false; }
      }
      return true;
    }

    int fs::getdir_(bool recursive, bool relative, bool keep_dir) {
      return getdir_(root_path, recursive, relative, keep_dir);
    }

    int fs::getdir_(std::string root, bool recursive, bool relative, bool keep_dir) {
      DIR *dp;
      struct dirent *dirp;
      if((dp  = opendir(root.c_str())) == NULL) {
        return -1;
      }

      while ((dirp = readdir(dp)) != NULL) {
        std::string local_path(dirp->d_name);
        if(0 != local_path.compare(".") && 0 != local_path.compare("..")) {
          std::string current_path = root;
          current_path.append(FILE_SEPARATOR).append(local_path);

          mgz::io::file current_file(current_path);
          if(!current_file.is_directory() || (current_file.is_directory() && keep_dir)) {
            if(!relative) {
              files_.push_back(current_file);
            } else {
              std::string path(current_path);
              std::string path_regex("^(");
              path_regex.append(root_path);
              if(root_path.find_last_of(FILE_SEPARATOR) != (root_path.size() - 1)) {
                path_regex.append(FILE_SEPARATOR);
              }
              path_regex.append(")(.*)$");
              mgz::util::replace_all(path_regex, FILE_SEPARATOR, FILE_SEPARATOR_REGEX);

              mgz::regex::RE r(path_regex);
              if(r.find(path) && 2 == r.captures()) {
                path = r.match(1);
              }
              files_.push_back(mgz::io::file(path));
            }
          } 

          if(current_file.is_directory() && recursive && !current_file.is_symlink()) {
            if(-1 == getdir_(current_path, recursive, relative, keep_dir)) {
              return -1;
            }
          }
        }
      }
      closedir(dp);
      return 0;
    }

     
    std::vector<fs_diff> fs::get_diff(fs dest, fsfilter *filter) {
      return get_diff(dest, true, filter);
    }

    std::vector<fs_diff> fs::get_diff(fs dest, bool keep_version, fsfilter *filter) {
      std::vector<fs_diff> diff;

      std::vector<file> dest_files = dest.all_files(true, true);
      EACH_RFILES_R(src_file, root_path) {
        std::vector<file>::iterator it;
        std::vector<file> current;
        current.push_back(*src_file);

        if(keep_version) {
          it = std::search(dest_files.begin(), dest_files.end(), current.begin(), current.end(), search_predicate_with_version);
        } else {
          it = std::search(dest_files.begin(), dest_files.end(), current.begin(), current.end(), search_predicate_without_version);
        }
        if(it != dest_files.end()) {
          int pos = std::find(dest_files.begin(), dest_files.end(), *it) - dest_files.begin();

          fs_diff d;
          d.left = mgz::io::file().join(root_path, src_file->get_path());
          d.left_root = mgz::io::file(root_path);

          d.right = mgz::io::file().join(dest.root_path, it->get_path());
          d.right_root = mgz::io::file(dest.root_path);

          d.status = BOTH;
          
          if(NULL == filter || filter->filter(d)) {
            diff.push_back(d);
          } else {
            //Logger::info("[FS/Filter] ignore files %s - %s", d.left.get_path().c_str(), d.right.get_path().c_str());
          }

          dest_files.erase(dest_files.begin()+pos);
        } else {
          fs_diff d;
          d.left = mgz::io::file().join(root_path, src_file->get_path());
          d.left_root = mgz::io::file(root_path);

          d.status = LEFT_ONLY;
          
          if(NULL == filter || filter->filter(d)) {
            diff.push_back(d);
          } else {
            //Logger::info("[FS/Filter] ignore \"left-only\" file %s", d.left.get_path().c_str());
          }
        }
      }

      std::vector<file>::iterator itr;
      for(itr = dest_files.begin(); itr < dest_files.end(); itr++) {
        fs_diff d;
        d.right = mgz::io::file().join(dest.root_path, itr->get_path());
        d.right_root = mgz::io::file(dest.root_path);

        d.status = RIGHT_ONLY;
        
        if(NULL == filter || filter->filter(d)) {
          diff.push_back(d);
        } else {
          //Logger::info("[FS/Filter] ignore \"right-only\" file %s", d.right.get_path().c_str());
        }
      }

      return diff;
    }

    bool fs::search_predicate_with_version(file v1, file v2) {
      return (v1.get_path()).compare(v2.get_path()) == 0;
    }

    bool fs::search_predicate_without_version(file v1, file v2) {
      return (v1.get_path_without_version()).compare(v2.get_path_without_version()) == 0;
    }
	  
    bool fs::exist() {
		  return mgz::io::file(root_path).exist();
    }
  
    std::vector<file> fs::all_files_filtered(/* const */ fsfilter &filter, bool keep_ignored) {
      std::vector<file> content=this->all_files(true,false);
      std::vector<file> filtered_content;
      for (std::vector<file>::iterator it = content.begin(); it < content.end(); it++) {
        mgz::io::fs_diff d;
        d.status=RIGHT_ONLY;
        d.right=*it;
        d.right_root=this->root_path;
        bool pass_the_filter=filter.filter(d);
        if (keep_ignored) {
          if (!pass_the_filter) { 
            filtered_content.push_back(*it);
          }
        } 
        else {
          if (pass_the_filter) { 
            filtered_content.push_back(*it);
          }
        }
      }
      return filtered_content;
    }
  }
}
  
  
