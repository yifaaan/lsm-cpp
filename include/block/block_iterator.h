#pragma once

#include <iterator>
#include <memory>
#include <optional>
#include <string>

class Block;

class BlockIterator {
 public:
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::pair<std::string, std::string>;
  using difference_type = std::ptrdiff_t;
  using pointer = const value_type*;
  using reference = const value_type&;

  BlockIterator() : block_(nullptr), current_index_(0) {}
  BlockIterator(std::shared_ptr<Block> block, size_t index);

  BlockIterator& operator++();
  BlockIterator operator++(int);
  bool operator==(const BlockIterator& other) const;
  bool operator!=(const BlockIterator& other) const;

  value_type operator*() const;
 private:
  // 当前指向的块
  std::shared_ptr<Block> block_;
  // 当前块中entry的索引
  size_t current_index_;
  mutable std::optional<value_type> cached_value_;
};