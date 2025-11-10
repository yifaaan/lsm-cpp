#pragma once

#include "../skiplist/skiplist.h"

#include <list>
#include <unordered_map>

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
 private:
  std::shared_ptr<SkipList> table_;
  std::list<std::shared_ptr<SkipList>> frozen_tables_;
  size_t frozen_bytes_;
};