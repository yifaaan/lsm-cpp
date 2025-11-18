#include "iterator/iterator.h"

bool operator<(const SearchItem& a, const SearchItem& b) {
  if (a.key != b.key) {
    return a.key < b.key;
  }
  return a.idx < b.idx;
}

bool operator>(const SearchItem& a, const SearchItem& b) {
  if (a.key != b.key) {
    return a.key > b.key;
  }
  return a.idx > b.idx;
}

bool operator==(const SearchItem& a, const SearchItem& b) {
  return a.key == b.key && a.idx == b.idx;
}