#include "security/ripem.h"
#include "gtest/gtest.h"

TEST(Hash, TestRipem128) {
  std::string n = mgz::security::ripem128("");
  EXPECT_EQ("cdf26213a150dc3ecb610f18f6b38b46", n);

  n = mgz::security::ripem128("Hello World!");
  EXPECT_EQ("24e23e5c25bc06c8aa43b696c1e11669", n);

  n = mgz::security::ripem128(std::string(1000000, 'a'));
  EXPECT_EQ("4a7f5723f954eba1216c9d8f6320431f", n);
}

TEST(Hash, TestRipem160) {
  std::string n = mgz::security::ripem160("");
  EXPECT_EQ("9c1185a5c5e9fc54612808977ee8f548b2258d31", n);

  n = mgz::security::ripem160("Hello World!");
  EXPECT_EQ("8476ee4631b9b30ac2754b0ee0c47e161d3f724c", n);

  n = mgz::security::ripem160(std::string(1000000, 'a'));
  EXPECT_EQ("52783243c1697bdbe16d37f97f68f08325dc1528", n);
}

