#include "net/mime.h"
#include "gtest/gtest.h"

TEST(Mime, TestMime) {
   EXPECT_TRUE(0 < mgz::net::mime_type("xml").types.size());
   EXPECT_TRUE(0 < mgz::net::mime_type("cpp").types.size());
   EXPECT_TRUE(0 < mgz::net::mime_type("css").types.size());
   EXPECT_TRUE(0 < mgz::net::mime_type("swf").types.size());
}

TEST(Mime, DefaultType) {
   mgz::net::mime_type java("java");
   EXPECT_EQ(1, java.types.size());
   EXPECT_EQ("text/plain", java[0].name);
}

TEST(Mime, TestEqual) {
  EXPECT_TRUE(mgz::net::mime_type("ppt") == mgz::net::mime_type("pot"));
  EXPECT_TRUE(mgz::net::mime_type("mp4") == mgz::net::mime_type("mpg4"));

  EXPECT_TRUE(mgz::net::mime_type("jpg") != mgz::net::mime_type("png"));
  EXPECT_TRUE(mgz::net::mime_type("ico") != mgz::net::mime_type("midi"));
}

TEST(Mime, AddCustomType) {
  mgz::net::mime_type mdt("mydummytype");
  EXPECT_EQ(1, mdt.types.size());
  EXPECT_EQ("text/plain", mdt[0].name);

  mgz::net::mime::new_type("mydummytype", "dummy/bin.type");

  mgz::net::mime_type mdtc("mydummytype");
  EXPECT_EQ(1, mdtc.types.size());
  EXPECT_EQ("dummy/bin.type", mdtc[0].name);
}
