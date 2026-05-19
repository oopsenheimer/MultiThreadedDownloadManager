#include "DownloaderV2.hpp"
#include <unistd.h>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <thread>
#include <vector>

void DownloaderV2::download() {
    const auto file_size = _http_client.get_content_length();
    auto num_thread = std::thread::hardware_concurrency();

    if (!_http_client.supports_ranges()) {
        std::cout << "[!] SERVER DOES NOT SUPPORTS RANGES\n";
        num_thread = 1;
    }

    std::cout << "[+] STARTING DOWNLOAD\n    FILE SIZE: " << file_size << '\n';
    prepare_file_stream(file_size);

    std::vector<std::thread> workers;
    auto chunk_size = file_size / num_thread;
    size_t start_byte = 0;

    for (decltype(num_thread) i = 0; i < num_thread; ++i) {
        auto end_byte = (i == num_thread - 1) ? (file_size - 1) : (start_byte + chunk_size - 1);
        workers.emplace_back(&DownloaderV2::download_worker, this, i, start_byte, end_byte);
        start_byte += chunk_size;
    }

    for (auto& worker : workers) {
        worker.join();
    }

    std::cout << "[+] DOWNLOAD COMPLETE\n    FILE NAME: " << _file_name << '\n'; 
}

bool DownloaderV2::prepare_file_stream(const size_t& file_size) {
    std::ofstream init(_file_name, std::ios::binary | std::ios::trunc);
    if (!init) {
        std::cerr << "FAILED TO CREATE OUTPUT FILE\n";
        return false;
    }
    if (file_size > 0) {
        init.seekp(file_size - 1);
        init.write("\0",1);
    }
    return true;
}

void DownloaderV2::download_worker(unsigned int thread_id ,size_t start, size_t end) {
    std::fstream file(_file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[-] THREAD " << thread_id << " FAILED TO OPEN FILE\n";
        return;
    }
    file.seekp(start);
    char buffer[8192];

    auto total_bytes = end - start + 1;
    decltype(total_bytes) bytes_read = 0;
    decltype(total_bytes) bytes_written = 0;
    ssize_t bytes_fetched = 0;

    while (bytes_written < total_bytes) {
        auto current_start = start + bytes_written;
        auto remaining_bytes = total_bytes - bytes_written;
        auto current_request_size = std::min(sizeof buffer, remaining_bytes);
        auto current_end = current_start + current_request_size - 1;

        bytes_fetched =
            _http_client.fetch_chunks(current_start, current_end, buffer, sizeof buffer);

        if (bytes_fetched < 0) {
            std::cerr << "[-] CONNECTION/NETWORK ERROR IN THREAD: " << thread_id << '\n';
            return ;
        }

        if (bytes_fetched == 0) {
            break;
        }

        file.write(buffer, bytes_fetched);
        bytes_written += bytes_fetched;
    }
}