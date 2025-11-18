#pragma once

#include <list>
#include <shared_mutex>
#include <unordered_map>

#include "../skiplist/skiplist.h"

class MemTableIterator;
// MemTable 负责维护内存中的有序 KV 数据，封装底层 SkipList，
// 并提供写入、查询、删除以及冻结/刷盘等操作接口。
class MemTable {
 public:
  MemTable();
  ~MemTable();

  void Put(const std::string& key, const std::string& value);
  std::optional<std::string> Get(const std::string& key) const;
  void Remove(const std::string& key);
  void Clear();
  void Flush();
  void FrozenCurrentTable();

  size_t current_size() const;
  size_t frozen_size() const;
  size_t total_size() const;

  MemTableIterator begin() const;
  MemTableIterator end() const;

 private:
  friend class MemTableIterator;

  std::shared_ptr<SkipList> table_;
  std::list<std::shared_ptr<SkipList>> frozen_tables_;
  size_t frozen_bytes_;
  mutable std::shared_mutex rw_mutex_;
};