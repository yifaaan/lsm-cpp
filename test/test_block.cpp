
#include <gtest/gtest.h>

#include <format>

#include "block/block.h"
#include "block/block_iterator.h"

class BlockTest : public ::testing::Test {
 protected:
  std::vector<uint8_t> GetEncodedBlock() {
    std::vector<uint8_t> encoded = {// Data Section
                                    // Entry 1: "apple" -> "red"
                                    5,
                                    0,  // key len
                                    'a', 'p', 'p', 'l',
                                    'e',  // key
                                    3,
                                    0,  // value len
                                    'r', 'e',
                                    'd',  // value

                                    // Entry 2: "banana" -> "yellow"
                                    6,
                                    0,  // key len
                                    'b', 'a', 'n', 'a', 'n',
                                    'a',  // key
                                    6,
                                    0,  // value len
                                    'y', 'e', 'l', 'l', 'o',
                                    'w',  // value

                                    // Entry 3: "orange" -> "orange"
                                    6,
                                    0,  // key len
                                    'o', 'r', 'a', 'n', 'g',
                                    'e',  // key
                                    6,
                                    0,  // value len
                                    'o', 'r', 'a', 'n', 'g',
                                    'e',  // value

                                    // Offset Section
                                    0, 0, 12, 0, 28, 0,

                                    // Num of elements
                                    3, 0};
    return encoded;
  }
};

TEST_F(BlockTest, DecodeTest) {
  auto encoded = GetEncodedBlock();
  auto block = Block::Decode(encoded);

  EXPECT_EQ(block->GetFirstKey(), "apple");

  EXPECT_EQ(block->GetValueBinary("apple").value(), "red");
  EXPECT_EQ(block->GetValueBinary("banana").value(), "yellow");
  EXPECT_EQ(block->GetValueBinary("orange").value(), "orange");
}

TEST_F(BlockTest, BinarySearchTest) {
  Block block;
  block.AddEntry("apple", "red");
  block.AddEntry("banana", "yellow");
  block.AddEntry("orange", "orange");

  EXPECT_EQ(block.GetValueBinary("apple").value(), "red");
  EXPECT_EQ(block.GetValueBinary("banana").value(), "yellow");
  EXPECT_EQ(block.GetValueBinary("orange").value(), "orange");

  EXPECT_FALSE(block.GetValueBinary("fds").has_value());
  EXPECT_FALSE(block.GetValueBinary("").has_value());
}

TEST_F(BlockTest, EdgeCaseTest) {
  Block block;

  EXPECT_EQ(block.GetFirstKey(), "");
  EXPECT_FALSE(block.GetValueBinary("any").has_value());

  block.AddEntry("", "");
  EXPECT_EQ(block.GetFirstKey(), "");
  EXPECT_EQ(block.GetValueBinary("").value(), "");

  block.AddEntry("key\0with\tnull", "value\rwith\nnull");
  std::string special_key("key\0with\tnull");
  std::string special_value("value\rwith\nnull");
  EXPECT_EQ(block.GetValueBinary(special_key).value(), special_value);
}

TEST_F(BlockTest, LargeDataTest) {
  Block block;
  const int n = 1000;

  for (int i = 0; i < n; i++) {
    auto key = std::format("key{:04}", i);
    auto value = std::format("value{}", i);
    block.AddEntry(key, value);
  }

  for (int i = 0; i < n; i++) {
    auto key = std::format("key{:04}", i);
    auto value = std::format("value{}", i);
    EXPECT_EQ(block.GetValueBinary(key).value(), value);
  }
}

TEST_F(BlockTest, ErrorHandingTest) {
  std::vector<uint8_t> invalid_data = {1, 2, 3};
  EXPECT_THROW(Block::Decode(invalid_data), std::runtime_error);

  std::vector<uint8_t> empty_data;
  EXPECT_THROW(Block::Decode(empty_data), std::runtime_error);
}

TEST_F(BlockTest, IteratorTest) {
  auto block = std::make_shared<Block>();

  EXPECT_EQ(block->begin(), block->end());

  const int n = 100;
  std::vector<std::pair<std::string, std::string>> test_data;

  for (int i = 0; i < n; i++) {
    auto key = std::format("key{:03}", i);
    auto value = std::format("value{:03}", i);

    block->AddEntry(key, value);
    test_data.emplace_back(key, value);
  }

  size_t count = 0;
  for (const auto& [k, v] : *block) {
    EXPECT_EQ(k, test_data[count].first);
    EXPECT_EQ(v, test_data[count].second);
    count++;
  }
  EXPECT_EQ(count, test_data.size());

  auto it = block->begin();
  EXPECT_EQ((*it).first, "key000");
  ++it;
  EXPECT_EQ((*it).first, "key001");
  it++;
  EXPECT_EQ((*it).first, "key002");

  auto encoded = block->Encode();
  auto decoded_block = Block::Decode(encoded);
  count = 0;
  for (const auto& [key, value] : *decoded_block) {
    EXPECT_EQ(key, test_data[count].first);
    EXPECT_EQ(value, test_data[count].second);
    count++;
  }
}