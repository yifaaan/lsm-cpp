#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

/*
----------------------------------------------------------------------------------------------------
|             Data Section             |              Offset Section | Extra |
----------------------------------------------------------------------------------------------------
| Entry #1 | Entry #2 | ... | Entry #N | Offset #1 | Offset #2 | ... | Offset
#N| num_of_elements |
----------------------------------------------------------------------------------------------------

-----------------------------------------------------------------------
|                           Entry #1                            | ... |
-----------------------------------------------------------------------
| key_len (2B) | key (keylen) | value_len (2B) | value (varlen) | ... |
-----------------------------------------------------------------------
*/

class BlockIterator;

class Block : public std::enable_shared_from_this<Block> {
 public:
  Block() = default;
  explicit Block(size_t capacity);

  std::vector<uint8_t> Encode() const;

  static std::shared_ptr<Block> Decode(const std::vector<uint8_t>& encoded);

  std::string GetFirstKey() const;

  // 获取idx索引位置的entry在data_中的偏移
  size_t GetOffsetAt(size_t idx) const;

  bool AddEntry(const std::string& key, const std::string& value);

  std::optional<std::string> GetValueBinary(const std::string& key) const;

  // Block所占字节数(Data Section + Offset Secton + Num elements)
  size_t size() const;

  bool IsEmpty() const;

  BlockIterator begin();
  BlockIterator end();

  struct Entry {
    std::string key;
    std::string value;
  };

 private:
  friend class BlockIterator;

  Entry GetEntryAt(size_t offset) const;

  // 从data_指定偏移处获取entry的key
  std::string GetKeyAt(size_t offset) const;

  // 从data_指定偏移处获取entry的value
  std::string GetValueAt(size_t offset) const;

  // key_at.compare(target)
  int CompareKeyAt(size_t offset, const std::string& target) const;

  // 上面的 Data Section
  std::vector<uint8_t> data_;
  // Offset Section（N 个 entry 的起始偏移）
  std::vector<uint16_t> offsets_;
  size_t capacity_;
};