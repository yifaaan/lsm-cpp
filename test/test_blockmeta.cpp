#include <gtest/gtest.h>

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

