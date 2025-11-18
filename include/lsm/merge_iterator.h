#pragma once

#include <memory>

#include "iterator/iterator.h"
#include "memtable/memtable_iterator.h"
#include "sst/sst_iterator.h"

class MergeIterator : public BaseIterator {
 public:
  MergeIterator(MemTableIterator it_a, SstIterator it_b);
 private:
  MemTableIterator it_a_;
  SstIterator it_b_;
  bool choose_a_ = false;
};