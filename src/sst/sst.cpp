#include "sst/sst.h"

#include "block/block.h"
#include "block/block_meta.h"

SST SST::Open(size_t sst_id, File file) {
  SST sst;
  sst.sst_id_ = sst_id;
  sst.file_ = std::move(file);

  size_t file_size = sst.file_.size();
  if (file_size < sizeof(uint32_t)) {
    throw std::runtime_error("Invalid SST file: too small");
  }

  auto offset_bytes =
      sst.file_.ReadToSlice(file_size - sizeof(uint32_t), sizeof(uint32_t));
  uint32_t meta_offset32 = 0;
  std::memcpy(&meta_offset32, offset_bytes.data(), sizeof(meta_offset32));
  sst.meta_block_offset_ = meta_offset32;

  size_t meta_section_size =
      file_size - sizeof(uint32_t) - sst.meta_block_offset_;
  auto meta_section_bytes =
      sst.file_.ReadToSlice(sst.meta_block_offset_, meta_section_size);
  sst.meta_entries_ = BlockMeta::DecodeMetasFromSlice(meta_section_bytes);

  if (!sst.meta_entries_.empty()) {
    sst.first_key_ = sst.meta_entries_.front().first_key_;
    sst.last_key_ = sst.meta_entries_.back().last_key_;
  }
  return sst;
}

SST SST::CreateWithMetaOnly(size_t sst_id, size_t file_size,
                            std::string_view first_key,
                            std::string_view last_key) {
  SST sst;
  sst.file_.set_size(file_size);
  sst.sst_id_ = sst_id;
  sst.first_key_ = first_key;
  sst.last_key_ = last_key;
  sst.meta_block_offset_ = 0;
  return sst;
}

std::shared_ptr<Block> SST::ReadBlock(size_t block_idx) {
  if (block_idx >= meta_entries_.size()) {
    throw std::out_of_range("block index out of range");
  }

  const auto& meta = meta_entries_[block_idx];
  size_t block_size;
  if (block_idx == meta_entries_.size() - 1) {
    block_size = meta_block_offset_ - meta.offset_;
  } else {
    block_size = meta_entries_[block_idx + 1].offset_ - meta.offset_;
  }

  if (block_size < sizeof(uint32_t)) {
    throw std::runtime_error("Invalid block size in SST");
  }

  size_t encoded_size = block_size - sizeof(uint32_t);
  auto block_data = file_.ReadToSlice(meta.offset_, encoded_size);
  return Block::Decode(block_data);
}

size_t SST::FindBlockIdx(std::string_view key) {
  if (meta_entries_.empty()) {
    throw std::runtime_error("No blocks in SST");
  }

  if (key < meta_entries_.front().first_key_ ||
      key > meta_entries_.back().last_key_) {
    throw std::runtime_error("Key out of SST range");
  }

  int l = 0, r = meta_entries_.size() - 1;
  while (l <= r) {
    int mid = l + (r - l) / 2;
    const auto& meta = meta_entries_[mid];
    if (key < meta.first_key_) {
      r = mid - 1;
    } else if (key > meta.last_key_) {
      l = mid + 1;
    } else {
      return mid;
    }
  }
  return l;
}

SSTBuilder::SSTBuilder(size_t block_size) : block_(block_size) {}

void SSTBuilder::Add(std::string_view key, std::string_view value) {
  if (first_key_.empty()) {
    first_key_ = key;
  }

  uint32_t hash = static_cast<uint32_t>(std::hash<std::string_view>()(key));
  key_hashes_.push_back(hash);

  if (block_.AddEntry(std::string(key), std::string(value))) {
    last_key_ = key;
    return;
  }

  // 当前block已满
  FinishBlock();

  block_.AddEntry(std::string(key), std::string(value));
  first_key_ = key;
  last_key_ = key;
}

void SSTBuilder::FinishBlock() {
  auto old_block = std::move(block_);
  auto encoded_block = old_block.Encode();

  meta_entries_.emplace_back(data_.size(), first_key_, last_key_);

  auto block_hash = static_cast<uint32_t>(std::hash<std::string_view>()(
      {reinterpret_cast<const char*>(encoded_block.data()),
       encoded_block.size()}));

  data_.reserve(data_.size() + encoded_block.size() + sizeof(uint32_t));
  data_.insert(data_.end(), encoded_block.begin(), encoded_block.end());

  const auto* hash_bytes = reinterpret_cast<const uint8_t*>(&block_hash);
  data_.insert(data_.end(), hash_bytes, hash_bytes + sizeof(block_hash));
}

SST SSTBuilder::Build(size_t sst_id, std::string_view path) {
  if (!block_.IsEmpty()) {
    FinishBlock();
  }

  if (meta_entries_.empty()) {
    throw std::runtime_error("Cannot build empty SST");
  }

  auto meta_block = BlockMeta::EncodeMetasToSlice(meta_entries_);
  uint32_t meta_offset = data_.size();

  std::vector<uint8_t> file_content = std::move(data_);

  file_content.insert(file_content.end(), meta_block.begin(), meta_block.end());
  size_t old_size = file_content.size();
  file_content.resize(old_size + sizeof(uint32_t));
  std::memcpy(file_content.data() + old_size, &meta_offset, sizeof(uint32_t));

  File f = File::CreateAndWrite(path, file_content);

  SST sst;
  sst.sst_id_ = sst_id;
  sst.file_ = std::move(f);
  sst.meta_block_offset_ = meta_offset;
  sst.meta_entries_ = std::move(meta_entries_);
  sst.first_key_ = sst.meta_entries_.front().first_key_;
  sst.last_key_ = sst.meta_entries_.back().last_key_;
  
  return sst;
}