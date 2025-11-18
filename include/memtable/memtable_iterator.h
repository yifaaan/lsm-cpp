#pragma once

#include <queue>

#include "iterator/iterator.h"
#include "memtable/memtable.h"
#include "skiplist/skiplist.h"



// MemTableIterator 以 key 有序的方式遍历 MemTable 中所有活跃/冻结 SkipList
// 合并后的 KV 记录，作为上层顺序读和刷盘的统一入口。
class MemTableIterator : public BaseIterator {
 public:
  // 默认构造一个 end 迭代器。
  MemTableIterator();
  // 从给定 MemTable 构造迭代器，指向合并后所有表的第一个 key。
  MemTableIterator(const MemTable& memtable);

  // 解引用得到当前最小 key 对应的 (key, value) 对。
  std::pair<std::string, std::string> operator*() const override;

  // 前置 ++，推进到下一条合并后的记录。
  MemTableIterator& operator++();
  // 后置 ++，返回推进前的迭代器副本。
  MemTableIterator operator++(int);

  bool operator==(const MemTableIterator& other) const;
  bool operator!=(const MemTableIterator& other) const;
  // 判断是否已经遍历完所有活跃/冻结 SkipList。
  bool IsEnd() const override;

 private:
  std::priority_queue<SearchItem, std::vector<SearchItem>,
                      std::greater<SearchItem>>
      items_;
};
