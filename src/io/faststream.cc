#include <fcntl.h>
#include <string.h>

#include "config.h"
#include "io/faststream.h"
#include "util/exception.h"
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#define HAVE_FASTSTREAM 1

#if defined(HAVE_SYS_MMAN_H) && defined(HAVE__SC_PAGESIZE)
#include <unistd.h>
#include <sys/mman.h>
#elif defined(HAVE_MAPVIEWOFFILE) && defined(HAVE_CREATEFILEMAPPING) && defined(HAVE_UNMAPVIEWOFFILE)
#else
#pragma message "faststream not avalaible on your OS"
#undef HAVE_FASTSTREAM
#endif

#ifdef HAVE_FASTSTREAM

bool isWow64() {
#ifdef HAVE_ISWOW64PROCESS
  bool bIsWow64 = FALSE;

  fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandle(TEXT("kernel32")),"IsWow64Process");

  if(NULL != fnIsWow64Process) {
    if(!fnIsWow64Process(GetCurrentProcess(),&bIsWow64)) {
      THROW(RuntimeException, "Can't determines WOW64.");
    }
  }
  return bIsWow64;
#else
  return false;
#endif
}

#if defined(HAVE_CREATEFILEMAPPING) && defined(HAVE_MAPVIEWOFFILE)
void *win_mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset) { 
  HANDLE handle = CreateFileMapping((HANDLE)_get_osfhandle(fd), NULL, prot /*PAGE_WRITECOPY*/, 0, 0, NULL); 

  if (handle != NULL) { 
    start = MapViewOfFile(handle, flags /*FILE_MAP_COPY*/, 0, offset, length); 
    CloseHandle(handle); 
  } 

  return start; 
} 
#endif

mgz::io::faststream::faststream(const std::string & path, mgz::io::mode m) : 
  path_(path), 
  mode_(m), 
  open_(false), 
  file_descriptor_(-1), 
  offset_(0),
  file_size_(0),
  read_size_(0) {
#ifdef HAVE__SC_PAGESIZE
  pagesize_ = sysconf(_SC_PAGESIZE);
  if(0 > pagesize_) {
    THROW(CantGetPagesize, "Cannot read system page size");
  }
#endif
#if defined(HAVE_GETSYSTEMINFO) || defined(HAVE_GETNATIVESYSTEMINFO)
  SYSTEM_INFO sysconf;
  if(isWow64()) {
#ifdef HAVE_GETNATIVESYSTEMINFO
    GetNativeSystemInfo(&sysconf);
#else
    GetSystemInfo(&sysconf);
#endif
  } else {
    GetSystemInfo(&sysconf);
  }
  pagesize_ = sysconf.dwPageSize;
#endif
}

mgz::io::faststream::faststream(mgz::io::file f, mode m) :
  path_(f.get_path()),
  mode_(m), 
  open_(false), 
  file_descriptor_(-1), 
  offset_(0),
  file_size_(0),
  read_size_(0) {
#ifdef HAVE__SC_PAGESIZE
  pagesize_ = sysconf(_SC_PAGESIZE);
  if(0 > pagesize_) {
    THROW(CantGetPagesize, "Cannot read system page size");
  }
#endif
#if defined(HAVE_GETSYSTEMINFO) || defined(HAVE_GETNATIVESYSTEMINFO)
  SYSTEM_INFO sysconf;
  if(isWow64()) {
#ifdef HAVE_GETNATIVESYSTEMINFO
    GetNativeSystemInfo(&sysconf);
#else
    GetSystemInfo(&sysconf);
#endif
  } else {
    GetSystemInfo(&sysconf);
  }
  pagesize_ = sysconf.dwPageSize;
#endif
}

mgz::io::faststream::~faststream() {
  close();
}

