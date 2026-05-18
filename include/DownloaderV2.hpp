#ifndef DOWNLOADER_V2
#define DOWNLOADER_V2
#include "HTTPClient.hpp"

class DownloaderV2 {
   private:
    std::string _targer_url;
    HTTPClient _http_client;
    HTTPMetadata _file_info;

   public:
    DownloaderV2(std::string target_url)
        : _targer_url(target_url) {};
    

};

#endif