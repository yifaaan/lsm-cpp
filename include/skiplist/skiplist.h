#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

struct Node {
  std::string key;
  std::string value;
  // 不同层级的下一个节点指针
  std::vector<std::shared_ptr<Node>> forward;

  Node(const std::string &k, const std::string &v, int level) : key(k), value(v), forward(level, nullptr) {}
};

class Iterator {
 public:
  explicit Iterator(std::shared_ptr<Node> node) : current(node) {}

  std::pair<std::string, std::string> operator*() const;

  Iterator &operator++();

  Iterator operator++(int);

  bool operator==(const Iterator &other) const;

  bool operator!=(const Iterator &other) const;

 private:
  std::shared_ptr<Node> current;
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
  std::vector<std::pair<std::string, std::string>> Flush();

  size_t size() const;

  void Clear();

  Iterator begin() const { return Iterator{head_->forward[0]}; }
  Iterator end() const { return Iterator{nullptr}; }

 private:
  // 生成的新节点的随机层数
  int random_level();

  // 头节点，不存放数据
  std::shared_ptr<Node> head_;
  // 最大层级数
  int max_level_;
  // 当前层级数
  int current_level_;
  // 跳表当前所占字节数
  size_t size_bytes_ = 0;
};