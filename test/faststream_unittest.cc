#include <map>
#include <limits.h>
#include "io/faststream.h"
#include "gtest/gtest.h"
#include "config-test.h"

TEST(Faststream, TestReadPath) {
  mgz::io::faststream f(MGZ_TESTS_PATH(file/example.txt), mgz::io::out);
  ASSERT_TRUE(f.open(1024));

  std::vector<unsigned char>data = f.read();
  ASSERT_EQ(data.size(), f.gcount());

  data = f.read();
  ASSERT_EQ(0, f.gcount());
  data = f.read();
  ASSERT_EQ(0, f.gcount());

  ASSERT_TRUE(f.close());
}

TEST(Faststream, TestReadFile) {
  mgz::io::file file(MGZ_TESTS_PATH(file/example.txt));
  mgz::io::faststream f(file, mgz::io::out);
  ASSERT_TRUE(f.open(1024));

  std::vector<unsigned char>data = f.read();
  ASSERT_EQ(data.size(), f.gcount());

  data = f.read();
  ASSERT_EQ(0, f.gcount());
  data = f.read();
  ASSERT_EQ(0, f.gcount());

  ASSERT_TRUE(f.close());
}

TEST(Faststream, TestWritePath) {
  mgz::io::faststream f("test.txt", mgz::io::in);
  ASSERT_TRUE(f.open(1025));

  for(int i = 1; i <= 500000; i++) {
    std::string d("Hello World!\n");
    std::vector<unsigned char> b(d.begin(), d.end());
    ASSERT_EQ(b.size()*i, f.write(b));
  }

  ASSERT_TRUE(f.close());
}
