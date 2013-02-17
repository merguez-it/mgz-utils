#include <iostream>
#include <map>
#include <limits.h>
#include "io/filesystem.h"

#include "gtest/gtest.h"
#include "config-test.h"

TEST(Filesystem, DiskSpace) {
#ifdef __WIN32__
  mgz::io::fs f("C:\\");
#else
  mgz::io::fs f("/");
#endif

  unsigned long free = f.free_space();
  unsigned long total = f.total_space();
  unsigned long used = f.used_space();

  EXPECT_TRUE(total == (used + free));
}

TEST(Filesystem, TestEACH_FILES) {
  EACH_FILES(ff, ".") {
    EXPECT_TRUE(ff->exist());
  }

  EACH_FILES_R(fg, "..") {
    EXPECT_TRUE(fg->exist());
  }

  std::vector<mgz::io::file> f = mgz::io::fs(".").all_files(false);
  EXPECT_TRUE(0 < f.size());

  std::vector<mgz::io::file> f2 = mgz::io::fs(MGZ_TESTS_PATH(file)).all_files(true, true);
  EXPECT_TRUE(0 < f.size());
}

TEST(Filesystem,TestExist) {
#ifdef __WIN32__
  mgz::io::fs f("C:\\");
#else
  mgz::io::fs f("/");
#endif
  EXPECT_TRUE(f.exist());
}

TEST(Filesystem,TestDoesntExist) {
#ifdef __WIN32__
  mgz::io::fs f("XX:\\");
#else
  mgz::io::fs f("/DoesntExist");
#endif
  EXPECT_FALSE(f.exist());
}

//TEST(Filesystem, TestAllFilesFiltered) {
//  mgz::io::fs f(MGZ_TESTS_PATH(vfsfilter/v1));
//  Glow::vfsfilter filter(mgz::io::file(MGZ_TESTS_PATH(vfsfilter/vfsfilter.properties)));
//  EXPECT_EQ(4U,f.all_files_filtered(filter).size());
//}
//
//TEST(Filesystem, TestAllFilesNotFiltered) {
//  mgz::io::fs f(MGZ_TESTS_PATH(vfsfilter/v1));
//  Glow::vfsfilter filter(mgz::io::file(MGZ_TESTS_PATH(vfsfilter/vfsfilter-no-subdir-no-jars.properties)));
//  EXPECT_EQ(4U,f.all_files_filtered(filter,true).size());
//}
