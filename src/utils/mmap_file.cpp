#include "utils/mmap_file.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <cstring>

bool MMapFile::Open(std::string_view path, bool create) {
  filename_ = std::string(path);

  int flags = O_RDWR;
  if (create) {
    flags |= O_CREAT;
  }

  if (fd_ = ::open(path.data(), flags, 0644); fd_ == -1) {
    return false;
  }

  struct stat st;
  if (fstat(fd_, &st) == -1) {
    Close();
    return false;
  }
  file_size_ = st.st_size;

  if (file_size_ > 0) {
    data_ =
        ::mmap(nullptr, file_size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    if (data_ == MAP_FAILED) {
      Close();
      return false;
    }
  }
  return true;
}

void MMapFile::Close() {
  if (data_ && data_ != MAP_FAILED) {
    ::munmap(data_, file_size_);
    data_ = nullptr;
  }

  if (fd_ != -1) {
    ::close(fd_);
    fd_ = -1;
  }
  file_size_ = 0;
}

bool MMapFile::Write(std::span<const void*> data, size_t size) {
  if (ftruncate(fd_, size) == -1) {
    return false;
  }

  if (data_ && data_ != MAP_FAILED) {
    ::munmap(data_, file_size_);
  }

  file_size_ = size;
  data_ = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (data_ == MAP_FAILED) {
    data_ = nullptr;
    return false;
  }

  std::memcpy(data_, data.data(), size);
  return true;
}

bool MMapFile::Sync() {
  if (data_ && data_ != MAP_FAILED) {
    return ::msync(data_, file_size_, MS_SYNC) == 0;
  }
  return true;
}

bool MMapFile::CreateAndMap(std::string_view path, size_t size) {
  if (fd_ = ::open(path.data(), O_RDWR | O_CREAT | O_TRUNC, 0644); fd_ == -1) {
    return false;
  }

  if (ftruncate(fd_, size) == -1) {
    Close();
    return false;
  }

  data_ = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
  if (data_ == MAP_FAILED) {
    Close();
    return false;
  }

  file_size_ = size;
  return true;
}