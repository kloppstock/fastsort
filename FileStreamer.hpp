#ifndef FILESTREAMER_HPP
#define FILESTREAMER_HPP

#include <fstream>
#include <functional>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

// a class for streaming data in and writing data out
template <typename T> class FileStreamer {
private:
  std::size_t min_chunk_size, max_chunk_size, entry_count;
  std::size_t head, tail;
  T *data;
  std::vector<std::thread> threads;
  std::vector<std::size_t> chunk_starts;
  std::fstream output;

public:
  // allocate chunks in the constructor
  FileStreamer(std::size_t entry_count, std::size_t min_chunk_size,
               std::size_t max_chunk_size)
      : min_chunk_size(min_chunk_size), max_chunk_size(max_chunk_size),
        entry_count(entry_count), head(0), tail(0), data(new T[entry_count]) {
    threads.reserve(entry_count / min_chunk_size);
    chunk_starts.reserve(entry_count / min_chunk_size);
  }

  // forbid copy construction because it doesn't make any sense
  FileStreamer(const FileStreamer &) = delete;

  // forbid assignment operator because it doesn't make any sense
  const FileStreamer &operator=(const FileStreamer &) = delete;

  // delete allocated memory and closes output file if it was open
  ~FileStreamer() {
    if (output.is_open())
      flush_write();

    delete[] data;
  }

  // read file and call func on every chunk loaded
  template <typename TFunc = std::function<void(T *, std::size_t, std::size_t)>>
  void read(std::string path, TFunc func) {
    // open file and throw and exception if this failed
    std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open())
      throw new std::runtime_error("Couldn't open file " + path +
                                   " for input!");

    // load all chunks
    std::size_t i = 0;
    std::size_t chunk_start = 0;
    std::size_t remainder = 0;
    while (i < entry_count * sizeof(T)) {
      // read chunk
      std::size_t max_read_count = std::min(
          max_chunk_size * sizeof(T) - remainder, entry_count * sizeof(T) - i);
      std::size_t read_bytes =
          input.readsome(&reinterpret_cast<char *>(data)[i], max_read_count);

      // throw an exception if read operation failed
      if (!input.good())
        throw new std::runtime_error("Reading from file " + path + " failed!");

      // increment the counter for read bytes
      i += read_bytes;

      // caclulate remainder of chunk
      remainder = i - chunk_start * sizeof(T);

      // determine how many entries can be processed
      std::size_t chunk_count = remainder / sizeof(T);

      // pass data to func in a new thread
      if (chunk_count > min_chunk_size || read_bytes == max_read_count) {
        threads.emplace_back(func, data, chunk_start, chunk_count);
        chunk_starts.emplace_back(chunk_start);

        // increment chunk start and remainder for next iteration
        remainder -= chunk_count * sizeof(T);
        chunk_start += chunk_count;
      }
    }

    // synchronize threads
    for (std::thread &t : threads)
      t.join();

    // close file
    input.close();
  }

  std::vector<std::size_t> getChunkStarts() const { return chunk_starts; }

  // write file
  std::function<void(T *, std::size_t)> write(std::string path) {
    // open file and throw exception if this failed
    output.open(path.c_str(), std::ios::out | std::ios::binary);
    if (!output.is_open())
      throw new std::runtime_error("Couldn't open file " + path +
                                   " for output!");

    // create file writing function
    std::function<void(T *, std::size_t)> result = [&, this, path](
                                                       T *data,
                                                       std::size_t chunk_size) {
      // write data and throw exception if this failed
      if (!output.good())
        throw new std::runtime_error("Writing to file " + path + " failed!");

      output.write(reinterpret_cast<char *>(data), sizeof(T) * chunk_size);
    };

    // return the result
    return result;
  }

  void flush_write() {
    output.flush();
    output.close();
  }

  // return pointer to internal data
  T *get() const { return data; }
};

#endif // FILESTREAMER_HPP
