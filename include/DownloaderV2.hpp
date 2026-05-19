#ifndef DOWNLOADER_V2
#define DOWNLOADER_V2
#include <cstddef>
#include <utility>
#include "HTTPClient.hpp"

class DownloaderV2 {
   private:
    std::string _file_name;
    std::string _target_url;
    HTTPClient _http_client;
   public:
    DownloaderV2(std::string target_url, std::string file_name = "download.bin")
        : _target_url(std::move(target_url)),
          _file_name(std::move(file_name)),
          _http_client(target_url){};
    void download();

   private:
    bool prepare_file_stream(const size_t& file_size);
    void download_worker(unsigned int thread_id, size_t start, size_t end);
};

#endif