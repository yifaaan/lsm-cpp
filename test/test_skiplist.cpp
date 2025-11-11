#include <gtest/gtest.h>

#include <algorithm>
#include <string>
#include <unordered_set>
#include <vector>

#include "skiplist/skiplist.h"

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
  SkipList s(16);
  s.Put("key1", "value1");
  s.Put("key2", "value2");
  s.Put("key3", "value3");

  std::vector<std::pair<std::string, std::string>> results;
  for (auto it = s.begin(); it != s.end(); ++it) {
    results.push_back(*it);
  }

  EXPECT_EQ(results.size(), 3);
  EXPECT_EQ(results[0].first, "key1");
  EXPECT_EQ(results[1].first, "key2");
  EXPECT_EQ(results[2].first, "key3");
}

TEST(SkipListTest, LargeScaleInsertAndFind) {
  SkipList s(16);
  const int n = 10000;

  for (int i = 0; i < n; i++) {
    std::string key = "key" + std::to_string(i);
    std::string value = "value" + std::to_string(i);
    s.Put(key, value);
  }

  for (int i = 0; i < n; i++) {
    std::string key = "key" + std::to_string(i);
    std::string expected = "value" + std::to_string(i);
    EXPECT_EQ(s.Get(key).value(), expected);
  }
}

TEST(SkipListTest, LargeScaleRemove) {
  SkipList s(16);
  const int n = 10000;

  for (int i = 0; i < n; i++) {
    std::string key = "key" + std::to_string(i);
    std::string value = "value" + std::to_string(i);
    s.Put(key, value);
  }

  for (int i = 0; i < n; i++) {
    std::string key = "key" + std::to_string(i);
    s.Remove(key);
  }

  for (int i = 0; i < n; i++) {
    std::string key = "key" + std::to_string(i);
    std::string expected = "value" + std::to_string(i);
    EXPECT_FALSE(s.Get(key).has_value());
  }
}

TEST(SkipListTest, DuplicateInsert) {
  SkipList s(16);
  s.Put("key1", "value1");
  s.Put("key1", "value2");
  s.Put("key1", "value3");
  EXPECT_EQ(s.Get("key1").value(), "value3");
}

TEST(SkipListTest, EmptySkipList) {
  SkipList s(16);
  EXPECT_FALSE(s.Get("key1").has_value());
}

TEST(SkipListTest, RandomInsert) {
  SkipList s(16);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 999);
  std::unordered_set<std::string> keys;
  for (int i = 0; i < 1000; i++) {
    std::string key = "key" + std::to_string(dis(gen));
    std::string value = "value" + std::to_string(dis(gen));
    if (keys.find(key) == keys.end()) {
      s.Put(key, value);
      keys.insert(key);
    } else {
      keys.erase(key);
      s.Remove(key);
    }
    if (keys.find(key) != keys.end()) {
      EXPECT_EQ(s.Get(key).value(), value);
    } else {
      EXPECT_FALSE(s.Get(key).has_value());
    }
  }
}

TEST(SkipListTest, MemorySizeTracing) {
  SkipList s(16);
  s.Put("key1", "value1");
  s.Put("key2", "value2");

  size_t expected_size = 10 + 10;
  EXPECT_EQ(s.size(), expected_size);

  s.Remove("key1");
  expected_size = 10;
  EXPECT_EQ(s.size(), expected_size);

  s.Clear();
  expected_size = 0;
  EXPECT_EQ(s.size(), expected_size);
}