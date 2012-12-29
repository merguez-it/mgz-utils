#include <map>
#include <limits.h>
#include "io/file.h"
#include "gtest/gtest.h"
#include "config-test.h"

TEST(FileUtil, TestFileType) {
   mgz::io::file d (".");
   EXPECT_TRUE(d.exist());
   EXPECT_TRUE(d.is_directory());
   EXPECT_FALSE(d.is_file());
   EXPECT_FALSE(d.is_symlink());
   EXPECT_TRUE(d.is_defined());
   EXPECT_EQ(0L, d.size());

   mgz::io::file f(MGZ_TESTS_PATH(file/example.txt));
   EXPECT_TRUE(f.exist());
   EXPECT_FALSE(f.is_directory());
   EXPECT_TRUE(f.is_file());
   EXPECT_EQ(47L, f.size());
 }

TEST(FileUtil, TestCreateAndRemoveDirectory) {
   mgz::io::file k ("toto/titi/tata");
   EXPECT_TRUE(k.mkdirs());
   EXPECT_TRUE(k.exist());
   EXPECT_TRUE(k.is_directory());
   EXPECT_TRUE(k.is_defined());
   EXPECT_FALSE(k.is_file());
   EXPECT_TRUE(k.remove());
   EXPECT_FALSE(k.exist());

   mgz::io::file l ("toto");
   EXPECT_FALSE(l.remove());
   EXPECT_TRUE(l.force_remove());
   EXPECT_FALSE(l.exist());
}

TEST(FileUtil, TestCopyFile) {
   mgz::io::file src (MGZ_TESTS_PATH(file/example.txt));
   mgz::io::file dest ("a/b/example.txt");
   EXPECT_FALSE(dest.exist());
   EXPECT_TRUE(src.copy(dest));
   EXPECT_TRUE(dest.exist());

   mgz::io::file d ("a");
   EXPECT_FALSE(d.remove());
   EXPECT_TRUE(d.force_remove());
   EXPECT_FALSE(d.exist());
}

TEST(FileUtil, TestCopyDir) {
   mgz::io::file src (MGZ_TESTS_PATH(../../include));
   mgz::io::file dest ("include_copy");

   EXPECT_FALSE(dest.exist());
   EXPECT_TRUE(src.copy(dest));
   EXPECT_TRUE(dest.exist());

   EXPECT_TRUE(dest.force_remove());
   EXPECT_FALSE(dest.exist());
}

TEST(FileUtil, TestCopyEmptyFile) {
  mgz::io::file src (MGZ_TESTS_PATH(file/empty.txt));
  mgz::io::file dest ("a/b/empty.txt");
  EXPECT_TRUE(dest.force_remove());
  EXPECT_TRUE(src.copy(dest));  
  EXPECT_TRUE(dest.exist());

  mgz::io::file a("a");
  a.force_remove();
  ASSERT_FALSE(a.exist());
}

TEST(FileUtil, TestCopyEmptyDir) {
  mgz::io::file src ("empty_folder");
  mgz::io::file dest ("empty_folder_copy");

  EXPECT_TRUE(src.force_remove());
  EXPECT_TRUE(dest.force_remove());
  src.mkdirs();
  EXPECT_TRUE(src.copy(dest));
  mgz::io::file d ("empty_folder_copy");
  EXPECT_TRUE(d.exist());

  src.force_remove();
  dest.force_remove();
}

TEST(FileUtil, TestExtension) {
   mgz::io::file f("toto.tar.gz");
   EXPECT_EQ("gz", f.get_extension());
}

TEST(FileUtil, TestExtensionWhenNoFileName) {
  mgz::io::file f(".gz");
  EXPECT_EQ("gz", f.get_extension());
}

TEST(FileUtil, TestGetWithoutExtension) {
  mgz::io::file f("titi/toto.tar.gz");
  EXPECT_EQ("toto.tar", f.get_name_without_extension());
}

TEST(FileUtil, TestGetWithoutExtensionWhenNoExtension) {
  mgz::io::file f("I got no extension");
  EXPECT_EQ("I got no extension", f.get_name_without_extension());
}

