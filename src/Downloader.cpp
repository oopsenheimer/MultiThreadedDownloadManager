#include "Downloader.hpp"

#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/system.h>

#include <cstddef>
#include <fstream>
#include <ios>
#include <string>

std::size_t Downloader::get_download_file_size() {
    auto handle = curl_easy_init();
    if (handle == nullptr)
        return 0;

    curl_easy_setopt(handle, CURLOPT_URL, target_url.c_str());
    curl_easy_setopt(handle, CURLOPT_NOBODY, 1L);
    curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1);

    CURLcode result = curl_easy_perform(handle);
    if (result != CURLE_OK) {
        curl_easy_cleanup(handle);
        return 0;
    }

    curl_off_t file_size;
    curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &file_size);
    curl_easy_cleanup(handle);
    return static_cast<std::size_t>(file_size);
}

bool Downloader::download_chunk(int thread_id, std::size_t start_byte, std::size_t end_byte) {
    std::ofstream file(FILE_NAME, std::ios::binary | std::ios::in | std::ios::out);
    file.seekp(static_cast<std::streamoff>(start_byte));

    auto handle = curl_easy_init();
    if (handle == nullptr)
        return false;

    auto range = std::to_string(start_byte) + "-" + std::to_string(end_byte);

    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_call_back);
    curl_easy_setopt(handle, CURLOPT_URL, target_url.c_str());
    curl_easy_setopt(handle, CURLOPT_RANGE, range.c_str());

    CURLcode result = curl_easy_perform(handle);

    curl_easy_cleanup(handle);

    if (result != CURLE_OK) {
        return false;
    }

    return true;
}

std::size_t Downloader::write_call_back(void* buffer, std::size_t size, std::size_t nmemb,
                                        void* stream) {
    auto total_size = size * nmemb;
    auto file_stream = static_cast<std::ofstream*>(stream);
    auto data = static_cast<const char*>(buffer);

    if (file_stream && data) {
        file_stream->write(data, static_cast<std::streamoff>(total_size));
    }

    return total_size;
}

void Downloader::prepare_write_stream(const std::string& filename, const std::size_t& total_size) {
    std::ofstream file(filename, std::ios::binary | std::ios::out);

    if (total_size) {
        file.seekp(static_cast<std::streamoff>(total_size - 1));
        file.write("\0", 1);
    }
}
