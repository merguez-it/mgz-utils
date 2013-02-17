#include "compress/archive/unzip.h"
// FIXME : #include "util/log.h"
#include "compress/compressor/raw.h"
#ifdef GLOW_WIN32
#include <string.h>
#endif
#include <algorithm>

namespace mgz {
  namespace compress {
    namespace archive {
      unzip::unzip(mgz::io::file & archive) : archive_(archive) {
        archive_size_ = archive_.size();
        is_.exceptions ( std::ifstream::badbit ); // dont set "failbit", as it may reflect normal conditions, when attempting to read more bytes than actually available in the file.
        is_.open(archive_.get_path().c_str(), std::ios::binary | std::ios::in);

        read_eocdh();
        read_cdh();
      }

      unzip::~unzip() {
        is_.close();
      }

      void unzip::inflate() {
        mgz::io::file to(".");
        inflate(to);
      }

      void unzip::inflate(mgz::io::file & to) {
        for(int i = 0; i < number_of_entries(); i++) {
          inflate_file_at_index(i, to);
        }
      }

      int unzip::number_of_entries() {
        return cdh_.size();
      }

      void unzip::read_eocdh() {
        long eocdh_buffer_size = EOCDH_STATIC_LENGTH + EOCDH_COMMENT_MAX_LENGTH;
        long eocdh_offset = archive_size_ - eocdh_buffer_size;

        if(0 > eocdh_offset) {
          eocdh_offset = 0;
          eocdh_buffer_size = archive_size_;
        }

        char magic[] = EOCDH_SIGNATURE_CHAR;

        std::vector<unsigned char> buffer(eocdh_buffer_size);
        is_.seekg(eocdh_offset);
        is_.read(reinterpret_cast<char*>(&buffer[0]), eocdh_buffer_size);
        is_.seekg(0);

        std::vector<unsigned char>::iterator pos = std::find_end(buffer.begin(), buffer.end(), magic, magic+4);
        if (buffer.end() == pos) {
          THROW(MalformatedEndOfCentralDirectoryHeader, "End of central directory record was not found");
        }
        int distance = std::distance(buffer.begin(), pos);
        std::vector<char> eocdh(buffer.begin() + distance, buffer.end());
        end_of_central_directory_header_static eocdh_static;
        memcpy(&eocdh_static, std::string(eocdh.begin(), eocdh.begin()+EOCDH_STATIC_LENGTH).c_str(), EOCDH_STATIC_LENGTH);

        eocdh_.static_part = eocdh_static;

        if(eocdh_static.comment_length != eocdh.size() - EOCDH_STATIC_LENGTH) {
          THROW(MalformatedEndOfCentralDirectoryHeader, "Wrong comment length");
        }

        if(eocdh_static.comment_length > 0) {
          eocdh_.comment = std::string(eocdh.begin()+EOCDH_STATIC_LENGTH, eocdh.end());
        }
      }

      void unzip::read_cdh() {
        is_.seekg(eocdh_.static_part.central_directory_offset);

        int nb_cdh = eocdh_.static_part.total_entries;

        while(0 < nb_cdh--) {
          central_directory_header cdh;
          is_.read(reinterpret_cast<char*>(&cdh.static_part), CDH_STATIC_LENGTH);
          if(CDH_SIGNATURE != cdh.static_part.signature) {
            THROW(MalformatedCentralDirectoryHeader, "Wrong signature");
          }

          if(0 >= cdh.static_part.file_name_length) {
            THROW(MalformatedCentralDirectoryHeader, "Wrong file name length");
          }
          cdh.data_part.file_name = std::string(cdh.static_part.file_name_length, 0);
          is_.read(&cdh.data_part.file_name[0], cdh.static_part.file_name_length);

          if(0 < cdh.static_part.extra_field_length) {
            cdh.data_part.extra_field.resize(cdh.static_part.extra_field_length);
            is_.read(reinterpret_cast<char *>(&cdh.data_part.extra_field[0]), cdh.static_part.extra_field_length);
          }

          if(0 < cdh.static_part.file_comment_length) {
            cdh.data_part.file_comment = std::string(cdh.static_part.file_comment_length, 0);
            is_.read(&cdh.data_part.file_comment[0], cdh.static_part.file_comment_length);
          }

          cdh_.push_back(cdh);
        }

        is_.seekg(0);
      }

      void unzip::inflate_file_at_index(int i) {
        mgz::io::file to(".");
        inflate_file_at_index(i, to);
      }

