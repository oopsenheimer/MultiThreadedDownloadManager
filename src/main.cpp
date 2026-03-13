#include "Downloader.hpp"

#include <cstddef>
#include <iostream>
#include <ostream>
#include <thread>
#include <vector>

constexpr auto TEST_URL = "https://littleosbook.github.io/book.pdf";

auto num_threads = std::thread::hardware_concurrency();

int main() {
    Downloader downloadManager(TEST_URL);
    auto file_size = downloadManager.get_download_file_size();
    Downloader::prepare_write_stream("out.bin", file_size);
    std::cout << "Size of " << TEST_URL << " = " << file_size << std::endl;

    std::vector<std::thread> workers;
    std::size_t start_byte = 0;
    std::size_t diff = file_size / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        std::size_t end_byte;

        if (i == num_threads - 1)
            end_byte = file_size - 1;
        else
            end_byte = start_byte + diff - 1;
        workers.emplace_back(&Downloader::download_chunk, &downloadManager, i, start_byte, end_byte);
        start_byte += diff;
    }

    for (auto& worker : workers) {
        worker.join();
    }


    // downloadManager.download_chunk(1, 0, 500);
}