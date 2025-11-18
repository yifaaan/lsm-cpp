#pragma once

#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <filesystem>

#include "memtable/memtable.h"
#include "sst/sst.h"

class LSMEngine {
 public:
  explicit LSMEngine(std::filesystem::path path);
  ~LSMEngine() = default;

  std::optional<std::string> Get(std::string_view key) const;
  void Put(std::string_view key, std::string_view value);
  void Remove(std::string_view key);
  void Flush();

  std::filesystem::path SstPath(size_t sst_id) const;

  // sst文件目录
  std::filesystem::path data_dir_;
  // 内存写入
  MemTable memtable_;
  // L0层所有的sst的id，最新文件在尾部
  std::list<size_t> l0_sst_ids_;
  // sst_id -> SST
  std::unordered_map<size_t, std::shared_ptr<SST>> ssts_;
};

class LSM {
 public:
  explicit LSM(std::filesystem::path path);
  ~LSM();

 private:
  LSMEngine engine_;
};