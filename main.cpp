#include <cstdlib>
#include <cstring>
#include <iostream>

#include "FileStreamer.hpp"
#include "Sort.hpp"

// holds entries according to spec
struct Key {
  unsigned char key[10];

  Key(){};

  Key(const Key &other) { memcpy(key, other.key, 10 * sizeof(key[0])); }

  bool operator==(const Key &other) const {
    return !memcmp(key, other.key, 10);
  }

  bool operator<(const Key &other) const {
    for (int i = 0; i < 10; ++i) {
      if (key[i] < other.key[i])
        return true;
      else if (key[i] > other.key[i])
        return false;
    }
    return false;
  }

  bool operator>(const Key &other) const {
    return !operator==(other) && !operator<(other);
  }
};

struct Entry {
  Key key;
  char value[90];
};

int main(int argc, char *argv[]) {
  // parse arguments
  std::string input_path = "";
  std::string output_path = "";
  std::size_t max_chunk_size = 4000000 / sizeof(Entry);
  std::size_t min_chunk_size = 100 * sizeof(Entry);
  std::size_t entry_count = 4000000000;
  for (int i = 1; i < argc; ++i) {
    // check for optional arguments
    if (std::strlen(argv[i]) > 3 && '-' == argv[i][0] && '=' == argv[i][2]) {
      // set chunk size
      if ('S' == argv[i][1])
        max_chunk_size = std::atoi(&argv[i][3]);
      // set chunk size
      else if ('s' == argv[i][1])
        min_chunk_size = std::atoi(&argv[i][3]);
      // set chunk count
      else if ('c' == argv[i][1])
        entry_count = std::atoi(&argv[i][3]);
    } else if (input_path == "") {
      // set input path
      input_path = argv[i];
    } else {
      // set output path
      output_path = argv[i];
    }
  }

  if ("" == input_path || "" == output_path || 0 == min_chunk_size ||
      0 == max_chunk_size || 0 == entry_count) {
    std::cout
        << "Usage: sort [-s=MIN_SIZE] [-S=MAX_SIZE] [-c=COUNT] INPUT OUTPUT\n";
    return EXIT_FAILURE;
  }

  std::cout << "input path: " << input_path << "\n";
  std::cout << "output path: " << output_path << "\n";
  std::cout << "maximal chunk size: " << max_chunk_size << "\n";
  std::cout << "minimal chunk size: " << min_chunk_size << "\n";
  std::cout << "entry count: " << entry_count << "\n";

  // stream file
  std::function<void(Entry *, std::size_t, std::size_t)> sort =
      [](Entry *entries, std::size_t chunk_start, std::size_t chunk_size) {
        quicksort<Entry>(&entries[chunk_start], chunk_size);
      };
  FileStreamer<Entry> fs(entry_count, min_chunk_size, max_chunk_size);
  fs.read(input_path, sort);

  // write file
  std::function<void(Entry *, std::size_t)> write = fs.write(output_path);
  merge_lists<Entry>(write, entry_count, fs.get(), max_chunk_size,
                     fs.getChunkStarts());
  fs.flush_write();

  return EXIT_SUCCESS;
}
