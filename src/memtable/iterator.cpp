#include "memtable/iterator.h"

#include <vector>

#include "memtable/memtable.h"

bool operator<(const SearchItem& a, const SearchItem& b) {
  if (a.key != b.key) {
    return a.key < b.key;
  }
  return a.mem_idx_ < b.mem_idx_;
}

bool operator>(const SearchItem& a, const SearchItem& b) {
  if (a.key != b.key) {
    return a.key > b.key;
  }
  return a.mem_idx_ > b.mem_idx_;
}

bool operator==(const SearchItem& a, const SearchItem& b) {
  return a.key == b.key && a.mem_idx_ == b.mem_idx_;
}

MemTableIterator::MemTableIterator() {}

MemTableIterator::MemTableIterator(const MemTable& memtable) {
  auto current_table = memtable.table_;
  for (auto it = current_table->begin(); it != current_table->end(); ++it) {
    items_.push(SearchItem{it.key(), it.value(), 0});
  }

  int level = 1;
  for (auto it = memtable.frozen_tables_.begin();
       it != memtable.frozen_tables_.end(); ++it) {
    auto frozen_table = *it;
    for (auto x = frozen_table->begin(); x != frozen_table->end(); ++x) {
      items_.push(SearchItem{x.key(), x.value(), level});
    }
    level++;
  }

  while (!items_.empty() && items_.top().value.empty()) {
    auto key = items_.top().value;
    while (!items_.empty() && items_.top().value == key) {
      items_.pop();
    }
  }
}

std::pair<std::string, std::string> MemTableIterator::operator*() const {
  return std::make_pair(items_.top().key, items_.top().value);
}

MemTableIterator& MemTableIterator::operator++() {
  if (items_.empty()) {
    return *this;
  }

  auto x = items_.top();
  items_.pop();
  while (!items_.empty() && items_.top().key == x.key) {
    items_.pop();
  }

  while (!items_.empty() && items_.top().value.empty()) {
    auto k = items_.top().key;
    while (!items_.empty() && items_.top().key == k) {
      items_.pop();
    }
  }
  return *this;
}

MemTableIterator MemTableIterator::operator++(int) {
  auto tmp = *this;
  ++(*this);
  return tmp;
}

bool MemTableIterator::operator==(const MemTableIterator& other) const {
  if (items_.empty() && other.items_.empty()) {
    return true;
  }
  if (items_.empty() || other.items_.empty()) {
    return false;
  }
  if (items_.top().key != other.items_.top().key ||
      items_.top().value != other.items_.top().value) {
    return false;
  }
  return true;
}

bool MemTableIterator::operator!=(const MemTableIterator& other) const {
  return !(*this == other);
}