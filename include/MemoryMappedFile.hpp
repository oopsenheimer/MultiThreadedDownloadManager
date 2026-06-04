#ifndef MEMORY_MAPPED_FILE
#define MEMORY_MAPPED_FILE

#include <sys/mman.h>
#include <cstddef>
#include <string>

class MemoryMappedFile {
    int _fd = -1;
    size_t _file_size = 0;
    char* _mapped_data = reinterpret_cast<char*>(MAP_FAILED);


   public:
    MemoryMappedFile(const std::string& file_name, const size_t& file_size);
    ~MemoryMappedFile();

    void write(size_t offset, const char* src_buffer, size_t num_bytes);

    auto data() noexcept { return _mapped_data; }
    auto size() const { return _file_size; }
};

#endif