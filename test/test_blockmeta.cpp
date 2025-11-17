#include <gtest/gtest.h>

#include <format>

#include "block/block_meta.h"

class BlockMetaTest : public ::testing::Test {
 protected:
  friend class BlockMeta;

  std::vector<BlockMeta> CreateTestMetas() {
    std::vector<BlockMeta> metas;

    // Block 1: offset = 0, "a100" -> "a199"
    BlockMeta meta1;
    meta1.offset_ = 0;
    meta1.first_key_ = "a100";
    meta1.last_key_ = "a199";
    metas.push_back(meta1);

    // Block 2: offset = 100, "a200" -> "a299"
    BlockMeta meta2;
    meta2.offset_ = 100;
    meta2.first_key_ = "a200";
    meta2.last_key_ = "a299";
    metas.push_back(meta2);

    BlockMeta meta3;
    meta3.offset_ = 200;
    meta3.first_key_ = "a300";
    meta3.last_key_ = "a399";
    metas.push_back(meta3);
    return metas;
  }
};

TEST_F(BlockMetaTest, BasicEncodeDecodeTest) {
  auto orginal_metas = CreateTestMetas();

  auto encoded_metas = BlockMeta::EncodeMetasToSlice(orginal_metas);
  EXPECT_FALSE(encoded_metas.empty());

  auto decoded_metas = BlockMeta::DecodeMetasFromSlice(encoded_metas);

  ASSERT_EQ(orginal_metas, decoded_metas);
}

TEST_F(BlockMetaTest, EmptyMetaTest) {
  std::vector<BlockMeta> empty_metas;

  auto encoded_metas = BlockMeta::EncodeMetasToSlice(empty_metas);
  EXPECT_FALSE(encoded_metas.empty());

  auto decoded_metas = BlockMeta::DecodeMetasFromSlice(encoded_metas);
  EXPECT_TRUE(decoded_metas.empty());
}

TEST_F(BlockMetaTest, SpecialCharTest) {
  std::vector<BlockMeta> metas;
  BlockMeta meta;
  meta.offset_ = 0;
  meta.first_key_ = std::string("key\0with\0null", 12);
  meta.last_key_ = std::string("value\0with\0null", 14);
  metas.push_back(meta);

  std::vector<uint8_t> encoded_data;
  auto encoded_metas = BlockMeta::EncodeMetasToSlice(metas);
  auto decoded_metas = BlockMeta::DecodeMetasFromSlice(encoded_metas);

  ASSERT_EQ(decoded_metas.size(), 1);
  EXPECT_EQ(decoded_metas[0].first_key_, std::string("key\0with\0null", 12));
  EXPECT_EQ(decoded_metas[0].last_key_, std::string("value\0with\0nul", 14));
}

TEST_F(BlockMetaTest, ErrorHandleTest) {
  std::vector<uint8_t> invalid_data = {1, 2, 3};
  EXPECT_THROW(BlockMeta::DecodeMetasFromSlice(invalid_data),
               std::runtime_error);

  std::vector<uint8_t> empty_data;
  EXPECT_THROW(BlockMeta::DecodeMetasFromSlice(empty_data), std::runtime_error);

  auto metas = CreateTestMetas();
  auto encoded_data = BlockMeta::EncodeMetasToSlice(metas);
  encoded_data.back() ^= 1;
  EXPECT_THROW(BlockMeta::DecodeMetasFromSlice(encoded_data),
               std::runtime_error);
}

TEST_F(BlockMetaTest, LargeDataTest) {
  std::vector<BlockMeta> metas;
  const int n = 10000;
  metas.reserve(n);

  for (int i = 0; i < n; i++) {
    BlockMeta meta;
    meta.offset_ = i * 100;
    auto first_key = std::format("key{:03}00", i);
    auto last_key = std::format("key{:03}00", i);
    meta.first_key_ = std::move(first_key);
    meta.last_key_ = std::move(last_key);
    metas.push_back(meta);
  }

  auto encoded_data = BlockMeta::EncodeMetasToSlice(metas);
  auto decoded_data = BlockMeta::DecodeMetasFromSlice(encoded_data);

  ASSERT_EQ(metas.size(), decoded_data.size());
  EXPECT_EQ(metas, decoded_data);
}

TEST_F(BlockMetaTest, OrderTest) {
  auto metas = CreateTestMetas();

  for (const auto& meta : metas) {
    EXPECT_LT(meta.first_key_, meta.last_key_);
  }

  for (int i = 1; i < metas.size(); i++) {
    EXPECT_LT(metas[i - 1].last_key_, metas[i].first_key_);
  }
}