TEST(FileUtil, TestGetWithoutExtensionWhenNoFileName) {
  mgz::io::file f(".tut");
  EXPECT_EQ("", f.get_name_without_extension());
}

TEST(FileUtil, TestUndefined) {
  mgz::io::file f;
  EXPECT_FALSE(f.is_defined());
}

TEST(FileUtil, TestExist) {
  mgz::io::file f(MGZ_TESTS_PATH(file/exist.txt));
  EXPECT_TRUE(f.exist());
}

TEST(FileUtil, TestNotExist) {
  mgz::io::file f(MGZ_TESTS_PATH(file/does-not-exist.txt));
  EXPECT_FALSE(f.exist());
}

TEST(FileUtil, TestExistButEmpty) {
  mgz::io::file f(MGZ_TESTS_PATH(file/empty.txt));
  EXPECT_TRUE(f.exist());
}

TEST(FileUtil, GetAbsolutePath) {
#ifdef __WIN32__
  std::string path("c:\\hello\\world.txt");
#else
  std::string path("/hello/world.txt");
#endif
  mgz::io::file f(path);
  EXPECT_STREQ(path.c_str(), f.get_absolute_path().c_str());
}

TEST(FileUtil, PathWithTilde) {
#ifdef __WIN32__
  std::string path("~\\world.txt");
#else
  std::string path("~/world.txt");
#endif
  mgz::io::file f(path);

  EXPECT_NE(path, f.get_path());
  EXPECT_EQ(f.get_absolute_path(), f.get_path());
  size_t tild = f.get_path().find("~");
  EXPECT_EQ(tild, std::string::npos);
}

TEST(FileUtil, RelativeFrom) {
#ifdef __WIN32__
  std::string path("c:\\hello\\dummy\\world.txt");
  mgz::io::file from("c:\\hello");
  std::string result("dummy\\world.txt");
#else
  std::string path("/hello/dummy/world.txt");
  mgz::io::file from("/hello");
  std::string result("dummy/world.txt");
#endif
  mgz::io::file f(path);
  EXPECT_STREQ(result.c_str(), f.relative_path_from(from).c_str());
}

