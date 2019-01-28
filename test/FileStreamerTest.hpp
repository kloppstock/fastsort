#ifndef FILESTREAMER_TEST_HPP
#define FILESTREAMER_TEST_HPP

#include "../FileStreamer.hpp"
#include <gtest/gtest.h>

using T = int;
std::string sample_path = "test.bin";
std::string out_path = "out.bin";
unsigned int sample_count = 1000;
unsigned int min_chunk_size = 123;
unsigned int max_chunk_size = 200;

class FileStreamerTest : public ::testing::Test {
public:
  std::vector<T> test_data;
  std::vector<T> read_data;
  std::function<void(T *, std::size_t, std::size_t)> read_func;
  bool exceeded_max_chunk_size;
  bool deceeded_min_chunk_size;

  FileStreamerTest() {}

  ~FileStreamerTest() {}

  void SetUp() override {
    // generate test data
    read_data.clear();
    read_data.resize(sample_count);
    test_data.clear();
    test_data.resize(sample_count);
    for (unsigned int i = 0; i < test_data.size(); ++i)
      test_data[i] = i;

    exceeded_max_chunk_size = false;
    deceeded_min_chunk_size = false;

    // set up read function
    read_func = [&](T *data, std::size_t chunk_start, std::size_t chunk_size) {
      if (chunk_size > max_chunk_size)
        exceeded_max_chunk_size = true;

      if (chunk_size < deceeded_min_chunk_size &&
          chunk_size + chunk_start != sample_count)
        deceeded_min_chunk_size = true;

      for (unsigned int i = chunk_start; i < chunk_start + chunk_size; ++i)
        read_data[i] = data[i];
    };

    // fill test file with data
    std::ofstream test_file(sample_path.c_str(), std::ios::binary);
    if (test_file.is_open())
      test_file.write(reinterpret_cast<char *>(test_data.data()),
                      sizeof(T) * test_data.size());

    // flush and close
    test_file.flush();
    test_file.close();
  }

  void TearDown() override {}
};

TEST_F(FileStreamerTest, ReadTest) {
  FileStreamer<T> fs(sample_count, min_chunk_size, max_chunk_size);

  fs.read(sample_path, read_func);

  EXPECT_FALSE(exceeded_max_chunk_size);
  EXPECT_FALSE(deceeded_min_chunk_size);
  EXPECT_EQ(test_data, read_data);
}

TEST_F(FileStreamerTest, WriteTest) {
  FileStreamer<T> fs(sample_count, min_chunk_size, max_chunk_size);

  std::function<void(T *, std::size_t)> write_func(fs.write(out_path));

  std::vector<T> data(max_chunk_size);
  for (unsigned int chunk = 0; chunk < (sample_count + max_chunk_size - 1) / max_chunk_size;
       ++chunk) {
    std::size_t chunk_size = 0;
    for (unsigned int i = 0; i < max_chunk_size; ++i) {
      std::size_t index = chunk * max_chunk_size + i;
      if (index < sample_count) {
        data[i] = test_data[index];
        ++chunk_size;
      }
    }
    write_func(data.data(), chunk_size);
  }

  fs.flush_write();

  read_data.clear();
  read_data.resize(sample_count);

  // read the test data
  std::ifstream read_file(out_path.c_str(), std::ios::binary);
  ASSERT_TRUE(read_file.is_open());
  EXPECT_TRUE(read_file.read(reinterpret_cast<char *>(read_data.data()),
                              sizeof(T) * sample_count));

  read_file.close();

  EXPECT_EQ(test_data, read_data);
}

#endif // FILESTREAMER_TEST_HPP
