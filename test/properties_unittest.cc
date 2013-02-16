#include <limits.h>
#include "io/properties.h"
#include "io/file.h"
#include "gtest/gtest.h"
#include "config-test.h"

TEST(Properties, GetEmptyValue) {
   mgz::io::properties properties;

   std::string data = properties.get_property("does not exist");
   EXPECT_TRUE(data.empty());
}

TEST(Properties, GetDefaultValue) {
   const std::string defaultValue = "default value";
   mgz::io::properties properties;

   std::string data = properties.get_property("does not exist", defaultValue);
   ASSERT_FALSE(data.empty());

   EXPECT_EQ(defaultValue, data);
}

TEST(Properties, SetValue) {
   const std::string value = "custom value";
   const std::string defaultValue = "default value";

   mgz::io::properties properties;

   properties.set_property("key", value);
   std::string data = properties.get_property("key", defaultValue);

   EXPECT_EQ(value, data);
}

TEST(Properties, ResetValue) {
   const std::string value1 = "value 1";
   const std::string value2 = "value 2";
   const std::string key = "key";

   mgz::io::properties properties;

   properties.set_property(key, value1);
   EXPECT_EQ(value1, properties.get_property(key));
   EXPECT_EQ(value1, properties.set_property(key, value2));
   EXPECT_EQ(value2, properties.get_property(key));
}

TEST(Properties, ReadProperties) {
   const std::string en ("hello");
   const std::string fr ("bonjour");
   const std::string es ("hola");

   mgz::io::file pf(MGZ_TESTS_PATH(properties/sample.properties));
   mgz::io::file pf2("sample2.properties");
   mgz::io::properties properties (pf);

   EXPECT_EQ(en, properties.get_property("english"));
   EXPECT_EQ(fr, properties.get_property("french"));
   EXPECT_EQ(es, properties.get_property("spanish"));

   properties.set_property("deutch", "halo");
   properties.set_property("all", "hello");
   properties.add_property("all", "bonjour");
   properties.add_property("all", "hola");
   properties.add_property("all", "halo");
   properties.store(pf2);

   EXPECT_TRUE(pf2.exist());
}

TEST(Properties, LoadXML) {
   const std::string en ("hello");
   const std::string fr ("bonjour");
   const std::string es ("hola");

   mgz::io::file x(MGZ_TESTS_PATH(properties/sample.xml));
   mgz::io::properties properties;
   properties.load_xml(x);

   EXPECT_EQ(en, properties.get_property("english"));
   EXPECT_EQ(fr, properties.get_property("french"));
   EXPECT_EQ(es, properties.get_property("spanish"));

   mgz::io::file x2("sample2.xml");

   properties.set_property("deutch", "halo");
   properties.set_property("all", "hello");
   properties.add_property("all", "bonjour");
   properties.add_property("all", "hola");
   properties.add_property("all", "halo");

   properties.store_xml(x2);

   EXPECT_TRUE(x2.exist());
}