TEST(FileUtil, TestWithoutVersion) {
   std::map<std::string, std::string> files;
   files["/foo/bar/junit-dep-4.5.jar"] = "/foo/bar/junit-dep.jar";
   files["/foo/bar/hamcrest-core-1.1.jar"] = "/foo/bar/hamcrest-core.jar";
   files["/foo/bar/foobar-entity-1.12-20110429.143913-147.jar"] = "/foo/bar/foobar-entity.jar";
   files["/foo/bar/hibernate-core-3.6.0.Final.jar"] = "/foo/bar/hibernate-core.jar";
   files["/foo/bar/antlr-2.7.6.jar"] = "/foo/bar/antlr.jar";
   files["/foo/bar/commons-collections-3.2.1.jar"] = "/foo/bar/commons-collections.jar";
   files["/foo/bar/dom4j-1.6.1.jar"] = "/foo/bar/dom4j.jar";
   files["/foo/bar/hibernate-commons-annotations-3.2.0.Final.jar"] = "/foo/bar/hibernate-commons-annotations.jar";
   files["/foo/bar/hibernate-jpa-2.0-api-1.0.0.Final.jar"] = "/foo/bar/hibernate-jpa.jar";
   files["/foo/bar/jta-1.1.jar"] = "/foo/bar/jta.jar";
   files["/foo/bar/slf4j-api-1.6.1.jar"] = "/foo/bar/slf4j-api.jar";
   files["/foo/bar/ehcache-core-1.7.2.jar"] = "/foo/bar/ehcache-core.jar";
   files["/foo/bar/hibernate-search-3.3.0.Final.jar"] = "/foo/bar/hibernate-search.jar";
   files["/foo/bar/hibernate-search-analyzers-3.3.0.Final.jar"] = "/foo/bar/hibernate-search-analyzers.jar";
   files["/foo/bar/lucene-analyzers-3.0.3.jar"] = "/foo/bar/lucene-analyzers.jar";
   files["/foo/bar/lucene-core-3.0.3.jar"] = "/foo/bar/lucene-core.jar";
   files["/foo/bar/xfire-java5-1.2.6.jar"] = "/foo/bar/xfire-java5.jar";
   files["/foo/bar/commons-httpclient-3.1.jar"] = "/foo/bar/commons-httpclient.jar";
   files["/foo/bar/joda-time-1.5.2.jar"] = "/foo/bar/joda-time.jar";
   files["/foo/bar/stax-1.2.0.jar"] = "/foo/bar/stax.jar";
   files["/foo/bar/asm-3.1.jar"] = "/foo/bar/asm.jar";
   files["/foo/bar/cglib-nodep-2.2.jar"] = "/foo/bar/cglib-nodep.jar";
   files["/foo/bar/mysql-connector-java-5.1.6.jar"] = "/foo/bar/mysql-connector-java.jar";
   files["/foo/bar/sqlite-dialect-1.1.jar"] = "/foo/bar/sqlite-dialect.jar";
   files["/foo/bar/sqlite-driver-0.53.1.jar"] = "/foo/bar/sqlite-driver.jar";
   files["/foo/bar/spring-jdbc-3.0.5.RELEASE.jar"] = "/foo/bar/spring-jdbc.jar";
   files["/foo/bar/spring-orm-3.0.5.RELEASE.jar"] = "/foo/bar/spring-orm.jar";
   files["/foo/bar/slf4j-log4j12-1.6.1.jar"] = "/foo/bar/slf4j-log4j12.jar";
   files["/foo/bar/javassist-3.4.GA.jar"] = "/foo/bar/javassist.jar";
   files["/foo/bar/joda-time-hibernate-vidal-1.2.jar"] = "/foo/bar/joda-time-hibernate-vidal.jar";
   files["/foo/bar/c3p0-0.9.1.2.jar"] = "/foo/bar/c3p0.jar";
   files["/foo/bar/stringtemplate-3.2.jar"] = "/foo/bar/stringtemplate.jar";
   files["/foo/bar/spring-core-3.0.5.RELEASE.jar"] = "/foo/bar/spring-core.jar";
   files["/foo/bar/spring-asm-3.0.5.RELEASE.jar"] = "/foo/bar/spring-asm.jar";
   files["/foo/bar/spring-context-3.0.5.RELEASE.jar"] = "/foo/bar/spring-context.jar";
   files["/foo/bar/spring-beans-3.0.5.RELEASE.jar"] = "/foo/bar/spring-beans.jar";
   files["/foo/bar/spring-expression-3.0.5.RELEASE.jar"] = "/foo/bar/spring-expression.jar";
   files["/foo/bar/spring-tx-3.0.5.RELEASE.jar"] = "/foo/bar/spring-tx.jar";
   files["/foo/bar/aopalliance-1.0.jar"] = "/foo/bar/aopalliance.jar";
   files["/foo/bar/spring-web-3.0.5.RELEASE.jar"] = "/foo/bar/spring-web.jar";
   files["/foo/bar/spring-aop-3.0.5.RELEASE.jar"] = "/foo/bar/spring-aop.jar";
   files["/foo/bar/velocity-1.5.jar"] = "/foo/bar/velocity.jar";
   files["/foo/bar/commons-logging-1.1.1.jar"] = "/foo/bar/commons-logging.jar";
   files["/foo/bar/log4j-1.2.14.jar"] = "/foo/bar/log4j.jar";
   files["/foo/bar/commons-lang-2.5.jar"] = "/foo/bar/commons-lang.jar";
   files["/foo/bar/commons-codec-1.3.jar"] = "/foo/bar/commons-codec.jar";
   files["/foo/bar/jettison-1.0.1.jar"] = "/foo/bar/jettison.jar";
   files["/foo/bar/stax-api-1.0.1.jar"] = "/foo/bar/stax-api.jar";
   files["/foo/bar/xstream-1.3.1.jar"] = "/foo/bar/xstream.jar";
   files["/foo/bar/xpp3_min-1.1.4c.jar"] = "/foo/bar/xpp3_min.jar";
   files["/foo/bar/multiplegradientpaint-1.0.jar"] = "/foo/bar/multiplegradientpaint.jar";
   files["/foo/bar/swing-worker-1.1.jar"] = "/foo/bar/swing-worker.jar";
   files["/foo/bar/swingx-0.9.5.jar"] = "/foo/bar/swingx.jar";
   files["/foo/bar/swing-layout-1.0.3.jar"] = "/foo/bar/swing-layout.jar";
   files["/foo/bar/eventbus-1.1-beta1.jar"] = "/foo/bar/eventbus.jar";
   files["/foo/bar/aspectjrt-1.6.2.jar"] = "/foo/bar/aspectjrt.jar";
   files["/foo/bar/aspectjweaver-1.6.2.jar"] = "/foo/bar/aspectjweaver.jar";
   files["/foo/bar/jsr250-api-1.0.jar"] = "/foo/bar/jsr250-api.jar";
   files["/foo/bar/jxlayer-3.0.2.jar"] = "/foo/bar/jxlayer.jar";
   files["/foo/bar/commons-io-1.4.jar"] = "/foo/bar/commons-io.jar";
   files["/foo/bar/core-renderer-1.0r8.jar"] = "/foo/bar/core-renderer.jar";
   files["/foo/bar/ostermillerutils-1.06.01.jar"] = "/foo/bar/ostermillerutils.jar";
   files["/foo/bar/itext-1.5.4.jar"] = "/foo/bar/itext.jar";
   files["/foo/bar/pdfrenderer-1.0.jar"] = "/foo/bar/pdfrenderer.jar";
   files["/foo/bar/mockito-all-1.8.5.jar"] = "/foo/bar/mockito-all.jar";
   files["/foo/bar/AppleJavaExtensions-1.3.jar"] = "/foo/bar/AppleJavaExtensions.jar";

   std::map<std::string, std::string>::iterator it;
   for(it = files.begin(); it != files.end(); it++) {
      mgz::io::file f((*it).first);
      mgz::io::file n((*it).second);
      EXPECT_EQ(n.get_path(), f.get_path_without_version());
   }
}

