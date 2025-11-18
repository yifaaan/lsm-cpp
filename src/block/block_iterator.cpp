#include "block/block_iterator.h"

#include <stdexcept>

#include "block/block.h"

BlockIterator::BlockIterator(std::shared_ptr<Block> block, size_t index)
    : block_(block), current_index_(index), cached_value_(std::nullopt) {}

BlockIterator::BlockIterator(std::shared_ptr<Block> b, const std::string& key)
    : block_(b), cached_value_(std::nullopt) {
  auto key_idx_pos = block_->GetIdxBinary(key);
  if (key_idx_pos.has_value()) {
    current_index_ = *key_idx_pos;
  } else {
    throw std::runtime_error("key not found");
  }
}

BlockIterator::BlockIterator(std::shared_ptr<Block> b)
    : block_(b), current_index_(0), cached_value_(std::nullopt) {}

BlockIterator& BlockIterator::operator++() {
  if (block_ && current_index_ < block_->offsets_.size()) {
    ++current_index_;
    cached_value_ = std::nullopt;
  }
  return *this;
}

BlockIterator BlockIterator::operator++(int) {
  BlockIterator tmp = *this;
  ++(*this);
  return tmp;
}

bool BlockIterator::operator==(const BlockIterator& other) const {
  if (!block_ && !other.block_) {
    return true;
  }
  if (!block_ || !other.block_) {
    return false;
  }
  return block_ == other.block_ && current_index_ == other.current_index_;
}

bool BlockIterator::operator!=(const BlockIterator& other) const {
  return !(*this == other);
}

bool BlockIterator::IsEnd() const {
  return current_index_ == block_->offsets_.size();
}

BlockIterator::value_type BlockIterator::operator*() const {
  if (!block_ || current_index_ >= block_->offsets_.size()) {
    throw std::out_of_range("Iterator out of range");
  }

  if (!cached_value_) {
    size_t offset = block_->GetOffsetAt(current_index_);
    cached_value_ =
        std::make_pair(block_->GetKeyAt(offset), block_->GetValueAt(offset));
  }
  return *cached_value_;
}