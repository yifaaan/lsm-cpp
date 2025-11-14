#include <gtest/gtest.h>

#include <string>
#include <thread>
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

TEST(MemTableTest, ConcurrentOperations) {
  MemTable table;
  const int num_readers = 4;
  const int num_writers = 2;
  const int num_operations = 1000;

  std::atomic<bool> start{false};
  std::atomic<int> completion_counter{num_readers + num_writers + 1};

  std::vector<std::string> inserted_keys;
  std::mutex keys_mutex;

  auto writer_func = [&](int thread_id) {
    while (!start) {
      std::this_thread::yield();
    }
    for (int i = 0; i < num_operations; i++) {
      std::string key =
          "key_" + std::to_string(thread_id) + "_" + std::to_string(i);
      std::string value =
          "value_" + std::to_string(thread_id) + "_" + std::to_string(i);
      if (i % 3 == 0) {
        table.Put(key, value);
        {
          std::lock_guard<std::mutex> lock{keys_mutex};
          inserted_keys.push_back(key);
        }
      } else if (i % 3 == 1) {
        table.Remove(key);
      } else {
        table.Put(key, value + "_update");
      }
      std::this_thread::sleep_for(std::chrono::microseconds{std::rand() % 100});
    }
    completion_counter--;
  };

  auto reader_func = [&](int thread_id) {
    while (!start) {
      std::this_thread::yield();
    }

    int found_count = 0;
    for (int i = 0; i < num_operations; ++i) {
      std::string key_to_find;
      {
        std::lock_guard<std::mutex> lock{keys_mutex};
        if (!inserted_keys.empty()) {
          key_to_find = inserted_keys[std::rand() % inserted_keys.size()];
        }
      }

      if (!key_to_find.empty()) {
        if (auto result = table.Get(key_to_find); result.has_value()) {
          found_count++;
        }
      }

      if (i % 100 == 0) {
        std::vector<std::pair<std::string, std::string>> items;
        for (auto it = table.begin(); it != table.end(); ++it) {
          items.push_back(*it);
        }
      }
      std::this_thread::sleep_for(std::chrono::microseconds{std::rand() % 100});
    }
    completion_counter--;
  };

  auto freeze_func = [&]() {
    while (!start) {
      std::this_thread::yield();
    }

    for (int i = 0; i < 5; i++) {
      std::this_thread::sleep_for(std::chrono::microseconds{100});
      table.FrozenCurrentTable();

      size_t frozen_size = table.frozen_size();
      EXPECT_GE(frozen_size, 0);

      size_t total_size = table.total_size();
      EXPECT_GE(total_size, frozen_size);
    }
    completion_counter--;
  };

  std::vector<std::thread> writers;
  for (int i = 0; i < num_writers; i++) {
    writers.emplace_back(writer_func, i);
  }

  std::vector<std::thread> readers;
  for (int i = 0; i < num_readers; i++) {
    readers.emplace_back(reader_func, i);
  }

  std::thread freeze_thread(freeze_func);

  std::this_thread::sleep_for(std::chrono::milliseconds{100});

  auto start_time = std::chrono::steady_clock::now();
  start = true;
  while (completion_counter > 0) {
    std::this_thread::yield();
  }
  auto end_time = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                      end_time - start_time)
                      .count();

  for (auto& w : writers) {
    w.join();
  }
  for (auto& r : readers) {
    r.join();
  }
  freeze_thread.join();

  size_t final_size = 0;
  for (auto it = table.begin(); it != table.end(); ++it) {
    final_size++;
  }

  EXPECT_GT(table.total_size(), 0);
  EXPECT_LE(final_size, num_writers * num_operations);
}