TEST(FileUtil, JoinOne) {
#ifdef __WIN32__
  mgz::io::file path("c:\\hello\\");
  mgz::io::file from("world.txt");
  std::string result("c:\\hello\\world.txt");
#else
  mgz::io::file path("/hello/");
  mgz::io::file from("world.txt");
  std::string result("/hello/world.txt");
#endif
  EXPECT_STREQ(result.c_str(), path.join(from).get_path().c_str());
}

TEST(FileUtil, JoinTwo) {
#ifdef __WIN32__
  mgz::io::file path("c:\\hello");
  mgz::io::file from("world.txt");
  std::string result("c:\\hello\\world.txt");
#else
  mgz::io::file path("/hello");
  mgz::io::file from("world.txt");
  std::string result("/hello/world.txt");
#endif
  EXPECT_STREQ(result.c_str(), mgz::io::file().join(path, from).get_path().c_str());
}

TEST(FileUtil, JoinOneStr) {
#ifdef __WIN32__
  mgz::io::file path("c:\\hello\\");
  std::string from("world.txt");
  std::string result("c:\\hello\\world.txt");
#else
  mgz::io::file path("/hello/");
  std::string from("world.txt");
  std::string result("/hello/world.txt");
#endif
  EXPECT_STREQ(result.c_str(), path.join(from).get_path().c_str());
}

TEST(FileUtil, JoinTwoStr) {
#ifdef __WIN32__
  std::string path("c:\\hello");
  std::string from("world.txt");
  std::string result("c:\\hello\\world.txt");
#else
  std::string path("/hello");
  std::string from("world.txt");
  std::string result("/hello/world.txt");
#endif
  EXPECT_STREQ(result.c_str(), mgz::io::file().join(path, from).get_path().c_str());
}

