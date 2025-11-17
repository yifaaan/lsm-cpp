#pragma once

/*
 * ------------------------------------------------------------------------
 * |         Block Section         |  Meta Section |       Extra          |
 * ------------------------------------------------------------------------
 * | data block | ... | data block |    metadata   | metadata offset (64) |
 * ------------------------------------------------------------------------

 * 其中, metadata 是一个数组加上一些描述信息, 数组每个元素由一个 BlockMeta
 编码形成 MetaEntry, MetaEntry 结构如下:
 * ---------------------------------------------------------------------------------------------------
 * | offset(32) | 1st_key_len(16) | 1st_key(1st_key_len) | last_key_len(16) |
 last_key(last_key_len) |
 * ---------------------------------------------------------------------------------------------------

 * Meta Section 的结构如下:
 * ---------------------------------------------------------------
 * | num_entries (32) | MetaEntry | ... | MetaEntry | Hash (32) |
 * ---------------------------------------------------------------
 * 其中, num_entries 表示 metadata 数组的长度, Hash 是 metadata
 数组的哈希值(只包括数组部分, 不包括 num_entries ), 用于校验 metadata 的完整性
 */

#include <vector>

#include "block/block.h"
#include "block/block_meta.h"
#include "utils/file.h"

class SSTBuilder;

class SST {
 public:
  friend class SSTBuilder;
  
  static SST Open(size_t sst_id, File file);

  static SST CreateWithMetaOnly(size_t sst_id, size_t file_size,
                                std::string_view first_key,
                                std::string_view last_key);

  std::shared_ptr<Block> ReadBlock(size_t block_idx);

  size_t FindBlockIdx(std::string_view key);

  size_t num_blocks() const { return meta_entries_.size(); }

  std::string_view first_key() const { return first_key_; }

  std::string_view last_key() const { return last_key_; }

  size_t sst_size() const { return file_.size(); }

  size_t sst_id() const { return sst_id_; }

 private:
  File file_;
  std::vector<BlockMeta> meta_entries_;
  uint32_t meta_block_offset_;
  size_t sst_id_;
  std::string first_key_;
  std::string last_key_;
};

class SSTBuilder {
 public:
  explicit SSTBuilder(size_t block_size); 

  void Add(std::string_view key, std::string_view value);
  
  void FinishBlock();

  size_t estimated_size() const { return data_.size(); }

  // 将内存数据写入SST文件
  SST Build(size_t sst_id, std::string_view path);

 private:
  // 正在写入的block
  Block block_;
  std::string first_key_;
  std::string last_key_;
  std::vector<BlockMeta> meta_entries_;
  // 所有已完成编码的block字节
  std::vector<uint8_t> data_;
  size_t block_size_;
  std::vector<uint32_t> key_hashes_;
};
