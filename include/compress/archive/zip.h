#ifndef __MGZ_COMPRESS_ARCHIVE_ZIP_H
#define __MGZ_COMPRESS_ARCHIVE_ZIP_H

#include <fstream>
#include <map>
#include "io/file.h"
#include "util/exception.h"
#include "compress/compressor.h"
#include "compress/archive/lib_zip.h"

// Exceptions possibly thrown
class NonExistingFileToCompressException{};
class CantOpenStreamException{};
class CantGetStreamPositionException{};
class NothingToCompressException{};

#define NOT_INITIALIZED_W 0x6969
#define NOT_INITIALIZED_L 0x69696969
#define OUT_BUFFER_SIZE 1024*1024*4 // 4 Mo de buffer de sortie.

namespace mgz {
  namespace compress {
    namespace archive {
      class zip {
        public:
          std::map<std::string,central_directory_header> catalog; // Should be filled using "add" prior to call deflate();

          void add_file(mgz::io::file &fileToAdd, const mgz::io::file &base_dir);  

          compress(const mgz::io::file &archive, unsigned short compression_method=CM_DEFLAT, int level=Glow::Command::COMPRESSION_LEVEL_4);

          void deflate(); // Do THE job.

          /* TODO.... or not to do...
             void remove_file(entry);
             void remove_file_at_index(int i);
             int number_of_entries();
             */

        private:
          mgz::io::file archive_;
          std::fstream archive_stream_;
          unsigned short compression_method_;
          int level_;

          //Initializes a central directory header entry, given an existing file or dir.
          central_directory_header header_from_file(mgz::io::file& fileToAdd, const mgz::io::file& base_dir);

          // Returns the current position in archive currently written (or die !)
          unsigned long writing_position();

          // Returns flags to be set in PKZIP archive entries, given an level of compression expressed as an int (0..9).
          // Those flags may depend on used compression method
          unsigned short LevelToZipFlag();

          // Initializes and returns a local file header given an initialized central directory file header. 
          local_file_header central_to_local(const central_directory_header& cdh);

          // Writes a local file header in the zip archive stream, and returns the offset of written header
          unsigned long write_local_file_header(const local_file_header& lfh);

          // Compress a file to an archive, given a compressor. Central directory header is updated according to compression results.
          central_directory_header compress_and_write_data(Glow::Command::compressor &comp, const central_directory_header &cdh);

          // Writes the central directory into the archive stream.
          void write_central_directory();

      };
    }
  }
}

#endif // __MGZ_COMPRESS_ARCHIVE_ZIP_H