TEST(FileUtil, TestMoveFileKeepingName) {
  // Préparation
  mgz::io::file src (MGZ_TESTS_PATH(file/example.txt));
  mgz::io::file  srcdir("move/from");
  mgz::io::file  dstdir("move/to");
  EXPECT_TRUE(mgz::io::file("move").force_remove());
  EXPECT_TRUE(srcdir.mkdirs());
  EXPECT_TRUE(src.copy(srcdir));
  mgz::io::file srcfile(srcdir.join("example.txt"));
  mgz::io::file dstfile(dstdir.join("example.txt"));
  // Test 
  EXPECT_TRUE(srcfile.move(dstfile));
  EXPECT_TRUE(dstfile.exist());
  EXPECT_FALSE(srcfile.exist());
  EXPECT_TRUE(mgz::io::file("move").force_remove());
}

TEST(FileUtil, TestMoveFileChangingName) {
  // Préparation
  mgz::io::file src (MGZ_TESTS_PATH(file/example.txt));
  mgz::io::file  srcdir("move/from");
  mgz::io::file  dstdir("move/to");
  EXPECT_TRUE(mgz::io::file("move").force_remove());
  EXPECT_TRUE(srcdir.mkdirs());
  EXPECT_TRUE(dstdir.mkdirs());
  EXPECT_TRUE(src.copy(srcdir));
  mgz::io::file srcfile(srcdir.join("example.txt"));
  mgz::io::file dstfile(dstdir.join("sample2.properties"));
  // Test 
  EXPECT_TRUE(srcfile.move(dstfile));
  EXPECT_TRUE(dstfile.exist());
  EXPECT_FALSE(srcfile.exist());
  EXPECT_TRUE(mgz::io::file("move").force_remove());
}

TEST(FileUtil, TestMoveDirectoryIntoExistingOne) {
  // moving "move/from" that contains one file into "move/to" => "move/to/from/example.txt"
  mgz::io::file src(MGZ_TESTS_PATH(file/example.txt));
  mgz::io::file  srcdir("move/from");
  mgz::io::file  dstdir("move/to"); // Final slash is not required, as the folder already exist (no ambiguity)
  EXPECT_TRUE(mgz::io::file("move").force_remove());
  EXPECT_TRUE(srcdir.mkdirs());
  EXPECT_TRUE(dstdir.mkdirs());
  EXPECT_TRUE(src.copy(srcdir));
  // Test
  EXPECT_TRUE(srcdir.move(dstdir));
  EXPECT_TRUE(dstdir.join("from").exist());
  EXPECT_FALSE(srcdir.exist());
  EXPECT_TRUE(dstdir.join("from").join("example.txt").exist());
  EXPECT_TRUE(mgz::io::file("move").force_remove());
}

TEST(FileUtil, TestMoveDirectoryIntoNonExistingOne) {
  mgz::io::file src(MGZ_TESTS_PATH(file/example.txt));
  mgz::io::file  srcdir("move/from");
  mgz::io::file  dstdir("move/to/"); // Final slash is important to resolve ambiguity (otherwise, would be processed as a "rename")
  EXPECT_TRUE(mgz::io::file("move").force_remove());
  EXPECT_TRUE(srcdir.mkdirs());
  EXPECT_TRUE(src.copy(srcdir));
  // Test
  EXPECT_TRUE(srcdir.move(dstdir));
  EXPECT_TRUE(dstdir.join("from").exist());
  EXPECT_FALSE(srcdir.exist());
  EXPECT_TRUE(dstdir.join("from").join("example.txt").exist());
  EXPECT_TRUE(mgz::io::file("move").force_remove());
}

