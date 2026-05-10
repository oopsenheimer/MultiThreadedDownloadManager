#include <sys/types.h>
#include "Downloader.hpp"
#include "Socket.hpp"

#include <cstddef>
#include <iostream>
#include <ostream>
#include <thread>
#include <vector>

#define SERVICE_HTTP "80"
#define SERVICE_HTTPS "443"

constexpr auto TEST_URL = "https://littleosbook.github.io/book.pdf";
constexpr auto EXAMPLE_URL = "www.example.com";

auto num_threads = std::thread::hardware_concurrency();

void curl_downloader() {
    Downloader downloadManager(TEST_URL);
    auto file_size = downloadManager.get_file_size();
    auto file_name = downloadManager.get_file_name();
    Downloader::prepare_write_stream(file_name, file_size);
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
        workers.emplace_back(&Downloader::download_chunk, &downloadManager, i, start_byte, end_byte, downloadManager.get_file_name());
        start_byte += diff;
    }

    for (auto& worker : workers) {
        worker.join();
    }
}

int main() {
    Socket my_transporter;
    if (my_transporter.connect_to_host(EXAMPLE_URL, SERVICE_HTTP) == false) {
        std::cout << "Failed to connect" << EXAMPLE_URL << std::endl;
        return -1;
    }
    std::cout << "Connected to " << EXAMPLE_URL << std::endl;

    std::string request =
        "GET / HTTP/1.1\r\n"
        "Host: www.example.com\r\n"
        "Connection: close\r\n\r\n";

    my_transporter.send_data(request);

    auto headers = my_transporter.receive_header();
    std::cout << "Headers Received\n";

    char buff[4096];
    ssize_t bytes_read;
    size_t total_bytes = 0;

    while ((bytes_read = my_transporter.receive_data(buff, sizeof(buff))) > 0) {
        std::cout.write(buff, bytes_read);
        total_bytes += bytes_read;
    }

    std::cout << "Download complete\n"
              << "Total bytes read: " << total_bytes << std::endl;
    return 0;
}