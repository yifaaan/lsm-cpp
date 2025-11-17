#include <gtest/gtest.h>

#include <filesystem>
#include <string>
#include <format>

#include "sst/sst.h"

class SSTTest : public ::testing::Test {
 protected:
  void SetUp() override {
    if (!std::filesystem::exists("test_data")) {
      std::filesystem::create_directory("test_data");
    }
  }

  void TearDown() override { std::filesystem::remove_all("test_data"); }

  SST CreateTestSST(size_t block_size, size_t num_entries) {
    SSTBuilder builder(block_size);
    for (int i = 0; i < num_entries; i++) {
      auto key = std::format("key{:04}", i);
      builder.Add(key, "value" + std::to_string(i));
    }
    return builder.Build(1, "test_data/test.sst");
  }
};

TEST_F(SSTTest, BasicReadAndWrite) {
  SSTBuilder builder(1024);

  builder.Add("key1", "value1");
  builder.Add("key2", "value2");
  builder.Add("key3", "value3");

  auto sst = builder.Build(1, "test_data/basic.sst");

  EXPECT_EQ(sst.first_key(), "key1");
  EXPECT_EQ(sst.last_key(), "key3");
  EXPECT_EQ(sst.sst_id(), 1);
  EXPECT_EQ(sst.sst_size(), 82);

  auto block = sst.ReadBlock(0);
  EXPECT_TRUE(block != nullptr);
  auto value = block->GetValueBinary("key2");
  EXPECT_TRUE(value.has_value());
  EXPECT_EQ(*value, "value2");
}

TEST_F(SSTTest, BlockSplitting) {
  SSTBuilder builder(64);
  for (int i = 0; i < 20; i++) {
    auto key = std::format("key{:04}", i);
    builder.Add(key, "value" + std::to_string(i));
  }

  auto sst = builder.Build(1, "test_data/split.sst");

  EXPECT_GT(sst.num_blocks(), 1);
  for (int i = 0; i < sst.num_blocks(); i++) {
    auto block = sst.ReadBlock(i);
    EXPECT_TRUE(block != nullptr);
  }
}

TEST_F(SSTTest, KeySearch) {
  auto sst = CreateTestSST(256, 100);

  std::string key = std::format("key{:04}", 50);
  size_t idx = sst.FindBlockIdx(key);
  auto block = sst.ReadBlock(idx);
  auto value = block->GetValueBinary(key);
  EXPECT_TRUE(value.has_value());
  EXPECT_EQ(*value, "value50");

  EXPECT_THROW(sst.FindBlockIdx("key9999"), std::runtime_error);
}

TEST_F(SSTTest, MetaData) {
  auto sst = CreateTestSST(512, 10);

  EXPECT_GT(sst.num_blocks(), 0);

  EXPECT_EQ(sst.first_key(), "key0000");
  EXPECT_EQ(sst.last_key(), "key0009");
}

TEST_F(SSTTest, EmptySST) {
  SSTBuilder builder(1024);

  EXPECT_THROW(builder.Build(1, "test_data/empty.sst"), std::runtime_error);
}

TEST_F(SSTTest, ReopenSST) {
  auto sst = CreateTestSST(256, 10);

  File f = File::Open("test_data/test.sst");
  auto reopen_sst = SST::Open(1, std::move(f));

  EXPECT_EQ(sst.first_key(), reopen_sst.first_key());
  EXPECT_EQ(sst.last_key(), reopen_sst.last_key());
  EXPECT_EQ(sst.num_blocks(), reopen_sst.num_blocks());
}

TEST_F(SSTTest, LargeTest) {
  SSTBuilder builder(4096);

  for (int i = 0; i < 10000; i++) {
    std::string key = std::format("key{:04}", i);
    std::string value = std::string(100, 'v') + std::to_string(i);
    builder.Add(key, value);
  }

  auto sst = builder.Build(1, "test_data/large.sst");

  EXPECT_GT(sst.num_blocks(), 1);
  EXPECT_EQ(sst.first_key(), "key0000");
  EXPECT_EQ(sst.last_key(), "key9999");

  std::vector<int> test_indices = {0, 100, 500, 5000, 9999};
  for (int i : test_indices) {
    std::string key = std::format("key{:04}", i);
    size_t idx = sst.FindBlockIdx(key);
    auto block = sst.ReadBlock(idx);
    auto value = block->GetValueBinary(key);
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(*value, std::string(100, 'v') + std::to_string(i));
  }
}