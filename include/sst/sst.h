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

// SST 表示一个已经落盘的 SSTable 文件视图，负责：
// - 按 block 读取数据；
// - 根据 key 在元数据中定位所属 block；
// - 提供首尾 key、block 数量等元信息查询。
class SST {
 public:
  // SSTBuilder 负责构建 SST 文件，需要直接访问 SST 的私有成员。
  friend class SSTBuilder;
  
  // 从已经存在的文件句柄中打开一个 SST。
  // 会读取文件尾部的 meta offset，再解析 Meta Section 得到 BlockMeta 数组。
  static SST Open(size_t sst_id, File file);

  // 仅根据元数据信息构造一个逻辑上的 SST 描述（不真正读取文件内容）。
  // 通常用于仅依赖 first/last key 和文件大小的场景，比如元信息索引。
  static SST CreateWithMetaOnly(size_t sst_id, size_t file_size,
                                std::string_view first_key,
                                std::string_view last_key);

  // 根据 block 的索引读取并解码指定的数据块。
  std::shared_ptr<Block> ReadBlock(size_t block_idx);

  // 在元数据中根据 key 二分查找其所在的 block 下标。
  // 若 key 超出整个 SST 的 key 范围，会抛出 std::runtime_error。
  size_t FindBlockIdx(std::string_view key);

  // 返回 SST 中包含的 block 数量。
  size_t num_blocks() const { return meta_entries_.size(); }

  // 返回整个 SST 的最小 key（第一个 block 的 first_key_）。
  std::string_view first_key() const { return first_key_; }

  // 返回整个 SST 的最大 key（最后一个 block 的 last_key_）。
  std::string_view last_key() const { return last_key_; }

  // 返回 SST 文件的字节大小。
  size_t sst_size() const { return file_.size(); }

  // 返回 SST 的标识 id。
  size_t sst_id() const { return sst_id_; }

 private:
  // 底层文件封装，负责 mmap/读取原始字节。
  File file_;
  // 每个 block 的元信息（偏移量、首尾 key）。
  std::vector<BlockMeta> meta_entries_;
  // Meta Section 在文件中的起始偏移（Block Section 的总长度）。
  uint32_t meta_block_offset_;
  // SST 的唯一标识。
  size_t sst_id_;
  // 整个 SST 范围内的最小 / 最大 key。
  std::string first_key_;
  std::string last_key_;
};

// SSTBuilder 负责将一串有序的 KV 流切分成若干 Block，
// 编码并写出到磁盘，最终生成一个可被 SST 打开的 SSTable 文件。
class SSTBuilder {
 public:
  // 指定目标 block 大小（字节），用于控制何时切分 block。
  explicit SSTBuilder(size_t block_size); 

  // 向当前 SST 中追加一条有序的 key/value 记录。
  // 若当前 block 容量不足，会先 FinishBlock 再开启新 block。
  void Add(std::string_view key, std::string_view value);
  
  // 将当前正在构建的 block 封板：
  // - 调用 Block::Encode 得到字节序列；
  // - 追加到 data_；
  // - 生成对应的 BlockMeta 记录到 meta_entries_。
  void FinishBlock();

  // 估算当前已经累积的 Block Section 大小（不含元数据）。
  size_t estimated_size() const { return data_.size(); }

  // 将内存中的数据编码并写入 SST 文件，返回构造好的 SST 视图。
  SST Build(size_t sst_id, std::string_view path);

 private:
  // 正在写入的 block。
  Block block_;
  // 当前 block 的首、尾 key。
  std::string first_key_;
  std::string last_key_;
  // 所有 block 的元信息。
  std::vector<BlockMeta> meta_entries_;
  // 所有已完成编码的 block 字节（按顺序拼接）。
  std::vector<uint8_t> data_;
  // 目标 block 大小（字节）。
  size_t block_size_;
  // 记录所有 key 的哈希值，后续可用于构建 BloomFilter 等结构。
  std::vector<uint32_t> key_hashes_;
};
