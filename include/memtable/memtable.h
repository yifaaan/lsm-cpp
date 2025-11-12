#pragma once

#include <list>
#include <shared_mutex>
#include <unordered_map>

#include "../skiplist/skiplist.h"

class MemTableIterator;

class MemTable {
 public:
  MemTable();
  ~MemTable();

  void Put(const std::string& key, const std::string& value);
  std::optional<std::string> Get(const std::string& key);
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