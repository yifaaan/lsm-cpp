#pragma once

#include "utils/file.h"

#include <cstring>
#include <format>

File::File() : file_(std::make_unique<MMapFile>()) {}

File::~File() = default;

File::File(File&& other) noexcept
    : file_(std::move(other.file_)), size_(other.size_) {
  other.size_ = 0;
}

File& File::operator=(File&& other) noexcept {
  if (this != &other) {
    file_ = std::move(other.file_);
    size_ = other.size_;
    other.size_ = 0;
  }
  return *this;
}

size_t File::size() const { return file_->size(); }

void File::set_size(size_t size) { size_ = size; }

File File::CreateAndWrite(std::string_view path, std::span<const uint8_t> buf) {
  File f;
  if (!f.file_->CreateAndMap(path, buf.size())) {
    throw std::runtime_error(
        std::format("Failed to create and map file: {}", path));
  }
  std::memcpy(f.file_->data(), buf.data(), buf.size());
  f.file_->Sync();
  return f;
}
