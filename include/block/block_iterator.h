#pragma once

#include <iterator>
#include <memory>
#include <optional>
#include <string>

#include "iterator/iterator.h"

class Block;

// BlockIterator 用于在单个 Block 内按 key 有序地顺序遍历 KV 记录，
// 并实现 BaseIterator 所需的最小接口。
class BlockIterator : public BaseIterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::pair<std::string, std::string>;
  using difference_type = std::ptrdiff_t;
  using pointer = const value_type*;
  using reference = const value_type&;

  // 默认构造一个 end 迭代器。
  BlockIterator() : block_(nullptr), current_index_(0) {}
  // 按 Block 内索引构造迭代器，指向第 index 条记录。
  BlockIterator(std::shared_ptr<Block> block, size_t index);
  // 通过在 Block 中二分查找 key 来定位起始位置，若 key 不存在将抛出异常。
  BlockIterator(std::shared_ptr<Block> b, const std::string& key);
  // 从给定 Block 的第一条记录开始遍历。
  explicit BlockIterator(std::shared_ptr<Block> b);

  // 前置 ++，移动到下一个 entry。
  BlockIterator& operator++();
  // 后置 ++，返回移动前的迭代器副本。
  BlockIterator operator++(int);
  // 比较两个迭代器是否指向同一 Block 的同一位置。
  bool operator==(const BlockIterator& other) const;
  bool operator!=(const BlockIterator& other) const;
  // 判断是否已到达该 Block 的 end 位置。
  bool IsEnd() const override;

  // 解引用得到当前 entry 的 (key, value) 对。
  value_type operator*() const override;

 private:
  // 当前指向的块
  std::shared_ptr<Block> block_;
  // 当前块中entry的索引
  size_t current_index_;
  mutable std::optional<value_type> cached_value_;
};