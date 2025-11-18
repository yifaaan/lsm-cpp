#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <string_view>
#include <vector>

#include "utils/mmap_file.h"

// File 封装基于 MMapFile 的文件读写接口，负责创建/打开 SST 文件，
// 提供按偏移读取指定字节切片的能力。
class File {
 public:
  File();
  File(const File&) = delete;
  File& operator=(const File&) = delete;
  File(File&& other) noexcept;
  File& operator=(File&& other) noexcept;
  ~File();

  size_t size() const;

  void set_size(size_t size);

  static File CreateAndWrite(std::string_view path,
                             std::span<const uint8_t> buf);

  static File Open(std::string_view path);

  std::vector<uint8_t> ReadToSlice(size_t offset, size_t length);

 private:
  std::unique_ptr<MMapFile> file_;
  size_t size_;
};