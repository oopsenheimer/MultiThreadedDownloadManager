#include "Downloader.hpp"

#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/system.h>

#include <cstddef>
#include <fstream>
#include <ios>
#include <iostream>
#include <regex>
#include <string>

std::string Downloader::get_file_name() {
    if (!headers_fetched)
        fetch_headers();
    return file_name;
}

std::size_t Downloader::get_file_size() {
    if (!headers_fetched)
        fetch_headers();
    return file_size;
}

void Downloader::fetch_headers() {
    auto handle = curl_easy_init();
    if (handle == nullptr)
        return;

    curl_easy_setopt(handle, CURLOPT_URL, target_url.c_str());
    curl_easy_setopt(handle, CURLOPT_NOBODY, 1);

    curl_easy_setopt(handle, CURLOPT_HEADERFUNCTION, header_call_back);
    curl_easy_setopt(handle, CURLOPT_HEADERDATA, &file_name);

    auto result = curl_easy_perform(handle);

    if (result == CURLE_OK) {
        curl_off_t size;
        curl_easy_getinfo(handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &size);
        file_size = static_cast<std::size_t>(size);
    } else {
        std::cerr << "Failed to fetch headers: " << curl_easy_strerror(result) << std::endl;
    }

    curl_easy_cleanup(handle);
    headers_fetched = true;
}

std::size_t Downloader::header_call_back(char* buffer, std::size_t size, std::size_t nmemb,
                                         void* stream) {
    std::size_t total_size = size * nmemb;
    std::string header_line(buffer, total_size);
    auto file_stream = static_cast<std::string*>(stream);

    std::regex pattern("Content-Type:\\s*([^\\s]+)", std::regex_constants::icase);
    std::smatch matches;

    if (std::regex_search(header_line, matches, pattern)) {
        std::string mime_type = matches[1].str();
        
        if (mime_type == "application/pdf") {
            *file_stream = "download.pdf";
        } 
        else if (mime_type == "image/jpeg") {
            *file_stream = "download.jpg";
        }
        else if (mime_type == "text/html") {
            *file_stream = "download.html";
        }

    }
    std::cout << header_line << std::endl;
    return total_size;
}

void Downloader::download_chunk(int thread_id, std::size_t start_byte, std::size_t end_byte,
                                const std::string& output_file_name) {
    std::ofstream file(output_file_name, std::ios::binary | std::ios::in | std::ios::out);
    file.seekp(static_cast<std::streamoff>(start_byte));

    auto handle = curl_easy_init();
    if (handle == nullptr)
        return;

    auto range = std::to_string(start_byte) + "-" + std::to_string(end_byte);

    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_call_back);
    curl_easy_setopt(handle, CURLOPT_URL, target_url.c_str());
    curl_easy_setopt(handle, CURLOPT_RANGE, range.c_str());

    CURLcode result = curl_easy_perform(handle);
    if (result != CURLE_OK) {
        std::cerr << "Thread " << thread_id << " failed: " << curl_easy_strerror(result)
                  << std::endl;
    }
    curl_easy_cleanup(handle);
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
