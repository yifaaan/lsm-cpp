#pragma once

#include <memory>

#include "block/block_iterator.h"
#include "iterator/iterator.h"

class SST;

// SstIterator 负责在单个 SST 内跨多个 Block 顺序遍历或从指定 key 开始
// 遍历 KV 记录，是连接 SST 与上层查询逻辑的迭代器封装。
class SstIterator : public BaseIterator {
 public:
  friend class SST;
  using value_type = std::pair<std::string, std::string>;

  // 基于给定 SST 构造迭代器，并指向该 SST 的第一个 key。
  explicit SstIterator(std::shared_ptr<SST> sst);

  // 基于给定 SST 构造迭代器，并定位到大于等于 key 的第一个位置。
  SstIterator(std::shared_ptr<SST> sst, const std::string& key);

  // 将迭代器移动到 SST 中的第一个 key。
  void SeekFirst();

  // 将迭代器移动到大于等于指定 key 的第一个位置。
  void Seek(const std::string& key);

  // 判断是否已经遍历完该 SST（到达 end）。
  bool IsEnd() const override;

  // 返回当前 entry 的 key，若迭代器无效会抛出异常。
  std::string key();
  // 返回当前 entry 的 value，若迭代器无效会抛出异常。
  std::string value();

  // 前置 ++，在当前 Block 内前进，必要时跳到下一个 Block。
  SstIterator& operator++();
  // 后置 ++，返回推进前的迭代器副本。
  SstIterator operator++(int);

  bool operator==(const SstIterator& other) const;
  bool operator!=(const SstIterator& other) const;
  
  // 解引用得到当前 entry 的 (key, value) 对，若迭代器无效会抛出异常。
  value_type operator*() const override;
  
 private:
  std::shared_ptr<SST> sst_;
  size_t block_idx_;
  std::shared_ptr<BlockIterator> block_iter_;
};
