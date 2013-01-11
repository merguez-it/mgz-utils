#include "security/sha2.h"
#include "gtest/gtest.h"

TEST(Hash, Test224) {
  mgz::security::sha224sum s224;
  const unsigned char data[] = "Hello World!";
  s224.update(data, 12);
  s224.finalize();
  std::string r = s224.hexdigest();
  EXPECT_EQ("4575bb4ec129df6380cedde6d71217fe0536f8ffc4e18bca530a7a1b", r);
}

TEST(Hash, Test256) {
  mgz::security::sha256sum s256;
  const unsigned char data[] = "Hello World!";
  s256.update(data, 12);
  s256.finalize();
  std::string r = s256.hexdigest();
  EXPECT_EQ("7f83b1657ff1fc53b92dc18148a1d65dfc2d4b1fa3d677284addd200126d9069", r);
}

TEST(Hash, Test384) {
  mgz::security::sha384sum s384;
  const unsigned char data[] = "Hello World!";
  s384.update(data, 12);
  s384.finalize();
  std::string r = s384.hexdigest();
  EXPECT_EQ("bfd76c0ebbd006fee583410547c1887b0292be76d582d96c242d2a792723e3fd6fd061f9d5cfd13b8f961358e6adba4a", r);
}

TEST(Hash, Test512) {
  mgz::security::sha512sum s512;
  const unsigned char data[] = "Hello World!";
  s512.update(data, 12);
  s512.finalize();
  std::string r = s512.hexdigest();
  EXPECT_EQ("861844d6704e8573fec34d967e20bcfef3d424cf48be04e6dc08f2bd58c729743371015ead891cc3cf1c9d34b49264b510751b1ff9e537937bc46b5d6ff4ecc8", r);
}

TEST(Hash, Test224_direct) {
  std::string n = mgz::security::sha224("Hello World!");
  EXPECT_EQ("4575bb4ec129df6380cedde6d71217fe0536f8ffc4e18bca530a7a1b", n);
}

TEST(Hash, Test256_direct) {
  std::string n = mgz::security::sha256("Hello World!");
  EXPECT_EQ("7f83b1657ff1fc53b92dc18148a1d65dfc2d4b1fa3d677284addd200126d9069", n);
}

TEST(Hash, TestSha384_direct) {
  std::string n = mgz::security::sha384("Hello World!");
  EXPECT_EQ("bfd76c0ebbd006fee583410547c1887b0292be76d582d96c242d2a792723e3fd6fd061f9d5cfd13b8f961358e6adba4a", n);
}

TEST(Hash, TestSha512_direct) {
  std::string n = mgz::security::sha512("Hello World!");
  EXPECT_EQ("861844d6704e8573fec34d967e20bcfef3d424cf48be04e6dc08f2bd58c729743371015ead891cc3cf1c9d34b49264b510751b1ff9e537937bc46b5d6ff4ecc8", n);
}
