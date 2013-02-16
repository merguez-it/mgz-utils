#include <fstream>
#include "compress/z.h"
#include "io/file.h"
#include "gtest/gtest.h"
#include "config-test.h"

TEST(Mgz, TestDeflateFILE) {
  mgz::io::file in_file(MGZ_TESTS_PATH(compress/z_test_deflate.txt));
  mgz::io::file out_file("z_test_deflate.file.gz");

  FILE *in = fopen(in_file.get_path().c_str(), "rb");
  FILE *out = fopen(out_file.get_path().c_str(), "wb");

  mgz::compress::Z z(mgz::compress::GZIP);
  z.deflate(in, out);

  fclose(in);
  fclose(out);

  EXPECT_TRUE(out_file.exist());
}

TEST(Mgz, TestDeflateSTREAM) {
  mgz::io::file in_file(MGZ_TESTS_PATH(compress/z_test_deflate.txt));
  mgz::io::file out_file("z_test_deflate.stream.gz");

  std::fstream in(in_file.get_path().c_str(), std::fstream::in | std::fstream::binary);
  std::fstream out(out_file.get_path().c_str(), std::fstream::out | std::fstream::binary);

  mgz::compress::Z z(mgz::compress::GZIP);
  z.deflate(in, out);

  in.close();
  out.close();

  EXPECT_TRUE(out_file.exist());
}

TEST(Mgz, TestDeflateVECTOR) {
  std::vector<unsigned char> out;

  mgz::compress::Z z(mgz::compress::GZIP);

  std::string data("Hello World!\n");
  std::vector<unsigned char> in(data.begin(), data.end());
  z.deflate(in, out);

  data = std::string("Hola Mundo!\n");
  in = std::vector<unsigned char>(data.begin(), data.end());
  z.deflate(in, out);

  data = std::string("Bonjour Monde!\n");
  in = std::vector<unsigned char>(data.begin(), data.end());
  z.deflate(in, out);

  data = std::string("\n");
  in = std::vector<unsigned char>(data.begin(), data.end());
  z.deflate(in, out);

  in.clear();
  z.deflate(in, out);

  ASSERT_TRUE(out.size() > 0);

  mgz::io::file out_file("z_test_deflate.vec.gz");
  std::ofstream writeFile;
  writeFile.open(out_file.get_path().c_str(), std::ios::out | std::ios::binary);
  if (!out.empty()) {
    writeFile.write(reinterpret_cast<char*>(&out[0]), out.size() * sizeof(out[0]));
  }

  EXPECT_TRUE(out_file.exist());
}

TEST(Mgz, TestInfate) {
  std::string outstr;
  mgz::io::file in_file(MGZ_TESTS_PATH(compress/z_test_inflate.txt.gz));
  FILE *in = fopen(in_file.get_path().c_str(), "rb");

  mgz::compress::Z z(mgz::compress::GZIP);
  ASSERT_TRUE(FLATE_IN == z.inflate_init());

  z.stream.avail_in = fread(z.stream.next_in, 1, BUFFER_SIZE, in);
  ASSERT_TRUE(z.stream.avail_in > 0);

  int rc;
  while((rc = z.inflate()) == FLATE_OUT) {
    outstr += std::string((const char*)z.stream.next_out, z.stream.avail_out);
  }
  ASSERT_TRUE(FLATE_OK == rc);
  ASSERT_EQ("Hello World!\nHola Mundo!\nBonjour Monde!\n\n", outstr);

  z.stream.avail_in += fread(z.stream.begin, 1, BUFFER_SIZE-z.stream.avail_in, in);
  ASSERT_TRUE(FLATE_END == z.inflate_end());

  fclose(in);
}

TEST(Mgz, TestInfateVECTOR) {
  std::vector<unsigned char> vec_in;
  std::vector<unsigned char> vec_out;

  mgz::io::file in_file(MGZ_TESTS_PATH(compress/z_test_inflate.txt.gz));
  std::fstream in(in_file.get_path().c_str(), std::fstream::in | std::fstream::binary);

  mgz::compress::Z z(mgz::compress::GZIP);

  unsigned char* buffer = (unsigned char*)malloc(BUFFER_SIZE);
  in.read((char*)buffer, BUFFER_SIZE);
  int in_size = in.gcount();
  vec_in = std::vector<unsigned char>(buffer, buffer+in_size);

  z.inflate(vec_in, vec_out);

  vec_in.clear();
  z.inflate(vec_in, vec_out);

  ASSERT_TRUE(vec_out.size() > 0);
  std::string outstr(vec_out.begin(), vec_out.end());
  ASSERT_EQ("Hello World!\nHola Mundo!\nBonjour Monde!\n\n", outstr);

  free(buffer);
  in.close();
}

TEST(Mgz, TestInfateFILE) {
  mgz::io::file in_file(MGZ_TESTS_PATH(compress/z_test_inflate.txt.gz));
  mgz::io::file out_file("z_test_inflate.txt");

  FILE *in = fopen(in_file.get_path().c_str(), "rb");
  FILE *out = fopen(out_file.get_path().c_str(), "wb");

  mgz::compress::Z z(mgz::compress::GZIP);
  z.inflate(in, out);

  fclose(in);
  fclose(out);

  EXPECT_TRUE(out_file.exist());
}

TEST(Mgz, TestDeflateInflate) {
  mgz::io::file ori_file(MGZ_TESTS_PATH(compress/lorem.txt));
  uint32_t ori_crc = ori_file.crc32();

  mgz::io::file deflate_file("lorem.txt.gz");
  mgz::io::file inflate_file("lorem.txt"); 

  {
    std::fstream in(ori_file.get_path().c_str(), std::fstream::in | std::fstream::binary);
    std::fstream out(deflate_file.get_path().c_str(), std::fstream::out | std::fstream::binary);

    mgz::compress::Z z(mgz::compress::GZIP);
    z.deflate(in, out);

    ASSERT_EQ(3826900168, z.get_crc32());
    ASSERT_EQ(2318608, z.get_compressed_size());
    ASSERT_EQ(9193929, z.get_uncompressed_size());

    in.close();
    out.close();
  }

  ASSERT_TRUE(deflate_file.exist());

  {
    std::fstream in(deflate_file.get_path().c_str(), std::fstream::in | std::fstream::binary);
    std::fstream out(inflate_file.get_path().c_str(), std::fstream::out | std::fstream::binary);

    mgz::compress::Z z(mgz::compress::GZIP);
    z.inflate(in, out);

    ASSERT_EQ(3826900168, z.get_crc32());
    ASSERT_EQ(2318608, z.get_compressed_size());
    ASSERT_EQ(9193929, z.get_uncompressed_size());

    in.close();
    out.close();
  }

  ASSERT_TRUE(inflate_file.exist());

  ASSERT_EQ(ori_crc, inflate_file.crc32());
}
