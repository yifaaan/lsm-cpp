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