#pragma once

#include <queue>
#include <vector>

#include "iterator/iterator.h"
#include "sst/sst_iterator.h"

class IterEntry {};

class L0Iterator : public BaseIterator {
 public:
  // 使用一组 L0 层的 SstIterator 构造迭代器，并合并它们形成一个有序视图。
  explicit L0Iterator(std::vector<SstIterator> sst_iters);

  // 判断是否已经遍历完所有 L0 层 SST 中的记录。
  bool IsEnd() const override;

  std::pair<std::string, std::string> operator*() const override;

  L0Iterator& operator++();
  L0Iterator operator++(int);

  bool operator==(const L0Iterator& other) const;
  bool operator!=(const L0Iterator& other) const;

 private:
  size_t idx_;
};