bool mgz::io::faststream::open(long i) {
  if(i <= 0) {
    THROW(InvalidPageSize, "Page number must be > 0");
  } else {
    pages(i);
  }

  if(mgz::io::out == mode_) {
    file_descriptor_ = ::open(path_.c_str(), O_RDONLY);
    get_file_size();
  } else {
    file_descriptor_ = ::open(path_.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0644);
  }

  open_ = (file_descriptor_ > -1);

  return open_;
}

bool mgz::io::faststream::close() {
  if(open_) {
    if(mgz::io::in == mode_) {
      flush(true);
    }
    if(-1 != ::close(file_descriptor_)) {
      open_ = false;
      file_descriptor_ = -1;
      return true;
    } else {
      return false;
    }
  }

  return true;
}

std::vector<unsigned char> mgz::io::faststream::read() {
  if(mgz::io::out != mode_) {
    THROW(CantReadFile, "Can't read file open with mgz::io::in mode");
  }
  std::vector<unsigned char> result;

  if(offset_ < file_size_) {
    long rest = (file_size_ - offset_);
    read_size_ = (rest >= pages_)?pages_:rest;
    result.resize(read_size_);

#if defined(HAVE_CREATEFILEMAPPING) && defined(HAVE_MAPVIEWOFFILE)
    void *data = win_mmap(NULL, pages_, PAGE_WRITECOPY, FILE_MAP_COPY, file_descriptor_, offset_);
    if(NULL == data) {
#else
    void *data = mmap(NULL, pages_, PROT_READ, MAP_SHARED, file_descriptor_, offset_);
    if((int*)data == (int*)-1) {
#endif
      THROW(ErrorWhileReadingFile, "Error reading file %s (offset: %ld)", path_.c_str(), offset_);
    }

    memcpy(&result[0], data, read_size_);
#ifdef HAVE_UNMAPVIEWOFFILE
    UnmapViewOfFile(data);
#else
    munmap(data, pages_);
#endif

    offset_ += pages_;
  } else {
    read_size_ = 0;
  }

  return result;
}

long mgz::io::faststream::write(const std::vector<unsigned char> & buffer) {
  if(mgz::io::in != mode_) {
    THROW(CantWriteFile, "Can't write file open with mgz::io::out mode");
  }

  buffer_.insert(buffer_.end(), buffer.begin(), buffer.end());
  if(buffer_.size() >= pages_) {
    flush();
  }

  return offset_ + buffer_.size();
}

void mgz::io::faststream::flush(bool force) {
  long size = buffer_.size();
  long length = size;

  if(!force) {
    long keep = size % pages_;
    length = size - keep;
  }

  if(-1 == lseek(file_descriptor_, offset_ + length, SEEK_SET)) {
    THROW(CantWriteFile, "Can't seek");
  }
  if(1 != ::write(file_descriptor_, "", 1)) {
    THROW(CantWriteFile, "Can't write");
  }

#if defined(HAVE_CREATEFILEMAPPING) && defined(HAVE_MAPVIEWOFFILE)
  void *data = win_mmap(NULL, length, PAGE_WRITECOPY, FILE_MAP_COPY, file_descriptor_, offset_);
  if(NULL == data) {
#else
  void *data = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, file_descriptor_, offset_);
  if((int*)data == MAP_FAILED) {
#endif
    THROW(ErrorWhileWrittingingFile, "Error writing file %s (offset: %ld)", path_.c_str(), offset_);
  }
  memcpy(data, &buffer_[0], length);

#ifdef HAVE_UNMAPVIEWOFFILE
  UnmapViewOfFile(data);
#else
  munmap(data, length);
#endif
  offset_ += length;

  buffer_.erase(buffer_.begin(), buffer_.begin() + length);
}


long mgz::io::faststream::gcount() const {
  return read_size_;
}

void mgz::io::faststream::pages(long i) {
  if(!open_) {
    pages_ = pagesize_ * i;
  }
}

void mgz::io::faststream::get_file_size() {
  struct stat sb; 
  if(fstat(file_descriptor_, & sb) != 0) {
    THROW(CantGetFileSize, "File does not exist or not readable");
  }
  file_size_ = sb.st_size;
}

#endif
