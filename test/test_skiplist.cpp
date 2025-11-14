#include <gtest/gtest.h>

#include <algorithm>
#include <latch>
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

TEST(SkipListTest, ConcurrentOperations) {
  SkipList s{16};
  const int num_readers = 4;
  const int num_writers = 2;
  const int num_operations = 1000;

  std::atomic<bool> start{false};
  std::latch completion_latch{num_readers + num_writers};

  std::vector<std::string> inserted_keys;
  std::mutex keys_mutex;

  auto writer_func = [&](int thread_id) {
    // 等待开始信号
    while (!start) {
      std::this_thread::yield();
    }

    // 执行写操作
    for (int i = 0; i < num_operations; ++i) {
      std::string key =
          "key_" + std::to_string(thread_id) + "_" + std::to_string(i);
      std::string value =
          "value_" + std::to_string(thread_id) + "_" + std::to_string(i);

      if (i % 2 == 0) {
        // 插入操作
        s.Put(key, value);
        {
          std::lock_guard<std::mutex> lock(keys_mutex);
          inserted_keys.push_back(key);
        }
      } else {
        // 删除操作
        s.Remove(key);
      }

      // 随机休眠一小段时间，模拟实际工作负载
      std::this_thread::sleep_for(std::chrono::microseconds(rand() % 100));
    }

    completion_latch.count_down();
  };

  // 读线程函数
  auto reader_func = [&](int thread_id) {
    // 等待开始信号
    while (!start) {
      std::this_thread::yield();
    }

    int found_count = 0;
    // 执行读操作
    for (int i = 0; i < num_operations; ++i) {
      // 随机选择一个已插入的key进行查询
      std::string key_to_find;
      {
        std::lock_guard<std::mutex> lock(keys_mutex);
        if (!inserted_keys.empty()) {
          key_to_find = inserted_keys[rand() % inserted_keys.size()];
        }
      }

      if (!key_to_find.empty()) {
        auto result = s.Get(key_to_find);
        if (result.has_value()) {
          found_count++;
        }
      }

      // 每隔一段时间进行一次遍历操作
      if (i % 100 == 0) {
        std::vector<std::pair<std::string, std::string>> items;
        for (auto it = s.begin(); it != s.end(); ++it) {
          items.push_back(*it);
        }
      }

      std::this_thread::sleep_for(std::chrono::microseconds(rand() % 50));
    }

    completion_latch.count_down();
  };

  // 创建并启动写线程
  std::vector<std::thread> writers;
  for (int i = 0; i < num_writers; ++i) {
    writers.emplace_back(writer_func, i);
  }

  // 创建并启动读线程
  std::vector<std::thread> readers;
  for (int i = 0; i < num_readers; ++i) {
    readers.emplace_back(reader_func, i);
  }

  // 给线程一点时间进入等待状态
  std::this_thread::sleep_for(std::chrono::milliseconds{100});

  // 记录开始时间
  auto start_time = std::chrono::steady_clock::now();

  // 发送开始信号
  start = true;

  // 等待所有线程完成
  completion_latch.wait();

  // 记录结束时间
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

  size_t final_size = 0;
  for (auto it = s.begin(); it != s.end(); ++it) {
    final_size++;
  }
  EXPECT_GT(final_size, 0);
  EXPECT_LE(final_size, num_writers * num_operations);
}