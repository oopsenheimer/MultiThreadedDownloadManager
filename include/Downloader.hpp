#pragma once

#include <cstddef>
#include <string>
#include <utility>

class Downloader {
   private:
    std::string target_url;

    std::string file_name = "download.bin";
    std::size_t file_size = 0;
    bool headers_fetched = false;

   private:
    void fetch_headers();

    static std::size_t write_call_back(void* buffer, std::size_t size, std::size_t nmemb,
                                       void* stream);
    static std::size_t header_call_back(char* buffer, std::size_t size, std::size_t nmemb, void* stream);

   public:
    Downloader(std::string target_url)
        : target_url(std::move(target_url)) {}

    std::size_t get_file_size();
    std::string get_file_name();

    static void prepare_write_stream(const std::string& filename, const std::size_t& total_size);
    void download_chunk(int thread_id, std::size_t start_byte, std::size_t end_byte, const std::string& output_file_name);
};