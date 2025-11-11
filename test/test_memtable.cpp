#include <gtest/gtest.h>

#include <string>
#include <vector>

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