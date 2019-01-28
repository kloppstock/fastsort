#ifndef SORT_TEST_HPP
#define SORT_TEST_HPP

#include "../Sort.hpp"
#include <algorithm>
#include <cstring>
#include <functional>
#include <gtest/gtest.h>

struct TEntry {
  int key;
  unsigned long long value;

  bool operator==(const TEntry &other) const {
    if (key == other.key && value == other.value)
      return true;
    return false;
  }
};

TEST(SortTest, QuicksortTest) {
  // initialize test data
  constexpr int keys[] = {8, 5, 7, 8, 9, 0, 6};
  constexpr std::size_t element_count = sizeof(keys) / sizeof(keys[0]);

  // generate entries
  std::vector<TEntry> entries;
  entries.reserve(element_count);
  for (unsigned int i = 0; i < element_count; ++i)
	  entries.push_back({keys[i], static_cast<std::size_t>(keys[i])});

  // generate reference
  std::vector<TEntry> reference = entries;
  std::qsort(reference.data(), element_count, sizeof(TEntry),
             [](const void *a, const void *b) {
               const decltype(TEntry::key) key_a =
                   reinterpret_cast<const TEntry *>(a)->key;
               const decltype(TEntry::key) key_b =
                   reinterpret_cast<const TEntry *>(b)->key;

               if (key_a < key_b)
                 return -1;
               if (key_a > key_b)
                 return 1;
               return 0;
             });

  std::vector<TEntry> sorted = entries;
  quicksort<TEntry>(sorted.data(), entries.size());
  std::vector<TEntry> sorted_vector(entries.size());

  EXPECT_EQ(reference, sorted);
}

TEST(SortTest, MergeListsTest) {
  // initialize test data
  constexpr TEntry lists[] = {
      {4, 4},   {5, 5},   {7, 7},   {8, 8},   {9, 9}, {2, 2},   {3, 3},
      {6, 6},   {7, 7},   {8, 8},   {2, 2},   {4, 4}, {5, 5},   {8, 8},
      {14, 14}, {4, 4},   {5, 5},   {5, 5},   {8, 8}, {11, 11}, {3, 3},
      {4, 4},   {10, 10}, {12, 12}, {15, 15}, {8, 8}, {12, 12}, {25, 28}};

  // initialize metadata
  std::vector<std::size_t> offsets = {0, 5, 10, 15, 20, 25};
  constexpr std::size_t entry_count = sizeof(lists) / sizeof(lists[0]);
  constexpr std::size_t max_chunk_size = 5;

  // allocate vector for the sorted output
  std::vector<TEntry> sorted;
  sorted.reserve(entry_count);
  bool exceeded_max_chunk_size = false;

  // create write lambda
  std::function<void(TEntry *, std::size_t)> write_func =
      [&sorted, &exceeded_max_chunk_size](TEntry *data,
                                          std::size_t chunk_size) {
        if (chunk_size > max_chunk_size)
          exceeded_max_chunk_size = true;

        for (unsigned int i = 0; i < chunk_size; ++i)
          sorted.emplace_back(data[i]);
      };

  // merge lists
  merge_lists<TEntry>(write_func, entry_count,
                      static_cast<TEntry const *const>(lists), max_chunk_size,
                      offsets);

  // generate reference
  std::vector<TEntry> reference(entry_count);
  memcpy(reference.data(), lists, sizeof(lists));
  std::qsort(reference.data(), entry_count, sizeof(TEntry),
             [](const void *a, const void *b) {
               const decltype(TEntry::key) key_a =
                   reinterpret_cast<const TEntry *>(a)->key;
               const decltype(TEntry::key) key_b =
                   reinterpret_cast<const TEntry *>(b)->key;

               if (key_a < key_b)
                 return -1;
               if (key_a > key_b)
                 return 1;
               return 0;
             });

  // compare results
  EXPECT_EQ(reference, sorted);
}

#endif // SORT_TEST_HPP
