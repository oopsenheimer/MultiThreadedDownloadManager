#pragma once

#include <cstddef>
#include <string>
#include <utility>

class Downloader {
   private:
    std::string target_url;
    const char* FILE_NAME = "out.bin";

   public:
    Downloader(std::string target_url)
        : target_url(std::move(target_url)) {}

    std::size_t get_download_file_size();

    bool download_chunk(int thread_id, std::size_t start_byte, std::size_t end_byte);

    static std::size_t write_call_back(void* buffer, std::size_t size, std::size_t nmemb,
                                       void* stream);
    static void prepare_write_stream(const std::string& filename, const std::size_t& total_size);
};