#include "memtable/memtable.h"

MemTable::MemTable() : frozen_bytes_(0) {
  table_ = std::make_shared<SkipList>(16);
}

MemTable::~MemTable() = default;

void MemTable::Put(const std::string& key, const std::string& value) {
  table_->Put(key, value);
}

std::optional<std::string> MemTable::Get(const std::string& key) {
  auto result = table_->Get(key);
  if (result.has_value() && !result.value().empty()) {
    return result;
  }

  // memtable没有，去frozen memtable
  for (auto& t : frozen_tables_) {
    auto result = t->Get(key);
    if (result.has_value() && !result.value().empty()) {
      return result;
    }
  }

  // TODO: find in SST
  return std::nullopt;
}

void MemTable::Remove(const std::string& key) { table_->Put(key, ""); }

void MemTable::Clear() { table_->Clear(); }

void MemTable::Flush() {
  // TODO:
}

void MemTable::FrozenCurrentTable() {
  frozen_bytes_ += table_->size();
  frozen_tables_.push_front(std::move(table_));
  table_ = std::make_shared<SkipList>(16);
}

size_t MemTable::current_size() const { return table_->size(); }

size_t MemTable::frozen_size() const { return frozen_bytes_; }

size_t MemTable::total_size() const { return current_size() + frozen_size(); }