TEST(FileUtil, TestRenameDirectoryToNonExistingOne) {
  mgz::io::file src(MGZ_TESTS_PATH(file/example.txt));
  mgz::io::file  srcdir("move/from");
  mgz::io::file  dstdir("move/to"); // Final is absent to ask for a rename 
  EXPECT_TRUE(mgz::io::file("move").force_remove());
  EXPECT_TRUE(srcdir.mkdirs());
  EXPECT_TRUE(src.copy(srcdir));
  // Test
  EXPECT_TRUE(srcdir.move(dstdir));
  EXPECT_TRUE(dstdir.exist());
  EXPECT_FALSE(srcdir.exist());
  EXPECT_TRUE(dstdir.join("example.txt").exist());
  EXPECT_TRUE(mgz::io::file("move").force_remove());
}

TEST(FileUtil, TestFileWithSpace) {
  mgz::io::file r("directory with space");
  mgz::io::file f("directory with space/and a second one/");
  EXPECT_TRUE(r.force_remove());
  EXPECT_TRUE(f.mkdirs());
  EXPECT_TRUE(f.exist());
  EXPECT_TRUE(f.force_remove());
  EXPECT_TRUE(r.exist());
  EXPECT_FALSE(f.exist());
  EXPECT_TRUE(r.force_remove());
  EXPECT_FALSE(r.exist());
}

TEST(FileUtil, TestExecutabilityFlags) {
#ifndef __WIN32__
  mgz::io::file appliMac(MGZ_TESTS_PATH(file/HelloWorld.app));
  ASSERT_TRUE(appliMac.exist());
  EXPECT_FALSE(appliMac.has_executable_flag());

  mgz::io::file executableMac(MGZ_TESTS_PATH(file/HelloWorld.app/Contents/MacOS/applet));
  ASSERT_TRUE(executableMac.exist());
  EXPECT_TRUE(executableMac.has_executable_flag());

  mgz::io::file executableShell(MGZ_TESTS_PATH(file/x-flag-set.sh));
  ASSERT_TRUE(executableShell.exist());
  EXPECT_TRUE(executableShell.has_executable_flag());

  mgz::io::file nonExecutableShell(MGZ_TESTS_PATH(file/x-flag-not-set.sh));
  ASSERT_TRUE(nonExecutableShell.exist());
  EXPECT_FALSE(nonExecutableShell.has_executable_flag());
#endif
}

TEST(FileUtil, SetExecutabilityFlags) {
#ifndef __WIN32__
  mgz::io::file nonExecutableShell(MGZ_TESTS_PATH(file/x-flag-not-set.sh));
  ASSERT_TRUE(nonExecutableShell.exist());
  ASSERT_FALSE(nonExecutableShell.has_executable_flag());
  mgz::io::file nonExecutableShellCopy("executable.sh");
  nonExecutableShellCopy.force_remove();
  nonExecutableShell.copy(nonExecutableShellCopy);
  ASSERT_TRUE(nonExecutableShellCopy.exist());
  EXPECT_FALSE(nonExecutableShellCopy.has_executable_flag());
  FILE* cmd=popen("./executable.sh","r");
  while (getc(cmd)!=EOF);
  int ret = pclose(cmd);
  EXPECT_EQ(WEXITSTATUS(ret),126);
  nonExecutableShellCopy.set_executable_flag();
  EXPECT_TRUE(nonExecutableShellCopy.has_executable_flag());
  cmd=popen("./executable.sh","r");
  while (getc(cmd)!=EOF);
  ret = pclose(cmd);
  EXPECT_EQ(WEXITSTATUS(ret),69);
  nonExecutableShellCopy.remove();
  EXPECT_FALSE(nonExecutableShellCopy.exist());
#endif
}

TEST(FileUtil, TestSetGetPermissions) {
#ifndef __WIN32__
  mgz::io::file fileWithPerms(MGZ_TESTS_PATH(file/SetAndTestPermissions.txt));
  ASSERT_TRUE(fileWithPerms.exist());
  mgz::io::file fileWithPermsCopy("SetAndTestPermissions.txt");
  ASSERT_TRUE(fileWithPermsCopy.force_remove());
  ASSERT_TRUE(fileWithPerms.copy(fileWithPermsCopy));
  fileWithPermsCopy.set_permissions(0754);
  EXPECT_TRUE((fileWithPermsCopy.get_mode() & 0754) == 0754);
  fileWithPermsCopy.remove();
  EXPECT_FALSE(fileWithPermsCopy.exist());
#endif
}

