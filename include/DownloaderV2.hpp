#ifndef DOWNLOADER_V2
#define DOWNLOADER_V2
#include <sys/types.h>
#include <atomic>
#include <cstddef>
#include <utility>
#include "HTTPClient.hpp"
#include "MemoryMappedFile.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

class DownloaderV2 {
   private:
    HTTPClient _http_client;
    std::string _file_name;
    std::string _target_url;

    struct ChunkContext {
        Socket _sock;
        size_t _current_offset;
        size_t _end_byte;
        std::atomic<bool> _finished{false};
    };

   public:
    DownloaderV2(std::string target_url, std::string file_name = "download.bin")
        : _http_client(target_url),
          _file_name(std::move(file_name)),
          _target_url(std::move(target_url)){};
    ssize_t download();

   private:
    bool prepare_file_stream(const size_t& file_size);
    MemoryMappedFile prepare_memory_map(const size_t& file_size);
    void download_worker(unsigned int thread_id, size_t start, size_t end);
    void download_mmap_worker(MemoryMappedFile& mmap, ChunkContext* chunk_context, int epoll_fd);
    
};

#endif