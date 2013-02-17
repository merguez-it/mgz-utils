#include "util/exception.h"
#include "compress/archive/lib_zip.h"
#include "compress/archive/unzip.h"
#include "compress/archive/zip.h"
#include "compress/compressor.h"
#include "io/filesystem.h"

#include "gtest/gtest.h"
#include "config-test.h"

unsigned int filter_DS_Store(const std::map<std::string, central_directory_header>& catalog) {
  unsigned int count=0;
  std::map<std::string,central_directory_header>::const_iterator it;
  for (it=catalog.begin();it != catalog.end();it++) {
    if ((*it).first.find(".DS_Store")==std::string::npos)
      count++;
  }
  return count;
}

TEST(Zip, UncompressStore) {
  mgz::io::file zip(MGZ_TESTS_PATH(zip/test_store.zip));
  mgz::io::file out("./ziptest");
  out.force_remove();
  mgz::io::file file1("ziptest/file1.txt");
  mgz::io::file file2("ziptest/pipo/file2.txt");

  mgz::compress::archive::unzip uz(zip);

  ASSERT_EQ(2, uz.number_of_entries());

  uz.inflate(out);
  ASSERT_TRUE(file1.exist());
  ASSERT_TRUE(file2.exist());
}

TEST(Zip, UncompressDeflate) {
  mgz::io::file zip(MGZ_TESTS_PATH(zip/test_deflate.zip));
  mgz::io::file out("./ziptest");
  out.force_remove();
  mgz::io::file file1("ziptest/file1.txt");
  mgz::io::file file2("ziptest/pipo/file2.txt");

  mgz::compress::archive::unzip uz(zip);

  ASSERT_EQ(2, uz.number_of_entries());

  uz.inflate(out);
  ASSERT_TRUE(file1.exist());
  ASSERT_TRUE(file2.exist());
}

TEST(Zip, add_one_file) {
  mgz::io::file f(MGZ_TESTS_PATH(zip/test.txt));
  mgz::io::file base_dir(MGZ_TESTS_PATH(zip));
  mgz::compress::archive::zip comp(mgz::io::file("osef.zip"));
  comp.add_file(f,base_dir);
  EXPECT_EQ(1U,comp.catalog.size());
  central_directory_header cdh=comp.catalog[f.get_absolute_path()];
  central_directory_header_static hdr=cdh.static_part;
  EXPECT_EQ(8,hdr.file_name_length);
  EXPECT_EQ(0,hdr.extra_field_length);
  EXPECT_EQ(0,hdr.file_comment_length);
  EXPECT_EQ(0,hdr.disk_start);
  EXPECT_EQ(0,hdr.internal_file_attributs);
  EXPECT_EQ(0U,hdr.external_file_attributs);
  EXPECT_EQ("test.txt",cdh.data_part.file_name);
  EXPECT_EQ(DESCRIPTORS_AFTER_DATA | DEFLATE_FAST , hdr.flags);
  EXPECT_EQ(CM_DEFLAT,hdr.compression_method);
  EXPECT_EQ((unsigned long)NOT_INITIALIZED_L,hdr.offset_of_local_header);
  EXPECT_EQ(0UL,hdr.descriptor.compressed_size);
  EXPECT_EQ(57U,hdr.descriptor.uncompressed_size);
  EXPECT_EQ(0UL,hdr.descriptor.crc32);
}

TEST(Zip, add_one_empty_directory) {
  mgz::io::file f(MGZ_TESTS_PATH(zip/vide_dir));
  f.force_remove();
  f.mkdirs();
  mgz::io::file base_dir(MGZ_TESTS_PATH(zip));
  mgz::compress::archive::zip comp(mgz::io::file("osef.zip"));
  comp.add_file(f,base_dir);
  int count=filter_DS_Store(comp.catalog);
  EXPECT_EQ(1,count);
  central_directory_header cdh=comp.catalog[f.get_absolute_path()];
  central_directory_header_static hdr=cdh.static_part;
  EXPECT_EQ(9,hdr.file_name_length);
  EXPECT_EQ(0,hdr.extra_field_length);
  EXPECT_EQ(0,hdr.file_comment_length);
  EXPECT_EQ(0,hdr.disk_start);
  EXPECT_EQ(0,hdr.internal_file_attributs);
  EXPECT_EQ((unsigned long)DOS_DIR_EXTERNAL_VALUE,hdr.external_file_attributs);
  EXPECT_EQ(std::string("vide_dir/"),cdh.data_part.file_name);
  EXPECT_EQ(NO_FLAGS , hdr.flags);
  EXPECT_EQ(CM_STORE,hdr.compression_method);
  EXPECT_EQ((unsigned long)NOT_INITIALIZED_L,hdr.offset_of_local_header);
  EXPECT_EQ(0UL,hdr.descriptor.compressed_size);
  EXPECT_EQ(0UL,hdr.descriptor.uncompressed_size);
  EXPECT_EQ(0UL,hdr.descriptor.crc32);
}

TEST(Zip, add_one_directory_not_empty) {
  mgz::io::file f(MGZ_TESTS_PATH(zip/to_zip_dir));
  mgz::io::file base_dir(MGZ_TESTS_PATH(zip));
  mgz::compress::archive::zip comp(mgz::io::file("osef.zip"));
  comp.add_file(f,base_dir);
  int count=filter_DS_Store(comp.catalog);
  EXPECT_EQ(5,count);
 }

TEST(Zip, add_non_existing_should_fail) {
  mgz::io::file f("IDontExist");
  mgz::io::file base_dir(MGZ_TESTS_PATH(zip));
  mgz::compress::archive::zip comp(mgz::io::file("osef.zip"));
  EXPECT_THROW(comp.add_file(f,base_dir),Exception<NonExistingFileToCompressException>);
}