TEST(FileUtil, TestCopyWithPermissions) {
#ifndef __WIN32__
  mgz::io::file fileWithPerms0654(MGZ_TESTS_PATH(file/fileWithPerms0654.txt));
  ASSERT_TRUE(fileWithPerms0654.exist());
  fileWithPerms0654.set_permissions(0654);
  mgz::io::file fileWithPerms0654Copy("fileWithPerms0654.txt");
  ASSERT_TRUE(fileWithPerms0654Copy.force_remove());
  fileWithPerms0654.copy(fileWithPerms0654Copy);
  
  EXPECT_TRUE(fileWithPerms0654Copy.exist());
  EXPECT_TRUE((fileWithPerms0654Copy.get_mode() & 0654) == 0654);

  fileWithPerms0654Copy.remove();
#endif
}

TEST(FileUtil, TestSymlink) {
#ifndef __WIN32__
  mgz::io::file a("a");
  mgz::io::file root = a.join("b/c/end");
  mgz::io::file f = root.join("empty.txt");
  mgz::io::file src (MGZ_TESTS_PATH(file/empty.txt));

  mgz::io::file link_root("testlink");
  mgz::io::file rlink = link_root.join("relative_dir");
  mgz::io::file alink = link_root.join("absolute_dir");
  mgz::io::file rfile = rlink.join("relative_file.txt");
  mgz::io::file afile = alink.join("absolute_file.txt");

  EXPECT_TRUE(src.copy(f));  
  ASSERT_TRUE(root.exist());
  ASSERT_TRUE(f.exist());

  rlink.relative_link(root);
  alink.absolute_link(root);

  ASSERT_TRUE(rlink.exist());
  ASSERT_TRUE(rlink.is_symlink());
  ASSERT_TRUE(rlink.is_directory());
  ASSERT_TRUE(alink.exist());
  ASSERT_TRUE(alink.is_symlink());
  ASSERT_TRUE(alink.is_directory());

  rfile.relative_link(f);
  afile.absolute_link(f);

  ASSERT_TRUE(rfile.exist());
  ASSERT_TRUE(rfile.is_symlink());
  ASSERT_TRUE(rfile.is_file());
  ASSERT_TRUE(afile.exist());
  ASSERT_TRUE(afile.is_symlink());
  ASSERT_TRUE(afile.is_file());

  mgz::io::file fileinrlink = rlink.join("empty.txt");
  EXPECT_TRUE(fileinrlink.is_under_symlink());

  mgz::io::file fileinalink = alink.join("empty.txt");
  EXPECT_TRUE(fileinalink.is_under_symlink());

  EXPECT_FALSE(f.is_under_symlink());

  link_root.force_remove();
  ASSERT_FALSE(link_root.exist());
  ASSERT_TRUE(root.exist());

  ASSERT_TRUE(a.force_remove());
  ASSERT_FALSE(a.exist());
#endif
}

TEST(FileUtil, TestNormalize) {
  mgz::io::file x("../toto/../titi/tata/file.txt");
  EXPECT_EQ(x.get_normalize_path(), x.get_absolute_path());

#ifndef __WIN32__
  mgz::io::file a("a");
  mgz::io::file f = a.join("b/c/empty.txt");
  mgz::io::file src(MGZ_TESTS_PATH(file/empty.txt));

  ASSERT_TRUE(src.copy(f));
  ASSERT_TRUE(f.exist());

  mgz::io::file alink("alink");
  mgz::io::file flink = alink.join("b/c/empty.txt");

  ASSERT_TRUE(alink.relative_link(a));

  ASSERT_TRUE(flink.exist());
  ASSERT_TRUE(flink.is_under_symlink());
  ASSERT_EQ(flink.get_absolute_path(), f.get_absolute_path());
  ASSERT_NE(flink.get_normalize_path(), f.get_normalize_path());

  alink.force_remove();
  a.force_remove();
  ASSERT_FALSE(a.exist());
#endif
}
