#pragma once

#include <string>

// BaseIterator 是所有 KV 迭代器的抽象基类，定义了解引用、相等比较和
// 结束判定等最小接口，便于在不同存储层之间复用遍历逻辑。
class BaseIterator {
 public:
  virtual std::pair<std::string, std::string> operator*() const;
  virtual bool operator==(const BaseIterator& other) const {
    return this == &other;
  }
  virtual bool operator!=(const BaseIterator& other) const {
    return !(*this == other);
  }
  virtual bool IsEnd() const;
};

// SearchItem 是用于优先队列/归并场景的辅助结构，
// 按 (key, idx) 进行有序比较以稳定地合并多个有序流。
struct SearchItem {
  std::string key;
  std::string value;
  int idx;
};

bool operator<(const SearchItem& a, const SearchItem& b);
bool operator>(const SearchItem& a, const SearchItem& b);
bool operator==(const SearchItem& a, const SearchItem& b);