#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "memtable/iterator.h"
#include "memtable/memtable.h"

TEST(MemTableTest, BasicOperations) {
  MemTable table;

  table.Put("key1", "value1");
  EXPECT_EQ(table.Get("key1").value(), "value1");

  table.Put("key1", "new_value");
  EXPECT_EQ(table.Get("key1").value(), "new_value");

  EXPECT_FALSE(table.Get("non exits").has_value());
}

TEST(MemTableTest, RemoveOperations) {
  MemTable table;

  table.Put("key1", "value1");
  table.Remove("key1");
  EXPECT_FALSE(table.Get("key1").has_value());

  table.Remove("non exits");
  EXPECT_FALSE(table.Get("non exits").has_value());
}

TEST(MemTableTest, FrozenMemTableOperations) {
  MemTable table;

  table.Put("key1", "value1");
  table.Put("key2", "value2");

  table.FrozenCurrentTable();

  table.Put("key3", "value3");

  EXPECT_EQ(table.Get("key1").value(), "value1");
  EXPECT_EQ(table.Get("key2").value(), "value2");
  EXPECT_EQ(table.Get("key3").value(), "value3");
}

TEST(MemTableTest, LargeScaleOperations) {
  MemTable table;
  const int n = 1000;

  for (int i = 0; i < n; i++) {
    std::string key = "key" + std::to_string(i);
    std::string value = "value" + std::to_string(i);
    table.Put(key, value);
  }

  for (int i = 0; i < n; i++) {
    std::string key = "key" + std::to_string(i);
    std::string expected = "value" + std::to_string(i);
    EXPECT_EQ(table.Get(key).value(), expected);
  }
}

TEST(MemTableTest, MemorySizeTracing) {
  MemTable table;

  EXPECT_EQ(table.total_size(), 0);

  table.Put("key1", "value1");
  EXPECT_EQ(table.current_size(), 10);

  size_t size_before_freeze = table.total_size();
  table.FrozenCurrentTable();
  EXPECT_EQ(table.frozen_size(), size_before_freeze);
}

TEST(MemTableTest, MultipleFrozenTables) {
  MemTable table;

  table.Put("key1", "value1");
  table.FrozenCurrentTable();

  table.Put("key2", "value2");
  table.FrozenCurrentTable();

  table.Put("key3", "value3");

  EXPECT_EQ(table.Get("key1"), "value1");
  EXPECT_EQ(table.Get("key2"), "value2");
  EXPECT_EQ(table.Get("key3"), "value3");
}

TEST(MemTableTest, IteratorComplexOperations) {
  MemTable table;

  table.Put("key1", "value1");
  table.Put("key2", "value2");
  table.Put("key3", "value3");

  std::vector<std::pair<std::string, std::string>> result1;
  for (auto it = table.begin(); it != table.end(); ++it) {
    result1.push_back(*it);
  }

  ASSERT_EQ(result1.size(), 3);
  EXPECT_EQ(result1[0].first, "key1");
  EXPECT_EQ(result1[0].second, "value1");
  EXPECT_EQ(result1[2].second, "value3");

  table.FrozenCurrentTable();

  table.Put("key2", "new_value2");
  table.Remove("key1");
  table.Put("key4", "value4");

  decltype(result1) result2;
  for (auto it = table.begin(); it != table.end(); ++it) {
    result2.push_back(*it);
  }
  ASSERT_EQ(result2.size(), 3);
  EXPECT_EQ(result2[0].first, "key2");
  EXPECT_EQ(result2[0].second, "new_value2");
  EXPECT_EQ(result2[2].second, "value4");

  table.FrozenCurrentTable();

  table.Put("key1", "new_value1");
  table.Remove("key3");
  table.Put("key2", "new_new_value2");
  table.Put("key5", "value5");

  decltype(result1) result3;
  for (auto it = table.begin(); it != table.end(); ++it) {
    result3.push_back(*it);
  }

  EXPECT_EQ(result3[0].first, "key1");
  EXPECT_EQ(result3[0].second, "new_value1");
  EXPECT_EQ(result3[1].first, "key2");
  EXPECT_EQ(result3[1].second, "new_new_value2");
  EXPECT_EQ(result3[2].first, "key4");
  EXPECT_EQ(result3[2].second, "value4");
  EXPECT_EQ(result3[3].first, "key5");
  EXPECT_EQ(result3[3].second, "value5");

  EXPECT_FALSE(table.Get("key3").has_value());
}