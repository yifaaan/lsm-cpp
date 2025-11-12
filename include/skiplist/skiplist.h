#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

struct SkipListNode {
  std::string key;
  std::string value;
  // 不同层级的下一个节点指针
  std::vector<std::shared_ptr<SkipListNode>> forward;

  SkipListNode(const std::string &k, const std::string &v, int level)
      : key(k), value(v), forward(level, nullptr) {}
};

class SkipListIterator {
 public:
  explicit SkipListIterator(std::shared_ptr<SkipListNode> node)
      : current_(node) {}

  std::pair<std::string, std::string> operator*() const;

  SkipListIterator &operator++();

  SkipListIterator operator++(int);

  bool operator==(const SkipListIterator &other) const;

  bool operator!=(const SkipListIterator &other) const;

  std::string key() const;
  std::string value() const;
  bool valid() const;

 private:
  std::shared_ptr<SkipListNode> current_;
};

class SkipList {
 public:
  explicit SkipList(int max_level);

  // 插入或更新
  void Put(const std::string &key, const std::string &value);

  std::optional<std::string> Get(const std::string &key) const;

  // 删除(置空的话要使用Put)
  void Remove(const std::string &key);

  // 刷出跳表数据，返回key有序键值对
  std::vector<std::pair<std::string, std::string>> Flush() const;

  size_t size() const;

  void Clear();

  SkipListIterator begin() const { return SkipListIterator{head_->forward[0]}; }
  SkipListIterator end() const { return SkipListIterator{nullptr}; }

 private:
  // 生成的新节点的随机层数
  int random_level();

  // 头节点，不存放数据
  std::shared_ptr<SkipListNode> head_;
  // 最大层级数
  int max_level_;
  // 当前最高层级数
  int current_level_;
  // 跳表当前所占字节数
  size_t size_bytes_ = 0;
};