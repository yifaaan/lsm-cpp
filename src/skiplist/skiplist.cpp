#include "skiplist/skiplist.h"

std::pair<std::string, std::string> SkipListIterator::operator*() const {
  return {current_->key, current_->value};
}

SkipListIterator& SkipListIterator::operator++() {
  if (current_) {
    current_ = current_->forward[0];
  }
  return *this;
}

SkipListIterator SkipListIterator::operator++(int) {
  SkipListIterator tmp = *this;
  ++(*this);
  return tmp;
}

bool SkipListIterator::operator==(const SkipListIterator& other) const {
  return current_ == other.current_;
}

bool SkipListIterator::operator!=(const SkipListIterator& other) const {
  return !(*this == other);
}

SkipList::SkipList(int max_level) : max_level_(max_level), current_level_(1) {
  head_ = std::make_shared<SkipListNode>("", "", max_level_);
}

int SkipList::random_level() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 1);
  int level = 1;
  while (dis(gen) && level < max_level_) {
    level++;
  }
  return level;
}

void SkipList::Put(const std::string& key, const std::string& value) {
  std::vector<std::shared_ptr<SkipListNode>> updates(max_level_, nullptr);
  auto x = head_;
  // 查找每层都需要更新的前驱节点
  for (int i = current_level_ - 1; i >= 0; i--) {
    while (x->forward[i] && x->forward[i]->key < key) {
      x = x->forward[i];
    }
    updates[i] = x;
  }

  // 最底层的下一个节点
  x = x->forward[0];
  // 如果有并且key相同就替换value
  if (x && x->key == key) {
    size_bytes_ += value.size() - x->value.size();
    x->value = value;
    return;
  }

  // 在x和下一个节点直接插入新节点
  // 新节点的高度
  int new_level = random_level();
  if (new_level > current_level_) {
    // 高出来的层的前驱只能为head_
    for (int i = current_level_; i < new_level; i++) {
      updates[i] = head_;
    }
    current_level_ = new_level;
  }

  auto new_node = std::make_shared<SkipListNode>(key, value, new_level);
  size_bytes_ += key.size() + value.size();

  for (int i = 0; i < new_level; i++) {
    new_node->forward[i] = updates[i]->forward[i];
    updates[i]->forward[i] = new_node;
  }
}

std::optional<std::string> SkipList::Get(const std::string& key) const {
  auto x = head_;
  for (int i = current_level_; i >= 0; i--) {
    while (x->forward[i] && x->forward[i]->key < key) {
      x = x->forward[i];
    }
  }
  x = x->forward[0];
  if (x && x->key == key) {
    return x->value;
  }
  return std::nullopt;
}

void SkipList::Remove(const std::string& key) {
  auto x = head_;
  // 需要更新的前驱
  std::vector<std::shared_ptr<SkipListNode>> updates(max_level_, nullptr);
  for (int i = current_level_; i >= 0; i--) {
    while (x->forward[i] && x->forward[i]->key < key) {
      x = x->forward[i];
    }
    updates[i] = x;
  }

  x = x->forward[0];
  if (x && x->key == key) {
    for (int i = 0; i < current_level_; i++) {
      if (updates[i]->forward[i] != x) {
        break;
      }
      updates[i]->forward[i] = x->forward[i];
    }
  }

  size_bytes_ -= x->key.size() + x->value.size();

  while (current_level_ > 1 && head_->forward[current_level_ - 1] == nullptr) {
    current_level_--;
  }
}

std::vector<std::pair<std::string, std::string>> SkipList::Flush() const {
  std::vector<std::pair<std::string, std::string>> data;
  auto x = head_->forward[0];
  while (x) {
    data.emplace_back(x->key, x->value);
    x = x->forward[0];
  }
  return data;
}

size_t SkipList::size() const { return size_bytes_; }

void SkipList::Clear() {
  head_ = std::make_shared<SkipListNode>("", "", max_level_);
  size_bytes_ = 0;
}

std::string SkipListIterator::key() const { return current_->key; }
std::string SkipListIterator::value() const { return current_->value; }
bool SkipListIterator::valid() const { return !current_->value.empty(); }