#pragma once

#include <unistd.h>

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

// MMapFile 是对底层 POSIX mmap 的轻量封装，负责打开/创建文件并将其
// 映射到内存，同时提供写入与同步到磁盘的能力。
class MMapFile {
 public:
  MMapFile() : fd_(-1), data_(nullptr), file_size_(0) {}
  MMapFile(const MMapFile&) = delete;
  MMapFile& operator=(const MMapFile&) = delete;

  ~MMapFile() { Close(); };

  // open and mmap
  bool Open(std::string_view filename, bool create);

  // create and mmap
  bool CreateAndMap(std::string_view filename, size_t size);

  void Close();

  void* data() { return data_; }

  size_t size() const { return file_size_; }

  bool Write(std::span<const void*>, size_t size);

  bool Sync();

 private:
  int fd_;
  void* data_;
  size_t file_size_;
  std::string filename_;
};