      void unzip::inflate_file_at_index(int i, mgz::io::file & to) {
        entry e = file_stat_at_index(i);
        
        switch(e.compression_method) {
          case CM_STORE:
            {
              mgz::io::file outfile = to.join(e.file_name);
              if(outfile.represents_directory()) {
                outfile.mkdirs();
              } else {
                if(!outfile.get_parent_file().exist()) {
                  outfile.get_parent_file().mkdirs();
                }
                // FIXME : Logger::info("Uncompress file %s", outfile.get_path().c_str());

                std::vector<unsigned char> buffer(e.compressed_size);

                is_.seekg(e.file_offset);
                is_.read(reinterpret_cast<char*>(&buffer[0]), e.uncompressed_size);
                is_.seekg(0);

                std::ofstream os(outfile.get_path().c_str(), std::ios::binary);
                os.write(reinterpret_cast<char*>(&buffer[0]), e.uncompressed_size);
                os.close();
              }
            }
            break;
          case CM_DEFLAT:
            {
              mgz::io::file outfile = to.join(e.file_name);
              if(!outfile.get_parent_file().exist()) {
                outfile.get_parent_file().mkdirs();
              }
              // FIXME : Logger::info("Uncompress file %s", outfile.get_path().c_str());
              is_.seekg(e.file_offset);
              std::fstream os(outfile.get_path().c_str(), std::ios::binary | std::ios::out);
              mgz::compress::compressor * cmp = new mgz::compress::raw(os, is_);
              cmp->decompress();
              if(e.crc32 != cmp->get_crc32()) {
                THROW(UncompressError, "Wrong CRC32 %ld, expected %ld for file %s", cmp->get_crc32(), e.crc32, e.file_name.c_str());
              }
              delete cmp;
              os.close();
              is_.clear();
              is_.seekg(0);
            }
            break;
          default:
            THROW(UnsupportedCompressionMethod, "Compressor (#%d) not supported", e.compression_method);
        }
      }

      local_file_header unzip::read_lfh_at_index(int i) {
        local_file_header lfh;

        if(cdh_.size() < i + 1) {
          THROW(UncompressError, "Entry %i does not exist", i);
        }

        central_directory_header cdh = cdh_[i];
        is_.seekg(cdh.static_part.offset_of_local_header);
        is_.read(reinterpret_cast<char*>(&lfh.static_part), LFH_STATIC_LENGTH);
        if(LFH_SIGNATURE != lfh.static_part.signature) {
          THROW(MalformatedLocalFileHeader, "Wrong signature");
        }

        if(0 >= lfh.static_part.file_name_length) {
          THROW(MalformatedLocalFileHeader, "Wrong file name length");
        }
        lfh.data_part.file_name = std::string(lfh.static_part.file_name_length, 0);
        is_.read(&lfh.data_part.file_name[0], lfh.static_part.file_name_length);

        if(0 < lfh.static_part.extra_field_length) {
          lfh.data_part.extra_field.resize(lfh.static_part.extra_field_length);
          is_.read(reinterpret_cast<char *>(&lfh.data_part.extra_field[0]), lfh.static_part.extra_field_length);
        }

        is_.seekg(0);

        return lfh;
      }

      entry unzip::file_stat_at_index(int i) {
        entry e;

        local_file_header lfh = read_lfh_at_index(i);
        central_directory_header cdh = cdh_[i];

        // TODO check lfh <-> cdh (raise if not)

        e.crc32 = cdh.static_part.descriptor.crc32;
        e.compressed_size = cdh.static_part.descriptor.compressed_size;
        e.uncompressed_size = cdh.static_part.descriptor.uncompressed_size;
        e.time = cdh.static_part.time;
        e.date = cdh.static_part.date;
        e.file_name = cdh.data_part.file_name;
        e.file_comment = cdh.data_part.file_comment;
        e.compression_method = cdh.static_part.compression_method;
        e.file_offset = cdh.static_part.offset_of_local_header + LFH_STATIC_LENGTH + lfh.static_part.file_name_length + lfh.static_part.extra_field_length;

        std::vector<unsigned char>::iterator pos = cdh.data_part.extra_field.begin();

        while(pos < cdh.data_part.extra_field.end()) {
          short header_id;
          short data_size;

          memcpy(&header_id, std::string(pos, pos + sizeof(header_id)).c_str(), sizeof(header_id));
          pos += sizeof(header_id);

          memcpy(&data_size, std::string(pos, pos + sizeof(data_size)).c_str(), sizeof(data_size));
          pos += sizeof(data_size);

          switch(header_id) {
            case EF_ZIP64:
              std::cout << "EF_ZIP64" << std::endl;
              break;
            case EF_NTFS:
              std::cout << "EF_NTFS" << std::endl;
              break;
            case EF_UNIX:
              std::cout << "EF_UNIX" << std::endl;
              break;
            default:
              // FIXME : Logger::warning("Unsupported extra field 0x%x", header_id);
              break;
          }


          pos += data_size;
        }

        return e;
      }
    }
  }
}
