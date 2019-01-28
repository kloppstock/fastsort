#ifndef SORT_HPP
#define SORT_HPP

#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>

template <typename T> void quicksort(T *data, int size) {
  // select head, tail and pivot
  decltype(T::key) pivot_element = data[0].key;
  int head = size - 1, tail = 0;

  // pre sort list
  do {
    // decrement head
    while (data[head].key > pivot_element)
      --head;

    // increment tail
    while (data[tail].key < pivot_element)
      ++tail;

    // swap data
    if (tail <= head) {
      std::swap(data[head], data[tail]);
	  ++tail;
	  --head;
	}
  } while (tail <= head);

  if (head > 0)
    quicksort(data, tail);
  if (tail < size - 1)
    quicksort(&data[tail], size - tail);
}

template <typename T>
void merge_lists(std::function<void(T *, std::size_t)> write_func,
                 std::size_t entry_count, T const *const data,
                 std::size_t chunk_size, std::vector<std::size_t> offsets) {
  // create vector to track processed elements
  std::vector<std::size_t> processed(offsets.size(), 0);

  // push back size to offsets to mark end
  offsets.push_back(entry_count);

  // calculate sizes of individual chunks
  std::vector<std::size_t> sizes(processed.size());
  for (unsigned int i = 0; i < sizes.size(); ++i)
    sizes[i] = offsets[i + 1] - offsets[i];

  // allocate space for chunk
  T *chunk = new T[chunk_size];

  // process chunks
  for (std::size_t c = 0; c < (entry_count + chunk_size - 1) / chunk_size;
       ++c) {

    // process one chunk at a time
    unsigned int element = 0;
    for (; element < chunk_size; ++element) {
      // create vector to track first elements
      std::vector<T> first_elements;
      first_elements.reserve(processed.size());

      // create vector to track max indices of first elements
      std::vector<std::size_t> max_indices;
      max_indices.reserve(processed.size());

      // get first elements and skip already processed chunks
      for (unsigned int i = 0; i < processed.size(); ++i) {
        if (processed[i] < sizes[i]) {
          first_elements.emplace_back(data[offsets[i] + processed[i]]);
          max_indices.emplace_back(i);
        }
      }

      // abort if all elements are processed
      if (first_elements.empty())
        break;

      // logarithmically find smallest value with accompanying index
      for (std::size_t second_half = (first_elements.size() + 1) / 2;
           second_half > 1; second_half = (second_half + 1) / 2) {
        for (unsigned int i = 0; i < second_half; ++i) {
          std::size_t second_half_index = i + second_half;
          if (second_half_index < first_elements.size() &&
              first_elements[i].key > first_elements[second_half_index].key) {
            first_elements[i] = first_elements[second_half_index];
            max_indices[i] = max_indices[second_half_index];
          }
        }
      }

      // make last comparison
      if (1 < first_elements.size() &&
          first_elements[0].key > first_elements[1].key) {
        first_elements[0] = first_elements[1];
        max_indices[0] = max_indices[1];
      }

      // increase process counter for the selected chunk
      ++processed[max_indices[0]];

      // put smallest element into output chunk
      chunk[element] = first_elements[0];
    }

    // write chunk
    write_func(chunk, element);
  }

  delete[] chunk;
}

#endif // SORT_HPP
