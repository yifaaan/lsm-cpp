#include "skiplist/skiplist.h"

#include <gtest/gtest.h>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_set>


TEST(SkipListTest, BasicOperations) {
  SkipList s(16);

  s.Put("key1", "value1");
  EXPECT_EQ(s.Get("key1").value(), "value1");

  s.Put("key1", "new_value");
  EXPECT_EQ(s.Get("key1").value(), "new_value");

  s.Remove("key1");
  EXPECT_FALSE(s.Get("key1").has_value());
}

TEST(SkipList, Iterator) {
  // SkipList s(16);
  // s.Put("key1", "value1");
  // s.Put("key2", "value2");
  // s.Put("key3", "value3");

  // std::vector<std::pair<std::string, std::string>> results;
  // for (auto it = s.begin(); )
}