#include "sst/sst_iterator.h"

#include "sst/sst.h"

SstIterator::SstIterator(std::shared_ptr<SST> sst)
    : sst_(sst), block_idx_(0), block_iter_(nullptr) {
  if (sst) {
    SeekFirst();
  }
}

SstIterator::SstIterator(std::shared_ptr<SST> sst, const std::string& key)
    : sst_(sst), block_idx_(0), block_iter_(nullptr) {
  if (sst) {
    Seek(key);
  }
}

void SstIterator::SeekFirst() {
  if (!sst_ || sst_->num_blocks() == 0) {
    block_iter_ = nullptr;
    return;
  }
  block_idx_ = 0;
  auto block = sst_->ReadBlock(block_idx_);
  block_iter_ = std::make_shared<BlockIterator>(block);
}

void SstIterator::Seek(const std::string& key) {
  if (!sst_) {
    block_iter_ = nullptr;
  }
  try {
    block_idx_ = sst_->FindBlockIdx(key);
    if (auto block = sst_->ReadBlock(block_idx_); block) {
      block_iter_ = std::make_shared<BlockIterator>(block, key);
      return;
    }
    block_iter_ = nullptr;
  } catch (const std::exception& e) {
    block_iter_ = nullptr;
    throw std::runtime_error("could not read a block from sst_");
  }
}

bool SstIterator::IsEnd() const { return !block_iter_; }

std::string SstIterator::key() {
  if (!block_iter_) {
    throw std::runtime_error("iterator is invalid");
  }
  return (**block_iter_).first;
}

std::string SstIterator::value() {
  if (!block_iter_) {
    throw std::runtime_error("iterator is invalid");
  }
  return (**block_iter_).second;
}

SstIterator& SstIterator::operator++() {
  if (!block_iter_) {
    return *this;
  }
  (*block_iter_)++;
  if (block_iter_->IsEnd()) {
    block_idx_++;
    if (block_idx_ < sst_->num_blocks()) {
      auto next_block = sst_->ReadBlock(block_idx_);
      block_iter_ = std::make_shared<BlockIterator>(next_block, 0);
    } else {
      block_iter_ = nullptr;
    }
  }
  return *this;
}

SstIterator SstIterator::operator++(int) {
  auto tmp = *this;
  ++(*this);
  return tmp;
}

bool SstIterator::operator==(const SstIterator& other) const {
  return sst_ == other.sst_ && block_idx_ == other.block_idx_ &&
         *block_iter_ == *other.block_iter_;
}

bool SstIterator::operator!=(const SstIterator& other) const {
  return !(*this == other);
}

SstIterator::value_type SstIterator::operator*() const {
  if (!block_iter_) {
    throw std::runtime_error("iterator is invalid");
  }
  return **block_iter_;
}