#include <time.h>
#include "gtest/gtest.h"

#include "net/uri.h"

TEST(URI, testSimple) {
  mgz::net::uri u = mgz::net::uri::parse("http://merguez-it.com");

  ASSERT_EQ("http", u.scheme());
  ASSERT_EQ("merguez-it.com", u.host());
  ASSERT_EQ(0, u.port());
}

TEST(URI, testFile) {
  mgz::net::uri u = mgz::net::uri::parse("file:///toto.txt");

  ASSERT_EQ("file", u.scheme());
  ASSERT_EQ("", u.host());
  ASSERT_EQ(0, u.port());
  ASSERT_EQ("/toto.txt", u.path());
}

TEST(URI, testWithPort) {
  mgz::net::uri u = mgz::net::uri::parse("http://merguez-it.com:80");

  ASSERT_EQ("http", u.scheme());
  ASSERT_EQ("merguez-it.com", u.host());
  ASSERT_EQ(80, u.port());
}

TEST(URI, testWithPath) {
  mgz::net::uri u = mgz::net::uri::parse("http://merguez-it.com/path/to/data");

  ASSERT_EQ("http", u.scheme());
  ASSERT_EQ("merguez-it.com", u.host());
  ASSERT_EQ(0, u.port());
  ASSERT_EQ("/path/to/data", u.path());
}

TEST(URI, testWithPortPath) {
  mgz::net::uri u = mgz::net::uri::parse("http://merguez-it.com:80/path/to/data");

  ASSERT_EQ("http", u.scheme());
  ASSERT_EQ("merguez-it.com", u.host());
  ASSERT_EQ(80, u.port());
  ASSERT_EQ("/path/to/data", u.path());
}

TEST(URI, testWithPortPathQuery) {
  mgz::net::uri u = mgz::net::uri::parse("http://merguez-it.com:80/path/to/data?key1=value1&key2=value2");

  ASSERT_EQ("http", u.scheme());
  ASSERT_EQ("merguez-it.com", u.host());
  ASSERT_EQ(80, u.port());
  ASSERT_EQ("/path/to/data", u.path());
  ASSERT_EQ("key1=value1&key2=value2", u.query_string());
  ASSERT_EQ("value1", u.query()["key1"]);
  ASSERT_EQ("value2", u.query()["key2"]);
}

TEST(URI, testWithPortPathQueryAndUser) {
  mgz::net::uri u = mgz::net::uri::parse("http://user:pass@merguez-it.com:80/path/to/data?key1=value1&key2=value2");

  ASSERT_EQ("http", u.scheme());
  ASSERT_EQ("merguez-it.com", u.host());
  ASSERT_EQ(80, u.port());
  ASSERT_EQ("/path/to/data", u.path());
  ASSERT_EQ("key1=value1&key2=value2", u.query_string());
  ASSERT_EQ("user", u.username());
  ASSERT_EQ("pass", u.password());
  ASSERT_EQ("value1", u.query()["key1"]);
  ASSERT_EQ("value2", u.query()["key2"]);
}

TEST(URI, testUserWithoutPassword) {
  mgz::net::uri u = mgz::net::uri::parse("http://user@merguez-it.com");

  ASSERT_EQ("http", u.scheme());
  ASSERT_EQ("merguez-it.com", u.host());
  ASSERT_EQ(0, u.port());
  ASSERT_EQ("user", u.username());
  ASSERT_EQ("", u.password());
}

TEST(URI, testPasswordWithoutUser) {
  mgz::net::uri u = mgz::net::uri::parse("http://:pass@merguez-it.com");

  ASSERT_EQ("http", u.scheme());
  ASSERT_EQ("merguez-it.com", u.host());
  ASSERT_EQ(0, u.port());
  ASSERT_EQ("", u.username());
  ASSERT_EQ("pass", u.password());
}

TEST(URI, testSinglePort) {
  mgz::net::uri u = mgz::net::uri::parse("http://:8090");

  ASSERT_EQ("http", u.scheme());
  ASSERT_EQ("", u.host());
  ASSERT_EQ(8090, u.port());
  ASSERT_EQ("", u.username());
  ASSERT_EQ("", u.password());
  ASSERT_EQ("", u.path());
  ASSERT_EQ("", u.query_string());
}

TEST(URI, testEncodeDecode) {
  ASSERT_TRUE(mgz::net::uri::encode("ABC") == "ABC");

  const std::string ORG("\0\1\2", 3);
  const std::string ENC("%00%01%02");
  ASSERT_TRUE(mgz::net::uri::encode(ORG) == ENC);
  ASSERT_TRUE(mgz::net::uri::decode(ENC) == ORG);

  ASSERT_TRUE(mgz::net::uri::encode("\xFF") == "%FF");
  ASSERT_TRUE(mgz::net::uri::decode("%FF") == "\xFF");
  ASSERT_TRUE(mgz::net::uri::decode("%ff") == "\xFF");

  // unsafe chars test, RFC1738
  const std::string UNSAFE(" <>#{}|\\^~[]`");
  std::string sUnsafeEnc = mgz::net::uri::encode(UNSAFE);
  ASSERT_TRUE(std::string::npos == sUnsafeEnc.find_first_of(UNSAFE));
  ASSERT_TRUE(mgz::net::uri::decode(sUnsafeEnc) == UNSAFE);

  // random test
  const int MAX_LEN = 128;
  char a[MAX_LEN];
  srand((unsigned)time(NULL));
  for (int i = 0; i < 100; i++) {
    for (int j = 0; j < MAX_LEN; j++) {
      a[j] = rand() % (1 << 8);
    }
    int nLen = rand() % MAX_LEN;
    std::string sOrg(a, nLen);
    std::string sEnc = mgz::net::uri::encode(sOrg);
    ASSERT_TRUE(sOrg == mgz::net::uri::decode(sEnc));
  }
}
