#ifndef __ZIP_H
#define __ZIP_H

#include <vector>
#include <string>

#define PKZIP_VERSION 0x000A

// Extra fields
#define EF_ZIP64                      0x0001
#define EF_AV_INFO                    0x0007
#define EF_OS2                        0x0009
#define EF_NTFS                       0x000A
#define EF_OPENVMS                    0x000C
#define EF_UNIX                       0x000D
#define EF_PATCH                      0x000F
#define EF_PKCS7_STORE                0x0014
#define EF_X509_LF                    0x0015
#define EF_X509_CD                    0x0016
#define EF_STRONG_ENC_H               0x0017
#define EF_RMC                        0x0018
#define EF_PKCS7_LIST                 0x0019
#define EF_IBM_UNCOMPRESSED           0x0065
#define EF_IBM_COMPRESSED             0x0066
#define EF_POSZIP                     0x4690
#define EF_4P_MAC                     0x07c8
#define EF_4P_ZIPIT_MAC               0x2605
#define EF_4P_ZIPIT_MAC_135           0x2705
#define EF_4P_ZIPIT_MAC_135_2         0x2805
#define EF_4P_INFOZIP_MAC             0x334d
#define EF_4P_ACORN_SPARKFS           0x4341
#define EF_4P_WINNT_ACL               0x4453
#define EF_4P_VMCMS                   0x4704
#define EF_4P_VMS                     0x470f
#define EF_4P_FWKCS_MD5               0x4b46
#define EF_4P_OS2_ACL                 0x4c41
#define EF_4P_INFOZIP_OPENVMS         0x4d49
#define EF_4P_XCEED                   0x4f4c
#define EF_4P_AOS_VS                  0x5356
#define EF_4P_XTS                     0x5455
#define EF_4P_XCEED_UNICODE           0x554e
#define EF_4P_INFOZIP_UNIX            0x5855
#define EF_4P_INFOZIP_UNICODE_COMMENT 0x6375
#define EF_4P_BEOS                    0x6542
#define EF_4P_INFOZIP_UNICODE_PATH    0x7075
#define EF_4P_ASI                     0x756e
#define EF_4P_INFOZIP_UNIX_2          0x7855
#define EF_4P_MS_OPGH                 0xa220
#define EF_4P_SMS_QDOS                0xfd4a

// Compression methods
#define CM_STORE        0
#define CM_SHRUNK       1
#define CM_REDUCE_1     2
#define CM_REDUCE_2     3
#define CM_REDUCE_3     4
#define CM_REDUCE_4     5
#define CM_IMPLOD       6
#define CM_TOKENIZING   7
#define CM_DEFLAT       8
#define CM_DEFLAT64     9
#define CM_PKWARE_DCLI 10
#define CM_BZIP2       12
#define CM_LZMA        14
#define CM_IBM_TERSE   18
#define CM_LZ77        19
#define CM_WAVPACK     97
#define CM_PPMD_I1     98

// Useful flags presets
// Bit 3
#define DESCRIPTORS_AFTER_DATA 0x0008
#define NO_FLAGS 0x0000
// Bit 1 & 2
#define DEFLATE_NORMAL 0x0000
#define DEFLATE_MAXIMUM 0x0002
#define DEFLATE_FAST 0x0004
#define DEFLATE_SUPER_FAST 0x0006
#define USE_UTF8_FILENAMES 0x0800
// Version made by
#ifdef GLOW_WIN32
#define VERSION_MADE_BY 10 // ou bien 0 ??
#else
#define VERSION_MADE_BY 19
#endif

#pragma pack(push,2) // PKZip fields are 2-bytes aligned, so we do the same for simplicity purpose

struct data_descriptor {
  unsigned int crc32;                // crc-32                      -  4 bytes
  unsigned int compressed_size;      // compressed size             -  4 bytes
  unsigned int uncompressed_size;    // uncompressed size           -  4 bytes
};

#define DDS_SIGNATURE 0x08074b50

struct signed_data_descriptor {
  unsigned int signature;
  data_descriptor descriptor;
};

#define LFH_SIGNATURE 0x04034b50
#define LFH_STATIC_LENGTH 0x1E
struct local_file_header_static {
  unsigned int signature;              // local file header signature -  4 bytes  (0x04034b50)
  unsigned short version;              // version needed to extract   -  2 bytes
  unsigned short flags;                // general purpose bit flag    -  2 bytes
  unsigned short compression_method;   // compression method          -  2 bytes
  unsigned short time;                 // last mod file time          -  2 bytes
  unsigned short date;                 // last mod file date          -  2 bytes
  data_descriptor descriptor;
  unsigned short file_name_length;     // file name length            -  2 bytes
  unsigned short extra_field_length;   // extra field length          -  2 bytes
};

struct local_file_header_data {
  std::string file_name;
  std::vector<unsigned char> extra_field;
};

struct local_file_header {
  local_file_header_static static_part;
  local_file_header_data data_part;
};

// Version made by
#ifdef GLOW_WIN32
#define VERSION_MADE_BY 10 // ou bien 0 ??
#else
#define VERSION_MADE_BY 19
#endif

#define CDH_STATIC_LENGTH 0x2E
#define CDH_SIGNATURE 0x02014b50
#define DOS_DIR_EXTERNAL_VALUE 0x00000010
struct central_directory_header_static {
  unsigned int signature;
  unsigned short version;
  unsigned short needed_version;
  unsigned short flags;
  unsigned short compression_method;
  unsigned short time;
  unsigned short date;
  data_descriptor descriptor;
  unsigned short file_name_length;
  unsigned short extra_field_length;
  unsigned short file_comment_length;
  unsigned short disk_start;
  unsigned short internal_file_attributs;
  unsigned int external_file_attributs;
  unsigned int offset_of_local_header;
};

struct central_directory_header_data {
  std::string file_name;
  std::vector<unsigned char> extra_field;
  std::string file_comment;
};

struct central_directory_header {
  central_directory_header_static static_part;
  central_directory_header_data data_part;
};

#define EOCDH_STATIC_LENGTH 0x16
#define EOCDH_COMMENT_MAX_LENGTH 0xFFFF
#define EOCDH_SIGNATURE 0x06054b50
#define EOCDH_SIGNATURE_CHAR {0x50, 0x4B, 0x05, 0x06}

struct end_of_central_directory_header_static {
  unsigned int signature;
  unsigned short disk_number;
  unsigned short start_disk;
  unsigned short total_entries;
  unsigned short number_of_entries;
  unsigned int central_directory_size;
  unsigned int central_directory_offset;
  unsigned short comment_length;
};

struct end_of_central_directory_header {
  end_of_central_directory_header_static static_part;
  std::string comment;
};

#pragma pack(pop) // Restore default struct alignment

#endif // __ZIP_H

