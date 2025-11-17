#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>
#include <stdexcept>
#include <string>
#include <vector>

/*
 * -------------------------------------------------------------------------------------------
 * |         Block Section         |          Meta Section         | Extra |
 * -------------------------------------------------------------------------------------------
 * | data block | ... | data block |            metadata           | meta block
 offset (32) |
 * -------------------------------------------------------------------------------------------

 * 其中, metadata 是一个数组加上一些描述信息, 数组每个元素由一个 BlockMeta
 编码形成 MetaEntry, MetaEntry 结构如下:
 * --------------------------------------------------------------------------------------------------------------
 * | offset (32) | first_key_len (16) | first_key (first_key_len) |
 last_key_len(16) | last_key (last_key_len) |
 * --------------------------------------------------------------------------------------------------------------

 * Meta Section 的结构如下:
 * --------------------------------------------------------------------------------------------------------------
 * | num_entries (32) | MetaEntry | ... | MetaEntry | Hash (32) |
 * --------------------------------------------------------------------------------------------------------------
 * 其中, num_entries 表示 metadata 数组的长度, Hash 是 metadata
 数组的哈希值(只包括数组部分, 不包括 num_entries ), 用于校验 metadata 的完整性
 */

// Block的信息
class BlockMeta {
 private:

  // // 向buffer写入value并返回更新后的buffer
  // // 需要buffer有足够的空间存储value
  // template <std::integral T>
  // std::span<uint8_t> WriteInteger(std::span<uint8_t> buffer, T value) {
  //   if (buffer.size() < sizeof(T)) {
  //     throw std::runtime_error("Buffer overflow");
  //   }
  //   std::memcpy(buffer.data(), &value, sizeof(T));
  //   return buffer.subspan(sizeof(T));
  // }

  // template <std::integral T>
  // static T ReadInteger(std::span<const uint8_t>& buffer) {
  //   if (buffer.size() < sizeof(T)) {
  //     throw std::runtime_error("Buffer overflow");
  //   }
  //   T value;
  //   std::memcpy(&value, buffer.data(), sizeof(T));
  //   buffer = buffer.subspan(sizeof(T));
  //   return value;
  // }

 public:
  BlockMeta();
  BlockMeta(size_t offset, const std::string& first_key,
            const std::string& last_key);

  bool operator==(const BlockMeta&) const = default;

  // 将一组BlockMeta序列化成字节数组
  static std::vector<uint8_t> EncodeMetasToSlice(
      std::span<const BlockMeta> meta_entries);

  // 从字节数组反序列化出BlockMeta
  static std::vector<BlockMeta> DecodeMetasFromSlice(
      std::span<const uint8_t> meta_data);

  // Block在文件中的偏移量
  size_t offset_;
  // Block中最小的key
  std::string first_key_;
  // Block中最大的key
  std::string last_key_;
};