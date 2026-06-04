#include "MemoryMappedFile.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <sys/mman.h>

void MemoryMappedFile::write(size_t offset, const char* src_buffer, size_t num_bytes) {
    if (offset + num_bytes > _file_size) {
        throw std::out_of_range("[-] MMAP Error: Write range exceeds allocated file size.");
    }

    std::memcpy(_mapped_data + offset, src_buffer, num_bytes);
}

MemoryMappedFile::MemoryMappedFile(const std::string& file_name, const size_t& file_size) {
    _fd = open(file_name.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    _file_size = file_size;
    if (_fd < 0) {
        throw std::runtime_error("[-] Couldnt open file");
    }

    ftruncate(_fd, file_size);

    _mapped_data =
        static_cast<char*>(mmap(nullptr, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, 0));

    if (_mapped_data == MAP_FAILED) {
        close(_fd);
        throw std::runtime_error("[-] MMAP FAILED");
    }
}

MemoryMappedFile::~MemoryMappedFile() {
    if (_mapped_data != MAP_FAILED) {
        munmap(_mapped_data, _file_size);
    }
    if (_fd >= 0) {
        close(_fd);
    }
}
