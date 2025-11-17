#include <gtest/gtest.h>

#include <filesystem>
#include <random>

#include "utils/file.h"

class FileTest : public ::testing::Test {
 protected:
  void SetUp() override {
    if (!std::filesystem::exists("test_data")) {
      std::filesystem::create_directory("test_data");
    }
  }

  void TearDown() override { std::filesystem::remove_all("test_data"); }

  std::vector<uint8_t> GenerateRandomData(size_t size) {
    std::vector<uint8_t> data;
    data.reserve(size);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    for (int i = 0; i < size; i++) {
      data.push_back(static_cast<uint8_t>(dis(gen)));
    }
    return data;
  }
};

TEST_F(FileTest, BasicReadAndWrite) {
  std::string path = "test_data/basic.data";
  std::vector<uint8_t> data = {1, 2, 3, 4, 5};

  auto file = File::CreateAndWrite(path, data);
  EXPECT_EQ(file.size(), data.size());

  auto opened_file = File::Open(path);
  EXPECT_EQ(opened_file.size(), data.size());

  auto read_data = opened_file.ReadToSlice(0, data.size());
  EXPECT_EQ(read_data, data);
}

TEST_F(FileTest, LargeFile) {
  std::string path = "test_data/large.data";
  size_t size = 1024 * 1024;
  auto data = GenerateRandomData(size);

  auto f = File::CreateAndWrite(path, data);
  EXPECT_EQ(f.size(), size);

  auto open_file = File::Open(path);
  size_t chunk_size = 1024;

  for (size_t offset = 0; offset < size; offset += chunk_size) {
    size_t current_chunk = std::min(chunk_size, size - offset);
    auto chunk = open_file.ReadToSlice(offset, current_chunk);
    for (int i = 0; i < current_chunk; i++) {
      EXPECT_EQ(chunk[i], data[offset + i]);
    }
  }
}

TEST_F(FileTest, PartialRead) {
  std::string path = "test_data/partial.data";
  std::vector<uint8_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  auto f = File::CreateAndWrite(path, data);
  auto open_file = File::Open(path);

  auto middle = open_file.ReadToSlice(2, 3);
  EXPECT_EQ(middle.size(), 3);
  EXPECT_EQ(middle[0], 3);
  EXPECT_EQ(middle[1], 4);
  EXPECT_EQ(middle[2], 5);

  auto start = open_file.ReadToSlice(0, 2);
  EXPECT_EQ(start[0], 1);
  EXPECT_EQ(start[1], 2);

  auto end = open_file.ReadToSlice(8, 2);
  EXPECT_EQ(end[0], 9);
  EXPECT_EQ(end[1], 10);
}

TEST_F(FileTest, ErrorCases) {
  std::string path = "test_data/error.data";
  std::vector<uint8_t> data = {1, 2, 3};

  auto f = File::CreateAndWrite(path, data);
  auto open_file = File::Open(path);

  EXPECT_THROW(open_file.ReadToSlice(2, 2), std::out_of_range);
  EXPECT_THROW(open_file.ReadToSlice(3, 1), std::out_of_range);
  EXPECT_THROW(open_file.ReadToSlice(0, 4), std::out_of_range);

  EXPECT_THROW(File::Open("nonexistent.data"), std::runtime_error);
}

TEST_F(FileTest, MoveSemantics) {
  std::string path = "test_data/move.data";
  std::vector<uint8_t> data = {1, 2, 3};

  auto file1 = File::CreateAndWrite(path, data);
  File file2 = std::move(file1);

  auto read_data = file2.ReadToSlice(0, data.size());
  EXPECT_EQ(read_data, data);
}