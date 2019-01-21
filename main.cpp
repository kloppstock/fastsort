#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>

// holds entries according to spec
struct Entry {
  char key[10];
  char value[90];
};

// a class for streaming data in and writing data out
template <typename T> class FileStreamer {
private:
  std::size_t chunk_size, entry_count;
  std::size_t head, tail;
  T *data;

public:
  // allocate chunks in the constructor
  FileStreamer(std::size_t chunk_size, std::size_t entry_count)
      : chunk_size(chunk_size), entry_count(entry_count), head(0), tail(0),
        data(new T[entry_count]) {}

  // forbid copy construction because it doesn't make any sense
  FileStreamer(const FileStreamer &) = delete;

  // forbid assignment operator because it doesn't make any sense
  const FileStreamer &operator=(const FileStreamer &) = delete;

  // delete allocated memory in destructor
  ~FileStreamer() { delete[] data; }

  // read file and call func on every chunk loaded
	template<typename TFunc = std::function<void(T *data, std::size_t chunk_size)>>
  void read(std::string path,
             TFunc func) {
    // open file and throw and exception if this failed
    std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
    if (!input.is_open())
      throw new std::runtime_error("Couldn't open file " + path +
                                   " for input!");

    // load all chunks
    std::size_t i = 0;
    std::size_t chunk_start = 0;
    while (i < entry_count) {
      // read chunk
      std::size_t max_read_count = std::min(chunk_size, entry_count - i);
	  std::size_t read_bytes = input.readsome(reinterpret_cast<char *>(&data[i]),
                          sizeof(T) * max_read_count);

	  // throw an exception if read operation failed
	  if(!input.good())
		  throw new std::runtime_error("Reading from file " + path + " failed!");

	  // determine how many entries can be processed
	  std::size_t chunk_count = (i - chunk_start) - (i - chunk_start) % sizeof(T);
	  
      // pass data to func
	  //! @todo: introduce min_chunk_size
	  //! @todo: start in new thread
      if (read_bytes > sizeof(T))
        func(&data[chunk_start], chunk_count);

	  // increment chunk start for next iteration
	  chunk_start += chunk_count;
    }

    // close file
    input.sync();
    input.close();
  }
	
  // write file
  //! @todo: fix file writing
  void write(std::string path) {
    // open file and throw exception if this failed
    std::fstream output(path.c_str(), std::ios::out | std::ios::binary);
    if (!output.is_open())
      throw new std::runtime_error("Couldn't open file " + path +
                                   " for output!");

    // write data and throw exception if this failed
    if (!output
             .write(reinterpret_cast<char *>(data),
                    sizeof(T) * chunk_size * chunk_count)
             .good())
      throw new std::runtime_error("Writing to file " + path + " failed!");

    // flush and close file
    output.sync();
    output.flush();
    output.close();
  }

  // return pointer to internal data
  T *get() const { return data; }
};

int main(int argc, char *argv[]) {
  // parse arguments
  std::string input_path = "";
  std::string output_path = "";
  std::size_t chunk_size = 4000000 / sizeof(Entry);
  std::size_t entry_count = 4000000000;
  for (int i = 1; i < argc; ++i) {
    // check for optional arguments
    if (std::strlen(argv[i]) > 3 && '-' == argv[i][0] && '=' == argv[i][2]) {
      // set chunk size
      if ('s' == argv[i][1])
        chunk_size = std::atoi(&argv[i][3]);
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

  if ("" == input_path || "" == output_path || 0 == chunk_size ||
      0 == entry_count) {
    std::cout << "Usage: sort [-s=SIZE] [-c=COUNT] INPUT OUTPUT\n";
    return EXIT_FAILURE;
  }

  //! @todo: implement sort function here
  //! @todo: use multithreaded quicksort with ln(cores) levels
  //! @todo: check if bitonic sort would make sense
  std::function<void(Entry * data, std::size_t chunk_size)> sort;

  // stream file
  FileStreamer<Entry> fs(chunk_size, entry_count);
  fs.read(input_path, sort);

  //! @todo: implement merge sort here

  // write file
  fs.write(output_path);

  return EXIT_SUCCESS;
}
