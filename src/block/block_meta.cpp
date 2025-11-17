#include "block/block_meta.h"

#include <cstring>
#include <functional>

BlockMeta::BlockMeta() : offset_(0), first_key_(""), last_key_("") {}

BlockMeta::BlockMeta(size_t offset, const std::string& first_key,
                     const std::string& last_key)
    : offset_(offset), first_key_(first_key), last_key_(last_key) {}

std::vector<uint8_t> BlockMeta::EncodeMetasToSlice(
    std::span<const BlockMeta> meta_entries) {
  // 每个entry序列化的格式
  // | offset(uint32_t) |
  // | first_key_len(uint16_t) |
  // | first_key_bytes |
  // | last_key_len(uint16_t) |
  // | last_key_bytes |
  // 最终序列化格式
  // |num_entries(uint32_t)|
  // |entry[0] ... entry[num_entries - 1]|
  // |hash(uint32_t)|
  auto num_entries = static_cast<uint32_t>(meta_entries.size());

  size_t total_size = sizeof(uint32_t);
  for (const auto& entry : meta_entries) {
    total_size += sizeof(uint32_t) +         // offset
                  sizeof(uint16_t) +         // first_key_len
                  entry.first_key_.size() +  // first_key_bytes
                  sizeof(uint16_t) +         // last_key_len
                  entry.last_key_.size();    // last_key_bytes
  }
  total_size += sizeof(uint32_t);  // hash

  std::vector<uint8_t> metadata(total_size);
  uint8_t* ptr = metadata.data();

  // num_entries
  std::memcpy(ptr, &num_entries, sizeof(num_entries));
  ptr += sizeof(num_entries);

  // entries
  for (const auto& entry : meta_entries) {
    // offset
    uint32_t offset = static_cast<uint32_t>(entry.offset_);
    std::memcpy(ptr, &offset, sizeof(offset));
    ptr += sizeof(offset);
    // first_key_len, first_key
    uint16_t first_key_len = entry.first_key_.size();
    std::memcpy(ptr, &first_key_len, sizeof(first_key_len));
    ptr += sizeof(first_key_len);
    std::memcpy(ptr, entry.first_key_.data(), first_key_len);
    ptr += first_key_len;
    // last_key_len, last_key
    uint16_t last_key_len = entry.last_key_.size();
    std::memcpy(ptr, &last_key_len, sizeof(last_key_len));
    ptr += sizeof(last_key_len);
    std::memcpy(ptr, entry.last_key_.data(), last_key_len);
    ptr += last_key_len;
  }

  // hash
  const uint8_t* entries_data_start = metadata.data() + sizeof(uint32_t);
  const uint8_t* entries_data_end = ptr;
  size_t data_len = entries_data_end - entries_data_start;
  uint32_t hash = std::hash<std::string_view>()(std::string_view(
      reinterpret_cast<const char*>(entries_data_start), data_len));
  std::memcpy(ptr, &hash, sizeof(hash));

  return metadata;
}

std::vector<BlockMeta> BlockMeta::DecodeMetasFromSlice(
    std::span<const uint8_t> meta_data) {
  if (meta_data.size() < sizeof(uint32_t) * 2) {
    throw std::runtime_error("Invalid metadata size");
  }

  std::vector<BlockMeta> block_metas;
  // num_entries
  uint32_t num_entries;
  const uint8_t* ptr = meta_data.data();
  std::memcpy(&num_entries, ptr, sizeof(num_entries));
  ptr += sizeof(num_entries);

  block_metas.reserve(num_entries);
  for (int i = 0; i < num_entries; i++) {
    BlockMeta meta;
    // offset
    uint32_t offset;
    std::memcpy(&offset, ptr, sizeof(offset));
    ptr += sizeof(offset);
    meta.offset_ = offset;
    // first_key
    uint16_t first_key_len;
    std::memcpy(&first_key_len, ptr, sizeof(first_key_len));
    ptr += sizeof(first_key_len);
    meta.first_key_.assign(reinterpret_cast<const char*>(ptr), first_key_len);
    // std::memcpy(meta.first_key_.data(), ptr, first_key_len);
    ptr += first_key_len;
    // last_key
    uint16_t last_key_len;
    std::memcpy(&last_key_len, ptr, sizeof(last_key_len));
    ptr += sizeof(last_key_len);
    meta.last_key_.assign(reinterpret_cast<const char*>(ptr), last_key_len);
    // std::memcpy(meta.last_key_.data(), ptr, last_key_len);
    ptr += last_key_len;

    block_metas.push_back(meta);
  }

  uint32_t stored_hash;
  std::memcpy(&stored_hash, ptr, sizeof(stored_hash));

  const uint8_t* entries_data_start = meta_data.data() + sizeof(uint32_t);
  const uint8_t* entries_data_end = ptr;
  size_t data_len = entries_data_end - entries_data_start;
  uint32_t hash = std::hash<std::string_view>()(std::string_view(
      reinterpret_cast<const char*>(entries_data_start), data_len));
  if (stored_hash != hash) {
    throw std::runtime_error("Metadata hash mismatch");
  }
  return block_metas;
}