#include "io/filesystem.h"
#include "compress/archive/zip.h"
#include "compress/archive/internal/common.h"
#include "compress/compressor.h"
#include "compress/compressor/gzip.h"
#include "util/exception.h"

namespace mgz {
  namespace compress {
    namespace archive {
      zip::zip(const mgz::io::file &archive, unsigned short compression_method, int level)
        : archive_(archive),compression_method_(compression_method), level_(level) {
          if (level < mgz::compress::COMPRESSION_LEVEL_0 || level > mgz::compress::COMPRESSION_LEVEL_9) {
            THROW(mgz::compress::UnsupportedCompressionLevelException,"Compression level %u not supported",level);
          }
        }

      unsigned long zip::writing_position() {
        long curpos=archive_stream_.tellp();
        if (-1==curpos) {
          THROW(CantGetStreamPositionException,"Cannot read the current position in %s archive",archive_.get_path().c_str());
        }
        return curpos;
      }

      unsigned short zip::LevelToZipFlag() {
        unsigned short LevelToZipFlagMatchings[]={DEFLATE_SUPER_FAST,DEFLATE_SUPER_FAST,DEFLATE_FAST,DEFLATE_FAST,DEFLATE_FAST,DEFLATE_NORMAL,DEFLATE_NORMAL,DEFLATE_NORMAL,DEFLATE_MAXIMUM,DEFLATE_MAXIMUM};
        return LevelToZipFlagMatchings[level_];
      }

      central_directory_header zip::header_from_file(mgz::io::file &fileToAdd, const mgz::io::file &base_dir) {
        central_directory_header cdh;
        central_directory_header_static &hdr=cdh.static_part;
        hdr.signature=CDH_SIGNATURE;
        hdr.version=VERSION_MADE_BY;
        hdr.needed_version=PKZIP_VERSION;
        struct tm date_time=fileToAdd.get_modification_datetime();
        hdr.time=dos_time_to_zip_time(date_time.tm_hour,date_time.tm_min,date_time.tm_sec);
        hdr.date=dos_date_to_zip_date(date_time.tm_year+1900,date_time.tm_mon+1,date_time.tm_mday);
        std::string relative_path=fileToAdd.relative_path_from(base_dir);
        unsigned int external_attribute_value=0;
        if (fileToAdd.is_directory() && !fileToAdd.represents_directory()) { // Sets folder "marker" (terminating '/') if needed
          relative_path+=FILE_SEPARATOR;
          external_attribute_value=DOS_DIR_EXTERNAL_VALUE;
        }
        std::string a_la_zip_path=mgz::io::file(relative_path).get_unix_path();
        hdr.file_name_length=a_la_zip_path.size();
        cdh.data_part.file_name=a_la_zip_path;
        hdr.extra_field_length=0;
        hdr.file_comment_length=0;
        hdr.disk_start=0;
        hdr.internal_file_attributs=0;
        hdr.external_file_attributs=external_attribute_value;
        hdr.descriptor.uncompressed_size=fileToAdd.size();
        hdr.flags = ( 0==hdr.descriptor.uncompressed_size ? NO_FLAGS : (DESCRIPTORS_AFTER_DATA | LevelToZipFlag()) );
        hdr.compression_method=( 0==hdr.descriptor.uncompressed_size ? CM_STORE : compression_method_ );
        hdr.offset_of_local_header=NOT_INITIALIZED_L;
        // Following fields are set lately, as soon as file is compressed
        hdr.descriptor.crc32=0;
        hdr.descriptor.compressed_size=0;
        return cdh;
      }

      local_file_header zip::central_to_local(const central_directory_header& cdh) {
        local_file_header lfh;
        local_file_header_static &lhdr=lfh.static_part;
        const central_directory_header_static &cdhs=cdh.static_part;
        lhdr.signature=LFH_SIGNATURE;
        lhdr.version=cdhs.needed_version;
        lhdr.compression_method=cdhs.compression_method;
        lhdr.flags=cdhs.flags;
        lhdr.time=cdhs.time;
        lhdr.date=cdhs.date;
        lhdr.descriptor.crc32=0; // Bit 3 of flags set (see DESCRIPTORS_AFTER_DATA flag usage) => desc. in local headers are not significant
        lhdr.descriptor.uncompressed_size=0; // Idem
        lhdr.descriptor.compressed_size=0; // Idem
        lhdr.file_name_length=cdhs.file_name_length;
        lhdr.extra_field_length=cdhs.extra_field_length;
        lfh.data_part.file_name=cdh.data_part.file_name;
        lfh.data_part.extra_field=cdh.data_part.extra_field;
        return lfh;
      }

