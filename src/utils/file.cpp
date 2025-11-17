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

File File::Open(std::string_view path) {
  File f;
  if (!f.file_->Open(path, false)) {
    throw std::runtime_error(std::format("Failed to open file {}", path));
  }
  return f;
}

std::vector<uint8_t> File::ReadToSlice(size_t offset, size_t length) {
  if (offset + length > file_->size()) {
    throw std::out_of_range("Read beyond file size");
  }
  std::vector<uint8_t> result(length);
  auto ptr = reinterpret_cast<const uint8_t*>(file_->data());
  std::memcpy(result.data(), ptr + offset, length);
  return result;
}