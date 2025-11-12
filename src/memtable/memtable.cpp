#include "memtable/memtable.h"

#include "memtable/iterator.h"

MemTable::MemTable() : frozen_bytes_(0) {
  table_ = std::make_shared<SkipList>(16);
}

MemTable::~MemTable() = default;

void MemTable::Put(const std::string& key, const std::string& value) {
  std::unique_lock<std::shared_mutex> lock{rw_mutex_};
  table_->Put(key, value);
}

std::optional<std::string> MemTable::Get(const std::string& key) {
  std::shared_lock<std::shared_mutex> lock{rw_mutex_};
  auto result = table_->Get(key);
  if (result.has_value()) {
    if (result->empty()) {
      return std::nullopt;
    }
    return result;
  }

  // memtable没有，去frozen memtable
  for (auto& t : frozen_tables_) {
    auto result = t->Get(key);
    if (result.has_value()) {
      if (result->empty()) {
        return std::nullopt;
      }
      return result;
    }
  }

  // TODO: find in SST
  return std::nullopt;
}

void MemTable::Remove(const std::string& key) {
  std::unique_lock<std::shared_mutex> lock{rw_mutex_};
  table_->Put(key, "");
}

void MemTable::Clear() {
  std::unique_lock<std::shared_mutex> lock{rw_mutex_};
  frozen_tables_.clear();
  table_->Clear();
}

void MemTable::Flush() {
  // TODO:
}

void MemTable::FrozenCurrentTable() {
  std::unique_lock<std::shared_mutex> lock{rw_mutex_};
  frozen_bytes_ += table_->size();
  frozen_tables_.push_front(std::move(table_));
  table_ = std::make_shared<SkipList>(16);
}

size_t MemTable::current_size() const {
  std::shared_lock<std::shared_mutex> lock{rw_mutex_};
  return table_->size();
}

size_t MemTable::frozen_size() const {
  std::shared_lock<std::shared_mutex> lock{rw_mutex_};
  return frozen_bytes_;
}

size_t MemTable::total_size() const {
  std::shared_lock<std::shared_mutex> lock{rw_mutex_};
  return current_size() + frozen_size();
}

MemTableIterator MemTable::begin() const {
  std::shared_lock<std::shared_mutex> lock{rw_mutex_};
  return MemTableIterator{*this};
}
MemTableIterator MemTable::end() const {
  std::shared_lock<std::shared_mutex> lock{rw_mutex_};
  return MemTableIterator{};
}