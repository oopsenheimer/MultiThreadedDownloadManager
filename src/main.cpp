#include <sys/types.h>
#include "DownloaderV2.hpp"
#include <string>

constexpr auto TEST_URL = "https://littleosbook.github.io/book.pdf";
constexpr auto EXAMPLE_URL = "http://link.testfile.org";

#define DOWNLOAD_HOST "david.choffnes.com"
#define DOWNLOAD_PATH "/classes/cs5700f22/10MB.log"

#define DOWNLOAD_URL "http://david.choffnes.com/classes/cs5700f22/10MB.log"

// void curl_downloader() {
//     Downloader downloadManager(TEST_URL);
//     auto file_size = downloadManager.get_file_size();
//     auto file_name = downloadManager.get_file_name();
//     Downloader::prepare_write_stream(file_name, file_size);
//     std::cout << "Size of " << TEST_URL << " = " << file_size << std::endl;

//     std::vector<std::thread> workers;
//     std::size_t start_byte = 0;
//     std::size_t diff = file_size / num_threads;

//     for (int i = 0; i < num_threads; ++i) {
//         std::size_t end_byte;

//         if (i == num_threads - 1)
//             end_byte = file_size - 1;
//         else
//             end_byte = start_byte + diff - 1;
//         workers.emplace_back(&Downloader::download_chunk, &downloadManager, i, start_byte, end_byte, downloadManager.get_file_name());
//         start_byte += diff;
//     }

//     for (auto& worker : workers) {
//         worker.join();
//     }
// }

// void directly_download() {
//     Socket my_transporter;
//     if (my_transporter.connect_to_host(EXAMPLE_URL, SERVICE_HTTP) == false) {
//         std::cout << "Failed to connect" << EXAMPLE_URL << std::endl;
//         return;
//     }
//     std::cout << "Connected to " << EXAMPLE_URL << std::endl;

//     std::string request =
//         "GET / HTTP/1.1\r\n"
//         "Host: www.example.com\r\n"
//         "Connection: close\r\n\r\n";

//     my_transporter.send_data(request);

//     auto headers = my_transporter.receive_header();
//     std::cout << "Headers Received\n";

//     char buff[4096];
//     ssize_t bytes_read;
//     size_t total_bytes = 0;
//     while ((bytes_read = my_transporter.receive_data(buff, sizeof(buff))) > 0) {
//         std::cout.write(buff, bytes_read);
//         total_bytes += bytes_read;
//     }

//     std::cout << "Download complete\n"
//               << "Total bytes read: " << total_bytes << std::endl;
// }

// void download_worker(int start_byte, int end_byte, const char* filename) {
//     Socket socket;

//     if (socket.connect_to_host(EXAMPLE_URL, SERVICE_HTTP) == false) {
//         std::cout << "FAILED TO CONNECT TO " << EXAMPLE_URL << std::endl;
//         return;
//     }
//     std::cout << "Connected to " << EXAMPLE_URL << std::endl;
//     std::string request =
//         "GET / HTTP/1.1\r\n"
//         "Host: " +
//         std::string(EXAMPLE_URL) +
//         "\r\n"
//         "Range: bytes=" +
//         std::to_string(start_byte) + "-" + std::to_string(end_byte) +
//         "\r\n"
//         "Connection: close\r\n\r\n";

//     socket.send_data(request);
//     auto headers = socket.receive_header();

//     if (headers.find("206 Partial Content") == std::string::npos) {
//         if (start_byte == 0) {
//             std::cout << "Server does not support ranges. Downloading sequentially..." << std::endl;
//         } else {
//             return;
//         }
//     }

//     std::cout << "Downloading\n";
    
//     std::fstream file(filename, std::ios::binary | std::ios::in | std::ios::out);
//     file.seekp(start_byte);

//     char buff[4096];
//     size_t bytes_read = 0;
//     size_t total_segment_limit = end_byte - start_byte + 1;
//     size_t current_received_bytes = 0;
//     while (current_received_bytes < total_segment_limit &&
//            (bytes_read = socket.receive_data(buff, sizeof(buff))) > 0) {

//         size_t to_write =
//             std::min((size_t)bytes_read, total_segment_limit - current_received_bytes);

//         file.write(buff, to_write);
//         current_received_bytes += to_write;
//     }
// }

// void multi_download() {
//     const char* filename = "example_out.html";
//     size_t file_size = 4000;

//     std::ofstream init(filename, std::ios::binary);
//     init.seekp(file_size - 1);
//     init.write("", 1);
//     init.close();

//     std::vector<std::thread> workers;
//     size_t start = 0;
//     size_t chunk_size = file_size / num_threads;

//     for (int i = 0; i < num_threads; ++i) {
//         size_t end = (i == num_threads - 1) ? file_size - 1 : start + chunk_size - 1;
//         workers.emplace_back(download_worker, start, end, filename);
//         start += chunk_size;
//     }

//     for (auto& t : workers) {
//         t.join();
//     }
// }


int main() {

    DownloaderV2 mydownloader(DOWNLOAD_URL);
    mydownloader.download();

}