      unsigned long zip::write_local_file_header(const local_file_header& lfh) {
        unsigned long pos =  writing_position();
        archive_stream_.write(reinterpret_cast<const char *>(&lfh.static_part),LFH_STATIC_LENGTH);
        archive_stream_.write(lfh.data_part.file_name.c_str(),lfh.data_part.file_name.size());
        archive_stream_.write(reinterpret_cast<const char *>(&lfh.data_part.extra_field),lfh.data_part.extra_field.size());
        return pos;
      }

      central_directory_header zip::compress_and_write_data (mgz::compress::compressor &comp, const central_directory_header& cdh) {
        central_directory_header result=cdh;
        central_directory_header_static &hdr=result.static_part;
        long positionflux=archive_stream_.tellp();
        comp.compress();
        positionflux=archive_stream_.tellp();
        signed_data_descriptor desc;
        desc.signature=DDS_SIGNATURE;
        desc.descriptor.crc32=comp.get_crc32();
        desc.descriptor.compressed_size=comp.get_compressed_size();
        desc.descriptor.uncompressed_size=comp.get_uncompressed_size();
        archive_stream_.write(reinterpret_cast<char *>(&desc),sizeof(signed_data_descriptor));
        hdr.descriptor=desc.descriptor;
        return result;
      }

      void zip::write_central_directory() {
        std::map<std::string,central_directory_header>::iterator it;
        unsigned long cd_offset = writing_position();
        for (it=catalog.begin(); it!=catalog.end(); it++) {
          archive_stream_.write(reinterpret_cast<char *>(&((*it).second.static_part)),CDH_STATIC_LENGTH);
          archive_stream_.write((*it).second.data_part.file_name.c_str(),(*it).second.data_part.file_name.size());
        }
        unsigned long cd_size=writing_position()-cd_offset;
        end_of_central_directory_header_static epilogue;
        epilogue.signature=EOCDH_SIGNATURE;
        epilogue.disk_number=0;
        epilogue.start_disk=0;
        epilogue.total_entries=catalog.size();
        epilogue.number_of_entries=epilogue.total_entries;
        epilogue.central_directory_size=cd_size;
        epilogue.central_directory_offset=cd_offset;
        epilogue.comment_length=0;
        archive_stream_.write(reinterpret_cast<char *>(&epilogue),EOCDH_STATIC_LENGTH);
      }

      void zip::add_file(mgz::io::file &fileToAdd, const mgz::io::file &base_dir) {
        if (!fileToAdd.exist() || !fileToAdd.is_defined()) {
          THROW(NonExistingFileToCompressException, "The file %s cannot be zipped as it does not exist",fileToAdd.get_path().c_str());
        }
        central_directory_header cdh=header_from_file(fileToAdd,base_dir);
        catalog[fileToAdd.get_absolute_path()]=cdh;
        if (fileToAdd.is_directory()) {
          mgz::io::fs f(fileToAdd);
          std::vector<mgz::io::file> files = f.content();
          std::vector<mgz::io::file>::iterator it;
          for(it = files.begin(); it < files.end(); it++) {
            add_file(*it, base_dir);
          }
        }
      }

      void zip::deflate() {
        if (catalog.size()==0) {
          THROW(NothingToCompressException, "Compress has nothing to do...exiting.");
        }
        char * out_buffer=new char[OUT_BUFFER_SIZE];
        archive_stream_.exceptions ( std::fstream::failbit | std::fstream::badbit );
        archive_stream_.rdbuf()->pubsetbuf(out_buffer,OUT_BUFFER_SIZE);
        archive_stream_.open(archive_.get_path().c_str(), std::ios::out | std::ios::binary);
        std::map<std::string,central_directory_header>::iterator it;
        for (it=catalog.begin();it != catalog.end();it++) {
          local_file_header lfh;
          central_directory_header& cdh=(*it).second;
          central_directory_header_static& cdhs=cdh.static_part;
          lfh=central_to_local(cdh);
          unsigned long offset_lfh=write_local_file_header(lfh);
          cdh.static_part.offset_of_local_header=offset_lfh;
          bool empty_it=cdhs.descriptor.uncompressed_size==0;
          if (!empty_it) {
            std::fstream to_zip_stream((*it).first.c_str(),std::ios::in | std::ios::binary);
            mgz::compress::gzip zipator(to_zip_stream,archive_stream_,level_);
            cdh=compress_and_write_data(zipator,cdh);
          }
        }
        write_central_directory();
        archive_stream_.flush();
        delete out_buffer;
      }
    }
  }
}
