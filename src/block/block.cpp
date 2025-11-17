#include "block/block.h"

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <functional>
#include <string_view>

#include "block/block_iterator.h"

Block::Block(size_t capacity) : capacity_(capacity) {}

std::vector<uint8_t> Block::Encode() const {
  // 数据段 + 偏移段 + 元素个数
  size_t total_bytes = data_.size() * sizeof(uint8_t) +
                       offsets_.size() * sizeof(uint16_t) + sizeof(uint16_t);
  std::vector<uint8_t> encoded(total_bytes, 0);

  std::copy(data_.begin(), data_.end(), encoded.begin());

  size_t offset_pos = data_.size() * sizeof(uint8_t);
  std::memcpy(encoded.data() + offset_pos, offsets_.data(),
              offsets_.size() * sizeof(uint16_t));

  size_t num_pos =
      data_.size() * sizeof(uint8_t) + offsets_.size() * sizeof(uint16_t);
  uint16_t num_elements = offsets_.size();
  std::memcpy(encoded.data() + num_pos, &num_elements, sizeof(num_elements));
  return encoded;
}

std::shared_ptr<Block> Block::Decode(const std::vector<uint8_t>& encoded) {
  // 兼容旧接口：默认不带 hash。
  return Decode(encoded, false);
}

std::shared_ptr<Block> Block::Decode(const std::vector<uint8_t>& encoded,
                                     bool with_hash) {
  auto block = std::make_shared<Block>();

  if (encoded.size() < sizeof(uint16_t)) {
    throw std::runtime_error("Encoded data must greater equal 2 bytes");
  }

  // 有效负载长度（去掉可选的 4 字节 hash 后的部分）
  size_t payload_size = encoded.size();
  size_t hash_pos = encoded.size();

  if (with_hash) {
    // 至少要容纳 hash + num_elements
    if (payload_size < sizeof(uint32_t) + sizeof(uint16_t)) {
      throw std::runtime_error("Encoded Block too small for hash");
    }

    hash_pos -= sizeof(uint32_t);
    payload_size = hash_pos;

    uint32_t stored_hash = 0;
    std::memcpy(&stored_hash, encoded.data() + hash_pos, sizeof(stored_hash));

    uint32_t computed_hash =
        std::hash<std::string_view>{}({reinterpret_cast<const char*>(
                                           encoded.data()),
                                       payload_size});
    if (stored_hash != computed_hash) {
      throw std::runtime_error("Block hash verification failed");
    }
  }

  if (payload_size < sizeof(uint16_t)) {
    throw std::runtime_error("Encoded data must greater equal 2 bytes");
  }

  // 读取元素个数（位于 payload 的末尾）
  uint16_t num_elements = 0;
  size_t num_pos = payload_size - sizeof(uint16_t);
  std::memcpy(&num_elements, encoded.data() + num_pos, sizeof(num_elements));

  // payload 至少要包含 offsets 区 + num_elements 自身
  size_t min_size = sizeof(uint16_t) +
                    static_cast<size_t>(num_elements) * sizeof(uint16_t);
  if (payload_size < min_size) {
    throw std::runtime_error("Invalid encoded Block: insufficient size");
  }

  size_t offsets_pos =
      num_pos - static_cast<size_t>(num_elements) * sizeof(uint16_t);
  block->offsets_.resize(num_elements);
  std::memcpy(block->offsets_.data(), encoded.data() + offsets_pos,
              static_cast<size_t>(num_elements) * sizeof(uint16_t));

  block->data_.assign(encoded.begin(), encoded.begin() + offsets_pos);
  return block;
}

std::string Block::GetFirstKey() const {
  if (data_.empty() || offsets_.empty()) {
    return {};
  }
  uint16_t key_len;
  std::memcpy(&key_len, data_.data(), sizeof(uint16_t));

  std::string key{
      reinterpret_cast<const char*>(data_.data() + sizeof(uint16_t)), key_len};
  return key;
}

size_t Block::GetOffsetAt(size_t idx) const { return offsets_.at(idx); }

bool Block::AddEntry(const std::string& key, const std::string& value) {
  if (size() + key.size() + value.size() + 3 * sizeof(uint16_t) > capacity_ &&
      !offsets_.empty()) {
    return false;
  }
  size_t old_size = data_.size();

  uint16_t key_len = key.size();
  uint16_t value_len = value.size();
  size_t entry_size = sizeof(uint16_t) + key_len + sizeof(uint16_t) + value_len;

  data_.resize(old_size + entry_size);

  size_t pos = old_size;
  std::memcpy(data_.data() + pos, &key_len, sizeof(key_len));
  pos += sizeof(key_len);

  std::memcpy(data_.data() + pos, key.data(), key_len);
  pos += key_len;

  std::memcpy(data_.data() + pos, &value_len, sizeof(value_len));
  pos += sizeof(value_len);

  std::memcpy(data_.data() + pos, value.data(), value_len);

  offsets_.push_back(old_size);
  return true;
}

std::string Block::GetKeyAt(size_t offset) const {
  uint16_t key_len;
  std::memcpy(&key_len, data_.data() + offset, sizeof(key_len));
  return std::string{
      reinterpret_cast<const char*>(data_.data() + offset + sizeof(key_len)),
      key_len};
}

std::string Block::GetValueAt(size_t offset) const {
  uint16_t key_len;
  std::memcpy(&key_len, data_.data() + offset, sizeof(key_len));

  uint16_t value_len;
  std::memcpy(&value_len, data_.data() + offset + sizeof(key_len) + key_len,
              sizeof(value_len));
  return std::string{
      reinterpret_cast<const char*>(data_.data() + offset + sizeof(key_len) +
                                    key_len + sizeof(value_len)),
      value_len};
}

int Block::CompareKeyAt(size_t offset, const std::string& target) const {
  auto key = GetKeyAt(offset);
  return key.compare(target);
}

std::optional<std::string> Block::GetValueBinary(const std::string& key) const {
  if (offsets_.empty()) {
    return std::nullopt;
  }
  int l = 0, r = offsets_.size() - 1;
  while (l <= r) {
    int mid = l + (r - l) / 2;
    int mid_offset = offsets_[mid];
    int cmp = CompareKeyAt(mid_offset, key);
    if (cmp == 0) {
      return GetValueAt(mid_offset);
    } else if (cmp < 0) {
      l = mid + 1;
    } else {
      r = mid - 1;
    }
  }
  return std::nullopt;
}

Block::Entry Block::GetEntryAt(size_t offset) const {
  return {GetKeyAt(offset), GetValueAt(offset)};
}

size_t Block::size() const {
  return data_.size() + offsets_.size() * sizeof(uint16_t) + sizeof(uint16_t);
}

bool Block::IsEmpty() const { return offsets_.empty(); }

BlockIterator Block::begin() { return BlockIterator{shared_from_this(), 0}; }

BlockIterator Block::end() {
  return BlockIterator{shared_from_this(), offsets_.size()};
}