TEST(Zip, zip_one_file_default_compression) {
  mgz::io::file f(MGZ_TESTS_PATH(zip/test.txt));
  mgz::io::file base_dir(MGZ_TESTS_PATH(zip));
  mgz::io::file archive("test_one_file.zip");
  archive.force_remove();
  mgz::compress::archive::zip comp(archive);
  comp.add_file(f,base_dir);
  comp.deflate();
  EXPECT_TRUE(archive.exist());
  //TODO: Trouver un outil tiers pour tester l'intégrité de l'archive.
  mgz::io::file out("./ziptest");
  out.force_remove();
  mgz::compress::archive::unzip uz(archive);
  uz.inflate(out);
  mgz::io::fs ls(out);
  std::vector<mgz::io::file> files = ls.content();
  EXPECT_TRUE(1==files.size());
  mgz::io::file check1("./ziptest/test.txt");
  EXPECT_EQ(57U,check1.size());
}

TEST(Zip, zip_one_file_different_levels) {
  mgz::io::file f(MGZ_TESTS_PATH(zip/big-test.txt));
  mgz::io::file base_dir(MGZ_TESTS_PATH(zip));
  mgz::io::file archive0("test_one_file-level-0.zip");
  mgz::io::file archive9("test_one_file-level-9.zip");
  archive0.force_remove();
  archive9.force_remove();
  mgz::compress::archive::zip comp0(archive0,CM_DEFLAT,mgz::compress::COMPRESSION_LEVEL_0);
  comp0.add_file(f,base_dir);
  mgz::compress::archive::zip comp9(archive9,CM_DEFLAT,mgz::compress::COMPRESSION_LEVEL_9);
  comp9.add_file(f,base_dir);
  comp0.deflate();
  comp9.deflate();
  EXPECT_TRUE(archive0.exist());
  EXPECT_TRUE(archive9.exist());
  // FIXME :: EXPECT_TRUE(archive0.size() > archive9.size());
}

TEST(Zip, zip_two_files) {
  mgz::io::file f(MGZ_TESTS_PATH(zip/test.txt));
  mgz::io::file f2(MGZ_TESTS_PATH(zip/test2.txt));

  mgz::io::file base_dir(MGZ_TESTS_PATH(zip));
  mgz::io::file archive("test_two_files.zip");
  archive.force_remove();
  mgz::compress::archive::zip comp(archive);
  comp.add_file(f,base_dir);
  comp.add_file(f2,base_dir);
  comp.deflate();
  EXPECT_TRUE(archive.exist());
  //TODO: Trouver un outil tiers pour tester l'intégrité de l'archive.
  mgz::io::file out("./ziptest");
  out.force_remove();
  mgz::compress::archive::unzip uz(archive);
  uz.inflate(out);
  mgz::io::fs ls(out);
  std::vector<mgz::io::file> files = ls.content();
  EXPECT_TRUE(2==files.size());
  mgz::io::file check1("./ziptest/test.txt");
  mgz::io::file check2("./ziptest/test2.txt");
  EXPECT_EQ(57U,check1.size());
  EXPECT_EQ(56U,check2.size());
}

TEST(Zip, zip_one_empty_dir) {
  mgz::io::file f(MGZ_TESTS_PATH(zip/vide_dir));
  f.force_remove();
  f.mkdirs();
  mgz::io::file base_dir(MGZ_TESTS_PATH(zip));
  mgz::io::file archive("vide_dir.zip");
  archive.force_remove();
  mgz::compress::archive::zip comp(archive);
  comp.add_file(f,base_dir);

  comp.deflate();
  EXPECT_TRUE(archive.exist());
 // //TODO: Trouver un outil tiers pour tester l'intégrité de l'archive.
  mgz::io::file out("./ziptest");
  out.force_remove();
  mgz::io::file empty_dir("ziptest/vide_dir");
  mgz::compress::archive::unzip uz(archive);
  uz.inflate(out);
  EXPECT_TRUE(empty_dir.exist());
  mgz::io::fs ls(empty_dir);
  std::vector<mgz::io::file> files = ls.content();
  EXPECT_TRUE(0==files.size());
}

//TEST(Zip, zip_one_dir_with_old_compress) {
//  mgz::io::file f(MGZ_TESTS_PATH(zip/to_zip_dir));
//  mgz::io::file base_dir(MGZ_TESTS_PATH(zip));
//  mgz::io::file archive("test_one_dir_with_old_compress.zip");
//  archive.force_remove();
//  Glow::Command::compress cmp(archive);
//  cmp.setBaseDir(base_dir);
//  cmp.add(f);
//  cmp.zip();
//  EXPECT_TRUE(archive.exist());
//}

 TEST(Zip, zip_one_dir) {
  mgz::io::file f(MGZ_TESTS_PATH(zip/to_zip_dir));
  mgz::io::file base_dir(MGZ_TESTS_PATH(zip));
  mgz::io::file archive("test_one_dir.zip");
  archive.force_remove();
  mgz::compress::archive::zip comp(archive,CM_DEFLAT,9);
  comp.add_file(f,base_dir);
  comp.deflate();
  EXPECT_TRUE(archive.exist());
  //TODO: Trouver un outil tiers pour tester l'intégrité de l'archive.
  mgz::io::file out("./ziptest");
  out.force_remove();
  mgz::io::file file1("ziptest/to_zip_dir/subdir/file2-1.0.0.txt");
  mgz::io::file file2("ziptest/to_zip_dir/test.txt");
  mgz::compress::archive::unzip uz(archive);
  uz.inflate(out);
  EXPECT_TRUE(file1.exist());
  EXPECT_TRUE(file2.exist());
  EXPECT_EQ(26U,file1.size());
  EXPECT_EQ(57U,file2.size());
 }

