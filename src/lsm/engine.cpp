#include "lsm/engine.h"

#include <format>

#include "consts.h"
#include "memtable/memtable_iterator.h"
#include "sst/sst.h"
#include "sst/sst_iterator.h"

LSMEngine::LSMEngine(std::filesystem::path path) : data_dir_(std::move(path)) {
  if (!std::filesystem::exists(data_dir_)) {
    std::filesystem::create_directory(data_dir_);
  } else {
    // TODO: load sst file
  }
}

std::optional<std::string> LSMEngine::Get(std::string_view key) const {
  // 现在memtable查找
  if (auto value = memtable_.Get(std::string(key))) {
    if (!value->empty()) {
      return value;
    }
    // 空表示删除
    return std::nullopt;
  }

  for (auto sst_id : l0_sst_ids_) {
    auto it = ssts_.find(sst_id);
    if (it == ssts_.end() || !it->second) {
      continue;
    }
    const auto sst = it->second;
    auto sst_iter = sst->Iterator(std::string(key));
    if (sst_iter != sst->end()) {
      const auto& [key, value] = *sst_iter;
      if (!value.empty()) {
        return value;
      }
      return std::nullopt;
    }
  }
  return std::nullopt;
}

void LSMEngine::Put(std::string_view key, std::string_view value) {
  memtable_.Put(std::string(key), std::string(value));
  if (memtable_.total_size() >= kMemSizeLimit) {
    Flush();
  }
}
void LSMEngine::Remove(std::string_view key) {
  memtable_.Remove(std::string(key));
}

void LSMEngine::Flush() {
  if (memtable_.total_size() == 0) {
    return;
  }
  size_t new_sst_id = l0_sst_ids_.empty() ? 0 : l0_sst_ids_.back() + 1;
  constexpr std::size_t kBlockSize = 4096;
  SSTBuilder builder(kBlockSize);

  for (auto it = memtable_.begin(); it != memtable_.end(); ++it) {
    const auto [key, value] = *it;
    builder.Add(key, value);
  }

  auto sst = builder.Build(new_sst_id, SstPath(new_sst_id).string());
  ssts_.emplace(new_sst_id, std::make_shared<SST>(std::move(sst)));

  l0_sst_ids_.push_back(new_sst_id);
  memtable_.Clear();
}

std::filesystem::path LSMEngine::SstPath(std::size_t sst_id) const {
  return data_dir_ / std::format("sst_{}", sst_id);
}

LSM::LSM(std::filesystem::path path) : engine_(std::move(path)) {}

LSM::~LSM() { engine_.Flush(); }