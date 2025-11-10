#include "skiplist/skiplist.h"

std::pair<std::string, std::string> Iterator::operator*() const {
  return {current->key, current->value};
}

Iterator& Iterator::operator++() {
  if (current) {
    current = current->forward[0];
  }
  return *this;
}

Iterator Iterator::operator++(int) {
  Iterator tmp = *this;
  ++(*this);
  return tmp;
}

bool Iterator::operator==(const Iterator& other) const {
  return current == other.current;
}

bool Iterator::operator!=(const Iterator& other) const {
  return !(*this == other);
}

SkipList::SkipList(int max_level) : max_level_(max_level), current_level_(1) {
  head_ = std::make_shared<Node>("", "", max_level_);
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

}